#include <pebble.h>

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;

static GFont s_time_font;
static GFont s_weather_font;

static BitmapLayer  *s_background_layer;

static GBitmap *s_background_bitmap;

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
    // Ceate GBitmap, then set to created BitmapLayer
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
    s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
    bitmap_layer_set_bitmap(s_background_layer,s_background_bitmap);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

    // Create time TextLayer
    s_time_layer = text_layer_create(GRect(2, 55, 144, 50));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorBlack);

    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_VGA_42));

    // Improve the layout to be more like a watchface
    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    // Create temperature Layer
    s_weather_layer = text_layer_create(GRect(0, 130, 144, 25));
    text_layer_set_background_color(s_weather_layer, GColorClear);
    text_layer_set_text_color(s_weather_layer, GColorWhite);
    text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
    text_layer_set_text(s_weather_layer, "Ashley...");

    // Create second custom font, apply it and add to Window
    s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_VGA_20));
    text_layer_set_font(s_weather_layer, s_weather_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));

    // Add it as a child layer to the Window's root layer
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    fonts_unload_custom_font(s_time_font);
    gbitmap_destroy(s_background_bitmap);
    bitmap_layer_destroy(s_background_layer);

    // destroy weather elements
    text_layer_destroy(s_weather_layer);
    fonts_unload_custom_font(s_weather_font);
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

