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

		for (int i = 0; i < 100; i++)
		{
			int ran1 = rand() % screen_width;
			int ran2 = rand() % screen_height;
			fill_Rectangle(my_own_buffer, screen_width, screen_height, 50*(rand() % 10), 50 * (rand() % 10), 100, 100, rand() % 255, rand() % 255, rand() % 255, 255);
		}
		
		//to_Grayscale(dat, my_own_buffer, screen_width, screen_height);

		//to_Color(my_own_buffer, dat, screen_width, screen_height);

		draw_Line(my_own_buffer, 200, 25, 150, 25, 255, 255, 255, 255);

		memcpy(your_draw_buffer->pixels, my_own_buffer, sizeof(unsigned char)*screen_width*screen_height * 4);



		//BLIT BUFFER TO SCREEN

		SDL_BlitScaled(your_draw_buffer, NULL, screen, NULL);
		SDL_UpdateWindowSurface(window);
	}

	return 0;
}