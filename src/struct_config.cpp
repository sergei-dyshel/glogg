/*
 * Copyright (C) 2018-2019 Sergei Dyshel and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    colorScheme_ = ColorScheme(root.member("colorScheme"));
    syntaxColl_ = SyntaxCollection(root.member("syntax"));
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