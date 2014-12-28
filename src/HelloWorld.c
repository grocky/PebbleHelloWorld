#include <pebble.h>

enum {
    KEY_TEMPERATURE = 0,
    KEY_CONDITIONS = 1
};

static Window *s_main_window;

static TextLayer *s_hour_layer;
static TextLayer *s_minute_layer;
static TextLayer *s_info_layer;

static GFont s_time_font;
static GFont s_info_font;

// static BitmapLayer  *s_background_layer;

// static GBitmap *s_background_bitmap;

static void update_time() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Create a long-lived buffer
    static char *h_buffer = "01";
    static char *m_buffer = "00";

    char *h_format = "";
    if(clock_is_24h_style() == true) {
        h_format = "%H";
    } else {
        h_format = "%I";
    }
    char *m_format = "%M";
    strftime(h_buffer, sizeof("01"), h_format, tick_time);
    strftime(m_buffer, sizeof("00"), m_format, tick_time);

    // APP_LOG(APP_LOG_LEVEL_INFO, "hour: %s, minute: %s", h_buffer, m_buffer);

    text_layer_set_text(s_hour_layer, h_buffer);
    text_layer_set_text(s_minute_layer, m_buffer);
}

static void main_window_load(Window *window) {
    APP_LOG(APP_LOG_LEVEL_INFO, "main_window_load()");
    // Ceate GBitmap, then set to created BitmapLayer
    // s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
    // s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
    // bitmap_layer_set_bitmap(s_background_layer,s_background_bitmap);
    // layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    // Create Hour TextLayer
    s_hour_layer = text_layer_create(GRect(5, 20, 100, 705));
    text_layer_set_background_color(s_hour_layer, GColorClear);
    text_layer_set_text_color(s_hour_layer, GColorBlack);
    text_layer_set_text(s_hour_layer, "00");

    // Create minute TextLayer
    s_minute_layer = text_layer_create(GRect(30, 70, 100, 75));
    text_layer_set_background_color(s_minute_layer, GColorClear);
    text_layer_set_text_color(s_minute_layer, GColorBlack);
    text_layer_set_text(s_minute_layer, "00");


    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TIME_FONT_70));

    // Improve the layout to be more like a watchface
    text_layer_set_font(s_hour_layer, s_time_font);
    text_layer_set_font(s_minute_layer, s_time_font);
    text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(s_minute_layer, GTextAlignmentCenter);

    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));

    // Create temperature Layer
    s_info_layer = text_layer_create(GRect(0, 0, window_bounds.size.w, 20));
    text_layer_set_background_color(s_info_layer, GColorBlack);
    text_layer_set_text_color(s_info_layer, GColorWhite);
    text_layer_set_text_alignment(s_info_layer, GTextAlignmentCenter);
    text_layer_set_text(s_info_layer, "1 sec Rocky...");

    // Create second custom font, apply it and add to Window
    s_info_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_INFO_FONT_15));
    text_layer_set_font(s_info_layer, s_info_font);
    layer_add_child(window_layer, text_layer_get_layer(s_info_layer));

    update_time();
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_hour_layer);
    text_layer_destroy(s_minute_layer);
    fonts_unload_custom_font(s_time_font);
    // gbitmap_destroy(s_background_bitmap);
    // bitmap_layer_destroy(s_background_layer);

    // destroy weather elements
    text_layer_destroy(s_info_layer);
    fonts_unload_custom_font(s_info_font);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();

    if(tick_time->tm_min % 30 == 0) {
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);

        dict_write_uint8(iter, 0, 0);

        app_message_outbox_send();
    }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "inbox_received_callback()");
    // Store incoming information
    static char temperature_buffer[8];
    static char conditions_buffer[32];
    static char weather_layer_buffer[32];

    // Read first item
    Tuple *t = dict_read_first(iterator);

    while(t != NULL) {
        switch(t->key) {
            case KEY_TEMPERATURE:
                snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int)t->value->int32);
                break;
            case KEY_CONDITIONS:
                snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
                break;
            default:
                APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
                break;
        }
        t = dict_read_next(iterator);
    }

    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_info_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped.");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed.");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send successful.");
}

static void init() {
    s_main_window = window_create();

    // set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    // Show the window on the watch, with animated=true
    window_stack_push(s_main_window, true);

    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    // Register callbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
    APP_LOG(APP_LOG_LEVEL_INFO, "deinit()");
    window_destroy(s_main_window);
}

int main(void) {
    APP_LOG(APP_LOG_LEVEL_INFO, "main()");
    init();

    // letts the watchapp wait for system events until it exits
    app_event_loop();

    deinit();
}

