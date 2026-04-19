# FittsMon – Mouse Corner Action Tool for X11 (Multi-Monitor Support)

**FittsMon** is a lightweight X11 utility that binds mouse actions (clicks, scrolls, enter/leave) to screen corners and edges, triggering custom commands.
Inspired by [Fitts's Law](https://en.wikipedia.org/wiki/Fitts%27s_law), it transforms screen edges into powerful productivity zones — now with full multi-monitor support.

---

## 🔧 Features

- **Bind mouse buttons** and wheel events to commands
- **Multiple event types**: button clicks, scroll wheel (throttled and non-throttled), cursor enter/leave
- **Minimal dependencies**, no daemon or tray clutter
- **Easy-to-edit** INI-style configuration
- **Multi-Monitor Support:** Enable fittsmon on specific monitors or all enabled monitors
- **Security hardening:** Command validation prevents shell injection attacks

---

## 📦 Dependencies

### General Dependencies

- `xcb`, `x11`, `xcb-randr` (X11 libraries)
- `glib-2.0` (GLib library)

### Arch / Manjaro (pacman)

```bash
sudo pacman -S libxcb libxrandr glib2 xcb-proto
```

### Debian / Ubuntu (apt)

```bash
sudo apt-get update
sudo apt-get install libxcb1-dev libxrandr-dev libglib2.0-dev xcb-util-dev
```

---

## 🛠️ Installation 

```bash
git clone https://github.com/musqz/fittsmon.git
cd fittsmon
make && make install  
```

On first run, a sample config will be created at:
```
~/.config/fittsmon/fittsmonrc
```

---

## 🚀 Usage

### Start FittsMon with primary monitor (default)

```bash
fittsmon
```

Automatically uses the primary monitor.

### Start FittsMon with specific monitors

Use the `--monitor` flag to include or exclude monitors.

```bash
fittsmon --monitor DP-0 HDMI-0
```

### List available monitors

Display all detected monitors and their properties:

```bash
fittsmon --list
```

Output shows monitor names like `eDP-1`, `HDMI-0`, `DP-0` (use these in the config file).

### Help

Display usage information and examples:

```bash
fittsmon --help
fittsmon -h
```

---

## ⚙️ Configuration

Edit the config at `~/.config/fittsmon/fittsmonrc`.

Each section targets a screen area on a monitor:

```ini
[MonitorName-Position]
Event=command
```

### Positions

- Corners: `TopLeft`, `TopRight`, `BottomLeft`, `BottomRight`
- Edges: `Top`, `Bottom`, `Left`, `Right`
- Center edges: `TopCenter`, `BottomCenter`

### Events

- **Mouse buttons:** `LeftButton`, `RightButton`, `MiddleButton`
- **Wheel scroll:** `WheelUp`, `WheelDown` (execute on every scroll)
- **Wheel throttled:** `WheelUpOnce`, `WheelDownOnce` (execute at most once per 2 seconds)
- **Pointer events:** `Enter`, `Leave`

### Example

```ini
[TopCenter]
WheelUp=amixer -D pulse set Master 5%+
WheelDown=amixer -D pulse set Master 5%-
MiddleButton=notify-send "Default: Middle Button"
RightButton=pactl set-sink-mute @DEFAULT_SINK@ toggle

[DP-0-TopCenter]
Enter=skippy-xd 
```

## 🖥️ Multi-Monitor Setup

1. **Run `fittsmon --list`** to get monitor names and properties
2. **Add sections in the config** using the format `[MonitorName-Position]`
3. **Start FittsMon** with or without specifying monitors

### Configuration Precedence

1. **Monitor-specific config** (`[MonitorName-Position]`) – highest priority
2. **Default config** (`[Position]`) – fallback for all enabled monitors

---

## 🔄 Autostart

Add to `.xinitrc`, `.xprofile`, or your WM's autostart script:

```bash
# Primary monitor only (default)
fittsmon &

# Specific monitors
fittsmon --monitor DP-0 HDMI-0 &
```

Or add to your desktop environment's session startup settings.

---

## 🔒 Security

Command validation prevents shell injection attacks:
- Commands containing shell metacharacters (`;`, `|`, `&`, `>`, `<`, `` ` ``, `$`, `()`, etc.) are rejected
- For complex operations, create a shell script and reference it instead

Example safe config:
```ini
[TopRight]
LeftButton=/home/user/scripts/my_action.sh arg1 arg2
```

---

## 📚 Man Page

For comprehensive documentation:

```bash
man fittsmon
```

Or view the man page source:

```bash
man ./fittsmon.1
```

---

## 📄 License

Same license as the original [`fittstool`](https://github.com/napcok/fittstool) project (GPL-2.0).
