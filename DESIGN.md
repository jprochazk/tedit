### Platforms

- Windows
- Linux
- Mac

### Layout

- Menu bar should be part of the title bar together with minimize/maximize/close buttons.
- Command palette appears in the top middle.
- Tools appear on the bottom middle and bottom left. Keybinds should correspond to their placement on the keyboard, e.g. bottom-left-most tool should be bound to `z` by default.
- While editing levels/worlds, entire viewport contains the current level/world, and camera supports pan/zoom
  - Note: Add keybinds for resetting camera position and "fit world in camera"
- Editor consists of floating panels which may be docked in arbitrary configurations
  - Browsers: Worlds, Levels, Layers, Templates, Objects, Atlases

TODO: describe entities

### Basic workflow

Start `tedit` -> opens project browser
Create project -> creates project with a single level, then opens it
Open project -> open project (should be in the same state when it was closed)

### Commands

Each action the user can perform is represented as a command.

All commands must be reversible, so that undo/redo buffer can be implemented. This does not mean that the command itself must have an `undo` method, it may means that there should be an API for updating `State` which also keeps a buffer of all changes (such as a diff), and those changes must be reversible.

Note that a "change" may last multiple frames. An example of this is drawing a line of tiles - the change starts when the user begins to drag the mouse, and ends once they stop and release the left mouse button.

Keybindings should also support sequences, e.g. `CTRL+K F` is two "keypresses" -> `CTRL+K` followed by `F`.

Commands can be bound to keys. The application provides some keybindings as part of the default configuration, but it can be modified as needed.

Commands can also be executed from the command palette. Opening the command palette is done using CTRL+P by default, but it can also be configured. The command palette cannot be unbound, but it can be rebound to a different key.

The command palette workflow should be similar to VSCode.

### Extensibility

User should be able to write scripts that interact with the editor. Scripts are written in a custom Python-like language. The editor should also expose a WebSocket server for features such as hot-reloading levels.

### Dependencies

```
windowing    winit  https://docs.rs/winit/latest/winit/
clipboard    <?>
file dialog  rfd    https://github.com/PolyMeilex/rfd
rendering    wgpu   https://github.com/gfx-rs/wgpu
UI           egui
```

