#include "sdlu.h"

#include <stdio.h>
#include <stdlib.h>

void sdlu_init(uint32_t flags) {
	const int code = SDL_Init(flags);

	if (code != 0) {
		fprintf(stderr, "%s: SDL_Init returned %d instead of 0 for success. "
			"[flags: %d] [Error: %s]\n",
			__func__, code, flags, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

SDL_Window *sdlu_create_window(
	const char *title,
	int x, int y,
	int w, int h,
	uint32_t flags)
{
	SDL_Window *window = SDL_CreateWindow(title, x, y, w, h, flags);

	if (window == NULL) {
		fprintf(stderr, "%s: SDL_CreateWindow returned NULL. "
			"[title: %s] [x: %d] [y: %d] [w: %d] [h: %d] "
			"[flags: %d] [Error: %s]\n",
			__func__,
			title, x, y, w, h,
			flags, SDL_GetError());

		exit(EXIT_FAILURE);
	}

	return window;
}

SDL_Surface *sdlu_get_window_surface(SDL_Window *window) {
	SDL_Surface* surface = SDL_GetWindowSurface(window);

	if (surface == NULL) {
		fprintf(stderr, "%s: SDL_GetWindowSurface returned NULL. "
			"[Error: %s]\n",
			__func__,
			SDL_GetError());

		exit(EXIT_FAILURE);
	}

	return surface;
}

void sdlu_update_window_surface(SDL_Window *window) {
	const int code = SDL_UpdateWindowSurface(window);

	if (code != 0) {
		fprintf(stderr, "%s: SDL_UpdateWindowSurface returned %d instead of "
			"0 for success. [Error: %s]\n",
			__func__, code,
			SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

void sdlu_fill_surface(
	SDL_Surface *surface,
	const uint8_t r, const uint8_t g, const uint8_t b)
{
	uint32_t color = SDL_MapRGB(surface->format, r, g, b);

	if (SDL_FillRect(surface, NULL, color) != 0) {
		fprintf(stderr, "%s: SDL_FillRect error: %s\n",
			__func__, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

void sdlu_set_relative_mouse_mode(SDL_bool enabled) {
	const int code = SDL_SetRelativeMouseMode(enabled);

	if (code != 0) {
		fprintf(stderr, "SDL_SetRelativeMouseMode(%d) error %d: %s\n",
			enabled, code, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

void sdlu_set_window_fullscreen(SDL_Window *w, uint32_t flags) {
	const int err = SDL_SetWindowFullscreen(w, flags);

	if (err != 0) {
		fprintf(stderr, "%s: SDL_SetWindowFullscreen(%u) error %d: %s\n",
			__func__, flags, err, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

void sdlu_set_pixel_safe(SDL_Surface* surface, const int x, const int y,
	const uint8_t r, const uint8_t g, const uint8_t b)
{
	if (x < 0 || x >= surface->w) {
		fprintf(stderr, "%s: Out of range x value: %d. surface->w: %d\n",
			__func__, x, surface->w);

		exit(EXIT_FAILURE);
	}

	if (y < 0 || y >= surface->h) {
		fprintf(stderr, "%s: Out of range y value: %d. surface->h: %d\n",
			__func__, y, surface->h);

		exit(EXIT_FAILURE);
	}

	sdlu_set_pixel(surface, x, y, r, g, b);
}

void sdlu_set_pixel(SDL_Surface* surface, const int x, const int y,
	const uint8_t r, const uint8_t g, const uint8_t b)
{
	switch (surface->format->BytesPerPixel) {
		case 1:
		{
			uint8_t *const pixels = surface->pixels;
			const uint32_t color = SDL_MapRGB(surface->format, r, g, b);
			pixels[x + y * surface->w] = (uint8_t)color;
			break;
		}
		case 2:
		{
			uint16_t *const pixels = surface->pixels;
			const uint32_t color = SDL_MapRGB(surface->format, r, g, b);
			pixels[x + y * surface->w] = (uint16_t)color;
			break;
		}
		case 3:
		{
			uint8_t *const pixels = surface->pixels;
			const uint32_t color = SDL_MapRGB(surface->format, r, g, b);
			int i = x + y * surface->w;
			pixels[i] = (uint8_t)(color >>  0);
			i += 1;
			pixels[i] = (uint8_t)(color >>  8);
			i += 1;
			pixels[i] = (uint8_t)(color >> 16);
			break;
		}
		case 4:
		{
			uint32_t *const pixels = surface->pixels;
			const uint32_t color = SDL_MapRGB(surface->format, r, g, b);
			pixels[x + y * surface->w] = color;
			break;
		}
		default:
			fprintf(stderr, "%s: Impossible BytesPerPixel value: %d. "
				"Expected in range [1, 4]. x: %d. y: %d. "
				"r: %d. g: %d. b: %d\n",
				__func__, surface->format->BytesPerPixel, x, y, r, g, b);

			exit(EXIT_FAILURE);
	}
}

void sdlu_blit_surface(
	SDL_Surface *src,
	const SDL_Rect *srcrect,
	SDL_Surface *dst,
	SDL_Rect *dstrect)
{
	int code = SDL_BlitSurface(src, srcrect, dst, dstrect);

	if (code != 0) {
		fprintf(stderr, "%s: Error code: %d. Error: %s\n",
			__func__, code, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

void sdlu_blit_scaled(
	SDL_Surface *src,
	const SDL_Rect *srcrect,
	SDL_Surface *dst,
	SDL_Rect *dstrect)
{
	int code = SDL_BlitScaled(src, srcrect, dst, dstrect);

	if (code != 0) {
		fprintf(stderr, "%s: Error: %d: %s\n",
			__func__, code, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

SDL_Renderer *sdlu_get_renderer(SDL_Window *window) {
	SDL_Renderer *renderer = SDL_GetRenderer(window);

	if (renderer == NULL) {
		fprintf(stderr, "%s: SDL_GetRenderer failed: %s\n",
			__func__, SDL_GetError());

		exit(EXIT_FAILURE);
	}

	return renderer;
}

void sdlu_set_render_draw_color(
	SDL_Renderer *renderer,
	uint8_t r,
	uint8_t g,
	uint8_t b,
	uint8_t a)
{
	const int code = SDL_SetRenderDrawColor(renderer, r, g, b, a);

	if (code != 0) {
		fprintf(stderr, "%s: error code: %d: %s\n",
			__func__, code, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

void sdlu_render_fill_rect(
	SDL_Renderer *renderer,
	const SDL_Rect *rect)
{
	const int code = SDL_RenderFillRect(renderer, rect);

	if (code != 0) {
		fprintf(stderr, "%s: SDL_RenderFillRect error code: %d: %s\n",
			__func__, code, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

void sdlu_render_clear(SDL_Renderer *renderer) {
	const int code = SDL_RenderClear(renderer);

	if (code != 0) {
		fprintf(stderr, "%s: Error: %d: %s\n",
			__func__, code, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

SDL_Texture *sdlu_create_texture_from_surface(
	SDL_Renderer *renderer,
	SDL_Surface *surface)
{
	SDL_Texture *const tex = SDL_CreateTextureFromSurface(renderer, surface);

	if (tex == NULL) {
		fprintf(stderr, "%s: Error: %s\n", __func__, SDL_GetError());

		exit(EXIT_FAILURE);
	}

	return tex;
}

void sdlu_render_copy(
	SDL_Renderer *renderer,
	SDL_Texture *texture,
	const SDL_Rect *srcrect,
	const SDL_Rect *dstrect)
{
	const int code = SDL_RenderCopy(renderer, texture, srcrect, dstrect);

	if (code != 0) {
		fprintf(stderr, "%s: Error: %d: %s\n",
			__func__, code, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

void sdlu_query_texture(
	SDL_Texture *texture,
	uint32_t *format,
	int *access,
	int *w,
	int *h)
{
	const int code = SDL_QueryTexture(texture, format, access, w, h);

	if (code != 0) {
		fprintf(stderr, "%s: Error: %d: %s\n",
			__func__, code, SDL_GetError());

		exit(EXIT_FAILURE);
	}
}

int sdlu_show_cursor(int toggle) {
	const int code = SDL_ShowCursor(toggle);

	if (code < 0) {
		fprintf(stderr, "%s: SDL_ShowCursor error: %d: %s\n",
			__func__, code, SDL_GetError());

		exit(EXIT_FAILURE);
	}

	return code;
}
