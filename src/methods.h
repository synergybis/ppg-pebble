#pragma once

static void click_handler_select(ClickRecognizerRef recognizer, void *context);
static void click_handler_up(ClickRecognizerRef recognizer, void *context);
static void click_handler_down(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void window_load(Window *window);
static void window_unload(Window *window);
static void init(void);
static void deinit(void);
static void ping(void);

int main(void);