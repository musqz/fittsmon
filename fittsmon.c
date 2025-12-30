[object Object],


,[object Object],

,[object Object],
,[object Object],
,[object Object],

,[object Object],
,[object Object],
,[object Object],
,[object Object],
,[object Object],


,[object Object],

,[object Object],
,[object Object],
,[object Object],

,[object Object],

,[object Object],
,[object Object],
  TopLeft, TopCenter, TopRight, Right, BottomRight, BottomCenter, BottomLeft, Left
};

,[object Object],
,[object Object],
  LeftButton, MiddleButton, RightButton, WheelUp, WheelDown, WheelUpOnce, WheelDownOnce, Enter, Leave
};

,[object Object],
,[object Object],
  LEFT_BUTTON=,[object Object],, MIDDLE_BUTTON, RIGHT_BUTTON, WHEEL_UP_BUTTON, WHEEL_DOWN_BUTTON
};

,[object Object],
,[object Object],

,[object Object],
,[object Object],
  ,[object Object], enabled;
  ,[object Object], x;
  ,[object Object], y;
  ,[object Object], h;
  ,[object Object], w;
  ,[object Object], commands[,[object Object],][,[object Object],];
  ,[object Object], xcb_window; ,[object Object],
  ,[object Object], last_time_up; ,[object Object],
  ,[object Object], last_time_down; ,[object Object],
  ,[object Object], monitor_name[,[object Object],]; ,[object Object],
};

,[object Object],
,[object Object], ,[object Object],
    ,[object Object], x;
    ,[object Object], y;
    ,[object Object], width;
    ,[object Object], height;
    ,[object Object], primary;
    ,[object Object], name[,[object Object],];
    ,[object Object], enabled;  ,[object Object],
} MonitorInfo;

,[object Object],
,[object Object],  ,[object Object],
MonitorInfo monitors[MAX_MONITORS];
,[object Object], monitor_count = ,[object Object],;

,[object Object],
,[object Object],  ,[object Object], ,[object Object],;
,[object Object],  ,[object Object], ,[object Object],;
,[object Object], ,[object Object], ,[object Object],;
,[object Object], ,[object Object], ,[object Object],;
,[object Object], ,[object Object],[object Object],;
,[object Object], ,[object Object], ,[object Object],;
,[object Object], ,[object Object],[object Object],;
,[object Object],  ,[object Object],[object Object],;
,[object Object], ,[object Object], ,[object Object],;
,[object Object],  ,[object Object],[object Object],;
,[object Object],  ,[object Object],[object Object],;
,[object Object], ,[object Object],[object Object],;
,[object Object],  ,[object Object],[object Object],;
,[object Object], ,[object Object],[object Object],;

,[object Object],

,[object Object],
,[object Object], ,[object Object],[object Object], {
    ,[object Object], (!s1 || !s2) ,[object Object], ,[object Object],;
    
    ,[object Object], (*s1 && *s2) {
        ,[object Object], (,[object Object],((,[object Object], ,[object Object],)*s1) != ,[object Object],((,[object Object], ,[object Object],)*s2)) {
            ,[object Object], ,[object Object],;
        }
        s1++;
        s2++;
    }
    ,[object Object], (*s1 || *s2) ? ,[object Object], : ,[object Object],;
}

,[object Object],
,[object Object], ,[object Object],[object Object], {
    ,[object Object], count = ,[object Object],;
    
    ,[object Object],
    ,[object Object], ,[object Object], *extension_reply = xcb_get_extension_data(connection, &xcb_randr_id);
    ,[object Object], (!extension_reply || !extension_reply->present) {
        ,[object Object],(,[object Object],);
        
        ,[object Object],
        monitors[,[object Object],].x = ,[object Object],;
        monitors[,[object Object],].y = ,[object Object],;
        monitors[,[object Object],].width = screen->width_in_pixels;
        monitors[,[object Object],].height = screen->height_in_pixels;
        monitors[,[object Object],].primary = ,[object Object],;
        monitors[,[object Object],].enabled = ,[object Object],;
        ,[object Object],(monitors[,[object Object],].name, ,[object Object],, ,[object Object],(monitors[,[object Object],].name) - ,[object Object],);
        
        ,[object Object], ,[object Object],;  ,[object Object],
    }
    
    ,[object Object],
    ,[object Object], version_cookie = xcb_randr_query_version(connection, ,[object Object],, ,[object Object],);
    ,[object Object], *version_reply = xcb_randr_query_version_reply(connection, version_cookie, ,[object Object],);
    ,[object Object], (!version_reply) {
        ,[object Object],(,[object Object],);
        
        ,[object Object],
        monitors[,[object Object],].x = ,[object Object],;
        monitors[,[object Object],].y = ,[object Object],;
        monitors[,[object Object],].width = screen->width_in_pixels;
        monitors[,[object Object],].height = screen->height_in_pixels;
        monitors[,[object Object],].primary = ,[object Object],;
        monitors[,[object Object],].enabled = ,[object Object],;
        ,[object Object],(monitors[,[object Object],].name, ,[object Object],, ,[object Object],(monitors[,[object Object],].name) - ,[object Object],);
        
        ,[object Object],(version_reply);
        ,[object Object], ,[object Object],;
    }
    ,[object Object],(version_reply);
    
    ,[object Object],
    ,[object Object], res_cookie = 
        xcb_randr_get_screen_resources_current(connection, screen->root);
    ,[object Object], *res_reply = 
        xcb_randr_get_screen_resources_current_reply(connection, res_cookie, ,[object Object],);
    
    ,[object Object], (!res_reply) {
        ,[object Object],(,[object Object],);
        
        ,[object Object],
        monitors[,[object Object],].x = ,[object Object],;
        monitors[,[object Object],].y = ,[object Object],;
        monitors[,[object Object],].width = screen->width_in_pixels;
        monitors[,[object Object],].height = screen->height_in_pixels;
        monitors[,[object Object],].primary = ,[object Object],;
        monitors[,[object Object],].enabled = ,[object Object],;
        ,[object Object],(monitors[,[object Object],].name, ,[object Object],, ,[object Object],(monitors[,[object Object],].name) - ,[object Object],);
        
        ,[object Object], ,[object Object],;  ,[object Object],
    }
    
    ,[object Object],
    ,[object Object], primary_cookie = 
        xcb_randr_get_output_primary(connection, screen->root);
    ,[object Object], *primary_reply = 
        xcb_randr_get_output_primary_reply(connection, primary_cookie, ,[object Object],);
    
    ,[object Object], primary_output = primary_reply ? primary_reply->output : ,[object Object],;
    ,[object Object],(primary_reply);
    
    ,[object Object],
    ,[object Object], *outputs = xcb_randr_get_screen_resources_current_outputs(res_reply);
    ,[object Object], outputs_len = xcb_randr_get_screen_resources_current_outputs_length(res_reply);
    
    ,[object Object], (,[object Object], i = ,[object Object],; i < outputs_len && count < MAX_MONITORS; i++) {
        ,[object Object], output_cookie = 
            xcb_randr_get_output_info(connection, outputs[i], XCB_CURRENT_TIME);
        ,[object Object], *output_reply = 
            xcb_randr_get_output_info_reply(connection, output_cookie, ,[object Object],);
        
        ,[object Object], (!output_reply || output_reply->connection != XCB_RANDR_CONNECTION_CONNECTED) {
            ,[object Object],(output_reply);
            ,[object Object],;
        }
        
        ,[object Object], crtc_cookie = 
            xcb_randr_get_crtc_info(connection, output_reply->crtc, XCB_CURRENT_TIME);
        ,[object Object], *crtc_reply = 
            xcb_randr_get_crtc_info_reply(connection, crtc_cookie, ,[object Object],);
        
        ,[object Object], (!crtc_reply) {
            ,[object Object],(output_reply);
            ,[object Object],;
        }
        
        ,[object Object],
        ,[object Object], (crtc_reply->width == ,[object Object], || crtc_reply->height == ,[object Object],) {
            ,[object Object],(crtc_reply);
            ,[object Object],(output_reply);
            ,[object Object],;
        }
        
        ,[object Object],
        ,[object Object], ,[object Object], *name = (,[object Object], ,[object Object], *)xcb_randr_get_output_info_name(output_reply);
        ,[object Object], name_len = xcb_randr_get_output_info_name_length(output_reply);
        
        ,[object Object], (name_len > ,[object Object],) {
            ,[object Object],
            monitors[count].x = crtc_reply->x;
            monitors[count].y = crtc_reply->y;
            monitors[count].width = crtc_reply->width;
            monitors[count].height = crtc_reply->height;
            monitors[count].primary = (outputs[i] == primary_output);
            monitors[count].enabled = ,[object Object],;  ,[object Object],
            
            ,[object Object],
            ,[object Object], max_len = ,[object Object],(monitors[count].name) - ,[object Object],;
            ,[object Object], copy_len = (name_len < (,[object Object],)max_len) ? (,[object Object],)name_len : max_len;
            ,[object Object],(monitors[count].name, name, copy_len);
            monitors[count].name[copy_len] = ,[object Object],;  ,[object Object],
            
            ,[object Object],(,[object Object],, 
                monitors[count].name, 
                monitors[count].width, 
                monitors[count].height,
                monitors[count].x, 
                monitors[count].y,
                monitors[count].primary ? ,[object Object], : ,[object Object],);
                
            count++;
        }
        
        ,[object Object],(crtc_reply);
        ,[object Object],(output_reply);
    }
    
    ,[object Object],(res_reply);
    
    ,[object Object],
    ,[object Object], (count == ,[object Object],) {
        ,[object Object],(,[object Object],);
        
        monitors[,[object Object],].x = ,[object Object],;
        monitors[,[object Object],].y = ,[object Object],;
        monitors[,[object Object],].width = screen->width_in_pixels;
        monitors[,[object Object],].height = screen->height_in_pixels;
        monitors[,[object Object],].primary = ,[object Object],;
        monitors[,[object Object],].enabled = ,[object Object],;
        ,[object Object],(monitors[,[object Object],].name, ,[object Object],, ,[object Object],(monitors[,[object Object],].name) - ,[object Object],);
        
        ,[object Object], ,[object Object],;  ,[object Object],
    }
    
    ,[object Object], count;
}

,[object Object],
,[object Object], ,[object Object],[object Object], {
    ,[object Object], (!name || ,[object Object],(name) == ,[object Object],) {
        ,[object Object],(,[object Object],, ,[object Object],);
        ,[object Object], ,[object Object],;
    }
    
    ,[object Object], found = ,[object Object],;
    ,[object Object], (,[object Object], i = ,[object Object],; i < monitor_count; i++) {
        ,[object Object],
        ,[object Object], (strcasecmp_custom(monitors[i].name, name) == ,[object Object],) {
            monitors[i].enabled = ,[object Object],;
            found = ,[object Object],;
            ,[object Object],(,[object Object],, 
                name, monitors[i].width, monitors[i].height,
                monitors[i].x, monitors[i].y);
            ,[object Object],;  ,[object Object],
        }
    }
    
    ,[object Object], (!found) {
        ,[object Object],(,[object Object],, ,[object Object],, name);
        ,[object Object], ,[object Object],;
    }
    ,[object Object], ,[object Object],;
}

,[object Object],
,[object Object], ,[object Object],[object Object], {
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    
    ,[object Object], (monitor_count == ,[object Object],) {
        ,[object Object],(,[object Object],);
        ,[object Object],;
    }
    
    ,[object Object], (,[object Object], i = ,[object Object],; i < monitor_count; i++) {
        ,[object Object],(,[object Object],, 
            monitors[i].name, 
            monitors[i].width, 
            monitors[i].height,
            monitors[i].x, 
            monitors[i].y,
            monitors[i].primary ? ,[object Object], : ,[object Object],);
    }
    ,[object Object],(,[object Object],);
}

,[object Object],
,[object Object],[object Object],
{
  ,[object Object], i;
  ,[object Object], values[] = {,[object Object],, XCB_EVENT_MASK_BUTTON_PRESS | 
    XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW};
 
  ,[object Object],
  ,[object Object], total_windows = ,[object Object], * monitor_count;
  
  ,[object Object],(,[object Object],);
  
  ,[object Object], (i = ,[object Object],; i < total_windows; i++) {
    unless (window_options[i].enabled) ,[object Object],;
    window_options[i].xcb_window = xcb_generate_id (connection);
        
    ,[object Object],
    ,[object Object],(,[object Object],,
      i, 
      (i%,[object Object], == ,[object Object],) ? ,[object Object], : (i%,[object Object], == ,[object Object],) ? ,[object Object], : (i%,[object Object], == ,[object Object],) ? ,[object Object], : 
      (i%,[object Object], == ,[object Object],) ? ,[object Object], : (i%,[object Object], == ,[object Object],) ? ,[object Object], : (i%,[object Object], == ,[object Object],) ? ,[object Object], : 
      (i%,[object Object], == ,[object Object],) ? ,[object Object], : ,[object Object],,
      window_options[i].x, window_options[i].y, window_options[i].w, window_options[i].h,
      window_options[i].monitor_name);
      
    xcb_create_window (connection, ,[object Object],, window_options[i].xcb_window, screen->root, window_options[i].x,
          window_options[i].y, window_options[i].w, window_options[i].h, ,[object Object],,
          XCB_WINDOW_CLASS_INPUT_ONLY, screen->root_visual,
          XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, values);

    ,[object Object],
    xcb_map_window (connection, window_options[i].xcb_window);

    xcb_flush (connection);
  }
  
  ,[object Object],(,[object Object],);
}

,[object Object],
,[object Object],[object Object],
{
  ,[object Object], total_windows = ,[object Object], * monitor_count;
  ,[object Object], (,[object Object], i = ,[object Object],; i < total_windows; i++) {
    ,[object Object], (window_options[i].xcb_window == win) ,[object Object], i;
  }
  ,[object Object], ,[object Object],;
}

,[object Object],
,[object Object], ,[object Object],
{
  ,[object Object], done = ,[object Object],;
  ,[object Object], cur_win;
  ,[object Object], *event;
  ,[object Object], *bp;
  ,[object Object], *enter;
  ,[object Object], *leave;
  
  ,[object Object], ((event = xcb_wait_for_event (connection))) {
    ,[object Object], (event->response_type & ~,[object Object],) {
      ,[object Object], XCB_BUTTON_PRESS:
        bp = (,[object Object], *)event;
        cur_win = server_find_window(bp->event);
        ,[object Object], (cur_win < ,[object Object],) {
          ,[object Object],(,[object Object],, ,[object Object],);
          ,[object Object],;
        }
        ,[object Object], (bp->detail) {
          ,[object Object], LEFT_BUTTON:
            ,[object Object], (str_defined(get_cmd(cur_win,LeftButton))) {
              ,[object Object], ,[object Object], *cmd = get_cmd(cur_win,LeftButton);
              ,[object Object], (is_safe_command(cmd)) {
                ,[object Object], ret = system(cmd);
                ,[object Object], (ret != ,[object Object],) ,[object Object],(,[object Object],, ,[object Object],, ret);
              }
            }
          ,[object Object],;
          
          ,[object Object], MIDDLE_BUTTON:
            ,[object Object], (str_defined(get_cmd(cur_win,MiddleButton))) {
              ,[object Object], ,[object Object], *cmd = get_cmd(cur_win,MiddleButton);
              ,[object Object], (is_safe_command(cmd)) {
                ,[object Object], ret = system(cmd);
                ,[object Object], (ret != ,[object Object],) ,[object Object],(,[object Object],, ,[object Object],, ret);
              }
            }
            ,[object Object],;
          ,[object Object], RIGHT_BUTTON:
            ,[object Object], (str_defined(get_cmd(cur_win,RightButton))) {
              ,[object Object], ,[object Object], *cmd = get_cmd(cur_win,RightButton);
              ,[object Object], (is_safe_command(cmd)) {
                ,[object Object], ret = system(cmd);
                ,[object Object], (ret != ,[object Object],) ,[object Object],(,[object Object],, ,[object Object],, ret);
              }
            }
          ,[object Object],;
          
          ,[object Object], WHEEL_UP_BUTTON:
            ,[object Object], (str_defined(get_cmd(cur_win,WheelUp))) {
              ,[object Object], ,[object Object], *cmd = get_cmd(cur_win,WheelUp);
              ,[object Object], (is_safe_command(cmd)) {
                ,[object Object], ret = system(cmd);
                ,[object Object], (ret != ,[object Object],) ,[object Object],(,[object Object],, ,[object Object],, ret);
              }
            }
            ,[object Object], ( str_defined(get_cmd(cur_win,WheelUpOnce)) && can_execute(cur_win, ,[object Object],) ) {
              ,[object Object], ,[object Object], *cmd = get_cmd(cur_win,WheelUpOnce);
              ,[object Object], (is_safe_command(cmd)) {
                ,[object Object], ret = system(cmd);
                ,[object Object], (ret != ,[object Object],) ,[object Object],(,[object Object],, ,[object Object],, ret);
              }
            }
          ,[object Object],;
          
          ,[object Object], WHEEL_DOWN_BUTTON:
            ,[object Object], (str_defined(get_cmd(cur_win,WheelDown))) {
              ,[object Object], ,[object Object], *cmd = get_cmd(cur_win,WheelDown);
              ,[object Object], (is_safe_command(cmd)) {
                ,[object Object], ret = system(cmd);
                ,[object Object], (ret != ,[object Object],) ,[object Object],(,[object Object],, ,[object Object],, ret);
              }
            }
            ,[object Object], (str_defined(get_cmd(cur_win,WheelDownOnce)) && can_execute(cur_win, ,[object Object],) ) {
              ,[object Object], ,[object Object], *cmd = get_cmd(cur_win,WheelDownOnce);
              ,[object Object], (is_safe_command(cmd)) {
                ,[object Object], ret = system(cmd);
                ,[object Object], (ret != ,[object Object],) ,[object Object],(,[object Object],, ,[object Object],, ret);
              }
            }
          ,[object Object],;
        }
      ,[object Object],;
      
      ,[object Object], XCB_ENTER_NOTIFY:
        enter = (,[object Object], *)event;
        cur_win = server_find_window(enter->event);
        ,[object Object], (cur_win >= ,[object Object], && str_defined(get_cmd(cur_win,Enter))) {
          ,[object Object], ,[object Object], *cmd = get_cmd(cur_win,Enter);
          ,[object Object], (is_safe_command(cmd)) {
            ,[object Object], ret = system(cmd);
            ,[object Object], (ret != ,[object Object],) ,[object Object],(,[object Object],, ,[object Object],, ret);
          }
        }
      ,[object Object],;
      
      ,[object Object], XCB_LEAVE_NOTIFY:
        leave = (,[object Object], *)event;
        cur_win = server_find_window(leave->event);
        ,[object Object], (cur_win >= ,[object Object], && str_defined(get_cmd(cur_win,Leave))) {
          ,[object Object], ,[object Object], *cmd = get_cmd(cur_win,Leave);
          ,[object Object], (is_safe_command(cmd)) {
            ,[object Object], ret = system(cmd);
            ,[object Object], (ret != ,[object Object],) ,[object Object],(,[object Object],, ,[object Object],, ret);
          }
        }
      ,[object Object],;
      
      ,[object Object],:
      ,[object Object],;
    }
    
    ,[object Object], (event);
    ,[object Object], (done) ,[object Object],;
  }
}

,[object Object],
,[object Object], ,[object Object],
{
  ,[object Object], current_time;
  ,[object Object],* last_exec;
  ,[object Object], ,[object Object], diff;
  
  unless (direction) last_exec = &window_options[corner].last_time_up; 
    ,[object Object], last_exec = &window_options[corner].last_time_down;
  
  time(&current_time);
  diff = (,[object Object], ,[object Object],) current_time - (,[object Object], ,[object Object],) *last_exec;
  
  ,[object Object], ( diff > ,[object Object], ) {
    *last_exec = current_time;
    ,[object Object], ,[object Object],;
  }
  
  ,[object Object], ,[object Object],; 
}

,[object Object],
,[object Object],
,[object Object], ,[object Object],
{
  ,[object Object], (!cmd || ,[object Object],(cmd) == ,[object Object],) ,[object Object], ,[object Object],;

  ,[object Object],
  ,[object Object], buf[,[object Object],];
  ,[object Object], len = ,[object Object],(cmd);
  ,[object Object], (len >= ,[object Object],(buf)) {
    ,[object Object],(,[object Object],, ,[object Object],, ,[object Object],(buf) - ,[object Object],, cmd);
    ,[object Object], ,[object Object],; ,[object Object],
  }
  ,[object Object],(buf, cmd);

  ,[object Object],
  ,[object Object], (len > ,[object Object], && (buf[len,[object Object],] == ,[object Object], || buf[len,[object Object],] == ,[object Object],)) {
    buf[--len] = ,[object Object],;
  }

  ,[object Object],
  ,[object Object], (len > ,[object Object], && buf[len,[object Object],] == ,[object Object],) {
    buf[--len] = ,[object Object],;
    ,[object Object], (len > ,[object Object], && (buf[len,[object Object],] == ,[object Object], || buf[len,[object Object],] == ,[object Object],)) {
      buf[--len] = ,[object Object],;
    }
  }

  ,[object Object],
  ,[object Object], ,[object Object], *dangerous = ,[object Object],;
  ,[object Object], safe_len = ,[object Object],(buf, dangerous);

  ,[object Object], (safe_len != ,[object Object],(buf)) {
    ,[object Object],(,[object Object],, ,[object Object],, cmd);
    ,[object Object], ,[object Object],;
  }

  ,[object Object], ,[object Object],;
}

,[object Object],
,[object Object], ,[object Object],
{
  ,[object Object], (monitor_index < ,[object Object], || monitor_index >= monitor_count) ,[object Object],;
  
  ,[object Object],
  ,[object Object], x = monitors[monitor_index].x;
  ,[object Object], y = monitors[monitor_index].y;
  ,[object Object], screen_width = monitors[monitor_index].width;
  ,[object Object], screen_height = monitors[monitor_index].height;
  
  ,[object Object],
  ,[object Object], base_idx = monitor_index * ,[object Object],;
  
  ,[object Object],
  ,[object Object], (,[object Object], i = ,[object Object],; i < ,[object Object],; i++) {
    ,[object Object], idx = base_idx + i;
    window_options[idx].enabled = monitors[monitor_index].enabled;
    window_options[idx].last_time_down = (,[object Object],) ,[object Object],;
    window_options[idx].last_time_up = (,[object Object],) ,[object Object],;
    ,[object Object],(window_options[idx].monitor_name, monitors[monitor_index].name, ,[object Object],(window_options[idx].monitor_name) - ,[object Object],);
  }
  
  ,[object Object],
  window_options[base_idx + TopLeft].w = ,[object Object],;
  window_options[base_idx + TopLeft].h = ,[object Object],;
  window_options[base_idx + TopCenter].w = screen_width*,[object Object],;
  window_options[base_idx + TopCenter].h = ,[object Object],;
  window_options[base_idx + TopRight].w = ,[object Object],;
  window_options[base_idx + TopRight].h = ,[object Object],;
  window_options[base_idx + Right].w = ,[object Object],;
  window_options[base_idx + Right].h = screen_height*,[object Object],;
  window_options[base_idx + BottomRight].w = ,[object Object],;
  window_options[base_idx + BottomRight].h = ,[object Object],;
  window_options[base_idx + BottomCenter].w = screen_width*,[object Object],;
  window_options[base_idx + BottomCenter].h = ,[object Object],;
  window_options[base_idx + BottomLeft].w = ,[object Object],;
  window_options[base_idx + BottomLeft].h = ,[object Object],;
  window_options[base_idx + Left].w = ,[object Object],;
  window_options[base_idx + Left].h = screen_height*,[object Object],;
  
  ,[object Object],
  window_options[base_idx + TopLeft].x = x;
  window_options[base_idx + TopLeft].y = y;
  window_options[base_idx + TopCenter].x = x + (screen_width - window_options[base_idx + TopCenter].w)/,[object Object],;
  window_options[base_idx + TopCenter].y = y;
  window_options[base_idx + TopRight].x = x + screen_width - ,[object Object],;
  window_options[base_idx + TopRight].y = y;
  window_options[base_idx + Right].x = x + screen_width - ,[object Object],;
  window_options[base_idx + Right].y = y + (screen_height - window_options[base_idx + Right].h)/,[object Object],;
  window_options[base_idx + BottomRight].x = x + screen_width - ,[object Object],;
  window_options[base_idx + BottomRight].y = y + screen_height - ,[object Object],;
  window_options[base_idx + BottomCenter].x = x + (screen_width - window_options[base_idx + BottomCenter].w)/,[object Object],;
  window_options[base_idx + BottomCenter].y = y + screen_height - ,[object Object],;
  window_options[base_idx + BottomLeft].x = x;
  window_options[base_idx + BottomLeft].y = y + screen_height - ,[object Object],;
  window_options[base_idx + Left].x = x;
  window_options[base_idx + Left].y = y + (screen_height - window_options[base_idx + Left].h)/,[object Object],;
}

,[object Object], 
,[object Object], ,[object Object],
{
  GKeyFile* config_file;
  gchar* sections[] = {,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],};
  gchar* events[] = {,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],, ,[object Object],};
  gchar* current_value;
  ,[object Object], i, j, m;
  gchar* group_name;

  config_file = g_key_file_new();
  
  unless (g_key_file_load_from_file(config_file, file_path, G_KEY_FILE_NONE, ,[object Object],)) ,[object Object],;
  
  ,[object Object],(,[object Object],, file_path);
  
  ,[object Object],
  ,[object Object], (i = ,[object Object],; i < ,[object Object],; i++) {
    group_name = g_strdup(sections[i]);
    
    ,[object Object], (g_key_file_has_group(config_file, group_name)) {
      ,[object Object],
      ,[object Object], (m = ,[object Object],; m < monitor_count; m++) {
        ,[object Object], (!monitors[m].enabled) ,[object Object],;
        
        ,[object Object], base_idx = m * ,[object Object],;
        window_options[base_idx + i].enabled = ,[object Object],;
        
        ,[object Object], (j = ,[object Object],; j < ,[object Object],; j++) {
          ,[object Object], ((current_value = g_key_file_get_value(config_file, group_name, events[j], ,[object Object],))) {
            gchar *cmd_to_use = current_value;
            gchar *allocated_cmd = ,[object Object],;
            
            unless (g_str_has_suffix(current_value, ,[object Object],)) {
              allocated_cmd = g_strdup_printf(,[object Object],, current_value);
              cmd_to_use = allocated_cmd;
            }
            
            ,[object Object],(window_options[base_idx + i].commands[j], cmd_to_use, ,[object Object],);
            window_options[base_idx + i].commands[j][,[object Object],] = ,[object Object],;
            ,[object Object],(,[object Object],, 
                  sections[i], events[j], cmd_to_use, monitors[m].name);
            
            g_free(allocated_cmd);
            g_free(current_value);
          }
        }
      }
    }
    
    g_free(group_name);
  }
  
  ,[object Object],
  ,[object Object], (m = ,[object Object],; m < monitor_count; m++) {
    ,[object Object], (!monitors[m].enabled) ,[object Object],;
    
    ,[object Object], (i = ,[object Object],; i < ,[object Object],; i++) {
      ,[object Object],
      group_name = g_strdup_printf(,[object Object],, monitors[m].name, sections[i]);
      
      ,[object Object], (g_key_file_has_group(config_file, group_name)) {
        ,[object Object], base_idx = m * ,[object Object],;
        window_options[base_idx + i].enabled = ,[object Object],;
        
        ,[object Object], (j = ,[object Object],; j < ,[object Object],; j++) {
          ,[object Object], ((current_value = g_key_file_get_value(config_file, group_name, events[j], ,[object Object],))) {
            gchar *cmd_to_use = current_value;
            gchar *allocated_cmd = ,[object Object],;
            
            unless (g_str_has_suffix(current_value, ,[object Object],)) {
              allocated_cmd = g_strdup_printf(,[object Object],, current_value);
              cmd_to_use = allocated_cmd;
            }
            
            ,[object Object],(window_options[base_idx + i].commands[j], cmd_to_use, ,[object Object],);
            window_options[base_idx + i].commands[j][,[object Object],] = ,[object Object],;
            ,[object Object],(,[object Object],, 
                  sections[i], events[j], cmd_to_use, monitors[m].name);
            
            g_free(allocated_cmd);
            g_free(current_value);
          }
        }
      }
      
      g_free(group_name);
    }
  }
  
  g_key_file_free(config_file);
}

,[object Object], 
,[object Object],[object Object],
{
  FILE *fp;
  ,[object Object],  i;
   
  ,[object Object],* lines[] = {
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],,
    ,[object Object],
  };

  fp = fopen(file_path, ,[object Object],);
  ,[object Object], (fp == ,[object Object],) ,[object Object],;

  ,[object Object], (i = ,[object Object],; i < ,[object Object],; i++) ,[object Object],(lines[i], fp);

  fclose (fp);
}

,[object Object], 
,[object Object], ,[object Object],
{
  ,[object Object], *path;
  
  ,[object Object],
  path = g_build_filename (g_get_user_config_dir(), ,[object Object],, ,[object Object],, ,[object Object],);
  ,[object Object], (g_file_test (path, G_FILE_TEST_EXISTS)) {
    config_read_file(path);
    g_free(path);
    ,[object Object],;
  }

  ,[object Object], *dir = g_build_filename (g_get_user_config_dir(), ,[object Object],, ,[object Object],);
  ,[object Object], (!g_file_test (dir, G_FILE_TEST_IS_DIR)) g_mkdir(dir, ,[object Object],);
  g_free(dir);

  path = g_build_filename (g_get_user_config_dir(), ,[object Object],, ,[object Object],, ,[object Object],);
  fill_file(path);
  ,[object Object],(,[object Object],, path);

  config_read_file (path);
  g_free(path);
}

,[object Object], ,[object Object],[object Object], {
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
    ,[object Object],(,[object Object],);
}


,[object Object],
,[object Object],[object Object],
{
  ,[object Object], *connection;
  ,[object Object], *screen;
  ,[object Object], i;
  
  ,[object Object],(,[object Object],);
  ,[object Object],(,[object Object],);
  ,[object Object],(,[object Object],);
  ,[object Object],(,[object Object],);
  ,[object Object],(,[object Object],);
  
  ,[object Object],
  connection = xcb_connect(,[object Object],, ,[object Object],);
  
  ,[object Object], (xcb_connection_has_error(connection)) {
    ,[object Object],(,[object Object],, ,[object Object],);
    ,[object Object], ,[object Object],;
  }
  
  ,[object Object],
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
  
  ,[object Object],
  ,[object Object],(,[object Object],);
  monitor_count = get_all_monitors(connection, screen);
  ,[object Object],(,[object Object],, monitor_count);
  
  ,[object Object],
  ,[object Object], monitors_enabled_count = ,[object Object],;
  
  ,[object Object], (argc > ,[object Object],) {
    ,[object Object],
    ,[object Object], (,[object Object],(argv[,[object Object],], ,[object Object],) == ,[object Object], || ,[object Object],(argv[,[object Object],], ,[object Object],) == ,[object Object],) {
      print_usage();
      print_monitor_list();
      xcb_disconnect(connection);
      ,[object Object], ,[object Object],;
    }
    
    ,[object Object], (,[object Object],(argv[,[object Object],], ,[object Object],) == ,[object Object], || ,[object Object],(argv[,[object Object],], ,[object Object],) == ,[object Object],) {
      print_monitor_list();
      xcb_disconnect(connection);
      ,[object Object], ,[object Object],;
    }
    
    ,[object Object],
    ,[object Object], (,[object Object],(argv[,[object Object],], ,[object Object],) == ,[object Object],) {
      ,[object Object], (argc < ,[object Object],) {
        ,[object Object],(,[object Object],, ,[object Object],);
        print_usage();
        print_monitor_list();
        xcb_disconnect(connection);
        ,[object Object], ,[object Object],;
      }
      
      ,[object Object],(,[object Object],, argc - ,[object Object],);
      
      ,[object Object],
      ,[object Object], (i = ,[object Object],; i < argc; i++) {
        ,[object Object],(,[object Object],, i - ,[object Object],, argc - ,[object Object],, argv[i]);
        ,[object Object], (enable_monitor_by_name(argv[i])) {
          monitors_enabled_count++;
        }
      }
      
      ,[object Object], (monitors_enabled_count == ,[object Object],) {
        ,[object Object],(,[object Object],, ,[object Object],);
        print_monitor_list();
        xcb_disconnect(connection);
        ,[object Object], ,[object Object],;
      }
      
      ,[object Object],(,[object Object],, monitors_enabled_count);
      
    } ,[object Object], {
      ,[object Object],
      ,[object Object],(,[object Object],, argc - ,[object Object],);
      
      ,[object Object], (i = ,[object Object],; i < argc; i++) {
        ,[object Object],(,[object Object],, i, argc - ,[object Object],, argv[i]);
        ,[object Object], (enable_monitor_by_name(argv[i])) {
          monitors_enabled_count++;
        }
      }
      
      ,[object Object], (monitors_enabled_count == ,[object Object],) {
        ,[object Object],(,[object Object],, ,[object Object],);
        print_monitor_list();
        xcb_disconnect(connection);
        ,[object Object], ,[object Object],;
      }
      
      ,[object Object],(,[object Object],, monitors_enabled_count);
    }
  } ,[object Object], {
    ,[object Object],
    ,[object Object],(,[object Object],);
    
    ,[object Object], primary_found = ,[object Object],;
    ,[object Object], (i = ,[object Object],; i < monitor_count; i++) {
      ,[object Object], (monitors[i].primary) {
        monitors[i].enabled = ,[object Object],;
        monitors_enabled_count = ,[object Object],;
        primary_found = ,[object Object],;
        ,[object Object],(,[object Object],, monitors[i].name);
        ,[object Object],;
      }
    }
    
    ,[object Object], (!primary_found && monitor_count > ,[object Object],) {
      monitors[,[object Object],].enabled = ,[object Object],;
      monitors_enabled_count = ,[object Object],;
      ,[object Object],(,[object Object],, monitors[,[object Object],].name);
    }
  }
  
  ,[object Object],
  ,[object Object],(,[object Object],);
  ,[object Object], enabled_check = ,[object Object],;
  ,[object Object], (i = ,[object Object],; i < monitor_count; i++) {
    ,[object Object], (monitors[i].enabled) {
      ,[object Object],(,[object Object],, monitors[i].name);
      enabled_check++;
    } ,[object Object], {
      ,[object Object],(,[object Object],, monitors[i].name);
    }
  }
  ,[object Object],(,[object Object],);
  
  ,[object Object], (enabled_check == ,[object Object],) {
    ,[object Object],(,[object Object],, ,[object Object],);
    xcb_disconnect(connection);
    ,[object Object], ,[object Object],;
  }
  
  ,[object Object],
  ,[object Object],(,[object Object],, enabled_check);
  ,[object Object], (i = ,[object Object],; i < monitor_count; i++) {
    ,[object Object], (monitors[i].enabled) {
      ,[object Object],(,[object Object],, monitors[i].name);
      init_options(i);
    }
  }
  ,[object Object],(,[object Object],);
  
  ,[object Object],
  config_read();
  ,[object Object],(,[object Object],);
  
  ,[object Object],
  server_create_windows(connection, screen);
  ,[object Object],(,[object Object],);
  
  ,[object Object],(,[object Object],);
  ,[object Object],(,[object Object],);
  ,[object Object],(,[object Object],);
  ,[object Object],(,[object Object],);
  ,[object Object],(,[object Object],);
  ,[object Object],(,[object Object],);
  
  ,[object Object],
  server_event_loop(connection);
  
  ,[object Object],
  xcb_disconnect(connection);
  
  ,[object Object], ,[object Object],;
}
