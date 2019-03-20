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
#include "template_utils.h"

#include <memory>

#include <QFile>
#include <QDir>

const QString StructConfig::COLORS_GLOB_PATTERN = "*.glogg-colors.yaml";
const QString StructConfig::SYNTAX_GLOB_PATTERN = "*.glogg-syntax.yaml";

static void scanSingleDir(const QDir& dir, StructConfigFiles &files)
{
    for (auto fileInfo : dir.entryInfoList({StructConfig::COLORS_GLOB_PATTERN}))
        files.colorsFiles.push_back(fileInfo.absoluteFilePath());
    for (auto fileInfo :
         dir.entryInfoList({StructConfig::SYNTAX_GLOB_PATTERN})) {
        files.syntaxFiles.push_back(fileInfo.absoluteFilePath());
    }
}

void StructConfig::scanDirs(const QStringList &dirs, StructConfigFiles &files)
{
    for (auto dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) {
            WARN << dirPath << " does not exist, skipping";
            continue;
        }
        scanSingleDir(dir, files);
    }
}

StructConfig::StructConfig(const QStringList &colorsFiles,
                           const QStringList &syntaxFiles,
                           bool runTests,
                           bool stopOnError)
{
    DEBUG << "Color scheme files:" << colorsFiles;
    DEBUG << "Syntax files:" << syntaxFiles;
    for (const auto &file : colorsFiles) {
        try {
            mergeMaps(colorSchemes_,
                      ColorScheme::loadAll(ConfigNode::parseFile(file)),
                      false /* do not override */);
            INFO << "Loaded color scheme file " << file;
        }
        catch (const ConfigError &exc) {
            if (stopOnError)
                throw exc;
            ERROR << "Error loading color scheme file" << file << ":" << exc;
        }
    }
    for (const auto &file : syntaxFiles) {
        SyntaxCollection fileColl;
        if (!stopOnError) {
            try {
                fileColl
                    = SyntaxCollection(ConfigNode::parseFile(file), runTests);
                syntaxColl_.merge(fileColl);
            } catch (const ConfigError &exc) {
                ERROR << "Error loading syntax file " << file << ": "
                    << exc;
            }
        } else {
            fileColl = SyntaxCollection(ConfigNode::parseFile(file), runTests);
            syntaxColl_.merge(fileColl);
        }
        INFO << "Loaded syntax file " << file << ","
             << fileColl.syntaxes().size() << "new syntax defs in file, "
             << syntaxColl_.syntaxes().size() << "syntax defs after merge";
    }
}

StructConfig::StructConfig(const StructConfigFiles &configFiles, bool runTests,
                           bool stopOnError)
    : StructConfig(configFiles.colorsFiles, configFiles.syntaxFiles, runTests,
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

void StructConfig::scanDirsAndFiles(const QStringList &dirsAndFiles,
                                    StructConfigFiles &files)
{
    for (const auto &path : dirsAndFiles) {
        QDir dir(path);
        if (dir.exists())
            scanSingleDir(dir, files);
        else if (QDir::match(COLORS_GLOB_PATTERN, path))
            files.colorsFiles.append(path);
        else if (QDir::match(SYNTAX_GLOB_PATTERN, path))
            files.syntaxFiles.append(path);
        else
            throw ASSERT << "File" << path << "not recognized";
    }
}