#include "pebble.h"
#include "methods.h"

#define MATH_PI 3.141592653589793238462
#define NUM_DISCS 20
#define DISC_DENSITY 0.25
#define ACCEL_RATIO 0.05
#define ACCEL_STEP_MS 50

#define ACCEL_REFRESH 100
#define HISTORY_MAX 144
#define SOUND_KEY 123456789
/* VARIABLES & FIELDS INTIALIZERS */

static Window *window;
static TextLayer *text_layer;
static BitmapLayer *image_layer;

/*static GRect window_frame;*/

static GBitmap *grenadeRegular;
static GBitmap *grenadePulled;
static AppTimer *timer;

bool isActive = false;

static int last_x = 0;
static AccelData accel;
static AccelData history[HISTORY_MAX];

/* APP MESSAGE METHODS */
 
static void sendString(int key, char * msg) {
  DictionaryIterator *hash;
  app_message_outbox_begin(&hash);

  Tuplet row =  TupletCString(key, msg);
  dict_write_tuplet(hash, &row);

  app_message_outbox_send();
}
static void ping(void) {
  int key = SOUND_KEY;
  char * msg = "Why Hello There!";
  sendString(key, msg);
}
/*static void playExplosion(void) {
  int key = SOUND_KEY;
  char * msg = "explosion.wav";
  sendString(key, msg);
}
static void playPinClick(void) {
  int key = SOUND_KEY;
  char * msg = "pullpin.wav";
  sendString(key, msg);
}*/
/* CLICK HANDLER METHODS */

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, click_handler_up);
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler_select);
  window_single_click_subscribe(BUTTON_ID_DOWN, click_handler_down);
}

static void click_handler_up(ClickRecognizerRef recognizer, void *context) {
  isActive = !isActive;
  if (isActive) {
    vibes_short_pulse();
    bitmap_layer_set_bitmap(image_layer, grenadePulled);
    accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
    accel_data_service_subscribe(0, NULL);
  }
  else {
    accel_data_service_unsubscribe();
  }

  set_timer();
}

static void click_handler_select(ClickRecognizerRef recognizer, void *context) {
  /* SELECT CODE */
     ping();
}

static void click_handler_down(ClickRecognizerRef recognizer, void *context) {
  /* DOWN CODE */

}

/* ACCELEROMETER METHODS */

static void set_timer() {
  if (isActive) timer = app_timer_register(ACCEL_REFRESH, accel_callback, NULL);
}

static void accel_callback() {
  if (!isActive) return;

  accel_service_peek(&accel);

  history[last_x].x = accel.x;
  history[last_x].y = accel.y;
  history[last_x].z = accel.z;
  last_x++;
  if (last_x >= HISTORY_MAX) last_x = 0;

  set_timer();
}

/* WINDOW LOAD & UNLOAD METHODS */

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  // GRect frame = window_frame = layer_get_frame(window_layer);

  GRect bounds = layer_get_bounds(window_layer);
  text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 36 } });
  text_layer_set_text(text_layer, ">");
  text_layer_set_text_alignment(text_layer, GTextAlignmentRight);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  grenadeRegular = gbitmap_create_with_resource(RESOURCE_ID_GRENADE_REG);
  grenadePulled = gbitmap_create_with_resource(RESOURCE_ID_GRENADE_PULLED);

  image_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(image_layer, grenadeRegular);
  bitmap_layer_set_alignment(image_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));

}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}
void out_sent_handler(DictionaryIterator *sent, void *context) {
// outgoing message was delivered
}

/* MAIN LOOP METHODS */


void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
// outgoing message failed
}


void in_received_handler(DictionaryIterator *received, void *context) {
// incoming message received
}


void in_dropped_handler(AppMessageResult reason, void *context) {
// incoming message dropped
}

static void init(void) {
   app_message_register_inbox_received(in_received_handler);
   app_message_register_inbox_dropped(in_dropped_handler);
   app_message_register_outbox_sent(out_sent_handler);
   app_message_register_outbox_failed(out_failed_handler);

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true /* Animated */);

   const uint32_t inbound_size = 64;
   const uint32_t outbound_size = 64;
   app_message_open(inbound_size, outbound_size);


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
