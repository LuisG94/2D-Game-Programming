//include SDL header
#include "SDL2-2.0.8\include\SDL.h"

//load libraries
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2main.lib")

#pragma comment(linker,"/subsystem:console")
#include <iostream>
#include <assert.h>
using namespace std;

SDL_Renderer *renderer = NULL;
int screen_width = 800;
int screen_height = 600;

SDL_Window *window = NULL;

//Alternative example
struct Pixel
{
	unsigned char r, g, b, a;
};

struct Boxes
{
	float x, y, w, h, sx, sy;
	int alive;
	unsigned char r, g, b, a;
};

struct Border
{
	float x, y, w, h;
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

int impulse_Collision(float x, float y, float w, float h, float *Xvec, float *Yvec, float mass_a, float x1, float y1, float w1, float h1, float *Xvec1, float *Yvec1, float mass_b)
{
	float normal_x;
	float normal_y;

	int k = direction_Collision(x, y, w, h, x1, y1, w1, h1);

	if (k == 1)
	{
		normal_x = 0;
		normal_y = -1;
	}
	else if (k == 2)
	{
		normal_x = 1;
		normal_y = 0;
	}
	else if (k == 3)
	{
		normal_x = 0;
		normal_y = 1;
	}
	else if (k == 4)
	{
		normal_x = -1;
		normal_y = 0;
	}
	else
	{
		normal_x = 0;
		normal_y = 0;
		return 0;
	}

	float relative_velocity_x = *Xvec1 - *Xvec;
	float relative_velocity_y = *Yvec1 - *Yvec;

	float dot = (relative_velocity_x * normal_x) + (relative_velocity_y * normal_y);

	if (dot < 0) return 1;

	float bounce = 1.0; //1.0 bounce, 0.0 mud
	float mass = -(1.0 + bounce) * dot / (mass_a + mass_b);

	float impulse_a_x = normal_x * mass * mass_a;
	float impulse_a_y = normal_y * mass * mass_a;

	float impulse_b_x = normal_x * mass * mass_b;
	float impulse_b_y = normal_y * mass * mass_b;

	*Xvec -= impulse_a_x;
	*Yvec -= impulse_a_y;

	*Xvec1 += impulse_b_x;
	*Yvec1 += impulse_b_y;

	float mt = 1.0 / (mass_a + mass_b);

	float tangent_x = normal_y;
	float tangent_y = -normal_x;

	float tangent_dot = (relative_velocity_x * tangent_x) + (relative_velocity_y * tangent_y);

	tangent_x = tangent_x * -tangent_dot * mt;
	tangent_y = tangent_y * -tangent_dot * mt;

	*Xvec -= tangent_x;
	*Yvec -= tangent_y;

	*Xvec1 += tangent_x;
	*Yvec1 += tangent_y;
	return 1;
}

int impulse_Immovable_Object(float x, float y, float w, float h, float *Xvec, float *Yvec, float mass_a, float x1, float y1, float w1, float h1)
{
	float normal_x;
	float normal_y;

	int k = direction_Collision(x, y, w, h, x1, y1, w1, h1);

	if (k == 1)
	{
		normal_x = 0;
		normal_y = -1;
	}
	else if (k == 2)
	{
		normal_x = 1;
		normal_y = 0;
	}
	else if (k == 3)
	{
		normal_x = 0;
		normal_y = 1;
	}
	else if (k == 4)
	{
		normal_x = -1;
		normal_y = 0;
	}
	else
	{
		normal_x = 0;
		normal_y = 0;
		return 0;
	}

	x = -*Xvec;
	y = -*Yvec;

	float dot = (x * normal_x) + (y * normal_y);

	if (dot < 0) return 1;

	float e = 1.0;
	float j = -(1.0 + e) * dot / (mass_a);

	float impulse_x = normal_x;
	float impulse_y = normal_y;

	impulse_x = impulse_x * j;
	impulse_y = impulse_y * j;

	*Xvec -= impulse_x;
	*Yvec -= impulse_y;
	return 1;
	
}

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(
		"POWEERRRRRR",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window,
		-1, SDL_RENDERER_SOFTWARE);

	SDL_Surface *your_draw_buffer = SDL_CreateRGBSurfaceWithFormat(0, screen_width, screen_height, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Surface *screen = SDL_GetWindowSurface(window);
	SDL_SetSurfaceBlendMode(your_draw_buffer, SDL_BLENDMODE_NONE);
	float *data = new float[screen_width * screen_height];

	unsigned char *my_own_buffer = (unsigned char*)malloc(sizeof(unsigned char)*screen_width*screen_height * 4);

	const int max_boxes = 5;
	float box_size = 25;

	Boxes *boxes = (Boxes*)malloc(sizeof(Boxes)* max_boxes);

	for (int i = 0; i < max_boxes; i++)
	{
		boxes[i].x = rand() % 740 + 30;
		boxes[i].y = rand() % 540 + 30;
		boxes[i].w = box_size;
		boxes[i].h = box_size;
		boxes[i].sx = 1 - 2.0 * rand() / RAND_MAX;
		boxes[i].sy = 1 - 2.0 * rand() / RAND_MAX;
		boxes[i].alive = 0;
		boxes[i].r = rand() % 255;
		boxes[i].g = rand() % 255;
		boxes[i].b = rand() % 255;
		boxes[i].a = 255;
	}

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

	float box_mass1 = 1;
	int collision;
	int bounces = 0;//counter for number of bounces

	unsigned int lastTime = 0, currentTime, elapsed_time;
	int spawn_time = 2000;
	int spawn = 0;
	
	int start_time = SDL_GetTicks();

	for (;;)
	{
		//consume all window events first
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
		}

		//screen buffer
		for (int i = 0; i < screen_width*screen_height; i++)
		{
			my_own_buffer[i * 4] = 0;//r
			my_own_buffer[i * 4 + 1] = 0;//g
			my_own_buffer[i * 4 + 2] = 0;//b
			my_own_buffer[i * 4 + 3] = 0;//a
		}

		currentTime = SDL_GetTicks();

		//printf("time inside forloop: %d\n", currentTime);

		elapsed_time = currentTime - start_time;

		//printf("elapsed time-----> %d\n",elapsed_time);

		if (elapsed_time >= spawn_time)
		{
			spawn = 1;
			start_time = SDL_GetTicks();
		}
		
		for (int i = 0; i < max_boxes; i++)
		{
			if (spawn == 1)
			{
				if (boxes[i].alive == 0)
				{
					spawn = 0;
					boxes[i].alive = 1;
				}
			}

			for (int j = i + 1; j < max_boxes; j++)
			{
				collision = impulse_Collision(boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h, &boxes[i].sx, &boxes[i].sy, 1, boxes[j].x, boxes[j].y, boxes[j].w, boxes[j].h, &boxes[j].sx, &boxes[j].sy, 1);

				/*if (collision == 1)
				{
					bounces++;
					printf("bounces----------%d\n", bounces);

					if (bounces >= 3)
					{
						boxes[j].alive = 0;
					}
				}*/
			}

			//Top Wall
			fill_Rectangle(my_own_buffer, screen_width, screen_height, top_wall.x, top_wall.y, top_wall.w, top_wall.h, top_wall.r, top_wall.g, top_wall.b, top_wall.a);
			int top_collision = impulse_Immovable_Object(boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h, &boxes[i].sx, &boxes[i].sy, box_mass1, top_wall.x, top_wall.y, top_wall.w, top_wall.h);
			
			//Left Wall
			fill_Rectangle(my_own_buffer, screen_width, screen_height, left_wall.x, left_wall.y, left_wall.w, left_wall.h, left_wall.r, left_wall.g, left_wall.b, left_wall.a);
			int left_collision = impulse_Immovable_Object(boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h, &boxes[i].sx, &boxes[i].sy, box_mass1, left_wall.x, left_wall.y, left_wall.w, left_wall.h);
			
			//Bottom Wall
			fill_Rectangle(my_own_buffer, screen_width, screen_height, bottom_wall.x, bottom_wall.y, bottom_wall.w, bottom_wall.h, bottom_wall.r, bottom_wall.g, bottom_wall.b, bottom_wall.a);
			int bottom_collision = impulse_Immovable_Object(boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h, &boxes[i].sx, &boxes[i].sy, box_mass1, bottom_wall.x, bottom_wall.y, bottom_wall.w, bottom_wall.h);
			
			//Right Wall
			fill_Rectangle(my_own_buffer, screen_width, screen_height, right_wall.x, right_wall.y, right_wall.w, right_wall.h, right_wall.r, right_wall.g, right_wall.b, right_wall.a);
			int right_collision = impulse_Immovable_Object(boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h, &boxes[i].sx, &boxes[i].sy, box_mass1, right_wall.x, right_wall.y, right_wall.w, right_wall.h);

			if (top_collision == 1 || left_collision == 1 || bottom_collision == 1 || right_collision == 1)
			{
				bounces++;
				printf("bounces-------->%d\n",bounces);
			}


			boxes[i].x += boxes[i].sx;
			boxes[i].y += boxes[i].sy;

			if (boxes[i].alive == 1)
			{
				fill_Rectangle(my_own_buffer, screen_width, screen_width, boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h, boxes[i].r, boxes[i].g, boxes[i].b, boxes[i].a);
			}
		}
		
		memcpy(your_draw_buffer->pixels, my_own_buffer, sizeof(unsigned char)*screen_width*screen_height * 4);

		//BLIT BUFFER TO SCREEN

		SDL_BlitScaled(your_draw_buffer, NULL, screen, NULL);
		SDL_UpdateWindowSurface(window);
	}
	return 0;
}