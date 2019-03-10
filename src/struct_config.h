#pragma once

#include "config_node.h"
#include "color_scheme.h"
#include "syntax.h"

struct StructConfigFiles {
    QStringList colorsFiles;
    QStringList syntaxFiles;
};

class StructConfig {
  public:
    StructConfig() = default;
    StructConfig(const QStringList &colorsFiles, const QStringList &syntaxFiles,
                 bool stopOnError = false);
    StructConfig(const StructConfigFiles &configFiles,
                          bool stopOnError = false);

    StructConfig(const StructConfig &) = delete;

    const ColorScheme &colorScheme() const { return colorSchemes_.at("default"); }
    const SyntaxCollection &syntaxColl() const { return syntaxColl_; }
    bool checkForIssues() const;

  private:
    std::map<QString, ColorScheme> colorSchemes_;
    SyntaxCollection syntaxColl_;
};

class StructConfigStore {
  public:
    static const StructConfig &current();
    static void loadDefault();
    static void reload();
    static StructConfigFiles scanDirs(const QStringList &dirs);
};