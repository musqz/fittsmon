/**************************************************************************
*
* fittsmon-wl : Wayland port of fittsmon. Screen-edge hot zones are tiny
*               transparent wlr-layer-shell surfaces that receive pointer
*               events. Needs a wlroots-style compositor (labwc, sway, ...).
*
* Copyright (C) 2009 Yasen Atanasov (yasen.atanasov@gmail.com)
* Modified 2025 by Eric Bakker (linuxbirdtweets@duck.com)
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
**************************************************************************/

#define _GNU_SOURCE   /* memfd_create() */

#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <linux/input-event-codes.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

#define str_defined(str) ( (str && strlen(str) > 0) ? 1 : 0 )
#define unless(a)        if ( ! (a) )

enum eScreenCorners {
  TopLeft, TopCenter, TopRight, Right, BottomRight, BottomCenter, BottomLeft, Left
};

enum eMouseButtons {
  LeftButton, MiddleButton, RightButton, WheelUp, WheelDown, WheelUpOnce, WheelDownOnce, Enter, Leave
};

#define MAX_MONITORS 8

struct zone {
  char enabled;
  int w;
  int h;
  char commands[9][200];
  time_t last_time_up;
  time_t last_time_down;
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;
  int mapped;
};

typedef struct {
  struct wl_output *output;
  int width;       /* current mode size in pixels */
  int height;
  int scale;
  char name[32];
  int enabled;
} MonitorInfo;

struct zone zones[8 * MAX_MONITORS];
MonitorInfo monitors[MAX_MONITORS];
int monitor_count = 0;

static struct wl_display *display;
static struct wl_registry *registry;
static struct wl_compositor *compositor;
static struct wl_shm *shm;
static struct wl_seat *seat;
static struct zwlr_layer_shell_v1 *layer_shell;
static struct zone *pointer_zone;  /* zone the pointer is currently in */

int  can_execute (struct zone *z, const int direction);
int  is_safe_command (const char *cmd);
void config_read ();
void config_read_file (const char *file_path);
void fill_file(const char *file_path);
void init_options (int monitor_index);
void print_monitor_list();

/* Run a configured command if it is set and passes the safety check */
static void
run_cmd (struct zone *z, int ev)
{
  const char *cmd = z->commands[ev];
  if (!str_defined(cmd) || !is_safe_command(cmd)) return;
  int ret = system(cmd);
  if (ret != 0) fprintf(stderr, "Command failed with code %d\n", ret);
}

/* libwayland aborts on a NULL listener slot, so unused events get this */
static void noop () { }

/* wl_output */

static void
output_mode (void *data, struct wl_output *o, uint32_t flags, int32_t w, int32_t h, int32_t refresh)
{
  MonitorInfo *m = data;
  (void)o; (void)refresh;
  if (flags & WL_OUTPUT_MODE_CURRENT) { m->width = w; m->height = h; }
}

static void
output_scale (void *data, struct wl_output *o, int32_t scale)
{
  MonitorInfo *m = data;
  (void)o;
  m->scale = scale;
}

static void
output_name (void *data, struct wl_output *o, const char *name)
{
  MonitorInfo *m = data;
  (void)o;
  snprintf(m->name, sizeof(m->name), "%s", name);
}

static const struct wl_output_listener output_listener = {
  .geometry = noop, .mode = output_mode, .done = noop, .scale = output_scale,
  .name = output_name, .description = noop,
};

/* wl_pointer */

static void
pointer_enter (void *data, struct wl_pointer *p, uint32_t serial, struct wl_surface *s, wl_fixed_t x, wl_fixed_t y)
{
  (void)data; (void)p; (void)serial; (void)x; (void)y;
  for (int i = 0; i < 8 * monitor_count; i++) {
    if (zones[i].surface == s) { pointer_zone = &zones[i]; run_cmd(pointer_zone, Enter); return; }
  }
}

static void
pointer_leave (void *data, struct wl_pointer *p, uint32_t serial, struct wl_surface *s)
{
  (void)data; (void)p; (void)serial; (void)s;
  if (pointer_zone) run_cmd(pointer_zone, Leave);
  pointer_zone = NULL;
}

static void
pointer_button (void *data, struct wl_pointer *p, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
  (void)data; (void)p; (void)serial; (void)time;
  if (!pointer_zone || state != WL_POINTER_BUTTON_STATE_PRESSED) return;
  switch (button) {
    case BTN_LEFT:   run_cmd(pointer_zone, LeftButton);   break;
    case BTN_MIDDLE: run_cmd(pointer_zone, MiddleButton); break;
    case BTN_RIGHT:  run_cmd(pointer_zone, RightButton);  break;
  }
}

/* One step per wheel notch; continuous (touchpad) scrolling is ignored */
static void
pointer_axis_discrete (void *data, struct wl_pointer *p, uint32_t axis, int32_t steps)
{
  (void)data; (void)p;
  if (!pointer_zone || axis != WL_POINTER_AXIS_VERTICAL_SCROLL) return;
  if (steps < 0) {
    run_cmd(pointer_zone, WheelUp);
    if (can_execute(pointer_zone, 0)) run_cmd(pointer_zone, WheelUpOnce);
  } else if (steps > 0) {
    run_cmd(pointer_zone, WheelDown);
    if (can_execute(pointer_zone, 1)) run_cmd(pointer_zone, WheelDownOnce);
  }
}

static const struct wl_pointer_listener pointer_listener = {
  .enter = pointer_enter, .leave = pointer_leave, .motion = noop,
  .button = pointer_button, .axis = noop, .frame = noop, .axis_source = noop,
  .axis_stop = noop, .axis_discrete = pointer_axis_discrete,
};

/* wl_seat */

static void
seat_capabilities (void *data, struct wl_seat *s, uint32_t caps)
{
  (void)data;
  static struct wl_pointer *pointer;
  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !pointer) {
    pointer = wl_seat_get_pointer(s);
    wl_pointer_add_listener(pointer, &pointer_listener, NULL);
  }
}

static const struct wl_seat_listener seat_listener = {
  .capabilities = seat_capabilities, .name = noop,
};

/* wl_registry */

static void
registry_global (void *data, struct wl_registry *r, uint32_t name, const char *iface, uint32_t version)
{
  (void)data;
  if (strcmp(iface, wl_compositor_interface.name) == 0) {
    compositor = wl_registry_bind(r, name, &wl_compositor_interface, 4);
  } else if (strcmp(iface, wl_shm_interface.name) == 0) {
    shm = wl_registry_bind(r, name, &wl_shm_interface, 1);
  } else if (strcmp(iface, zwlr_layer_shell_v1_interface.name) == 0) {
    layer_shell = wl_registry_bind(r, name, &zwlr_layer_shell_v1_interface, 1);
  } else if (strcmp(iface, wl_seat_interface.name) == 0 && !seat) {
    seat = wl_registry_bind(r, name, &wl_seat_interface, 5);
    wl_seat_add_listener(seat, &seat_listener, NULL);
  } else if (strcmp(iface, wl_output_interface.name) == 0 && monitor_count < MAX_MONITORS) {
    /* wl_output.name needs version 4 */
    if (version < 4) {
      fprintf(stderr, "Warning: wl_output v%u has no names, skipping output\n", version);
      return;
    }
    MonitorInfo *m = &monitors[monitor_count++];
    m->scale = 1;
    m->output = wl_registry_bind(r, name, &wl_output_interface, 4);
    wl_output_add_listener(m->output, &output_listener, m);
  }
}

static const struct wl_registry_listener registry_listener = {
  .global = registry_global, .global_remove = noop,
};

/* Zones */

/* Transparent w x h ARGB buffer: shm memory is zero-filled */
static struct wl_buffer *
make_buffer (int w, int h)
{
  size_t size = (size_t)w * h * 4;
  int fd = memfd_create("fittsmon", MFD_CLOEXEC);
  if (fd < 0 || ftruncate(fd, size) < 0) { perror("memfd"); exit(1); }

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  struct wl_buffer *buf = wl_shm_pool_create_buffer(pool, 0, w, h, w * 4, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(pool);
  close(fd);
  return buf;
}

static void
layer_configure (void *data, struct zwlr_layer_surface_v1 *ls, uint32_t serial, uint32_t w, uint32_t h)
{
  struct zone *z = data;
  (void)w; (void)h;
  zwlr_layer_surface_v1_ack_configure(ls, serial);
  if (z->mapped) return;
  wl_surface_attach(z->surface, make_buffer(z->w, z->h), 0, 0);
  wl_surface_commit(z->surface);
  z->mapped = 1;
}

static void
layer_closed (void *data, struct zwlr_layer_surface_v1 *ls)
{
  struct zone *z = data;
  (void)ls;
  z->enabled = 0;
}

static const struct zwlr_layer_surface_v1_listener layer_listener = {
  .configure = layer_configure, .closed = layer_closed,
};

/* Anchoring to the edge(s) centres the zone along the free axis */
static const uint32_t zone_anchor[8] = {
  [TopLeft]      = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP    | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT,
  [TopCenter]    = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP,
  [TopRight]     = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP    | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
  [Right]        = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
  [BottomRight]  = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
  [BottomCenter] = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
  [BottomLeft]   = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT,
  [Left]         = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT,
};

static void
create_zones (void)
{
  for (int i = 0; i < 8 * monitor_count; i++) {
    struct zone *z = &zones[i];
    unless (z->enabled) continue;

    z->surface = wl_compositor_create_surface(compositor);
    z->layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell, z->surface,
        monitors[i / 8].output, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "fittsmon");
    zwlr_layer_surface_v1_add_listener(z->layer_surface, &layer_listener, z);
    zwlr_layer_surface_v1_set_size(z->layer_surface, z->w, z->h);
    zwlr_layer_surface_v1_set_anchor(z->layer_surface, zone_anchor[i % 8]);
    zwlr_layer_surface_v1_set_exclusive_zone(z->layer_surface, -1);
    wl_surface_commit(z->surface);
  }
}

int
can_execute (struct zone *z, const int direction)
{
  time_t current_time;
  time_t *last_exec = direction ? &z->last_time_down : &z->last_time_up;

  time(&current_time);
  if ((long int) current_time - (long int) *last_exec > 2) {
    *last_exec = current_time;
    return 1;
  }
  return 0;
}

/* Validate command string for dangerous shell metacharacters */
int
is_safe_command (const char *cmd)
{
  if (!cmd || strlen(cmd) == 0) return 0;

  char buf[256];
  size_t len = strlen(cmd);
  if (len >= sizeof(buf)) {
    fprintf(stderr, "Warning: Command too long (max %zu bytes): %s\n", sizeof(buf) - 1, cmd);
    return 0;
  }
  strcpy(buf, cmd);

  while (len > 0 && (buf[len-1] == ' ' || buf[len-1] == '\t')) buf[--len] = '\0';

  /* a trailing '&' (background) is allowed */
  if (len > 0 && buf[len-1] == '&') {
    buf[--len] = '\0';
    while (len > 0 && (buf[len-1] == ' ' || buf[len-1] == '\t')) buf[--len] = '\0';
  }

  const char *dangerous = ";|><`$()\\\"'";
  if (strcspn(buf, dangerous) != strlen(buf)) {
    fprintf(stderr, "Warning: Potentially dangerous command blocked (contains shell metacharacters): %s\n", cmd);
    return 0;
  }

  return 1;
}

void
init_options (int monitor_index)
{
  int sw = monitors[monitor_index].width / monitors[monitor_index].scale;
  int sh = monitors[monitor_index].height / monitors[monitor_index].scale;
  int base_idx = monitor_index * 8;

  for (int i = 0; i < 8; i++) {
    zones[base_idx + i].enabled = monitors[monitor_index].enabled;
    zones[base_idx + i].last_time_down = (time_t) 0;
    zones[base_idx + i].last_time_up = (time_t) 0;
  }

  zones[base_idx + TopLeft].w = 5;      zones[base_idx + TopLeft].h = 5;
  zones[base_idx + TopCenter].w = sw*0.6; zones[base_idx + TopCenter].h = 2;
  zones[base_idx + TopRight].w = 6;     zones[base_idx + TopRight].h = 5;
  zones[base_idx + Right].w = 3;        zones[base_idx + Right].h = sh*0.6;
  zones[base_idx + BottomRight].w = 6;  zones[base_idx + BottomRight].h = 6;
  zones[base_idx + BottomCenter].w = sw*0.6; zones[base_idx + BottomCenter].h = 2;
  zones[base_idx + BottomLeft].w = 5;   zones[base_idx + BottomLeft].h = 5;
  zones[base_idx + Left].w = 2;         zones[base_idx + Left].h = sh*0.6;
}

void
config_read_file (const char *file_path)
{
  GKeyFile* config_file;
  gchar* sections[] = {"TopLeft", "TopCenter", "TopRight", "Right", "BottomRight", "BottomCenter", "BottomLeft", "Left"};
  gchar* events[] = {"LeftButton", "MiddleButton", "RightButton", "WheelUp", "WheelDown", "WheelUpOnce", "WheelDownOnce", "Enter", "Leave"};
  gchar* current_value;
  int i, j, m;
  gchar* group_name;

  config_file = g_key_file_new();

  unless (g_key_file_load_from_file(config_file, file_path, G_KEY_FILE_NONE, NULL)) {
    g_key_file_free(config_file);
    return;
  }

  /* "[Position]" groups apply to every enabled monitor, then
     "[Monitor-Position]" groups override them */
  for (m = 0; m < monitor_count; m++) {
    if (!monitors[m].enabled) continue;

    for (i = 0; i < 8; i++) {
      for (int pass = 0; pass < 2; pass++) {
        group_name = pass ? g_strdup_printf("%s-%s", monitors[m].name, sections[i])
                          : g_strdup(sections[i]);

        if (g_key_file_has_group(config_file, group_name)) {
          struct zone *z = &zones[m * 8 + i];
          z->enabled = 1;

          for (j = 0; j < 9; j++) {
            if ((current_value = g_key_file_get_value(config_file, group_name, events[j], NULL))) {
              gchar *cmd = g_str_has_suffix(current_value, "&")
                         ? g_strdup(current_value)
                         : g_strdup_printf("%s &", current_value);

              snprintf(z->commands[j], sizeof(z->commands[j]), "%s", cmd);
              printf("%s %s : %s (monitor: %s)\n", sections[i], events[j], cmd, monitors[m].name);

              g_free(cmd);
              g_free(current_value);
            }
          }
        }

        g_free(group_name);
      }
    }
  }

  g_key_file_free(config_file);
}

void
fill_file(const char *file_path)
{
  FILE *fp;

  char* lines[] = {
    "#fittsmonrc example\n",
    "#volume control in the top right corner:\n\n",
    "[TopRight]\n",
    "WheelUp=amixer -q sset Master 2+\n",
    "WheelDown=amixer -q sset Master 2-\n",
    "RightButton=amixer -q sset Master toggle\n",
    "LeftButton=xterm -C alsamixer\n\n\n",
    "#Multi-monitor configuration example:\n",
    "#[DP-0-TopLeft]\n",
    "#LeftButton=notify-send \"Clicked on DP-0 Top Left\"\n\n",
    "#[HDMI-0-BottomRight]\n",
    "#RightButton=notify-send \"Clicked on HDMI-0 Bottom Right\"\n\n",
    "#Available positions: Left, TopLeft, TopCenter, TopRight, Right, BottomRight, BottomCenter, BottomLeft\n",
    "#Available events: LeftButton, RightButton, MiddleButton, WheelUp, WheelDown, WheelUpOnce, WheelDownOnce, Enter, Leave\n",
    "#Monitor-specific config format: [MonitorName-Position]\n"
  };

  fp = fopen(file_path, "wb");
  if (fp == NULL) return;

  for (size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) fputs(lines[i], fp);

  fclose (fp);
}

void
config_read ()
{
  char *path;

  path = g_build_filename (g_get_user_config_dir(), "fittsmon", "fittsmonrc", NULL);
  if (g_file_test (path, G_FILE_TEST_EXISTS)) {
    config_read_file(path);
    g_free(path);
    return;
  }

  char *dir = g_build_filename (g_get_user_config_dir(), "fittsmon", NULL);
  if (!g_file_test (dir, G_FILE_TEST_IS_DIR)) g_mkdir(dir, 0777);
  g_free(dir);

  fill_file(path);
  printf("Created a sample fittsmonrc for you in %s \n", path);

  config_read_file (path);
  g_free(path);
}

void
print_monitor_list ()
{
  printf("Available monitors:\n");
  for (int i = 0; i < monitor_count; i++)
    printf("  %s (%dx%d, scale %d)\n", monitors[i].name,
           monitors[i].width, monitors[i].height, monitors[i].scale);
}

static void
print_usage ()
{
  printf("fittsmon-wl - Map mouse button events on screen corners to commands (Wayland)\n\n");
  printf("USAGE:\n");
  printf("  fittsmon-wl [-h | -l]\n");
  printf("  fittsmon-wl [--monitor] name ...\n\n");
  printf("With no monitor given, the first output is used.\n");
  printf("Config: ~/.config/fittsmon/fittsmonrc (same format as fittsmon)\n");
}

int
main(int argc, char* argv[])
{
  int i;

  if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
    print_usage();
    return 0;
  }

  display = wl_display_connect(NULL);
  if (!display) {
    fprintf(stderr, "Failed to connect to Wayland display\n");
    return 1;
  }

  registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, NULL);
  wl_display_roundtrip(display);  /* globals */
  wl_display_roundtrip(display);  /* output name/mode/scale */

  if (!compositor || !shm || !layer_shell) {
    fprintf(stderr, "Compositor lacks wl_compositor, wl_shm or zwlr_layer_shell_v1\n");
    return 1;
  }

  if (argc > 1 && (strcmp(argv[1], "--list") == 0 || strcmp(argv[1], "-l") == 0)) {
    print_monitor_list();
    return 0;
  }

  if (monitor_count == 0) {
    fprintf(stderr, "No outputs found\n");
    return 1;
  }

  if (argc > 1) {
    int first = strcmp(argv[1], "--monitor") == 0 ? 2 : 1;
    int any = 0;
    for (i = first; i < argc; i++) {
      int found = 0;
      for (int m = 0; m < monitor_count; m++) {
        if (strcmp(monitors[m].name, argv[i]) == 0) { monitors[m].enabled = found = any = 1; }
      }
      unless (found) fprintf(stderr, "Warning: Monitor '%s' not found\n", argv[i]);
    }
    unless (any) {
      print_monitor_list();
      return 1;
    }
  } else {
    monitors[0].enabled = 1;
    printf("No monitor specified, using first output: %s\n", monitors[0].name);
  }

  for (i = 0; i < monitor_count; i++) init_options(i);

  config_read();
  create_zones();

  while (wl_display_dispatch(display) != -1) { }

  return 0;
}
