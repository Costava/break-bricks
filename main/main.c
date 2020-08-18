#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "charu.h"
#include "easy_alloc.h"
#include "game.h"
#include "nsec.h"
#include "rand.h"
#include "sdlu.h"

// If buf is NULL, prints to stderr and exits
// Datetime format: yyyy-mm-dd_hh:mm:ss.MMM_UTC
// Will write a NUL terminator (unless `buf_len` is 0)
// Will not overflow the buffer so the whole datetime will not be written
//  when the buffer is too small
// Returns true if space left in buf afterwards, else returns false
bool write_datetime(char *buf, const size_t buf_len) {
	if (buf_len == 0) {
		return false;
	}

	if (buf == NULL) {
		fprintf(stderr, "%s: buf must not be NULL\n", __func__);
		exit(EXIT_FAILURE);
	}

	struct timespec ts;

	if (timespec_get(&ts, TIME_UTC) == 0) {
		fprintf(stderr, "%s: timespec_get error\n", __func__);
		exit(EXIT_FAILURE);
	}

	struct tm datetime;
	const struct tm *tm = gmtime_r(&ts.tv_sec, &datetime);

	if (tm == NULL) {
		fprintf(stderr, "%s: gmtime_r failed\n", __func__);

		exit(EXIT_FAILURE);
	}

	char *const end = buf + (buf_len - 1);

	// An intermediate buffer to work with
	char info[16];

	// Concat year
	snprintf(info, 10, "%d", datetime.tm_year + 1900);
	buf = charu_copy_until_end(buf, end, info);
	// Alternatively we could use == and also check > for unexpected error
	if (buf >= end) return false;

	// Concat hyphen
	buf = charu_copy_until_end(buf, end, "-");
	if (buf >= end) return false;

	// Concat month
	snprintf(info, 10, "%.2d", datetime.tm_mon + 1);
	buf = charu_copy_until_end(buf, end, info);
	if (buf >= end) return false;

	// Concat hyphen
	buf = charu_copy_until_end(buf, end, "-");
	if (buf >= end) return false;

	// Concat day of month
	snprintf(info, 10, "%.2d", datetime.tm_mday);
	buf = charu_copy_until_end(buf, end, info);
	if (buf >= end) return false;

	// Concat underscore
	buf = charu_copy_until_end(buf, end, "_");
	if (buf >= end) return false;

	// Concat hours
	snprintf(info, 10, "%.2d", datetime.tm_hour);
	buf = charu_copy_until_end(buf, end, info);
	if (buf >= end) return false;

	// Concat colon
	buf = charu_copy_until_end(buf, end, ":");
	if (buf >= end) return false;

	// Concat minutes
	snprintf(info, 10, "%.2d", datetime.tm_min);
	buf = charu_copy_until_end(buf, end, info);
	if (buf >= end) return false;

	// Concat colon
	buf = charu_copy_until_end(buf, end, ":");
	if (buf >= end) return false;

	// Concat seconds
	snprintf(info, 10, "%.2d", datetime.tm_sec);
	buf = charu_copy_until_end(buf, end, info);
	if (buf >= end) return false;

	// Concat decimal point
	buf = charu_copy_until_end(buf, end, ".");
	if (buf >= end) return false;

	const int milliseconds = ts.tv_nsec / 1000000;

	// Concat milliseconds
	snprintf(info, 10, "%.3d", milliseconds);
	buf = charu_copy_until_end(buf, end, info);
	if (buf >= end) return false;

	// Concat "_UTC" suffix
	buf = charu_copy_until_end(buf, end, "_UTC");
	if (buf >= end) return false;

	return true;
}

bool append_datetime(char *buf, const size_t buf_size) {
	const int bufstrlen = strlen(buf);

	char *const start = buf + bufstrlen;

	return write_datetime(start, buf_size - bufstrlen);
}

// Return `val` clamped to the range [min, max] (both inclusive)
// NaN is not checked for
double clamp_double(const double val, const double min, const double max) {
	if (val < min) return min;
	if (val > max) return max;

	return val;
}

// Wrap the double around into the [0, 1] range
double wrap_double01(double value) {
	if (value > 1.0) {
		return fmod(value, 1.0);
	}

	// This can be improved
	while (value < 0.0) {
		value += 1.0;
	}

	return value;
}

/* Overlap formula from: https://stackoverflow.com/questions/306316/
determine-if-two-rectangles-overlap-each-other */
bool rects_overlap(
	const double ax,
	const double ay,
	const double aw,
	const double ah,
	const double bx,
	const double by,
	const double bw,
	const double bh)
{
	// We need right and bottom for both rects

	const double ar = ax + aw;
	const double ab = ay - ah;

	const double br = bx + bw;
	const double bb = by - bh;

	return ax < br && ar > bx && ay > bb && ab < by;
}

enum collision {
	COLL_NONE = 0,
	COLL_TOP,
	COLL_RIGHT,
	COLL_BOTTOM,
	COLL_LEFT
};

/* Based on: https://gamedev.stackexchange.com/questions/29786/a-simple-
2d-rectangle-collision-algorithm-that-also-determines-which-sides-that */
// Returns COLL_TOP if b hits the top of a, etc.
enum collision collide_rects(
	const double ax,
	const double ay,
	const double aw,
	const double ah,
	const double bx,
	const double by,
	const double bw,
	const double bh)
{
	const double a_center_x = ax + (aw * 0.5);
	const double a_center_y = ay - (ah * 0.5);

	const double b_center_x = bx + (bw * 0.5);
	const double b_center_y = by - (bh * 0.5);

	const double w = 0.5 * (aw + bw);
	const double h = 0.5 * (ah + bh);
	const double dx = a_center_x - b_center_x;
	const double dy = a_center_y - b_center_y;

	if (fabs(dx) <= w && fabs(dy) <= h) {
		const double wy = w * dy;
		const double hx = h * dx;

		if (wy > hx) {
			if (wy > -hx) {
				return COLL_BOTTOM;
			}

			return COLL_RIGHT;
		}
		else {
			if (wy > -hx) {
				return COLL_LEFT;
			}

			return COLL_TOP;
		}
	}

	return COLL_NONE;
}

// Populate the `out_` values as a random rect inside the given rect
void rand_rect_inside_rect(
	const double pos_x,
	const double pos_y,
	const double size_x,
	const double size_y,
	double *const out_pos_x,
	double *const out_pos_y,
	double *const out_size_x,
	double *const out_size_y)
{
	// Arbitrarily decided that the rect is [0.05, 0.95] of the parent sizes
	*out_size_x = rand_double(0.05 * size_x, 0.95 * size_x);
	*out_size_y = rand_double(0.05 * size_y, 0.95 * size_y);

	const double parent_right = pos_x + size_x;
	const double parent_bottom = pos_y - size_y;

	*out_pos_x = rand_double(pos_x, parent_right - *out_size_x);
	*out_pos_y = rand_double(parent_bottom + *out_size_y, pos_y);
}

// You can't pass around a pointer to 'all the variables in a scope'
// Pass around a struct instead (is this a good idea?)
struct world {
	SDL_Window *window;
	SDL_Surface *surface;
	SDL_Renderer *renderer;

	bool quit;
	bool is_fullscreen;

	struct game game;
};

int main(void) {
	// printf("Compiled on %s %s\n", __DATE__, __TIME__);
	// printf("HELLO\n");
	// printf("Number %.2d\n", 1);

	struct world world;

	sdlu_init(SDL_INIT_VIDEO);

	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	world.window = sdlu_create_window(
		"Break Bricks",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		// 450, 130,
		640, 840,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	// Reminder: Do not free this surface
	world.surface = sdlu_get_window_surface(world.window);
	world.renderer = sdlu_get_renderer(world.window);

	// Make window initially all a solid color
	sdlu_fill_surface(world.surface, 27, 60, 20);
	sdlu_update_window_surface(world.window);

	srand(time(NULL));

	game_init(&world.game, world.renderer);

	game_setup(&world.game);

	sdlu_show_cursor(SDL_DISABLE);

	world.is_fullscreen = false;
	world.quit = false;

	uint64_t old_time = nsec_time();

	while (!world.quit) {
		const uint64_t new_time = nsec_time();
		const uint64_t delta = new_time - old_time;
		const double ddelta = (double)delta;
		// printf("old_time: %ld new_time: %ld\n", old_time, new_time);
		old_time = new_time;

		// Reset game if dead or the level was cleared

		const bool dead =
			world.game.num_balls == 0 && world.game.num_particles == 0;
		const bool level_cleared =
			world.game.num_bricks == 0 && world.game.num_particles == 0;

		if (dead || level_cleared) {
			game_setup(&world.game);
		}

		// How much delta x movement the paddle did this frame
		double paddle_dx = 0.0;

		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) { switch (event.type) {
			case SDL_QUIT:
			{
				world.quit = true;
				break;
			}
			case SDL_WINDOWEVENT: { switch (event.window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					world.surface = sdlu_get_window_surface(world.window);

					sdlu_fill_surface(world.surface, 27, 60, 20);
					sdlu_update_window_surface(world.window);
					break;
				}
				case SDL_WINDOWEVENT_FOCUS_GAINED:
				{
					// If relative mouse mode is used,
					//  may need to set true when focus gained
					//  and false when focus lost in order to prevent
					//  strange/unwanted behavior where window receives
					//  keyboard events but not mouse events.
					// More info at:
					/* https://gist.github.com/Costava/
					6f41383a57a5445a71f12f3e0ef25e74 */
					// sdlu_set_relative_mouse_mode(SDL_TRUE);
					break;
				}
				case SDL_WINDOWEVENT_FOCUS_LOST:
				{
					// sdlu_set_relative_mouse_mode(SDL_FALSE);
					break;
				}
			} break; }// End of case SDL_WINDOWEVENT scope and its inner switch
			case SDL_MOUSEBUTTONDOWN:
				// // For debugging
				// printf("num_bricks: %d num_particles: %d\n",
				// 	world.game.num_bricks, world.game.num_particles);
				break;
			case SDL_MOUSEMOTION:
			{
				const double mouse_game_x = game_x_screen_to_coord(
					event.motion.x,
					world.game.viewport_center_x,
					world.game.viewport_size_x,
					world.surface->w);

				double new_x = mouse_game_x
					- (world.game.paddle.size_x / 2.0);

				const double play_area_left = world.game.play_area_origin_x
					- (world.game.play_area_size_x / 2.0);

				const double play_area_right = world.game.play_area_origin_x
					+ (world.game.play_area_size_x / 2.0);

				const double paddle_max_right =
					play_area_right - world.game.paddle.size_x;

				if (new_x < play_area_left) {
					new_x = play_area_left;
				}
				else if (new_x > paddle_max_right) {
					new_x = paddle_max_right;
				}

				paddle_dx = new_x - world.game.paddle.pos_x;

				world.game.paddle.pos_x = new_x;

				break;
			}
			case SDL_KEYUP:
			{
				const SDL_Keycode keycode = event.key.keysym.sym;

				switch (keycode) {
					case SDLK_w:
					{
						for (unsigned int i = 0; i < world.game.num_balls;
						     i += 1)
						{
							struct ball *const ball = world.game.balls[i];

							ball->vel_x *= 2.0;
							ball->vel_y *= 2.0;
						}

						break;
					}
					case SDLK_s:
					{
						for (unsigned int i = 0; i < world.game.num_balls;
						     i += 1)
						{
							struct ball *const ball = world.game.balls[i];

							ball->vel_x *= 0.5;
							ball->vel_y *= 0.5;
						}

						break;
					}
					case SDLK_r:
					{
						game_setup(&world.game);

						break;
					}
					case SDLK_f:
					{
						if (world.is_fullscreen) {
							// Alternatively use SDL_WINDOW_FULLSCREEN
							sdlu_set_window_fullscreen(world.window,
								SDL_WINDOW_FULLSCREEN_DESKTOP);
						}
						else {
							sdlu_set_window_fullscreen(world.window, 0);
						}

						world.is_fullscreen = !world.is_fullscreen;

						break;
					}
					case SDLK_F12:
					{
						// Save screenshot to:
						//  Screenshot_yyyy-mm-dd_hh:mm:ss.MMM_UTC.bmp

						char filename[256];
						strcpy(filename, "./screenshots/Screenshot_");
						append_datetime(filename, 256 - strlen(filename) - 1);
						charu_concat(filename, 256, ".bmp");

						const int rrp_code = SDL_RenderReadPixels(
							world.renderer, NULL,
							world.surface->format->format,
							world.surface->pixels,
							world.surface->pitch);

						if (rrp_code != 0) {
							fprintf(stderr, "FAILED TO SAVE SCREENSHOT. "
								"SDL_RenderReadPixels error: %d: %s\n",
								rrp_code, SDL_GetError());

							break;
						}

						const int sbmp_code = SDL_SaveBMP(
							world.surface, filename);

						if (sbmp_code != 0) {
							fprintf(stderr, "FAILED TO SAVE SCREENSHOT. "
								"SDL_SaveBMP error: %d: %s\n",
								sbmp_code, SDL_GetError());

							break;
						}

						printf("Saved screenshot: %s\n", filename);

						break;
					}
				}

				break;
			}
		}}// End of 'while polling events' and 'switch on event type'

		// Update particles
		for (unsigned int i = 0; i < world.game.num_particles; i += 1) {
			struct particle *const particle = world.game.particles[i];

			particle->age_ns += delta;

			if (particle->age_ns >= particle->lifetime_ns) {
				game_remove_particle(&world.game, i);
				i -= 1;
				continue;
			}

			const double p_grav = 0.00000000000004;

			// particle->vel_x *= 0.999;// Probably looks better without this
			particle->vel_y -= p_grav * ddelta;

			particle->pos_x += particle->vel_x * ddelta;
			particle->pos_y += particle->vel_y * ddelta;
		}

		// Update ball positions
		for (unsigned int i = 0; i < world.game.num_balls; i += 1) {
			struct ball *const ball = world.game.balls[i];

			ball->pos_x += ball->vel_x * ddelta;
			ball->pos_y += ball->vel_y * ddelta;

			const double play_area_left = world.game.play_area_origin_x
				- (world.game.play_area_size_x / 2.0);

			const double play_area_right = world.game.play_area_origin_x
				+ (world.game.play_area_size_x / 2.0);

			const double play_area_top = world.game.play_area_origin_y
				+ (world.game.play_area_size_y / 2.0);

			const double play_area_bottom = world.game.play_area_origin_y
				- (world.game.play_area_size_y / 2.0);

			const double ball_right = ball->pos_x + ball->size_x;
			const double ball_bottom = ball->pos_y - ball->size_y;

			// Camera shake
			const double bump_distance = 20.0;

			// Bounce off the walls and ceiling
			if (ball->pos_x < play_area_left) {
				ball->pos_x = play_area_left;
				ball->vel_x = -ball->vel_x;

				world.game.viewport_center_x += bump_distance;
			}

			if (ball_right > play_area_right) {
				ball->pos_x = play_area_right - ball->size_x;
				ball->vel_x = -ball->vel_x;

				world.game.viewport_center_x -= bump_distance;
			}

			if (ball->pos_y > play_area_top) {
				ball->pos_y = play_area_top;
				ball->vel_y = -ball->vel_y;

				world.game.viewport_center_y -= bump_distance;
			}

			bool remove_ball = false;

			if (ball_bottom < play_area_bottom) {
				// Ball dies

				remove_ball = true;

				// Spawn particles
				for (int p = 0; p < 400; p +=1 ) {
					struct particle *particle = easy_malloc(
						sizeof(struct particle));

					rand_rect_inside_rect(
						ball->pos_x, ball->pos_y,
						ball->size_x, ball->size_y,
						&particle->pos_x,
						&particle->pos_y,
						&particle->size_x,
						&particle->size_y);

					particle->vel_x = rand_double(-0.000008, 0.000008);
					particle->vel_y = rand_double(0.000008, 0.000020);

					particle->lifetime_ns = 3000000000;
					particle->age_ns = 0;
					particle->r = rand_int(0, 255);
					particle->g = rand_int(0, 255);
					particle->b = rand_int(0, 255);
					particle->a = rand_int(0, 255);

					game_append_particle(&world.game, particle);
				}
			}

			// Check collision with paddle

			enum collision coll = collide_rects(
				world.game.paddle.pos_x,
				world.game.paddle.pos_y,
				world.game.paddle.size_x,
				world.game.paddle.size_y,
				ball->pos_x,
				ball->pos_y,
				ball->size_x,
				ball->size_y);

			if (coll != 0) {
				// printf("coll: %d\n", coll);

				// At 0.0 because is buggy and makes ball fly super fast
				const double paddle_additive_speed_mult = 0.0;

				const double diff_x = paddle_dx / ddelta
					* paddle_additive_speed_mult;

				// Maybe a switch should be used here

				// If bouncing off the top
				if (coll == COLL_TOP) {
					ball->pos_y = world.game.paddle.pos_y + ball->size_y;

					ball->vel_y = -ball->vel_y;
					ball->vel_x += diff_x;
				}

				// diff_x should not be added if the ball and paddle
				//  are moving in the same direction
				//  at time of collision (is this correct? why?)
				//  but close enough

				// If hit left side
				if (coll == COLL_LEFT) {
					ball->pos_x = world.game.paddle.pos_x - ball->size_x;

					ball->vel_x = -ball->vel_x + diff_x;
				}
				else if (coll == COLL_RIGHT) {
					ball->pos_x = world.game.paddle.pos_x
						+ world.game.paddle.size_x;

					ball->vel_x = -ball->vel_x + diff_x;
				}
			}
			else {
				// No paddle collision
				// Check if ball collides with a brick

				for (unsigned int b = 0; b < world.game.num_bricks; b += 1) {
					struct brick *const brick = world.game.bricks[b];

					enum collision brick_coll = collide_rects(
						brick->pos_x,
						brick->pos_y,
						brick->size_x,
						brick->size_y,
						ball->pos_x,
						ball->pos_y,
						ball->size_x,
						ball->size_y);

					switch (brick_coll) {
						case COLL_TOP:
							ball->pos_y = brick->pos_y + ball->size_y;
							ball->vel_y = -ball->vel_y;
							break;
						case COLL_BOTTOM:
							ball->pos_y = brick->pos_y - brick->size_y;
							ball->vel_y = -ball->vel_y;
							break;
						case COLL_LEFT:
							ball->pos_x = brick->pos_x - ball->size_x;
							ball->vel_x = -ball->vel_x;
							break;
						case COLL_RIGHT:
							ball->pos_x = brick->pos_x + brick->size_x;
							ball->vel_x = -ball->vel_x;
							break;
						case COLL_NONE:
							break;
					}

					if (brick_coll != COLL_NONE) {
						// Spawn particles
						for (int p = 0; p < 10; p +=1 ) {
							struct particle *particle = easy_malloc(
								sizeof(struct particle));

							rand_rect_inside_rect(
								brick->pos_x, brick->pos_y,
								brick->size_x, brick->size_y,
								&particle->pos_x,
								&particle->pos_y,
								&particle->size_x,
								&particle->size_y);

							double base_vx = ball->vel_x * 0.7;
							double base_vy = ball->vel_y * 0.7;

							// Invert because the ball already bounced
							//  (velocity was mirrored previously)
							if (brick_coll == COLL_LEFT ||
							    brick_coll == COLL_RIGHT)
							{
								base_vx *= -1.0;
							}
							else {
								base_vy *= -1.0;
							}

							particle->vel_x = base_vx
								+ rand_double(-0.0000012, 0.0000012);
							particle->vel_y = base_vy
								+ rand_double(-0.0000008, 0.0000016);

							particle->lifetime_ns = 3000000000;
							particle->age_ns = 0;
							particle->r = rand_int(0, 255);
							particle->g = rand_int(0, 255);
							particle->b = rand_int(0, 255);
							particle->a = rand_int(0, 255);

							game_append_particle(&world.game, particle);
						}

						// Remove brick
						game_remove_brick(&world.game, b);

						break;
					}
				}
			}

			if (remove_ball) {
				game_remove_ball(&world.game, i);
				i -= 1;
			}
		}

		// Camera shake with springs
		const double accel_x = -world.game.camera_spring_constant
			* (world.game.viewport_center_x - world.game.play_area_origin_x)
			/ world.game.camera_mass;
		const double accel_y = -world.game.camera_spring_constant
			* (world.game.viewport_center_y - world.game.play_area_origin_y)
			/ world.game.camera_mass;

		world.game.camera_vel_x += accel_x;
		world.game.camera_vel_y += accel_y;

		world.game.viewport_center_x += world.game.camera_vel_x;
		world.game.viewport_center_y += world.game.camera_vel_y;

		world.game.camera_vel_x *= 0.95;
		world.game.camera_vel_y *= 0.95;

		// Render

		// Fill screen with solid color
		sdlu_set_render_draw_color(world.renderer, 27, 60, 20, 255);
		sdlu_render_clear(world.renderer);

		// Color the play area
		const int pa_x = game_x_coord_to_screen(
			world.game.play_area_origin_x - (world.game.play_area_size_x / 2.0),
			world.game.viewport_center_x,
			world.game.viewport_size_x,
			world.surface->w);

		const int pa_y = game_y_coord_to_screen(
			world.game.play_area_origin_y + (world.game.play_area_size_y / 2.0),
			world.game.viewport_center_y,
			world.game.viewport_size_y,
			world.surface->h);

		const int pa_w = game_length_to_screen(world.game.play_area_size_x,
			world.game.viewport_size_x, world.surface->w);
		const int pa_h = game_length_to_screen(world.game.play_area_size_y,
			world.game.viewport_size_y, world.surface->h);

		sdlu_set_render_draw_color(world.renderer, 55, 120, 40, 255);
		SDL_Rect pa_rect = { .x = pa_x, .y = pa_y, .w = pa_w, .h = pa_h };
		sdlu_render_fill_rect(world.renderer, &pa_rect);

		// Render bricks
		for (unsigned int i = 0; i < world.game.num_bricks; i += 1) {
			struct brick *const brick = world.game.bricks[i];

			// Move the brick's texture before rendering
			brick->inner_tex_x_prop += brick->inner_tex_x_prop_speed * ddelta;
			brick->inner_tex_y_prop += brick->inner_tex_y_prop_speed * ddelta;

			brick->inner_tex_x_prop = wrap_double01(brick->inner_tex_x_prop);
			brick->inner_tex_y_prop = wrap_double01(brick->inner_tex_y_prop);

			const int x = game_x_coord_to_screen(
				brick->pos_x,
				world.game.viewport_center_x,
				world.game.viewport_size_x,
				world.surface->w);

			const int y = game_y_coord_to_screen(
				brick->pos_y,
				world.game.viewport_center_y,
				world.game.viewport_size_y,
				world.surface->h);

			const int w = game_length_to_screen(
				brick->size_x, world.game.viewport_size_x, world.surface->w);
			const int h = game_length_to_screen(
				brick->size_y, world.game.viewport_size_y, world.surface->h);

			// Solid color border/background
			sdlu_set_render_draw_color(world.renderer, 255, 255, 0, 255);
			SDL_Rect bg_rect = { .x = x, .y = y, .w = w, .h = h };
			sdlu_render_fill_rect(world.renderer, &bg_rect);

			const int border_thickness = 4;
			SDL_Rect inner_rect = {
				.x = x + border_thickness,
				.y = y + border_thickness,
				.w = w - 2 * border_thickness,
				.h = h - 2 * border_thickness
			};

			uint32_t format;
			int access;
			int tex_w;
			int tex_h;

			sdlu_query_texture(brick->inner_tex,
				&format, &access, &tex_w, &tex_h);

			const int tex_x = brick->inner_tex_x_prop
				* (tex_w - brick->inner_tex_w);
			const int tex_y = brick->inner_tex_y_prop
				* (tex_h - brick->inner_tex_h);

			SDL_Rect srcrect = {
				.x = tex_x,
				.y = tex_y,
				.w = brick->inner_tex_w,
				.h = brick->inner_tex_h};

			sdlu_render_copy(
				world.renderer, brick->inner_tex, &srcrect, &inner_rect);
		}

		// Render balls
		for (unsigned int i = 0; i < world.game.num_balls; i += 1) {
			const struct ball *const ball = world.game.balls[i];

			const int x = game_x_coord_to_screen(
				ball->pos_x,
				world.game.viewport_center_x,
				world.game.viewport_size_x,
				world.surface->w);

			const int y = game_y_coord_to_screen(
				ball->pos_y,
				world.game.viewport_center_y,
				world.game.viewport_size_y,
				world.surface->h);

			// printf("ball (x, y): (%d, %d)\n", x, y);

			const int w = game_length_to_screen(
				ball->size_x, world.game.viewport_size_x, world.surface->w);
			const int h = game_length_to_screen(
				ball->size_y, world.game.viewport_size_y, world.surface->h);

			// printf("(w, h): (%d, %d)\n", w, h);

			SDL_Rect rect = { .x = x, .y = y, .w = w, .h = h };

			sdlu_render_copy(world.renderer, ball->tex, NULL, &rect);
		}

		// Render paddle
		{
			const int x = game_x_coord_to_screen(
				world.game.paddle.pos_x,
				world.game.viewport_center_x,
				world.game.viewport_size_x,
				world.surface->w);

			const int y = game_y_coord_to_screen(
				world.game.paddle.pos_y,
				world.game.viewport_center_y,
				world.game.viewport_size_y,
				world.surface->h);

			int w = game_length_to_screen(world.game.paddle.size_x,
				world.game.viewport_size_x, world.surface->w);
			int h = game_length_to_screen(world.game.paddle.size_y,
				world.game.viewport_size_y, world.surface->h);

			sdlu_set_render_draw_color(world.renderer, 255, 255, 255, 255);
			SDL_Rect rect = { .x = x, .y = y, .w = w, .h = h };
			sdlu_render_fill_rect(world.renderer, &rect);
		}

		// Render particles
		for (unsigned int i = 0; i < world.game.num_particles; i += 1) {
			game_render_particle(
				&world.game,
				world.game.particles[i],
				world.surface->w,
				world.surface->h,
				world.renderer);
		}

		// Update screen
		SDL_RenderPresent(world.renderer);
	}

	SDL_DestroyWindow(world.window);

	game_desetup(&world.game);
	game_deinit(&world.game);

	IMG_Quit();

	SDL_Quit();

	return EXIT_SUCCESS;
}
