#include <pebble.h>

#define CHANNEL_MAX 3

#define R     0
#define G     1
#define B     2
#define ALPHA CHANNEL_MAX

static Window *s_main_window;
static TextLayer *s_output_layer;
static Layer *s_selection_layer;
static Layer *s_canvas_layer;

static uint8_t rgb_values[3];
static char color_text_buffer[11];
static uint8_t channel_index;

static char* uint2str(uint8_t value) {
  switch(value) {
    case 0: return "00";
    case 1: return "01";
    case 2: return "10";
    case 3: return "11";
    default: return "XX";
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw the color
  graphics_context_set_fill_color(ctx, (GColor8){
    .a = ALPHA,
    .r = rgb_values[R],
    .g = rgb_values[G],
    .b = rgb_values[B]
  });
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 0, GCornerNone);
}

static void selection_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void update_display() {
  // Update color
  layer_mark_dirty(s_canvas_layer);

  // Update string
  snprintf(color_text_buffer, sizeof(color_text_buffer), "0b11%s%s%s", 
    uint2str(rgb_values[R]), uint2str(rgb_values[G]), uint2str(rgb_values[B]));
  text_layer_set_text(s_output_layer, color_text_buffer);

  // Selection box location
  switch(channel_index) {
    case R:
      layer_set_frame(s_selection_layer, GRect(62, 20, 18, 4));
      break;
    case G:
      layer_set_frame(s_selection_layer, GRect(80, 20, 18, 4));
      break;
    case B:
      layer_set_frame(s_selection_layer, GRect(98, 20, 18, 4));
      break;
    default: break;
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(rgb_values[channel_index] < CHANNEL_MAX) {
    rgb_values[channel_index]++;
  }

  update_display();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(rgb_values[channel_index] > 0) {
    rgb_values[channel_index]--;
  }

  update_display();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Cycle selected channel
  channel_index++;
  if(channel_index == CHANNEL_MAX) {
    channel_index = R;
  }

  update_display();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_canvas_layer = layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  s_output_layer = text_layer_create(GRect(0, -6, window_bounds.size.w, 30));
  text_layer_set_text_color(s_output_layer, GColorBlack);
  text_layer_set_background_color(s_output_layer, GColorWhite);
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));

  s_selection_layer = layer_create(GRect(0, 0, 0, 0));
  layer_set_update_proc(s_selection_layer, selection_update_proc);
  layer_add_child(window_layer, s_selection_layer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_output_layer);
  layer_destroy(s_canvas_layer);
  layer_destroy(s_selection_layer);
}

static void init() {
  s_main_window = window_create();
#ifdef PBL_SDK_2
  window_set_fullscreen(s_main_window, true);
#endif
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // First color
  rgb_values[R] = 0;
  rgb_values[G] = 0;
  rgb_values[B] = 0;
  update_display();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
