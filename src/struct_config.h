#pragma once

#include "config_node.h"
#include "color_scheme.h"
#include "syntax.h"

class StructConfig {
public:
    StructConfig() = default;

    void Load();
    const ColorScheme &colorScheme() const { return colorScheme_; }
    const Syntax &syntax() const { return syntax_; }
    bool checkForIssues() const;

    static const StructConfig &instance();
    static void loadDefault();
    static void reload();

private:
    void load();

    ColorScheme colorScheme_;
    Syntax syntax_;
}; // class StructConfig