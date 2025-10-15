/**************************************************************************
*
* fittsmon : map mouse button events on screen corners to commands.
*             Modified to support multiple monitors on Manjaro Linux.
*
* Copyright (C) 2009 Yasen Atanasov (yasen.atanasov@gmail.com)
* Modified 2025
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**************************************************************************/


/* INCLUDES */

#include <xcb/xcb.h>
#include <xcb/randr.h>  // Added for RandR extension to handle monitors
#include <glib/gstdio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>   /* getenv(), etc. */
#include <time.h>


/* MACROS */

#define str_defined(str) ( (str && strlen(str) > 0) ? 1 : 0 )
#define unless(a)        if ( ! (a) )
#define get_cmd(win,cmd) window_options[win].commands[cmd]

/* CONSTANTS/OPTIONS */

/* Screen Corners */
enum eScreenCorners {
  TopLeft, TopCenter, TopRight, Right, BottomRight, BottomCenter, BottomLeft, Left
};

/* Mouse button indexes for commands */
enum eMouseButtons {
  LeftButton, MiddleButton, RightButton, WheelUp, WheelDown, WheelUpOnce, WheelDownOnce, Enter, Leave
};

/* XCB mouse button indexes */
enum eXCBButtonIndexes {
  LEFT_BUTTON=1, MIDDLE_BUTTON, RIGHT_BUTTON, WHEEL_UP_BUTTON, WHEEL_DOWN_BUTTON
};

/* Maximum number of monitors */
#define MAX_MONITORS 8

/* STRUCTS */
struct str_window_options {
  char enabled;
  int x;
  int y;
  int h;
  int w;
  char commands[9][200];
  xcb_window_t xcb_window; /* pointer to the newly created window.      */
  time_t last_time_up; /* last time a wheel event on a corner has been made */
  time_t last_time_down; /* last time a wheel event on a corner has been made */
  char monitor_name[32]; /* The name of the monitor this window belongs to */
};

/* Monitor info struct */
typedef struct {
    int x;
    int y;
    int width;
    int height;
    int primary;
    char name[32];
    int enabled;  // Flag to indicate if this monitor is enabled by user args
} MonitorInfo;

/*GLOBALS*/
struct str_window_options window_options[8 * MAX_MONITORS];  // 8 zones per monitor, up to MAX_MONITORS
MonitorInfo monitors[MAX_MONITORS];
int monitor_count = 0;

/* function prototypes */
int  can_execute (const int corner, const int direction);
void config_read ();
void config_read_file (const char *file_path);
void fill_file(const char *file_path);
void init_options (int monitor_index);
void server_create_windows(xcb_connection_t *connection, xcb_screen_t *screen);
int  server_find_window(xcb_window_t win);
void server_event_loop (xcb_connection_t *connection);
int  get_all_monitors(xcb_connection_t *connection, xcb_screen_t *screen);
void enable_monitor_by_name(const char *name);
void print_monitor_list();

/* implementations */

/* Get all monitors connected to the system */
int get_all_monitors(xcb_connection_t *connection, xcb_screen_t *screen) {
    int count = 0;
    
    // Check if RandR extension is available
    const xcb_query_extension_reply_t *extension_reply = xcb_get_extension_data(connection, &xcb_randr_id);
    if (!extension_reply || !extension_reply->present) {
        printf("RandR extension not available, using whole screen as one monitor\n");
        
        // Create a fallback monitor entry for the whole screen
        monitors[0].x = 0;
        monitors[0].y = 0;
        monitors[0].width = screen->width_in_pixels;
        monitors[0].height = screen->height_in_pixels;
        monitors[0].primary = 1;
        monitors[0].enabled = 1;
        strncpy(monitors[0].name, "default", sizeof(monitors[0].name) - 1);
        
        return 1;  // We have one monitor
    }
    
    // Check RandR version
    xcb_randr_query_version_cookie_t version_cookie = xcb_randr_query_version(connection, 1, 5);
    xcb_randr_query_version_reply_t *version_reply = xcb_randr_query_version_reply(connection, version_cookie, NULL);
    if (!version_reply) {
        printf("Failed to get RandR version, using whole screen\n");
        
        // Create a fallback monitor entry
        monitors[0].x = 0;
        monitors[0].y = 0;
        monitors[0].width = screen->width_in_pixels;
        monitors[0].height = screen->height_in_pixels;
        monitors[0].primary = 1;
        monitors[0].enabled = 1;
        strncpy(monitors[0].name, "default", sizeof(monitors[0].name) - 1);
        
        free(version_reply);
        return 1;
    }
    free(version_reply);
    
    // Get screen resources
    xcb_randr_get_screen_resources_current_cookie_t res_cookie = 
        xcb_randr_get_screen_resources_current(connection, screen->root);
    xcb_randr_get_screen_resources_current_reply_t *res_reply = 
        xcb_randr_get_screen_resources_current_reply(connection, res_cookie, NULL);
    
    if (!res_reply) {
        printf("Failed to get screen resources, using whole screen\n");
        
        // Create a fallback monitor entry
        monitors[0].x = 0;
        monitors[0].y = 0;
        monitors[0].width = screen->width_in_pixels;
        monitors[0].height = screen->height_in_pixels;
        monitors[0].primary = 1;
        monitors[0].enabled = 1;
        strncpy(monitors[0].name, "default", sizeof(monitors[0].name) - 1);
        
        return 1;  // We have one monitor
    }
    
    // Get the primary output
    xcb_randr_get_output_primary_cookie_t primary_cookie = 
        xcb_randr_get_output_primary(connection, screen->root);
    xcb_randr_get_output_primary_reply_t *primary_reply = 
        xcb_randr_get_output_primary_reply(connection, primary_cookie, NULL);
    
    xcb_randr_output_t primary_output = primary_reply ? primary_reply->output : 0;
    free(primary_reply);
    
    // Get outputs
    xcb_randr_output_t *outputs = xcb_randr_get_screen_resources_current_outputs(res_reply);
    int outputs_len = xcb_randr_get_screen_resources_current_outputs_length(res_reply);
    
    for (int i = 0; i < outputs_len && count < MAX_MONITORS; i++) {
        xcb_randr_get_output_info_cookie_t output_cookie = 
            xcb_randr_get_output_info(connection, outputs[i], XCB_CURRENT_TIME);
        xcb_randr_get_output_info_reply_t *output_reply = 
            xcb_randr_get_output_info_reply(connection, output_cookie, NULL);
        
        if (!output_reply || output_reply->connection != XCB_RANDR_CONNECTION_CONNECTED) {
            free(output_reply);
            continue;
        }
        
        xcb_randr_get_crtc_info_cookie_t crtc_cookie = 
            xcb_randr_get_crtc_info(connection, output_reply->crtc, XCB_CURRENT_TIME);
        xcb_randr_get_crtc_info_reply_t *crtc_reply = 
            xcb_randr_get_crtc_info_reply(connection, crtc_cookie, NULL);
        
        if (!crtc_reply) {
            free(output_reply);
            continue;
        }
        
        // Get output name
        const char *name = (const char *)xcb_randr_get_output_info_name(output_reply);
        int name_len = xcb_randr_get_output_info_name_length(output_reply);
        
        if (name_len > 0) {
            // Copy monitor information
            monitors[count].x = crtc_reply->x;
            monitors[count].y = crtc_reply->y;
            monitors[count].width = crtc_reply->width;
            monitors[count].height = crtc_reply->height;
            monitors[count].primary = (outputs[i] == primary_output);
            monitors[count].enabled = 0;  // By default, no monitor is enabled
            
            // Copy monitor name (with size limit)
            int copy_len = (name_len < sizeof(monitors[count].name) - 1) ? name_len : sizeof(monitors[count].name) - 1;
            strncpy(monitors[count].name, name, copy_len);
            monitors[count].name[copy_len] = '\0';  // Ensure null termination
            
            printf("Found monitor: %s (%dx%d at %d,%d) %s\n", 
                monitors[count].name, 
                monitors[count].width, 
                monitors[count].height,
                monitors[count].x, 
                monitors[count].y,
                monitors[count].primary ? "[Primary]" : "");
                
            count++;
        }
        
        free(crtc_reply);
        free(output_reply);
    }
    
    free(res_reply);
    
    // If no monitors were found, use the whole screen as one monitor
    if (count == 0) {
        printf("No monitors found, using whole screen\n");
        
        monitors[0].x = 0;
        monitors[0].y = 0;
        monitors[0].width = screen->width_in_pixels;
        monitors[0].height = screen->height_in_pixels;
        monitors[0].primary = 1;
        monitors[0].enabled = 1;
        strncpy(monitors[0].name, "default", sizeof(monitors[0].name) - 1);
        
        return 1;  // We have one monitor
    }
    
    return count;
}

/* Enable a monitor by its name */
void enable_monitor_by_name(const char *name) {
    for (int i = 0; i < monitor_count; i++) {
        if (strcmp(monitors[i].name, name) == 0) {
            monitors[i].enabled = 1;
            printf("Enabled monitor: %s\n", name);
            return;
        }
    }
    printf("Warning: Monitor '%s' not found\n", name);
}

/* Print list of available monitors */
void print_monitor_list() {
    printf("Available monitors:\n");
    for (int i = 0; i < monitor_count; i++) {
        printf("  %s (%dx%d at %d,%d) %s\n", 
            monitors[i].name, 
            monitors[i].width, 
            monitors[i].height,
            monitors[i].x, 
            monitors[i].y,
            monitors[i].primary ? "[Primary]" : "");
    }
}

void
server_create_windows(xcb_connection_t *connection, xcb_screen_t *screen)
{
  int i;
  uint32_t values[] = {1, XCB_EVENT_MASK_BUTTON_PRESS | 
    XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW};
 
  // Total windows is 8 per enabled monitor
  int total_windows = 8 * monitor_count;
  
  for (i = 0; i < total_windows; i++) {
    unless (window_options[i].enabled) continue;
    window_options[i].xcb_window = xcb_generate_id (connection);
        
    /* InputOnly window to get the focus when no other window can get it */
    printf("Created a window - x:%d y:%d width:%d height:%d (monitor: %s)\n",
      window_options[i].x, window_options[i].y, window_options[i].w, window_options[i].h,
      window_options[i].monitor_name);
      
    xcb_create_window (connection, 0, window_options[i].xcb_window, screen->root, window_options[i].x,
          window_options[i].y, window_options[i].w, window_options[i].h, 0,
          XCB_WINDOW_CLASS_INPUT_ONLY, screen->root_visual,
          XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, values);

    /* Map the window on the screen */
    xcb_map_window (connection, window_options[i].xcb_window);

    xcb_flush (connection);
  }
}

int
server_find_window(xcb_window_t win)
{
  int total_windows = 8 * monitor_count;
  for (int i = 0; i < total_windows; i++) {
    if (window_options[i].xcb_window == win) return i;
  }
  return -1;
}

void
server_event_loop (xcb_connection_t *connection)
{
  char done = 0;
  int cur_win;
  xcb_generic_event_t *event;
  xcb_button_press_event_t *bp;
  xcb_enter_notify_event_t *enter;
  xcb_leave_notify_event_t *leave;
  
  while ((event = xcb_wait_for_event (connection))) {
    switch (event->response_type & ~0x80) {
      case XCB_BUTTON_PRESS:
        bp = (xcb_button_press_event_t *)event;
        cur_win = server_find_window(bp->event);
        /* printf("this event is coming from window %d \n", cur_win); */
        switch (bp->detail) {
          case LEFT_BUTTON:
            if (str_defined(get_cmd(cur_win,LeftButton))) 
              system(get_cmd(cur_win,LeftButton));
          break;
          
          case MIDDLE_BUTTON:
            if (str_defined(get_cmd(cur_win,MiddleButton))) 
              system(get_cmd(cur_win,MiddleButton));
            break;
          case RIGHT_BUTTON:
            if (str_defined(get_cmd(cur_win,RightButton))) 
              system(get_cmd(cur_win,RightButton));
          break;
          
          case WHEEL_UP_BUTTON:
            if (str_defined(get_cmd(cur_win,WheelUp))) 
              system(get_cmd(cur_win,WheelUp));
            if ( str_defined(get_cmd(cur_win,WheelUpOnce)) && can_execute(cur_win, 0) )
              system(get_cmd(cur_win,WheelUpOnce));
          break;
          
          case WHEEL_DOWN_BUTTON:
            if (str_defined(get_cmd(cur_win,WheelDown))) 
              system(get_cmd(cur_win,WheelDown));
            if (str_defined(get_cmd(cur_win,WheelDownOnce)) && can_execute(cur_win, 1) )
              system(get_cmd(cur_win,WheelDownOnce));
          break;
        }
      break;
      
      case XCB_ENTER_NOTIFY:
        enter = (xcb_enter_notify_event_t *)event;
        cur_win = server_find_window(enter->event);
        if (str_defined(get_cmd(cur_win,Enter))) 
          system(get_cmd(cur_win,Enter));
      break;
      
      case XCB_LEAVE_NOTIFY:
        leave = (xcb_leave_notify_event_t *)event;
        cur_win = server_find_window(leave->event);
        if (str_defined(get_cmd(cur_win,Leave))) 
          system(get_cmd(cur_win,Leave));
      break;
      
      default:
      break;
    }
    
    free (event);
    if (done) break;
  }
}

int
can_execute (const int corner, int direction)
{
  time_t current_time;
  time_t* last_exec;
  long int diff;
  
  unless (direction) last_exec = &window_options[corner].last_time_up; 
    else last_exec = &window_options[corner].last_time_down;
  
  time(&current_time);
  diff = (long int) current_time - (long int) *last_exec;
  
  if ( !last_exec || ( diff > 2 ) ) {
    *last_exec = current_time;
    return 1;
  }
  
  return 0; 
}

void
init_options (int monitor_index)
{
  if (monitor_index < 0 || monitor_index >= monitor_count) return;
  
  // Get monitor dimensions and position
  int x = monitors[monitor_index].x;
  int y = monitors[monitor_index].y;
  int screen_width = monitors[monitor_index].width;
  int screen_height = monitors[monitor_index].height;
  
  // Calculate the base index for this monitor's windows (8 windows per monitor)
  int base_idx = monitor_index * 8;
  
  // Initialize all 8 windows for this monitor
  for (int i = 0; i < 8; i++) {
    int idx = base_idx + i;
    window_options[idx].enabled = monitors[monitor_index].enabled;
    window_options[idx].last_time_down = (time_t) 0;
    window_options[idx].last_time_up = (time_t) 0;
    strncpy(window_options[idx].monitor_name, monitors[monitor_index].name, sizeof(window_options[idx].monitor_name) - 1);
  }
  
  // Set window sizes relative to screen
  window_options[base_idx + TopLeft].w = 5;
  window_options[base_idx + TopLeft].h = 5;
  window_options[base_idx + TopCenter].w = screen_width*0.6;
  window_options[base_idx + TopCenter].h = 2;
  window_options[base_idx + TopRight].w = 6;
  window_options[base_idx + TopRight].h = 5;
  window_options[base_idx + Right].w = 3;
  window_options[base_idx + Right].h = screen_height*0.6;
  window_options[base_idx + BottomRight].w = 6;
  window_options[base_idx + BottomRight].h = 6;
  window_options[base_idx + BottomCenter].w = screen_width*0.6;
  window_options[base_idx + BottomCenter].h = 2;
  window_options[base_idx + BottomLeft].w = 5;
  window_options[base_idx + BottomLeft].h = 5;
  window_options[base_idx + Left].w = 2;
  window_options[base_idx + Left].h = screen_height*0.6;
  
  // Position the windows relative to the monitor offset
  window_options[base_idx + TopLeft].x = x;
  window_options[base_idx + TopLeft].y = y;
  window_options[base_idx + TopCenter].x = x + (screen_width - window_options[base_idx + TopCenter].w)/2;
  window_options[base_idx + TopCenter].y = y;
  window_options[base_idx + TopRight].x = x + screen_width - 5;
  window_options[base_idx + TopRight].y = y;
  window_options[base_idx + Right].x = x + screen_width - 2;
  window_options[base_idx + Right].y = y + (screen_height - window_options[base_idx + Right].h)/2;
  window_options[base_idx + BottomRight].x = x + screen_width - 5;
  window_options[base_idx + BottomRight].y = y + screen_height - 5;
  window_options[base_idx + BottomCenter].x = x + (screen_width - window_options[base_idx + BottomCenter].w)/2;
  window_options[base_idx + BottomCenter].y = y + screen_height - 2;
  window_options[base_idx + BottomLeft].x = x;
  window_options[base_idx + BottomLeft].y = y + screen_height - 5;
  window_options[base_idx + Left].x = x;
  window_options[base_idx + Left].y = y + (screen_height - window_options[base_idx + Left].h)/2;
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
  
  unless (g_key_file_load_from_file(config_file, file_path, G_KEY_FILE_NONE, NULL)) return;
  
  // Process "Default" section first - this will apply to all enabled monitors
  for (i = 0; i < 8; i++) {
    group_name = g_strdup(sections[i]);
    
    if (g_key_file_has_group(config_file, group_name)) {
      // Apply default settings to all enabled monitors
      for (m = 0; m < monitor_count; m++) {
        if (!monitors[m].enabled) continue;
        
        int base_idx = m * 8;
        window_options[base_idx + i].enabled = 1;
        
        for (j = 0; j < 9; j++) {
          if ((current_value = g_key_file_get_value(config_file, group_name, events[j], NULL))) {
            unless (g_str_has_suffix(current_value, "&"))
              current_value = g_strdup_printf("%s &", current_value);
              
            strcpy(window_options[base_idx + i].commands[j], current_value);
            printf("Default %s %s : %s (monitor: %s)\n", 
                  sections[i], events[j], current_value, monitors[m].name);
          }
        }
      }
    }
    
    g_free(group_name);
  }
  
  // Now process monitor-specific sections (overrides defaults)
  for (m = 0; m < monitor_count; m++) {
    if (!monitors[m].enabled) continue;
    
    for (i = 0; i < 8; i++) {
      // Create group name in format "MonitorName-TopLeft", etc.
      group_name = g_strdup_printf("%s-%s", monitors[m].name, sections[i]);
      
      if (g_key_file_has_group(config_file, group_name)) {
        int base_idx = m * 8;
        window_options[base_idx + i].enabled = 1;
        
        for (j = 0; j < 9; j++) {
          if ((current_value = g_key_file_get_value(config_file, group_name, events[j], NULL))) {
            unless (g_str_has_suffix(current_value, "&"))
              current_value = g_strdup_printf("%s &", current_value);
              
            strcpy(window_options[base_idx + i].commands[j], current_value);
            printf("%s %s : %s (monitor: %s)\n", 
                  sections[i], events[j], current_value, monitors[m].name);
          }
        }
      }
      
      g_free(group_name);
    }
  }
  
  g_key_file_free(config_file);
}

void 
fill_file(const char *file_path)
{
  FILE *fp;
  int  i;
   
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

  for (i = 0; i < 15; i++) fputs(lines[i], fp);

  fclose (fp);
}

void 
config_read ()
{
  char *path;
  
  /* check fittsmonrc in user directory */
  path = g_build_filename (g_get_user_config_dir(), "fittsmon", "fittsmonrc", NULL);
  if (g_file_test (path, G_FILE_TEST_EXISTS)) {
    config_read_file(path);
    g_free(path);
    return;
  }

  char *dir = g_build_filename (g_get_user_config_dir(), "fittsmon", NULL);
  if (!g_file_test (dir, G_FILE_TEST_IS_DIR)) g_mkdir(dir, 0777);
  g_free(dir);

  path = g_build_filename (g_get_user_config_dir(), "fittsmon", "fittsmonrc", NULL);
  fill_file(path);
  printf("Created a sample fittsmonrc for you in %s \n", path);

  config_read_file (path);
  g_free(path);
}

void print_usage() {
    const char* green_color = "\x1b[32m";
    const char* reset_color = "\x1b[0m";

    // Start green color
    printf("%s", green_color);

    printf("\nUsage: fittsmon [monitor1] [monitor2] ...\n");
    printf("Example: fittsmon DP-0 HDMI-0\n\n");

    printf("- If no monitors are given, only the primary is used.\n");
    printf("- If using multiple, list all of them.\n");
    printf("- Monitor order doesn't matter.\n\n");

    printf("Config: Edit ~/.config/fittsmon/fittsmonrc to add actions.\n");
    printf("Example section:\n\n");
    printf("  [DP-0-TopCenter]\n");
    printf("  LeftButton=notify-send \"Clicked\"\n");
    printf("  WheelUp=amixer -D pulse set Master 5%%+\n");
    printf("  Enter=\n");
    printf("  Leave=\n\n");

    printf("Use 'fittsmon --list' to show monitor names.\n");
    printf("Use 'fittsmon --help' to show this message.\n\n");

    // Reset color back to default
    printf("%s", reset_color);
}



int
main(int argc, char* argv[])
{
  xcb_connection_t *connection;
  xcb_screen_t *screen;
  int i;
  
  /* open connection to X server. */
  connection = xcb_connect(NULL, NULL);
  
  if (xcb_connection_has_error(connection)) {
    fprintf(stderr, "Failed to connect to X server\n");
    return 1;
  }
  
  /* get screen */
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
  
  /* Process command line arguments */
  if (argc > 1) {
    /* Check for help or list command */
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      print_usage();
      /* print_monitor_list();
      xcb_disconnect(connection); */
      return 0;
    }
    
    /* Get all monitors */
  monitor_count = get_all_monitors(connection, screen);
  
    if (strcmp(argv[1], "--list") == 0 || strcmp(argv[1], "-l") == 0) {
      /* print_monitor_list();*/
      xcb_disconnect(connection);
      return 0;
    }
    
    /* Enable specified monitors */
    for (i = 1; i < argc; i++) {
      enable_monitor_by_name(argv[i]);
    }
  } else {
    /* No arguments, enable primary monitor or first monitor */
    int primary_found = 0;
    for (i = 0; i < monitor_count; i++) {
      if (monitors[i].primary) {
        monitors[i].enabled = 1;
        primary_found = 1;
        printf("No monitor specified, using primary monitor: %s\n", monitors[i].name);
        break;
      }
    }
    
    if (!primary_found && monitor_count > 0) {
      monitors[0].enabled = 1;
      printf("No primary monitor found, using first monitor: %s\n", monitors[0].name);
    }
  }
  
  /* Initialize each enabled monitor */
  for (i = 0; i < monitor_count; i++) {
    if (monitors[i].enabled) {
      init_options(i);
    }
  }
  
  /* Read configuration */
  config_read();
  
  /* Create windows for all enabled monitors */
  server_create_windows(connection, screen);
  
  /* Event loop */
  server_event_loop(connection);
  
  /* Close connection to server */
  xcb_disconnect(connection);
  
  return 0;
}
