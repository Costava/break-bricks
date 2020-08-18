#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

// `pos_` values are top-left of the rectangle
// Positive x-axis goes towards right of screen
// Positive y-axis goes towards top of screen

struct ball {
	double pos_x;
	double pos_y;

	double vel_x;
	double vel_y;

	double size_x;
	double size_y;

	SDL_Texture *tex;
};

struct paddle {
	double pos_x;
	double pos_y;

	double size_x;
	double size_y;
};

struct particle {
	double pos_x;
	double pos_y;
	double size_x;
	double size_y;

	double vel_x;
	double vel_y;

	// How long the particle lives in nanoseconds
	uint64_t lifetime_ns;
	uint64_t age_ns;// Current age in nanoseconds

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct brick {
	double pos_x;
	double pos_y;

	double size_x;
	double size_y;

	SDL_Texture *inner_tex;

	// The rect of the tex to use

	// [0, 1] values
	double inner_tex_x_prop;
	double inner_tex_y_prop;

	int inner_tex_w;
	int inner_tex_h;

	// Units/ns value to update the inner_tex_<>_prop values
	double inner_tex_x_prop_speed;
	double inner_tex_y_prop_speed;
};

// Game state
// What you would serialize to save the game
struct game {
	unsigned int balls_len;// Allocated length of balls buffer
	struct ball **balls;
	unsigned int num_balls;// Number of balls in the buffer

	unsigned int bricks_len;
	struct brick **bricks;
	unsigned int num_bricks;

	unsigned int brick_texs_len;
	SDL_Texture **brick_texs;
	unsigned int num_brick_texs;

	unsigned int particles_len;
	struct particle **particles;
	unsigned int num_particles;

	struct paddle paddle;

	// The game coordinate at the center of the screen
	double viewport_center_x;
	double viewport_center_y;
	// The game coordinate size of the screen
	double viewport_size_x;
	double viewport_size_y;

	double camera_mass;
	double camera_spring_constant;
	double camera_vel_x;
	double camera_vel_y;

	// The center and dimensions of the play area
	// So that the camera can move independently
	double play_area_origin_x;
	double play_area_origin_y;
	double play_area_size_x;
	double play_area_size_y;

	bool is_setup;

	SDL_Texture *ball_tex;
};

// Things to do once (no need to repeat if playing a second match)
// Remember that IMG_Init must happen before this
void game_init(struct game *const game, SDL_Renderer *renderer);

// Deallocate and clean up the work done in `game_init`
// If you have called setup, you should desetup before calling this
void game_deinit(struct game *const game);

void game_setup(struct game *const game);

// Game is not valid until you call `game_setup` again
void game_desetup(struct game *const game);

void game_append_ball(struct game *const game, struct ball *const ball);

void game_append_particle(
	struct game *const game,
	struct particle *const particle);

void game_append_brick(struct game *const game, struct brick *const brick);

// Remove ball at index `i`
void game_remove_ball(struct game *const game, const unsigned int i);

// Remove particle at index `i`
void game_remove_particle(struct game *const game, const unsigned int i);

// Remove brick at index `i`
void game_remove_brick(struct game *const game, const unsigned int i);

// Translate x pixel coordinate to game coordinate value
double game_x_screen_to_coord(
	const int screen_x,
	const double viewport_center_x,
	const double viewport_size_x,
	const int screen_size_x);

// Translate the game coord value to a pixel value
int game_x_coord_to_screen(
	const double val,
	const double viewport_center_x,
	const double viewport_size_x,
	const int screen_size_x);

// Translate the game coord value to a pixel value
int game_y_coord_to_screen(
	const double val,
	const double viewport_center_y,
	const double viewport_size_y,
	const int screen_size_y);

// Return game coordinates length as integer number of pixels
int game_length_to_screen(
	const double length,
	const double game_length,
	const int num_pixels);

void game_fill_rect_static(
	const double pos_x,
	const double pos_y,
	const double size_x,
	const double size_y,
	const double viewport_center_x,
	const double viewport_center_y,
	const double viewport_size_x,
	const double viewport_size_y,
	const int pixels_x,// Screen surface size
	const int pixels_y,
	SDL_Renderer *const renderer,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b,
	const uint8_t a);

void game_fill_rect(
	const struct game *const game,
	const double pos_x,
	const double pos_y,
	const double size_x,
	const double size_y,
	const int pixels_x,// Screen surface size
	const int pixels_y,
	SDL_Renderer *const renderer,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b,
	const uint8_t a);

void game_render_particle(
	const struct game *const game,
	const struct particle *const particle,
	const int pixels_x,
	const int pixels_y,
	SDL_Renderer *const renderer);

#ifdef __cplusplus
}
#endif

#endif
