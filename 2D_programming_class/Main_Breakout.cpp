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
	int alive, w, h;
	float x, y;
	unsigned char r, g, b, a;
};

struct Walls
{
	int x, y;
	int w, h;
};

struct Ball
{
	int size;
	float x, y;
	float fx, fy;
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

void impulse_Collision(float x, float y, float w, float h, float *Xvec, float *Yvec, float mass_a, float x1, float y1, float w1, float h1, float *Xvec1, float *Yvec1, float mass_b)
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
	}

	float relative_velocity_x = *Xvec1 - *Xvec;
	float relative_velocity_y = *Yvec1 - *Yvec;

	float dot = (relative_velocity_x * normal_x) + (relative_velocity_y * normal_y);

	if (dot < 0) return;

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

	float box_size = 30;
	float box_mass1 = 1;
	const int max_boxes = 40;
	int paddle_height = 20;
	int paddle_width = 200;
	int paddle_x = 300;
	int paddle_y = screen_height - 50;
	int max_rows = 4;
	int max_cols = 10;
	//int box_x = 65;
	//int box_y = 100;

	int bounce = 0;//counter for number of bounces

	Boxes *boxes = (Boxes*)malloc(sizeof(Boxes)* max_boxes);

	for (int i = 0; i < max_boxes; i++)
	{
		boxes[i].alive = 1;
		boxes[i].x = 65;
		boxes[i].y = 100;
		boxes[i].w = 50;
		boxes[i].h = 30;
		boxes[i].r = 0;
		boxes[i].g = 250;
		boxes[i].b = 0;
		boxes[i].a = 255;
	}
	
	
	Ball main_ball;
	main_ball.size = 15;
	main_ball.x = paddle_x + 90;
	main_ball.y = paddle_y - 40;
	main_ball.fx = 1;
	main_ball.fy = 1;
	main_ball.r = 255;
	main_ball.g = 255;
	main_ball.b = 255;
	main_ball.a = 255;
	
	Walls top_wall;
	top_wall.x = 0;
	top_wall.y = 0;
	top_wall.w = screen_width;
	top_wall.h = 10;

	Walls bottom_wall;
	bottom_wall.x = 0;
	bottom_wall.y = screen_height - 10;
	bottom_wall.w = screen_width;
	bottom_wall.h = 10;

	Walls left_wall;
	left_wall.x = 0;
	left_wall.y = 0;
	left_wall.w = 10;
	left_wall.h = screen_height;

	Walls right_wall;
	right_wall.x = screen_width - 10;
	right_wall.y = 0;
	right_wall.w = 10;
	right_wall.h = screen_height;

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

		//Top Wall
		fill_Rectangle(my_own_buffer, screen_width, screen_height, top_wall.x, top_wall.y, top_wall.w, top_wall.h, 255, 0, 0, 255);
		impulse_Immovable_Object(main_ball.x, main_ball.y, main_ball.size, main_ball.size, &main_ball.fx, &main_ball.fy, 1, top_wall.x, top_wall.y, top_wall.w, top_wall.h);

		//Left Wall
		fill_Rectangle(my_own_buffer, screen_width, screen_height, left_wall.x, left_wall.y, left_wall.w, left_wall.h, 255, 0, 0, 255);
		impulse_Immovable_Object(main_ball.x, main_ball.y, main_ball.size, main_ball.size, &main_ball.fx, &main_ball.fy, 1, left_wall.x, left_wall.y, left_wall.w, left_wall.h);

		//Bottom Wall
		fill_Rectangle(my_own_buffer, screen_width, screen_height, bottom_wall.x, bottom_wall.y, bottom_wall.w, bottom_wall.h, 255, 0, 0, 255);
		impulse_Immovable_Object(main_ball.x, main_ball.y, main_ball.size, main_ball.size, &main_ball.fx, &main_ball.fy, 1, bottom_wall.x, bottom_wall.y, bottom_wall.w, bottom_wall.h);

		//Right Wall
		fill_Rectangle(my_own_buffer, screen_width, screen_height, right_wall.x, right_wall.y, right_wall.w, right_wall.h, 255, 0, 0, 255);
		impulse_Immovable_Object(main_ball.x, main_ball.y, main_ball.size, main_ball.size, &main_ball.fx, &main_ball.fy, 1, right_wall.x, right_wall.y, right_wall.w, right_wall.h);

		//paddle
		fill_Rectangle(my_own_buffer, screen_width, screen_height, paddle_x, paddle_y, paddle_width, paddle_height, 255, 255, 255, 255);
		impulse_Immovable_Object(main_ball.x, main_ball.y, main_ball.size, main_ball.size, &main_ball.fx, &main_ball.fy, 1, paddle_x, paddle_y, paddle_width, paddle_height);

		//ball
		fill_Rectangle(my_own_buffer, screen_width, screen_height, main_ball.x, main_ball.y, main_ball.size, main_ball.size, main_ball.r, main_ball.g, main_ball.b, main_ball.a);

		main_ball.x += main_ball.fx;
		main_ball.y += main_ball.fy;

		int box_y = 100;
		int box_x = 65;
		int x;
		for (int i = 0; i < 5; i++)
		{
			//boxes[i].x = 65;
			//boxes[i].y = 100;
			if(boxes[i].alive == 1)
			{
				fill_Rectangle(my_own_buffer, screen_width, screen_height, box_x, box_y, boxes[i].w, boxes[i].h, boxes[i].r, boxes[i].g, boxes[i].b, boxes[i].a);
				x = impulse_Immovable_Object(main_ball.x, main_ball.y, main_ball.size, main_ball.size, &main_ball.fx, &main_ball.fy, 1, boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h);
			}

			if (x == 1)
			{
				boxes[i].alive = 0;
			}

			box_x = box_x + 70;
		}
		

		//draw boxes in rows and cols
		/*for (int i = 0; i < max_rows; i++)
		{
			//int box_x = 65;
			boxes[i].x = 65;
			//boxes[i].y = box_y;

			for (int j = 0; j < max_cols; j++)
			{
				
				if (boxes[i].alive > 0)
				{
					fill_Rectangle(my_own_buffer, screen_width, screen_height, boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h, boxes[i].r, boxes[i].g, boxes[i].b, boxes[i].a);
					int x = impulse_Immovable_Object(main_ball.x, main_ball.y, main_ball.size, main_ball.size, &main_ball.fx, &main_ball.fy, 1, boxes[i].x, boxes[i].y, boxes[i].w, boxes[i].h);
					if (x == 1)
						boxes[i].alive = 0;

				}
				//if (boxes[i].alive <= 0) continue;
				
				//printf("%d\n", x);


				boxes[i].x = boxes[i].x + 70;
			}
			//boxes[i].y = boxes[i].y + 60;
		}*/

		memcpy(your_draw_buffer->pixels, my_own_buffer, sizeof(unsigned char)*screen_width*screen_height * 4);

		//BLIT BUFFER TO SCREEN

		SDL_BlitScaled(your_draw_buffer, NULL, screen, NULL);
		SDL_UpdateWindowSurface(window);

		//screen buffer
		for (int i = 0; i < screen_width*screen_height; i++)
		{
			my_own_buffer[i * 4] = 0;//r
			my_own_buffer[i * 4 + 1] = 0;//g
			my_own_buffer[i * 4 + 2] = 0;//b
			my_own_buffer[i * 4 + 3] = 0;//a
		}
	}

	return 0;
}