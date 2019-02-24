#include "struct_config.h"

#include "log.h"
#include "utils.h"

#include <memory>

#include <QFile>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QDir>

static const QString ENV_VAR_NAME = "GLOGG_CONFIG";
static const QString COLORS_GLOB_PATTERN = "*.glogg-colors.yaml";
static const QString SYNTAX_GLOB_PATTERN = "*.glogg-syntax.yaml";

static std::unique_ptr<StructConfig> theStructConfig;

const StructConfig &StructConfig::instance()
{
    return *theStructConfig;
}
void StructConfig::loadDefault()
{
    theStructConfig = std::make_unique<StructConfig>();
}

void StructConfig::reload(const QStringList &dirs)
{
    auto newConfig = std::make_unique<StructConfig>();
    newConfig->load(dirs);
    theStructConfig = std::move(newConfig);
}

static QString getConfigDir()
{
    auto stdPath
        = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    auto env = QProcessEnvironment::systemEnvironment();
    if (!env.contains(ENV_VAR_NAME))
        return stdPath;
    auto path = env.value(ENV_VAR_NAME);
    INFO << ENV_VAR_NAME << "is" << path;
    return path;
}

void StructConfig::load(const QStringList &dirs) {
    YAML::Node yamlRoot;
    auto configDirs = dirs.empty() ? QStringList{getConfigDir()} : dirs;
    for (auto dirPath : configDirs) {
        QDir dir(dirPath);
        if (!dir.exists()) {
            WARN << dirPath << " does not exist, skipping";
            continue;
        }
        for (auto fileInfo : dir.entryInfoList({COLORS_GLOB_PATTERN})) {
            auto filePath  = fileInfo.absoluteFilePath();
            try {
                colorScheme_ = ColorScheme(ConfigNode(filePath));
            }
            catch (const std::exception &exc) {
                ERROR << "Error loading color scheme file " << filePath << ": "
                      << exc.what();
            }
            INFO << "Loaded color scheme file " << filePath;
        }
        for (auto fileInfo : dir.entryInfoList({SYNTAX_GLOB_PATTERN})) {
            auto filePath  = fileInfo.absoluteFilePath();
            try {
                syntaxColl_ = SyntaxCollection(ConfigNode(filePath));
            }
            catch (const std::exception &exc) {
                ERROR << "Error loading syntax file " << filePath << ": "
                      << exc.what();
            }
            INFO << "Loaded syntax file " << filePath;
        }
    }
}

bool StructConfig::checkForIssues() const
{
    bool hasIssues = false;
    for (const auto &syntax : syntaxColl_.syntaxes())
        for (const auto &scope : syntax.usedScopes())
            if (!colorScheme_.hasScope(scope)) {
                ERROR << "Scope" << scope << "used in rule but not defined";
                hasIssues = true;
            }
    return hasIssues;
}