#include "struct_config.h"

#include "log.h"
#include "utils.h"
#include "template_utils.h"

#include <memory>

#include <QFile>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

static const QString ENV_VAR_NAME = "GLOGG_CONFIG_DEVEL";
static const QString COLORS_GLOB_PATTERN = "*.glogg-colors.yaml";
static const QString SYNTAX_GLOB_PATTERN = "*.glogg-syntax.yaml";

static std::unique_ptr<StructConfig> theStructConfig;

static QString getUserDir()
{
    auto env = QProcessEnvironment::systemEnvironment();
    if (!env.contains(ENV_VAR_NAME))
        return "";
    auto path = env.value(ENV_VAR_NAME);
    INFO << ENV_VAR_NAME << "is" << path;
    return path;
}

static QString getBuiltinDir()
{
    auto curDir = QDir(QCoreApplication::applicationDirPath());
    curDir.setNameFilters({SYNTAX_GLOB_PATTERN, COLORS_GLOB_PATTERN});
    while (true) {
        auto dir = curDir;
        if (dir.cd("config") && !dir.entryList().empty()) {
            INFO << "Detected source tree root in " << curDir.absolutePath();
            return dir.absolutePath();
        }
        dir = curDir;
        if (dir.cd("share") && dir.cd("glogg") && dir.cd("config")
            && !dir.entryList().empty()) {
            INFO << "Detected installation root in " << curDir.absolutePath();
            return dir.absolutePath();
        }
        if (!curDir.cdUp())
            break;
    }
    throw ASSERT << "Could not detect source/installation root";
}

const StructConfig &StructConfigStore::current()
{
    return *theStructConfig;
}

void StructConfigStore::loadDefault()
{
    theStructConfig = std::make_unique<StructConfig>();
}

void StructConfigStore::reload()
{
    QDir stdDir(
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    stdDir.cd("glogg");
    auto dirs
        = scanDirs({getBuiltinDir(), stdDir.absolutePath(), getUserDir()});
    auto newConfig
        = std::make_unique<StructConfig>(dirs.colorsFiles, dirs.syntaxFiles);
    theStructConfig = std::move(newConfig);
}

StructConfigFiles StructConfigStore::scanDirs(const QStringList &dirs)
{
    StructConfigFiles result;
    for (auto dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) {
            WARN << dirPath << " does not exist, skipping";
            continue;
        }
        for (auto fileInfo : dir.entryInfoList({COLORS_GLOB_PATTERN}))
            result.colorsFiles.push_back(fileInfo.absoluteFilePath());
        for (auto fileInfo : dir.entryInfoList({SYNTAX_GLOB_PATTERN})) {
            result.syntaxFiles.push_back(fileInfo.absoluteFilePath());
        }
    }
    return result;
}

StructConfig::StructConfig(const QStringList &colorsFiles,
                           const QStringList &syntaxFiles,
                           bool stopOnError)
{
    for (const auto &file : colorsFiles) {
        try {
            mergeMaps(colorSchemes_,
                      ColorScheme::loadAll(ConfigNode::parseFile(file)),
                      false /* do not override */);
            INFO << "Loaded color scheme file " << file;
        }
        catch (const std::exception &exc) {
            if (stopOnError)
                throw exc;
            ERROR << "Error loading color scheme file" << file << ":" << exc;
        }
    }
    for (const auto &file : syntaxFiles) {
        try {
            syntaxColl_.merge(ConfigNode::parseFile(file));
            INFO << "Loaded syntax file " << file;
        } catch (const std::exception &exc) {
            if (stopOnError)
                throw exc;
            ERROR << "Error loading syntax file " << file << ": "
                  << exc;
        }
    }
}

StructConfig::StructConfig(const StructConfigFiles &configFiles,
                           bool stopOnError)
    : StructConfig(configFiles.colorsFiles, configFiles.syntaxFiles,
                   stopOnError)
{}

bool StructConfig::checkForIssues() const
{
    bool hasIssues = false;
    for (const auto &nameAndScheme: colorSchemes_)
        for (const auto &syntax : syntaxColl_.syntaxes())
            for (const auto &scope : syntax.usedScopes())
                if (!nameAndScheme.second.hasScope(scope)) {
                    WARN << "Scope" << scope
                         << "used in rule but not defined in scheme"
                         << nameAndScheme.first;
                    hasIssues = true;
                }
    return hasIssues;
}