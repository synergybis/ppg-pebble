#include "pebble.h"
#include "methods.h"

#define MATH_PI 3.141592653589793238462
#define NUM_DISCS 20
#define DISC_DENSITY 0.25
#define ACCEL_RATIO 0.05
#define ACCEL_STEP_MS 50

static Window *window;
static TextLayer *text_layer;

static GRect window_frame;

static AppTimer *timer;

/* CLICK HANDLER METHODS */

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, click_handler_up);
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler_select);
  window_single_click_subscribe(BUTTON_ID_DOWN, click_handler_down);
}

static void click_handler_up(ClickRecognizerRef recognizer, void *context) {
  /* UP CODE */
}

static void click_handler_select(ClickRecognizerRef recognizer, void *context) {
  /* SELECT CODE */
}

static void click_handler_down(ClickRecognizerRef recognizer, void *context) {
  /* DOWN CODE */
}

/* WINDOW LOAD & UNLOAD METHODS */

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  // GRect frame = window_frame = layer_get_frame(window_layer);

  GRect bounds = layer_get_bounds(window_layer);
  text_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h / 2 - 10 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "press to pull pin --->");
  text_layer_set_text_alignment(text_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

/* MAIN LOOP METHODS */

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true /* Animated */);

}

static void deinit(void) {
  accel_data_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
