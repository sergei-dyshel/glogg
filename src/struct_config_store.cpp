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

#include "struct_config_store.h"

#include "persistentinfo.h"
#include "template_utils.h"

#include <QCoreApplication>
#include <QDir>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QMessageBox>

static const QString ENV_VAR_NAME = "GLOGG_CONFIG_DEVEL";
static const QString DEFAULT_COLOR_SCHEME = "default";
static const QString COLOR_SCHEME_SETTING = "colorScheme";

static std::unique_ptr<StructConfigStore> the;

StructConfigStore &StructConfigStore::get()
{
    return *the;
}

void StructConfigStore::init()
{
    the = std::make_unique<StructConfigStore>();
}

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
    curDir.setNameFilters(
        {StructConfig::SYNTAX_GLOB_PATTERN, StructConfig::COLORS_GLOB_PATTERN});
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

StructConfigStore::StructConfigStore()
    : settings_(GetPersistentInfo().settings()),
      colorSchemeName_(
          settings_.getString(COLOR_SCHEME_SETTING, DEFAULT_COLOR_SCHEME))
{
}

const ColorScheme &StructConfigStore::colorScheme() const
{
    if (colorSchemeName_ == DEFAULT_COLOR_SCHEME)
        return defaultColorScheme_;
    return config_.colorSchemes().at(colorSchemeName_);
}


void StructConfigStore::reload()
{
    QDir stdDir(
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    stdDir.cd("glogg");
    StructConfigFiles files;
    StructConfig::scanDirs(
        {getBuiltinDir(), stdDir.absolutePath(), getUserDir()}, files);
    StructConfig newConfig(files, false /* do not run tests */,
                           false /* do not stop on error */);
    newConfig.checkForIssues();
    config_ = std::move(newConfig);
    if (colorSchemeName_ != DEFAULT_COLOR_SCHEME
        && !config_.colorSchemes().count(colorSchemeName_)) {
        QMessageBox msgBox;
        msgBox.setText("Current color scheme " + colorSchemeName_
                       + " no longer exists");
        msgBox.exec();
        colorSchemeName_ = DEFAULT_COLOR_SCHEME;
    }
}

QStringList StructConfigStore::colorSchemeNames() const
{
    QStringList result = mapKeysSet<QStringList>(config_.colorSchemes());
    result.push_front(DEFAULT_COLOR_SCHEME);
    return result;
}

void StructConfigStore::setColorScheme(const QString &name)
{
    if (name != DEFAULT_COLOR_SCHEME && !config_.colorSchemes().count(name))
        throw ASSERT << "Color scheme" << name << "not found";
    colorSchemeName_ = name;
}

void StructConfigStore::saveSettings()
{
    settings_.setValue(COLOR_SCHEME_SETTING, colorSchemeName_);
}

