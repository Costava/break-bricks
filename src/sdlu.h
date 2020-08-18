#ifndef SDLU_H
#define SDLU_H

// SDL2 utility and wrapper functions

#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize SDL using flags
// If error, print to stderr and exit
void sdlu_init(uint32_t flags);

// Create SDL window with given args
// If error, print to stderr and exit
SDL_Window *sdlu_create_window(
	const char *title,
	int x, int y,
	int w, int h,
	uint32_t flags);

// Return surface for given window
// If error, print to stderr and exit
SDL_Surface *sdlu_get_window_surface(SDL_Window *window);

// Update surface of given window
// If error, print to stderr and exit
void sdlu_update_window_surface(SDL_Window *window);

// Fill given surface with given (r,g,b) color
// If error, print to stderr and exit
void sdlu_fill_surface(
	SDL_Surface *surface,
	const uint8_t r, const uint8_t g, const uint8_t b);

// Set relative mouse mode
// If error, print to stderr and exit
void sdlu_set_relative_mouse_mode(SDL_bool enabled);

// Set fullscreen mode for given window
// If error, print and exit
void sdlu_set_window_fullscreen(SDL_Window *w, uint32_t flags);

// Checks that `x` and `y` are on the surface
//  (if either is not, prints to stderr and exits),
//  then calls `sdlu_set_pixel`
void sdlu_set_pixel_safe(SDL_Surface* surface, const int x, const int y,
	const uint8_t r, const uint8_t g, const uint8_t b);

// Set pixel (x, y) with color r/g/b for given surface
void sdlu_set_pixel(SDL_Surface* surface, const int x, const int y,
	const uint8_t r, const uint8_t g, const uint8_t b);

// If error, print and exit
void sdlu_blit_surface(
	SDL_Surface *src,
	const SDL_Rect *srcrect,
	SDL_Surface *dst,
	SDL_Rect *dstrect);

// If error, print and exit
void sdlu_blit_scaled(
	SDL_Surface *src,
	const SDL_Rect *srcrect,
	SDL_Surface *dst,
	SDL_Rect *dstrect);

// Get renderer for the window
// If error, print and exit
SDL_Renderer *sdlu_get_renderer(SDL_Window *window);

// If error, print and exit
void sdlu_set_render_draw_color(
	SDL_Renderer *renderer,
	uint8_t r,
	uint8_t g,
	uint8_t b,
	uint8_t a);

void sdlu_render_fill_rect(
	SDL_Renderer *renderer,
	const SDL_Rect *rect);

void sdlu_render_clear(SDL_Renderer *renderer);

SDL_Texture *sdlu_create_texture_from_surface(
	SDL_Renderer *renderer,
	SDL_Surface *surface);

void sdlu_render_copy(
	SDL_Renderer *renderer,
	SDL_Texture *texture,
	const SDL_Rect *srcrect,
	const SDL_Rect *dstrect);

void sdlu_query_texture(
	SDL_Texture *texture,
	uint32_t *format,
	int *access,
	int *w,
	int *h);

int sdlu_show_cursor(int toggle);

#ifdef __cplusplus
}
#endif

#endif
