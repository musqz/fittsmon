# FittsMon

## _Mouse Corner Action Tool for X11  with Multi-Monitor Support._

> I actively use FittsMon on my daily Mabox *Openbox Desktop Linux system without any issues.

### A perfect match = [skippy-xd](https://github.com/felixfung/skippy-xd) & fittsmon

**Daily used setup:**

- mabox linux 'openbox'
- 2 monitors with different size

_Should work with other desktop managers or window managers._

**FittsMon** is a lightweight X11 utility that binds mouse actions (clicks, scrolls, enter/leave) to screen corners and edges, triggering custom commands.
Inspired by [Fitts’s Law](https://en.wikipedia.org/wiki/Fitts%27s_law), it transforms screen edges into powerful productivity zones.

---

## 🔧 Features

- **Bind mouse buttons** and wheel events to commands
- **Minimal dependencies**, no daemon or tray clutter
- **Easy-to-edit** INI-style configuration
- **Multi-Monitor Support:**  
You can specify which monitors to use by providing their names when starting the program.

---

## 📦 Dependencies

### General Dependencies

- `xcb`, `x11`, `xcb-randr`
- `glib-2.0`

### Arch Manjaro (pacman)

To install the necessary packages on Arch Manjaro, run:

```
sudo pacman -S libxcb libxrandr glib2 xcb-proto
```

### Debian/Ubuntu (apt)

To install the necessary packages on Debian or Ubuntu, run: 

Note: Not tested.

```
sudo apt-get update
sudo apt-get install libxcb1-dev libxrandr-dev libglib2.0-dev xcb-util-dev
```

---

## 🛠️ Installation

```
git clone https://github.com/musqz/fittsmon.git
cd fittsmon
make
sudo make install
```

On first run, a sample config will be created at:

```
~/.config/fittsmon/fittsmonrc
```

---

## 🚀 Usage

### Start FittsMon single monitor

```
fittsmon
```

### Start FittsMon multi monitors

In case of multiple monitors, specify which monitors to use, including primary display. Order does not matter.

```
fittsmon DP-0 HDMI-0
```

If `HDMI-0` is `Extended` and used only in the command, only rules for HDMI-0 will be active, 

and the primary display will have no rules applied.

This way you can turn off the Default display from fittsmon activity.

### List available monitors

```
# See available monitors.
# This will output names like `eDP-1`, `HDMI-0`, `DP-0`, which you can use in the config.

$ fittsmon --list 
```

### Help

```
# Usage help text.

$ fittsmon -h | --help 
```

---

## ⚙️ Configuration

Edit the config at `~/.config/fittsmon/fittsmonrc`.

Each section targets a screen area on a monitor:

```
[MonitorName-Position]
Event=command

```

### Positions

- Corners: `TopLeft`, `TopRight`, `BottomLeft`, `BottomRight`
- Edges: `Top`, `Bottom`, `Left`, `Right`
- Extras: `TopCenter`, `BottomCenter`

### Events

- Mouse buttons: `LeftButton`, `RightButton`, `MiddleButton`
- Wheel: `WheelUp`, `WheelDown`, `WheelUpOnce`, `WheelDownOnce`
- Pointer: `Enter`, `Leave`

### Example

```ini
[TopCenter]
WheelUp=amixer -D pulse set Master 5%+
WheelDown=amixer -D pulse set Master 5%-
WheelUpOnce=notify-send "Default: Wheel Up Once"
WheelDownOnce=notify-send "Default: Wheel Down Once"
MiddleButton=notify-send "Default: Middle Button"
RightButton=pactl set-sink-mute @DEFAULT_SINK@ toggle
LeftButton=notify-send "Default: Left Button"
Enter=
Leave=

[DP-0-TopCenter]
WheelUp=amixer -D pulse set Master 5%+
WheelDown=amixer -D pulse set Master 5%-
WheelUpOnce=notify-send "Default: Wheel Up Once"
WheelDownOnce=notify-send "Default: Wheel Down Once"
MiddleButton=notify-send "Default: Middle Button"
RightButton=pactl set-sink-mute @DEFAULT_SINK@ toggle
LeftButton=notify-send "Default: Left Button"
Enter=
Leave=
```

### NOTE

> To avoid strange behaviour. Keep all lines in each section.

```
WheelUp=
WheelDown=
WheelUpOnce=
WheelDownOnce=
MiddleButton=
RightButton=
LeftButton=
Enter=
Leave=
```
---

## 🖥️ Multi-Monitor Setup

1. **Run `fittsmon --list`** to get monitor names  
2. **Add sections in the config** using the format `[MonitorName-Position]`  
3. **Start FittsMon** with or without specifying monitors

---

## 🔄 Autostart

Add something like `fittsmon DP-0 HDM-1 &` to:
- `.xinitrc`, `.xprofile`, or your WM's autostart script
- Your desktop environment's session startup settings

---

## 📄 License

Same license as the original [`fittstool`](https://github.com/napcok/fittstool) project.

## Disclaimer 

The original code has been adapted using AI assistance. 

The script is so tiny and runs so nice on the openbox desktop (2 monitors), 

so why not share it.

Due to a lack of expertise in C programming, this repository will not be actively maintained. 
