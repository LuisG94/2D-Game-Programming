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

void set_Pixel(unsigned char* buffer, int width, int x, int y, int r, int g, int b, int a)
{
	int first_byte_of_the_pixel = y * width * 3 + x * 3;
	buffer[first_byte_of_the_pixel] = r;
	buffer[first_byte_of_the_pixel + 1] = g;
	buffer[first_byte_of_the_pixel + 2] = b;
	buffer[first_byte_of_the_pixel + 3] = a;
}

//Alternative example
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

void to_Grayscale(float *dest, unsigned char *src, int w, int h)
{
	for (int i = 0; i < screen_width*screen_height; i++)
	{
		Pixel *p = (Pixel*)src;
		dest[i] = (p[i].r + p[i].g + p[i].b)/(255.0*3.0);
		
	}
}

void to_Color(unsigned char *dest, float *src, int w, int h)
{
	for (int i = 0; i < screen_width*screen_height; i++)
	{
		Pixel *p = (Pixel*)dest;
		p[i].r = 255.0 * src[i];
		p[i].g = 255.0 * src[i];
		p[i].b = 255.0 * src[i];
		p[i].a = 255.0;
	}
}

void draw_Line(unsigned char *dest, int x0, int y0, int x1, int y1, int r, int g, int b, int a)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	//if (x0 == x1)
	//{
	//	for ()
	//	{

	//	}
	//}
	for (int x = 0; x < dx; x++)
	{
		int y = y0 + dy * (x - x0) / dx;
		set_Pixel_Alternative(dest, screen_width, x, y, r, g, b, a);
	}
}

int Direction_Collision(float *x, float *y, float *w, float *h, float *x1, float *y1, float *w1, float *h1)
{
	int no_collision = 0;
	int top_collision = 1;
	int right_collision = 2;
	int bottom_collision = 3;
	int left_collision = 4;

	float W = 0.5 * (*w + *w1);
	float H = 0.5 * (*h + *h1);
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

void Impulse_Collision(float *x, float *y, float *w, float *h, float *Xvec, float *Yvec, float mass_a, float *x1, float *y1, float *w1, float *h1, float *Xvec1, float *Yvec1, float mass_b)
{
	float normal_x;
	float normal_y;

	int k = Direction_Collision(x, y, w, h, x1, y1, w1, h1);

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

	float relative_velocity_x = *Xvec1;
	float relative_velocity_y = *Yvec1;

	float subtract_x = (&relative_velocity_x - Xvec);
	float subtract_y = (&relative_velocity_y - Yvec);

	float dot = (relative_velocity_x * normal_x) + (relative_velocity_y * normal_y);

	if (dot < 0) return;

	float bounce = 1.0; //1.0 bounce, 0.0 mud
	float mass = -(1.0 + bounce) * dot / (mass_a + mass_b);

	float impulse_a_x = normal_x;
	float impulse_a_y = normal_y;

	impulse_a_x *= mass * mass_a;//scaling
	impulse_a_y *= mass * mass_a;

	float impulse_b_x = normal_x;
	float impulse_b_y = normal_y;

	impulse_b_x *= mass * mass_b;//scaling
	impulse_b_y *= mass * mass_b;

	float sub_x = (Xvec - &impulse_a_x);
	float sub_y = (Yvec - &impulse_a_y);

	float add_x = (Xvec1 - &impulse_b_x);
	float add_y = (Yvec1 - &impulse_b_y);

	float mt = 1.0 / (mass_a + mass_b);

	float tangent_x = normal_y;
	float tangent_y = -normal_x;

	float tangent_dot = (relative_velocity_x * tangent_x) + (relative_velocity_y * tangent_y);

	tangent_x *= -tangent_dot * mt;
	tangent_y *= -tangent_dot * mt;

	float sub_x_1 = (Xvec - &tangent_x);
	float sub_y_1 = (Yvec - &tangent_y);

	float add_x_1 = (Xvec1 - &tangent_x);
	float add_y_1 = (Yvec1 - &tangent_y);
}

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	prev_key_state[256];
	keys = (unsigned char*)SDL_GetKeyboardState(NULL);

	window = SDL_CreateWindow(
		"POWEERRRRRR",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window,
		-1, SDL_RENDERER_SOFTWARE);

	SDL_Surface *your_draw_buffer = SDL_CreateRGBSurfaceWithFormat(0, screen_width, screen_height, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Surface *screen = SDL_GetWindowSurface(window);
	SDL_SetSurfaceBlendMode(your_draw_buffer, SDL_BLENDMODE_NONE);
	float *dat = new float[screen_width * screen_height];

	unsigned char *my_own_buffer = (unsigned char*)malloc(sizeof(unsigned char)*screen_width*screen_height * 4);

	for (;;)
	{
		memcpy(prev_key_state, keys, 256);

		

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
			my_own_buffer[i * 4] = 200;
			my_own_buffer[i * 4 + 1] = 200;
			my_own_buffer[i * 4 + 2] = 200;
			my_own_buffer[i * 4 + 3] = 0;
		}

		/*for (int i = 0; i < 100; i++)
		{
			int ran1 = rand() % screen_width;
			int ran2 = rand() % screen_height;
			fill_Rectangle(my_own_buffer, screen_width, screen_height, 50*(rand() % 10), 50 * (rand() % 10), 100, 100, rand() % 255, rand() % 255, rand() % 255, 255);
		}*/

		fill_Rectangle(my_own_buffer, screen_width, screen_height, 25, 50, 60, 60, 255, 255, 255, 255);
		fill_Rectangle(my_own_buffer, screen_width, screen_height, 220, 50, 60, 60, 255, 255, 255, 255);
		
		//to_Grayscale(dat, my_own_buffer, screen_width, screen_height);

		//to_Color(my_own_buffer, dat, screen_width, screen_height);

		//draw_Line(my_own_buffer, 200, 25, 150, 25, 255, 255, 255, 255);

		memcpy(your_draw_buffer->pixels, my_own_buffer, sizeof(unsigned char)*screen_width*screen_height * 4);



		//BLIT BUFFER TO SCREEN

		SDL_BlitScaled(your_draw_buffer, NULL, screen, NULL);
		SDL_UpdateWindowSurface(window);
	}

	return 0;
}