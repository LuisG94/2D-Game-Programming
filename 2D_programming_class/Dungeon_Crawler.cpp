#include <iostream>
#include <assert.h>
using namespace std;

//include SDL header
#include "SDL2-2.0.8\include\SDL.h"
#include "SDL2-2.0.8\include\SDL_image.h"

//load libraries
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2main.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2_image.lib")

#pragma comment(linker,"/subsystem:console")

SDL_Renderer *renderer = NULL;
int screen_width = 800;
int screen_height = 600;

unsigned char prev_key_state[256];
unsigned char *keys = NULL;
SDL_Window *window = NULL;

struct Boxes
{
	float x, y, w, h, health_lvl, attack_pwr, mana_lvl, xvel, yvel;
	unsigned char r, g, b, a;
	int alive;
};

struct Border
{
	float x, y, w, h;
	unsigned char r, g, b, a;
};

struct Pixel
{
	unsigned char r, g, b, a;
};

void set_Pixel_Alternative(unsigned char* buffer, int width, int x, int y, int r, int g, int b, int a)
{
	Pixel *p = (Pixel*)buffer;
	p[y*width + x].r = r;
	p[y*width + x].g = g;
	p[y*width + x].b = b;
	p[y*width + x].a = a;
}

void fill_Rectangle(unsigned char*buffer, int buffer_width, int buffer_height, int rect_x, int rect_y, int rect_w, int rect_h, int r, int g, int b, int a)
{
	for (int i = 0; i < rect_w; i++)
	{
		for (int j = 0; j < rect_h; j++)
		{
			int x = i + rect_x;
			int y = j + rect_y;
			set_Pixel_Alternative(buffer, screen_width, x, y, r, g, b, a);
		}
	}

}

int direction_Collision(float x, float y, float w, float h, float x1, float y1, float w1, float h1)
{
	int no_collision = 0;
	int top_collision = 1;
	int right_collision = 2;
	int bottom_collision = 3;
	int left_collision = 4;

	float W = 0.5 * (w + w1);
	float H = 0.5 * (h + h1);
	float dx = x - x1 + 0.5*(w - w1);
	float dy = y - y1 + 0.5*(h - h1);

	if (dx*dx <= W * W && dy*dy <= H * H)
	{
		float wy = W * dy;
		float hx = H * dx;

		if (wy > hx)
		{
			if (wy + hx > 0)
			{
				return bottom_collision;
			}
			else
				return left_collision;
		}
		else
		{
			if (wy + hx > 0)
			{
				return right_collision;
			}
			else
				return top_collision;
		}
	}
	return no_collision;
}

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	prev_key_state[256];
	keys = (unsigned char*)SDL_GetKeyboardState(NULL);

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;

	window = SDL_CreateWindow(
		"Dungeon Crawler",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window,
		-1, SDL_RENDERER_SOFTWARE);

	SDL_Surface *your_draw_buffer = SDL_CreateRGBSurfaceWithFormat(0, screen_width, screen_height, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Surface *screen = SDL_GetWindowSurface(window);
	SDL_SetSurfaceBlendMode(your_draw_buffer, SDL_BLENDMODE_NONE);
	float *data = new float[screen_width * screen_height];

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);

	unsigned char *my_own_buffer = (unsigned char*)malloc(sizeof(unsigned char)*screen_width*screen_height * 4);

	//unsigned char *diff_buffer = (unsigned char*)malloc(sizeof(unsigned char)*screen_width*screen_height * 4);

	const int max_enemies = 8;

	Boxes *enemies = (Boxes*)malloc(sizeof(Boxes)*max_enemies);

	for (int i = 0; i < max_enemies; i++)
	{
		enemies[i].x = rand() % 740;
		enemies[i].y = rand() % 540;
		enemies[i].w = 20;
		enemies[i].h = 30;
		enemies[i].alive = 0;
		enemies[i].r = 115;
		enemies[i].g = 115;
		enemies[i].b = 115;
		enemies[i].a = 255;
	}

	Boxes player;
	player.x = 50;
	player.y = 50;
	player.w = 25;
	player.h = 35;
	player.health_lvl = 100;
	player.attack_pwr = 20;
	player.mana_lvl = 5;
	player.xvel = 1;
	player.yvel = 1;

	Boxes enemy;
	enemy.x = 350;
	enemy.y = 250;
	enemy.w = 25;
	enemy.h = 55;
	enemy.health_lvl = 100;

	Border top_wall;
	top_wall.x = 0;
	top_wall.y = 0;
	top_wall.w = screen_width;
	top_wall.h = 20;
	top_wall.r = 0;
	top_wall.g = 255;
	top_wall.b = 0;
	top_wall.a = 255;

	Border bottom_wall;
	bottom_wall.x = 0;
	bottom_wall.y = screen_height - 20;
	bottom_wall.w = screen_width;
	bottom_wall.h = 20;
	bottom_wall.r = 0;
	bottom_wall.g = 255;
	bottom_wall.b = 0;
	bottom_wall.a = 255;

	Border left_wall;
	left_wall.x = 0;
	left_wall.y = 0;
	left_wall.w = 20;
	left_wall.h = screen_height;
	left_wall.r = 0;
	left_wall.g = 255;
	left_wall.b = 0;
	left_wall.a = 255;

	Border right_wall;
	right_wall.x = screen_width - 20;
	right_wall.y = 0;
	right_wall.w = 20;
	right_wall.h = screen_height;
	right_wall.r = 0;
	right_wall.g = 255;
	right_wall.b = 0;
	right_wall.a = 255;

	//SDL_Rect Dest_Rect;


	for (;;)
	{
		memcpy(prev_key_state, keys, 256);
		//SDL_Rect characer_box;
		//SDL_Rect enemy_box;

		//characer_box.x = player.x; characer_box.y = player.y; characer_box.h = player.h; characer_box.w = player.w;
		//enemy_box.x = enemy.x; enemy_box.y = enemy.y; enemy_box.w = enemy.w; enemy_box.h = enemy.h;

		//consume all window events first
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
		}

		if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A])
		{
			player.x--;
		}
		if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D])
		{
			player.x++;
		}
		if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W])
		{
			player.y--;
		}
		if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S])
		{
			player.y++;
		}

		int enemy_distance_x = (enemy.x) - (player.x + player.w);
		printf("The X distance between the player and the enemy is: %d\n", enemy_distance_x);
		int enemy_distance_y = (enemy.y + enemy.h) - (player.y + player.h);
		printf("The Y distance between the player and the enemy is: %d\n", enemy_distance_y);

		if (enemy_distance_x <= 10 && keys[SDL_SCANCODE_F])
		{
			enemy.health_lvl -= 10;
			printf("The enemy's level: %d\n", enemy.health_lvl);
			if (enemy.health_lvl <= 0)
			{
				printf("The enemy is dead\n");
				//SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				//SDL_RenderFillRect(renderer, &enemy_box);

			}
		}

		//wall collision
		if (player.x <= 20)//left
		{
			player.x += 1;
		}
		if (player.x + player.w >= screen_width - 20)//right
		{
			player.x -= 1;
		}
		if (player.y <= 20)//top
		{
			player.y += 1;
		}
		if (player.y + player.h >= screen_height - 20)//bottom
		{
			player.y -= 1;
		}

		for (int i = 0; i < max_enemies; i++)
		{
			int x = direction_Collision(player.x, player.y, player.w, player.h, enemies[i].x, enemies[i].y, enemies[i].w, enemies[i].h);

			if (x == 1)//top
			{
				player.y -= 1;
			}
			if (x == 2)//right
			{
				player.x += 1;
			}
			if (x == 3)//bottom
			{
				player.y += 1;
			}
			if (x == 4)//left
			{
				player.x -= 1;
			}
		}
		

		


	
		//draw game here
		//use your own drawing function on the unsigned char buffer

		


		//SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		//SDL_RenderClear(renderer);

		//draw box for character
		
		
		//SDL_SetRenderDrawColor(renderer, 255, 240, 0, 255);
		//SDL_RenderFillRect(renderer, &characer_box);
		//SDL_SetRenderDrawColor(renderer, 255, 12, 144, 255);
		//SDL_RenderFillRect(renderer, &enemy_box);

		//SDL_RenderPresent(renderer);
		//SDL_RenderClear(renderer);

		//SDL_RenderCopy(renderer, texture);

		//draw game done

		//figure out what to do with my_own_buffer for lights
		//possibly copy/modify pixels into another buffer

		//LIGHT CODE
		//fill_Rectangle(my_own_buffer, screen_width, screen_height, 50, 20, 60, 60, 255, 255, 0, 255);

		//int mx, my;
		//SDL_GetMouseState(&mx, &my);

		//float light_x = mx;
		//float light_y = my;
		//for (int i = 0; i < screen_height; i++)
		//{
		//	for (int j = 0; j < screen_width; j++)
		//	{
		//		
		//		//calculate distance from pixel to light
		//		//calculate intensity of light
		//		float dist_sqr = (light_x - j)*(light_x - j) + (light_y - i)*(light_y - i);
		//		float one_over_dist_sqr = 500.0 / dist_sqr;

		//		float r = my_own_buffer[i*screen_width * 4 + j * 4];
		//		r *= one_over_dist_sqr;
		//		if (r > 255) r = 255;
		//		float g = my_own_buffer[i*screen_width * 4 + j * 4 + 1];
		//		g *= one_over_dist_sqr;
		//		if (g > 255) g = 255;
		//		float b = my_own_buffer[i*screen_width * 4 + j * 4 + 2];
		//		b *= one_over_dist_sqr;
		//		if (b> 255) b = 255;
		//		my_own_buffer[i*screen_width * 4 + j * 4] = r;
		//		my_own_buffer[i*screen_width * 4 + j * 4 + 1] = g;
		//		my_own_buffer[i*screen_width * 4 + j * 4 + 2] = b;
		//	}
		//}


		for (int i = 0; i < screen_width*screen_height; i++)
		{
			my_own_buffer[i * 4] = 0;
			my_own_buffer[i * 4 + 1] = 0;
			my_own_buffer[i * 4 + 2] = 0;
			my_own_buffer[i * 4 + 3] = 0;
		}

		//----player box
		fill_Rectangle(my_own_buffer, screen_width, screen_height, player.x, player.y, player.w, player.h, 255, 0, 0, 255);
		//----enemy box
		//fill_Rectangle(my_own_buffer, screen_width, screen_height, enemy.x, enemy.y, enemy.w, enemy.h, 0, 255, 200, 255);
		//-------borders
		fill_Rectangle(my_own_buffer, screen_width, screen_height, top_wall.x, top_wall.y, top_wall.w, top_wall.h, top_wall.r, top_wall.g, top_wall.b, top_wall.a);
		fill_Rectangle(my_own_buffer, screen_width, screen_height, left_wall.x, left_wall.y, left_wall.w, left_wall.h, left_wall.r, left_wall.g, left_wall.b, left_wall.a);
		fill_Rectangle(my_own_buffer, screen_width, screen_height, bottom_wall.x, bottom_wall.y, bottom_wall.w, bottom_wall.h, bottom_wall.r, bottom_wall.g, bottom_wall.b, bottom_wall.a);
		fill_Rectangle(my_own_buffer, screen_width, screen_height, right_wall.x, right_wall.y, right_wall.w, right_wall.h, right_wall.r, right_wall.g, right_wall.b, right_wall.a);

		for (int i = 0; i < max_enemies; i++)
		{
			fill_Rectangle(my_own_buffer, screen_width, screen_height, enemies[i].x, enemies[i].y, enemies[i].w, enemies[i].h, enemies[i].r, enemies[i].g, enemies[i].b, enemies[i].a);
		}
		
		//SDL steps to copy your unsignedchar buffer into screen
		memcpy(your_draw_buffer->pixels, my_own_buffer, sizeof(unsigned char)*screen_width*screen_height * 4);
		SDL_BlitScaled(your_draw_buffer, NULL, screen, NULL);
		SDL_UpdateWindowSurface(window);
	}

	return 0;
}