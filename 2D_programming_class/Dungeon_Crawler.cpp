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

	

	SDL_Rect Dest_Rect;


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

	
		//draw game here
		//use your own drawing function on the unsignedchar bugffer
		fill_Rectangle(my_own_buffer, screen_width, screen_height, 50, 20, 200, 60, 255, 0, 0, 255);
		//draw game done

		//figure out what to do with my_own_buffer for lights
		//possibly copy/modify pixels into another buffer
		//LIGHT CODE
		fill_Rectangle(my_own_buffer, screen_width, screen_height, 50, 20, 60, 60, 255, 255, 0, 255);

		int mx, my;
		SDL_GetMouseState(&mx, &my);

		float light_x = mx;
		float light_y = my;
		for (int i = 0; i < screen_height; i++)
		{
			for (int j = 0; j < screen_width; j++)
			{
				
				//calculate distance from pixel to light
				//calculate intensity of light
				float dist_sqr = (light_x - j)*(light_x - j) + (light_y - i)*(light_y - i);
				float one_over_dist_sqr = 100.0 / dist_sqr;

				float r = my_own_buffer[i*screen_width * 4 + j * 4];
				r *= one_over_dist_sqr;
				if (r > 255) r = 255;
				float g = my_own_buffer[i*screen_width * 4 + j * 4 + 1];
				g *= one_over_dist_sqr;
				if (g > 255) g = 255;
				float b = my_own_buffer[i*screen_width * 4 + j * 4 + 2];
				b *= one_over_dist_sqr;
				if (b> 255) b = 255;
				my_own_buffer[i*screen_width * 4 + j * 4] = r;
				my_own_buffer[i*screen_width * 4 + j * 4 + 1] = g;
				my_own_buffer[i*screen_width * 4 + j * 4 + 2] = b;
			}
		}
		//LOGHT CODE
		

		//SDL steps to copy your unsignedchar buffer into screen
		memcpy(your_draw_buffer->pixels, my_own_buffer, sizeof(unsigned char)*screen_width*screen_height * 4);
		SDL_BlitScaled(your_draw_buffer, NULL, screen, NULL);
		SDL_UpdateWindowSurface(window);
	}

	return 0;
}