Detailed list of changes
=========================

## Search (filter) bar improvements:
- Shrinked to single line of controls to maximize log view area.
- Button text labels replaced by icons.
- "Pin" (button to the right of edit box) frequent searches to the top of the history dropdown.
- Search bar expression is parsed as `<include regex>|||<exclude regex>`.
- Invalid expression is highlighted by yellow background.

## Keyboard/navigation improvements:
- Pressing `t` in log view focuses search bar.
- `tab` always moves focus to log view.
- Move splitter with `-/+` and with larger step.

## Tabbar improvements:
- Next/previous tab with `Ctrl+PgUp/PgDown`.
- Draw red border around main/filtered log view.
- Restore tab labels to default size.
- Allow renaming tabs (persistent).
- Allow changing tab label background color.
- New context menu action to close previous tabs.
- New context menu action to open tab's file in other instance.

## Quick find bar improvments:
- Pressing `ESC` cancels search and focuses last used log view.
- Cancelling quick find clears highlighting.
- The bar remains visible during search.

# Log view improvemnts:
- Increase mark bullet size.
- `h/l` scroll with larger step.
- Multiline or portion selections copied to clipboard.
- Highlight selected pattern with one of predefined colors.

# General UI improvements and fixes:
- Status bar text can be selected.
- Window is not reopened when opening files from external instances.

Development
===========

Build system was migrated to CMake for ease of building.
Root [Makefile](#Makefile) defines common tasks for *debug* and *release* builds.

```
# Debug build in ./build/debug directory:
make configure_debug PREFIX=/usr/local
make debug

# Release build in ./build/release directory:
make configure_release PREFIX=/usr/local
make debug

make install  # installs release build only
```

Debug build also compiles and runs unit tests and validates schema for
bundled syntax and color scheme files. For schema validation some Python dependencies are required:
```
pip install jsonschema yaml
```

Syntax highlighting
===================

TODO: