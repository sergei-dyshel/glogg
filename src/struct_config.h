#pragma once

#include "config_node.h"
#include "color_scheme.h"
#include "syntax.h"

class StructConfig {
  public:
    StructConfig() = default;

    const ColorScheme &colorScheme() const { return colorScheme_; }
    const SyntaxCollection &syntaxColl() const { return syntaxColl_; }
    bool checkForIssues() const;

    static const StructConfig &instance();
    static void loadDefault();
    static void reload(const QStringList &dirs = {});

  private:
    void load(const QStringList &dirs = {});

    ColorScheme colorScheme_;
    SyntaxCollection syntaxColl_;
};