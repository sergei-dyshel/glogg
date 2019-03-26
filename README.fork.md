Detailed list of changes
=========================

- Proper grammar-based syntax highlighting.
- Search (filter) bar improvements:
  - Shrinked to single line of controls to maximize log view area.
  - Button text labels replaced by icons.
  - "Pin" frequent searches to the top of the history dropdown.
  - Search bar expression is parsed as `<include regex>|||<exclude regex>`.
  - Pressing `t` in log view focuses searchbar (TODO:)
  - `tab` always moves focus to either log view.
- Keyboard/navigation improvements:
- Tabbar improvements:
  - Next/previous tab with `Ctrl+PgUp/PgDown`.
  - Draw red border around main/filtered log view.
  - Restore tab labels to default size.
- Quick find bar improvments:
  - Pressing `ESC` focuses last used log view.
  - Cancelling quick find clears highlighting.
- Log view improvemnts:
  - Increase mark bullet size.
  - Move splitter with `-/+` and with larger step.
  - `h/l` scroll with larger step.
- General UI improvements:
  - Status bar text is selectable.