#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;

static void update_time() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    
    // Create a long-lived buffer
    static char buffer[] = "00:00";
    
    char *format = "";
    if(clock_is_24h_style() == true) {
        format = "%H:%M";
    } else {
        format = "%I:%M";
    }
    strftime(buffer, sizeof("00:00"), format, tick_time);
    
    text_layer_set_text(s_time_layer, buffer);
}

static void main_window_load(Window *window) {
    // Create time TextLayer
    s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    
    // Improve the layout to be more like a watchface
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));  
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void init() {
    s_main_window = window_create();
    
    // set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    
    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    
    // Show the window on the watch, with animated=true
    window_stack_push(s_main_window, true);
    // Make sure the time is displayed from the start
    update_time();
}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    
    // letts the watchapp wait for system events until it exits
    app_event_loop();
    
    deinit();
}

