#include "struct_config.h"

#include "log.h"
#include "utils.h"

#include <memory>

#include <QFile>
#include <QProcessEnvironment>
#include <QStandardPaths>

static const QString ENV_VAR_NAME = "GLOGG_JSON_CONFIG";
static const QString CONFIG_FILE_NAME = "glogg.yaml";

static std::unique_ptr<StructConfig> theStructConfig;

const StructConfig &StructConfig::instance()
{
    return *theStructConfig;
}
void StructConfig::loadDefault()
{
    theStructConfig = std::make_unique<StructConfig>();
}

void StructConfig::reload()
{
    auto newConfig = std::make_unique<StructConfig>();
    newConfig->load();
    theStructConfig = std::move(newConfig);
}

static QString getPathFromEnv()
{
    QString path;
    auto env = QProcessEnvironment::systemEnvironment();
    if (!env.contains(ENV_VAR_NAME))
        return "";
    path = env.value(ENV_VAR_NAME);
    if (path == "") {
        LOG(logWARNING) << ENV_VAR_NAME << " is empty!";
        return "";
    }
    QFile file(path);
    if (!file.exists()) {
        LOG(logWARNING) << ENV_VAR_NAME << " contains " << path
                        << " which does not exist";
        return "";
    }
    return path;
}

void StructConfig::load() {
    QString path = getPathFromEnv();
    YAML::Node yamlRoot;
    if (path == "") {
        path = QStandardPaths::locate(QStandardPaths::AppConfigLocation,
                                 CONFIG_FILE_NAME);
        if (path == "") {
            LOG(logWARNING) << "Could not locate " << CONFIG_FILE_NAME
                            << " in standard locations";
            return;
        }
    }
    try {
        yamlRoot = YAML::LoadFile(path.toStdString());
        if (!yamlRoot.IsMap()) {
            LOG(logERROR) << "Root node type is not mapping";
        }
    } catch (YAML::ParserException) {
        LOG(logERROR) << "Error parsing " << path;
    }

    ConfigNode root = ConfigNode("", yamlRoot);
    colorScheme_ = ColorScheme(root.memberNoExcept("colorScheme"));
    syntaxColl_ = SyntaxCollection  (root.memberNoExcept("syntax"));
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