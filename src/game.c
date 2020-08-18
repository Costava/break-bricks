#include "game.h"

#include <SDL2/SDL_image.h>

#include "easy_alloc.h"
#include "rand.h"
#include "sdlu.h"

void game_init(struct game *const game, SDL_Renderer *renderer) {
	game->balls_len = 64;
	game->balls = easy_malloc(sizeof(struct ball*) * game->balls_len);
	game->num_balls = 0;

	game->brick_texs_len = 10;
	game->brick_texs = easy_malloc(
		sizeof(SDL_Texture*) * game->brick_texs_len);
	game->num_brick_texs = 0;

	const char* brick_tex_paths[] = {
		"./assets/bark.jpg",
		"./assets/brush.png",
		"./assets/mossy.jpg",
		"./assets/pasta.jpg",
		"./assets/weeds.png",
		NULL};

	while (game->num_brick_texs < game->brick_texs_len &&
		   brick_tex_paths[game->num_brick_texs] != NULL) {
		SDL_Surface *const surf =
			IMG_Load(brick_tex_paths[game->num_brick_texs]);
		SDL_Texture *tex = sdlu_create_texture_from_surface(
			renderer, surf);
		SDL_FreeSurface(surf);

		game->brick_texs[game->num_brick_texs] = tex;
		game->num_brick_texs += 1;
	}

	game->bricks_len = 128;
	game->bricks = easy_malloc(sizeof(struct brick*) * game->bricks_len);
	game->num_bricks = 0;

	game->particles_len = 16384;
	game->particles = easy_malloc(
		sizeof(struct particle*) * game->particles_len);
	game->num_particles = 0;

	game->paddle.pos_x = -300.0;
	game->paddle.pos_y = -1700.0;
	game->paddle.size_x = 600.0;
	game->paddle.size_y = 100.0;

	game->viewport_center_x = 0.0;
	game->viewport_center_y = 0.0;

	game->viewport_size_x = 3200.0;
	game->viewport_size_y = 4200.0;

	game->camera_mass = 1.0;
	game->camera_spring_constant = 0.0001;
	game->camera_vel_x = 0.0;
	game->camera_vel_y = 0.0;

	game->play_area_origin_x = 0.0;
	game->play_area_origin_y = 0.0;
	game->play_area_size_x = 3000.0;
	game->play_area_size_y = 4000.0;

	game->is_setup = false;

	SDL_Surface *const cat_surf = IMG_Load("./assets/cat.png");
	game->ball_tex = sdlu_create_texture_from_surface(renderer, cat_surf);
	SDL_FreeSurface(cat_surf);
}

void game_deinit(struct game *const game) {
	free(game->balls);

	for (unsigned int i = 0; i < game->num_brick_texs; i += 1) {
		SDL_DestroyTexture(game->brick_texs[i]);
	}
	free(game->brick_texs);

	free(game->bricks);

	free(game->particles);

	SDL_DestroyTexture(game->ball_tex);
}

void game_setup(struct game *const game) {
	if (game->is_setup) {
		game_desetup(game);
	}

	game->is_setup = true;

	game->num_bricks = 0;
	game->num_balls = 0;
	game->num_particles = 0;

	// Center the camera
	game->viewport_center_x = game->play_area_origin_x;
	game->viewport_center_y = game->play_area_origin_y;

	game->camera_vel_x = 0.0;
	game->camera_vel_y = 0.0;

	// Make bricks

	const double brick_size_x = 360.0 + rand_double(0.0, 50.0);
	const double brick_size_y = 170.0 + rand_double(0.0, 50.0);

	const double max_y = game->play_area_origin_y + game->play_area_size_y * 0.45;
	const double min_y = game->play_area_origin_y
		- game->play_area_size_y * 0.12
		+ brick_size_y;

	const double brick_margin_x = 10.0;
	const double brick_margin_y = 10.0;

	const double max_x = game->play_area_origin_x
		+ game->play_area_size_x * 0.45
		- (brick_size_x + 2.0 * brick_margin_x);

	const int half_num_columns = max_x / (brick_size_x + 2.0 * brick_margin_x);

	// Create bricks
	for (double y = max_y; y > min_y;
		 y -= brick_size_y + 2.0 * brick_margin_y)
	{
		// Maybe skip a row
		if (rand() < (RAND_MAX * 0.3)) {
			continue;
		}

		 for (double x = -(half_num_columns + 1)
		                 * (brick_size_x + 2.0 * brick_margin_x);
		      x <= half_num_columns * (brick_size_x + 2.0 * brick_margin_x)
		           + 1.0 /* +1 because floating point inaccurary */;
		      x += brick_size_x + 2.0 * brick_margin_x)
		{
			struct brick *brick = easy_malloc(sizeof(struct brick));

			*brick = (struct brick) {
				.pos_x = x + brick_margin_x,
				.pos_y = y - brick_margin_y,
				.size_x = brick_size_x,
				.size_y = brick_size_y
			};

			const int tex_index = rand() % game->num_brick_texs;
			// printf("tex_index: %d\n", tex_index);
			brick->inner_tex = game->brick_texs[tex_index];

			brick->inner_tex_x_prop = rand_double01();
			brick->inner_tex_y_prop = rand_double01();

			brick->inner_tex_w = rand_int(200, 400);
			brick->inner_tex_h =
				brick->inner_tex_w * brick->size_y / brick->size_x;

			brick->inner_tex_x_prop_speed =
				rand_double01() * rand_double01() * rand_double(-1.0, 1.0)
				* 0.0000000009;
			brick->inner_tex_y_prop_speed =
				rand_double01() * rand_double01() * rand_double(-1.0, 1.0)
				* 0.0000000009;

			game_append_brick(game, brick);

			// printf("num_bricks: %d\n", game->num_bricks);
		}
	}

	// Create ball
	struct ball *ball = easy_malloc(sizeof(struct ball));
	*ball = (struct ball) {
		.pos_x = 0.0,
		.pos_y = -1300.0,
		.vel_x =  0.000002,
		.vel_y =  0.000003,
		.size_x = 137.9257,
		.size_y = 300.0,
		.tex = game->ball_tex
	};

	game_append_ball(game, ball);
}

void game_desetup(struct game *const game) {
	if (!game->is_setup) {
		return;
	}

	game->is_setup = false;

	// Free bricks
	for (unsigned int i = 0; i < game->num_bricks; i += 1) {
		free(game->bricks[i]);
	}

	// Free balls
	for (unsigned int i = 0; i < game->num_balls; i += 1) {
		free(game->balls[i]);
	}

	// Free particles
	for (unsigned int i = 0; i < game->num_particles; i += 1) {
		free(game->particles[i]);
	}
}

void game_append_ball(struct game *const game, struct ball *const ball) {
	if (game->num_balls == game->balls_len) {
		game->balls_len = game->balls_len * 2;
		game->balls = easy_realloc(
			game->balls, sizeof(struct ball*) * game->balls_len);
	}
	else if (game->num_balls > game->balls_len) {
		fprintf(stderr, "%s: Buffer overflow detected "
			"[num_balls: %d] [balls_len: %d]",
			__func__, game->num_balls, game->balls_len);

		exit(EXIT_FAILURE);
	}

	game->balls[game->num_balls] = ball;
	game->num_balls += 1;
}

void game_append_particle(
	struct game *const game,
	struct particle *const particle)
{
	if (game->num_particles == game->particles_len) {
		game->particles_len = game->particles_len * 2;
		game->particles = easy_realloc(
			game->particles, sizeof(struct particle*) * game->particles_len);
	}
	else if (game->num_particles > game->particles_len) {
		fprintf(stderr, "%s: Buffer overflow detected "
			"[num_particles: %d] [particles_len: %d]",
			__func__, game->num_particles, game->particles_len);

		exit(EXIT_FAILURE);
	}

	game->particles[game->num_particles] = particle;
	game->num_particles += 1;
}

void game_append_brick(struct game *const game, struct brick *const brick) {
	if (game->num_bricks == game->bricks_len) {
		game->bricks_len = game->bricks_len * 2;
		game->bricks = easy_realloc(
			game->bricks, sizeof(struct ball*) * game->bricks_len);
	}
	else if (game->num_bricks > game->bricks_len) {
		fprintf(stderr, "%s: Buffer overflow detected "
			"[num_bricks: %d] [bricks_len: %d]",
			__func__, game->num_bricks, game->bricks_len);

		exit(EXIT_FAILURE);
	}

	game->bricks[game->num_bricks] = brick;
	game->num_bricks += 1;
}

void game_remove_ball(struct game *const game, const unsigned int i) {
	if (i >= game->num_balls) {
		fprintf(stderr, "%s: Ball index too high. [i: %d] [num_balls: %d]\n",
			__func__, i, game->num_balls);

		exit(EXIT_FAILURE);
	}

	free(game->balls[i]);

	if (game->num_balls > 1) {
		game->balls[i] = game->balls[game->num_balls - 1];
	}

	game->num_balls -= 1;
}

void game_remove_particle(struct game *const game, const unsigned int i) {
	if (i >= game->num_particles) {
		fprintf(stderr, "%s: Particle index too high. "
			"[i: %d] [num_particles: %d]\n",
			__func__, i, game->num_particles);

		exit(EXIT_FAILURE);
	}

	free(game->particles[i]);

	if (game->num_particles > 1) {
		game->particles[i] = game->particles[game->num_particles - 1];
	}

	game->num_particles -= 1;
}

void game_remove_brick(struct game *const game, const unsigned int i) {
	if (i >= game->num_bricks) {
		fprintf(stderr, "%s: Brick index too high. "
			"[i: %d] [num_bricks: %d]\n",
			__func__, i, game->num_bricks);

		exit(EXIT_FAILURE);
	}

	free(game->bricks[i]);

	if (game->num_bricks > 1) {
		game->bricks[i] = game->bricks[game->num_bricks - 1];
	}

	game->num_bricks -= 1;
}

double game_x_screen_to_coord(
	const int screen_x,
	const double viewport_center_x,
	const double viewport_size_x,
	const int screen_size_x)
{
	return ((double)screen_x) / (screen_size_x - 1) * viewport_size_x
		+ ((double)viewport_center_x)
		- (viewport_size_x / 2.0);
}

int game_x_coord_to_screen(
	const double val,
	const double viewport_center_x,
	const double viewport_size_x,
	const int screen_size_x)
{
	const double viewport_left = viewport_center_x - (viewport_size_x / 2.0);

	return round(
		((val - viewport_left) / viewport_size_x) * (screen_size_x - 1));
}

int game_y_coord_to_screen(
	const double val,
	const double viewport_center_y,
	const double viewport_size_y,
	const int screen_size_y)
{
	const double viewport_bottom = viewport_center_y - (viewport_size_y / 2.0);

	return round
		(
			((val - viewport_bottom) / viewport_size_y) * -(screen_size_y - 1)
		)
		+ screen_size_y;
}

int game_length_to_screen(
	const double length,
	const double game_length,
	const int num_pixels)
{
	return round(length * num_pixels / game_length);
}

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
	const uint8_t a)
{
	const int x = game_x_coord_to_screen(
		pos_x,
		viewport_center_x,
		viewport_size_x,
		pixels_x);

	const int y = game_y_coord_to_screen(
		pos_y,
		viewport_center_y,
		viewport_size_y,
		pixels_y);

	const int w = game_length_to_screen(size_x, viewport_size_x, pixels_x);
	const int h = game_length_to_screen(size_y, viewport_size_y, pixels_y);

	sdlu_set_render_draw_color(renderer, r, g, b, a);
	SDL_Rect rect = { .x = x, .y = y, .w = w, .h = h };
	sdlu_render_fill_rect(renderer, &rect);
}

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
	const uint8_t a)
{
	game_fill_rect_static(
		pos_x,
		pos_y,
		size_x,
		size_y,
		game->viewport_center_x,
		game->viewport_center_y,
		game->viewport_size_x,
		game->viewport_size_y,
		pixels_x,
		pixels_y,
		renderer,
		r, g, b, a);
}

void game_render_particle(
	const struct game *const game,
	const struct particle *const particle,
	const int pixels_x,
	const int pixels_y,
	SDL_Renderer *const renderer)
{
	game_fill_rect(
		game,
		particle->pos_x,
		particle->pos_y,
		particle->size_x,
		particle->size_y,
		pixels_x,
		pixels_y,
		renderer,
		particle->r,
		particle->g,
		particle->b,
		particle->a);
}
