#pragma once

static void click_handler_select(ClickRecognizerRef recognizer, void *context);
static void click_handler_up(ClickRecognizerRef recognizer, void *context);
static void click_handler_down(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void set_timer();
static void accel_callback();
static void window_load(Window *window);
static void window_unload(Window *window);
static void init(void);
static void deinit(void);

/*
static void playBroFist(void);
static void playLaser(void);
*/
static void playPinClick(void);
static void playExplosion();


static bool scanAccelProfileGrenade(void);
/*static bool scanAccelProfileBroFist(void);*/


int main(void);