#include "pebble.h"
#include "methods.h"

#define MATH_PI 3.141592653589793238462
#define NUM_DISCS 20
#define DISC_DENSITY 0.25
#define ACCEL_RATIO 0.05
#define ACCEL_STEP_MS 50

#define ACCEL_REFRESH 100
#define HISTORY_MAX 100
#define SOUND_KEY 1234567890
#define ACCEL_KEY 987654321
#define NADE_TIMER 5000 //ms
#define NADE_THRESH  1500 //mG
#define WHOOSH_THRESH 2000 //mG

/* VARIABLES & FIELDS INTIALIZERS */

static Window *window;
static TextLayer *text_layer;
static TextLayer *feedback_layer;
static TextLayer *debug_layer;
static BitmapLayer *image_layer;

/*static GRect window_frame;*/

static GBitmap *grenadeRegular;
static GBitmap *grenadePulled;
static AppTimer *thrownTimer;
static AppTimer *timer;

bool isActive = false;

static int last_x = 0;

static AccelData accel;
static AccelData history[HISTORY_MAX];

/* Acceleration Profile Methods */

static float numeric_square(const float onalright) {
  return onalright * onalright;
}
static bool scanAccelProfileWhoosh(void) {
  float sum = 0.0;
  sum += numeric_square(history[last_x].x);
  sum += numeric_square(history[last_x].y);
  sum += numeric_square(history[last_x].z);

  static char buffs[3][32];
  snprintf(buffs[0], sizeof("X: XXXXX"), "X: %d", history[last_x].x);
  snprintf(buffs[1], sizeof("Y: YYYYY"), "Y: %d", history[last_x].y);
  snprintf(buffs[2], sizeof("Z: ZZZZZ"), "Z: %d", history[last_x].z);
  text_layer_set_text(debug_layer, buffs[1]);

  return (history[last_x].y > WHOOSH_THRESH);
}
static bool scanAccelProfileGrenade(void) {
  float sum = 0.0;
  sum += numeric_square(history[last_x].x);
  sum += numeric_square(history[last_x].y);
  sum += numeric_square(history[last_x].z);

  static char buffs[3][32];
  snprintf(buffs[0], sizeof("X: XXXXX"), "X: %d", history[last_x].x);
  snprintf(buffs[1], sizeof("Y: YYYYY"), "Y: %d", history[last_x].y);
  snprintf(buffs[2], sizeof("Z: ZZZZZ"), "Z: %d", history[last_x].z);
  text_layer_set_text(debug_layer, buffs[1]);

  return (history[last_x].y > NADE_THRESH);
}
/*static bool scanAccelProfileBroFist(void) {
 return false;
}*/
/* APP MESSAGE METHODS */

static void sendString(int key, char * msg) {
  DictionaryIterator *hash;
  app_message_outbox_begin(&hash);

  Tuplet row =  TupletCString(key, msg);
  dict_write_tuplet(hash, &row);

  app_message_outbox_send();
}
static void sendAcceleration(char * msg) {
  sendString(ACCEL_KEY, msg);
}
static void playExplosion() {
  int key = SOUND_KEY;
  char * msg = "explosion";
  sendString(key, msg);
  app_timer_cancel(thrownTimer);
  isActive = !isActive;
  bitmap_layer_set_bitmap(image_layer, grenadeRegular);
  text_layer_set_text(feedback_layer, "BOOM");
}
static void playPinClick() {
  int key = SOUND_KEY;
  char * msg = "pullpin";
  sendString(key, msg);
  text_layer_set_text(feedback_layer, "PULLED");
}
static void playWhoosh() {
  int key = SOUND_KEY;
  char * msg = "whoosh";
  sendString(key, msg);
  text_layer_set_text(debug_layer, "WHOOSH");
}
/*static void playBroFist(void) {
  int key = SOUND_KEY;
  char * msg = "yes.wav";
  sendString(key, msg);
}*/

/* CLICK HANDLER METHODS */

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, click_handler_up);
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler_select);
  window_single_click_subscribe(BUTTON_ID_DOWN, click_handler_down);
}

static void click_handler_up(ClickRecognizerRef recognizer, void *context) {
  if (!isActive) {
    isActive = !isActive;
    vibes_short_pulse();
    text_layer_set_text(text_layer, "");
    bitmap_layer_set_bitmap(image_layer, grenadePulled);
    accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
    accel_data_service_subscribe(0, NULL);
    playPinClick();
    set_timer();
  }
}

static void click_handler_select(ClickRecognizerRef recognizer, void *context) {
  /* SELECT CODE */
}

static void click_handler_down(ClickRecognizerRef recognizer, void *context) {
  /* DOWN CODE */

}

/* ACCELEROMETER METHODS */

static void set_timer() {
  if (isActive) timer = app_timer_register(ACCEL_REFRESH, accel_callback, NULL);
}
//accel
static void accel_callback() {

  accel_service_peek(&accel);

  history[last_x].x = accel.x;
  history[last_x].y = accel.y;
  history[last_x].z = accel.z;
  if (isActive && scanAccelProfileGrenade()) {
    thrownTimer = app_timer_register(NADE_TIMER, playExplosion, NULL);
    accel_data_service_unsubscribe();
    last_x = 0;
    text_layer_set_text(feedback_layer, "THROWN");
  }
  else {
    last_x++;
    if (last_x >= HISTORY_MAX) {
      playExplosion();
      vibes_double_pulse();
      accel_data_service_unsubscribe();
      last_x = 0;
    }
    else {
      set_timer();
    }
  }
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

  feedback_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h - 20 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(feedback_layer, "");
  text_layer_set_text_alignment(feedback_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(feedback_layer));

  debug_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h - 20 }, .size = { 70, 20 } });
  text_layer_set_text(debug_layer, "");
  layer_add_child(window_layer, text_layer_get_layer(debug_layer));

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
