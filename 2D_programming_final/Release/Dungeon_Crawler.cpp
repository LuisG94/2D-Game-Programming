#pragma warning(disable:4996)
#include <iostream>
#include <assert.h>
#include <time.h>
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

struct Vec2D
{
	float x, y;
};

struct Sprite
{
	SDL_Texture* texture;
	Vec2D frame_position;
	Vec2D frame_size;
	unsigned int animation_speed;
	int num_of_frames;

	Vec2D* pos;
	Vec2D* vel;
	Vec2D* size;

	unsigned int* life;
	unsigned int* creation_time;
	unsigned int* last_frame_update;
	unsigned char* current_frame;

	unsigned char* active;

	int arr_size;

};

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

float distance_from_enemy(Boxes p, Boxes *e)
{
	float enemy_distance_x = (e->x) - (p.x + p.w);
	printf("The X distance between the player and the enemy is: %d\n", enemy_distance_x);
	float enemy_distance_y = (e->y + e->h) - (p.y + p.h);
	printf("The Y distance between the player and the enemy is: %d\n", enemy_distance_y);

	return enemy_distance_x;
}

void init(Sprite *s, int arr_size, Vec2D *frame_pos, Vec2D *frame_size, SDL_Texture *texture, unsigned int animation_speed)
{
	s->frame_position = *frame_pos;
	s->frame_size = *frame_size;
	s->animation_speed = animation_speed;
	s->texture = texture;
	s->num_of_frames = 4;
	s->arr_size = arr_size;
	s->active = (unsigned char*)malloc(sizeof(unsigned char) * arr_size);
	s->current_frame = (unsigned char*)malloc(sizeof(unsigned char) * arr_size);
	s->creation_time = (unsigned int*)malloc(sizeof(unsigned int) * arr_size);
	s->life = (unsigned int*)malloc(sizeof(unsigned int) * arr_size);
	s->last_frame_update = (unsigned int*)malloc(sizeof(unsigned int) * arr_size);
	s->pos = (Vec2D*)malloc(sizeof(Vec2D) * arr_size);
	s->vel = (Vec2D*)malloc(sizeof(Vec2D) * arr_size);
	s->size = (Vec2D*)malloc(sizeof(Vec2D) * arr_size);

	for (int i = 0; i < arr_size; i++) s->active[i] = 0;
}

void create(Sprite *s, unsigned int current_time, int how_many)
{
	for (int i = 0; i < s->arr_size; i++)
	{
		if (s->active == 0)
		{
			s->active[i] = 1;
			s->pos[i] = { 0.0,0.0 };
			s->vel[i] = { (float)(9.0 * (1.0 - 2.0 * rand() / RAND_MAX)),(float)(9.0 * (1.0 - 2.0 * rand() / RAND_MAX)) };
			s->size[i] = { (float)(8.0 + 32.0 * rand() / RAND_MAX),(float)(8.0 + 32.0 * rand() / RAND_MAX) };
			s->creation_time[i] = current_time;
			s->life[i] = 2000 + rand() % 10000;

			how_many--;
			if (how_many <= 0) break;
		}

	}

}

void update(Sprite *s, const Vec2D *x_bound, const Vec2D *y_bound, unsigned int current_time)
{
	for (int i = 0; i < s->arr_size; i++)
	{
		if (s->active[i] == 1)
		{
			if(current_time - s->creation_time[i] > s->life[i])
			{
				s->active[i] = 0;
				continue;
			}

			s->pos[i].x += s->vel[i].x;
			s->pos[i].y += s->vel[i].y;

			if (s->pos[i].x < x_bound->x || s->pos[i].x > x_bound->y)
			{
				s->vel[i].x *= -1;
			}
			if (s->pos[i].y < y_bound->x || s->pos[i].y > y_bound->y)
			{
				s->vel[i].y *= -1;
			}
			if(current_time - s->last_frame_update[i] > s->animation_speed)
			{
				s->last_frame_update[i] = current_time;
				s->current_frame[i]++;
				s->current_frame[i] %= s->num_of_frames;
			}
		}
	}
}

void draw(Sprite* s)
{
	for (int i = 0; i < s->arr_size; i++)
	{
		if (s->active[i] == 1)
		{
			int current_frame_x = s->frame_position.x + s->current_frame[i] * s->frame_size.x;
			SDL_Rect src = {s->pos[i].x,s->pos[i].y,s->size[i].x,s->size[i].y};
			SDL_Rect dest = {s->pos[i].x,s->pos[i].y,s->size[i].x,s->size[i].y};
			SDL_RenderCopyEx(renderer, s->texture, &src, &dest, 0, NULL, SDL_FLIP_NONE);
		}

	}
}

int main(int argc, char** argv)
{
	char* font_filename = (char*)"fontsheet.png";


	SDL_Init(SDL_INIT_VIDEO);
	srand(time(0));

	SDL_Window* window;
	SDL_Renderer* renderer;
	//SDL_Texture *texture;

	window = SDL_CreateWindow(
		"Dungeon Crawler",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window,
		-1, SDL_RENDERER_ACCELERATED);

	SDL_Surface* font_surface = IMG_Load(font_filename);
	//assert(font_surface);

	SDL_Texture* font_texture = SDL_CreateTextureFromSurface(renderer, font_surface);
	SDL_FreeSurface(font_surface);

	prev_key_state[255];
	keys = (unsigned char*)SDL_GetKeyboardState(NULL);

	unsigned int last_text_change_time = SDL_GetTicks();

	char random_text[17];
	int random_text_size = 32;
	for (int i = 0; i < 16; i++)
	{
		random_text[i] = 'a' + rand() % 26;
	}
	random_text[16] = 0;


	SDL_Surface* surface = IMG_Load("images/dvd.png");
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	printf("TRYING TO DRAW\n");

	unsigned int frame_counter = 0;
	unsigned int last_frame_time = SDL_GetTicks();
	unsigned int last_time_spawned = 0;

	Sprite sprites;
	Vec2D frame_pos = {0,0};
	Vec2D frame_size = {64,64};
	init(&sprites, 100000, &frame_pos, &frame_size, texture, 100);
	

	SDL_Surface *your_draw_buffer = SDL_CreateRGBSurfaceWithFormat(0, screen_width, screen_height, 32, SDL_PIXELFORMAT_RGBA32);
	SDL_Surface *screen = SDL_GetWindowSurface(window);
	SDL_SetSurfaceBlendMode(your_draw_buffer, SDL_BLENDMODE_NONE);
	float *data = new float[screen_width * screen_height];


	/*TTF_Font* font = TTF_OpenFont("gotharctica", 20);
	SDL_Color color = {255,255,255,255};
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, "You're Dead", color);
	SDL_Texture* text = SDL_CreateTextureFromSurface(renderer,textSurface);
	SDL_Rect textRect;
	textRect.x = 320;
	textRect.y = screen_height - 50;

	SDL_QueryTexture(text, NULL, NULL, &textRect.w, &textRect.h);
	SDL_FreeSurface(textSurface);
	textSurface = nullptr;*/

	//texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);

	unsigned char *my_own_buffer = (unsigned char*)malloc(sizeof(unsigned char)*screen_width*screen_height * 4);


	const int max_enemies = 3;

	Boxes *enemies = (Boxes*)malloc(sizeof(Boxes)*max_enemies);

	for (int i = 0; i < max_enemies; i++)
	{
		enemies[i].alive = 0;
		enemies[i].health_lvl = 100;
	}

	SDL_Rect* badguys = (SDL_Rect*)malloc(sizeof(SDL_Rect) * max_enemies);

	for (int i = 0; i < max_enemies; i++)
	{
		badguys[i].x = 150;
		badguys[i].y = rand() % 440 + 40;
		badguys[i].w = 20;
		badguys[i].h = 30;
	}
	
	//double px = 50;
	//double py = 50;

	SDL_Rect health_bar;
	health_bar.x = 10;
	health_bar.y = 555;
	health_bar.w = 200;
	health_bar.h = 10;

	SDL_Rect mana_bar;
	mana_bar.x = 10;
	mana_bar.y = 570;
	mana_bar.w = 140;
	mana_bar.h = 10;

	Boxes Player;
	Player.x = 50;
	Player.y = 50;
	Player.w = 25;
	Player.h = 35;
	Player.health_lvl = 100;
	Player.attack_pwr = 20;
	Player.mana_lvl = 5;
	Player.xvel = 0;
	Player.yvel = 0;

	SDL_Rect player;
	player.x = Player.x;
	player.y = Player.y;
	player.w = Player.w;
	player.h = Player.h;

	SDL_Rect top_wall;
	top_wall.x = 0;
	top_wall.y = 0;
	top_wall.w = screen_width;
	top_wall.h = 20;

	SDL_Rect bottom_wall;
	bottom_wall.x = 0;
	bottom_wall.y = screen_height - 75;
	bottom_wall.w = screen_width;
	bottom_wall.h = 20;

	SDL_Rect left_wall;
	left_wall.x = 0;
	left_wall.y = 0;
	left_wall.w = 20;
	left_wall.h = screen_height - 75;

	SDL_Rect right_wall;
	right_wall.x = screen_width - 20;
	right_wall.y = 0;
	right_wall.w = 20;
	right_wall.h = screen_height - 75;

	SDL_Rect wall1;
	wall1.x = 100;
	wall1.y = 20;
	wall1.w = 20;
	wall1.h = screen_height - 250;

	SDL_Rect wall2;
	wall2.x = 200;
	wall2.y = 20;
	wall2.w = 20;
	wall2.h = screen_height - 150;

	SDL_Rect wall3;
	wall3.x = 300;
	wall3.y = 150;
	wall3.w = 20;
	wall3.h = screen_height - 220;

	SDL_Rect wall4;
	wall4.x = 400;
	wall4.y = 20;
	wall4.w = 20;
	wall4.h = screen_height - 220;

	SDL_Rect wall5;
	wall5.x = 500;
	wall5.y = 20;
	wall5.w = 20;
	wall5.h = screen_height - 550;

	SDL_Rect wall6;
	wall6.x = 500;
	wall6.y = 300;
	wall6.w = 20;
	wall6.h = 225;

	SDL_Rect wall7;
	wall7.x = 700;
	wall7.y = 350;
	wall7.w = 90;
	wall7.h = 20;

	SDL_Rect wall8;
	wall8.x = 700;
	wall8.y = 245;
	wall8.w = 90;
	wall8.h = 20;

	SDL_Rect left_text_wall;
	left_text_wall.x = 0;
	left_text_wall.y = 545;
	left_text_wall.w = 5;
	left_text_wall.h = 55;

	SDL_Rect top_text_wall;
	top_text_wall.x = 5;
	top_text_wall.y = 545;
	top_text_wall.w = screen_width - 5;
	top_text_wall.h = 5;

	SDL_Rect right_text_wall;
	right_text_wall.x = screen_width - 5;
	right_text_wall.y = 545;
	right_text_wall.w = 5;
	right_text_wall.h = 55;

	SDL_Rect bottom_text_wall;
	bottom_text_wall.x = 5;
	bottom_text_wall.y = screen_height - 5;
	bottom_text_wall.w = screen_width - 5;
	bottom_text_wall.h = 5;

	SDL_Rect text_wall4;
	text_wall4.x = 310;
	text_wall4.y = 545;
	text_wall4.w = 10;
	text_wall4.h = 55;

	
	for (;;)
	{
		unsigned int current_time = SDL_GetTicks();

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

		if (frame_counter++ > 100)
		{
			frame_counter = 0;
			float elapsed = current_time - last_frame_time;
			printf("frame time: %.2f\n", elapsed / 100.0);

			last_frame_time = current_time;
		}

		if (current_time - last_time_spawned > 1000)
		{
			last_time_spawned = current_time;
			create(&sprites, current_time, 1000);
		}

		if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A])
		{
			player.x -= 1;
			
		}

		if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D])
		{
			player.x += 1;
		}
		if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W])
		{
			player.y -= 1;
		}
		if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S])
		{
			player.y += 1;
		}
		if (keys[SDL_SCANCODE_F])
		{
			health_bar.w = health_bar.w - 1;
			if (health_bar.w <= health_bar.x)
			{
				health_bar.w = 0;
			}
		}
		if (keys[SDL_SCANCODE_G])
		{
			health_bar.w = health_bar.w + 1;

			if (health_bar.w > 200)
			{
				health_bar.w = 200;
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
		if (player.y + player.h >= screen_height - 75)//bottom
		{
			player.y -= 1;
		}

		//collision, player with walls
		int x1 = direction_Collision(player.x, player.y, player.w, player.h, wall1.x, wall1.y, wall1.w, wall1.h);
		int x2 = direction_Collision(player.x, player.y, player.w, player.h, wall2.x, wall2.y, wall2.w, wall2.h);
		int x3 = direction_Collision(player.x, player.y, player.w, player.h, wall3.x, wall3.y, wall3.w, wall3.h);
		int x4 = direction_Collision(player.x, player.y, player.w, player.h, wall4.x, wall4.y, wall4.w, wall4.h);
		int x5 = direction_Collision(player.x, player.y, player.w, player.h, wall5.x, wall5.y, wall5.w, wall5.h);
		int x6 = direction_Collision(player.x, player.y, player.w, player.h, wall6.x, wall6.y, wall6.w, wall6.h);
		int x7 = direction_Collision(player.x, player.y, player.w, player.h, wall7.x, wall7.y, wall7.w, wall7.h);
		int x8 = direction_Collision(player.x, player.y, player.w, player.h, wall8.x, wall8.y, wall8.w, wall8.h);

		if (x1 == 1 || x2 == 1 || x3 == 1 || x4 == 1 || x5 == 1 || x6 == 1 || x7 == 1 || x8 == 1)//top
		{
			player.y -= 1;
		}
		if (x1 == 2 || x2 == 2 || x3 == 2 || x4 == 2 || x5 == 2 || x6 == 2 || x7 == 2 || x8 == 2)//right
		{
			player.x += 1;
		}
		if (x1 == 3 || x2 == 3 || x3 == 3 || x4 == 3 || x5 == 3 || x6 == 3 || x7 == 3 || x8 == 3)//bottom
		{
			player.y += 1;
		}
		if (x1 == 4 || x2 == 4 || x3 == 4 || x4 == 4 || x5 == 4 || x6 == 4 || x7 == 4 || x8 == 4)//left
		{
			player.x -= 1;
		}

		/*
		
			UPDATE////////
		
		*/

		Vec2D x_bound = { 0,800 };
		Vec2D y_bound = { 0,600 };
		update(&sprites, &x_bound, &y_bound, current_time);

		for (int i = 0; i < max_enemies; i++)
		{
			enemies[i].alive = 1;


			int x = direction_Collision(player.x, player.y, player.w, player.h, badguys[i].x, badguys[i].y, badguys[i].w, badguys[i].h);

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
		//SDL_RenderCopy(renderer, texture);

		/*
		
			DRAW/////////
		
		*/

		SDL_SetRenderDrawColor(renderer, 0,0,0,255);
		SDL_RenderClear(renderer);

		draw(&sprites);


		//clear the screen with black
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		//----player box
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		SDL_RenderFillRect(renderer, &player);

		//-------borders
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &top_wall);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &bottom_wall);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &left_wall);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &right_wall);

		//-------Walls
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &wall1);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &wall2);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &wall3);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &wall4);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &wall5);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &wall6);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &wall7);

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &wall8);

		//---------Text_walls
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &left_text_wall);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &top_text_wall);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &right_text_wall);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &bottom_text_wall);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &text_wall4);
		
		//------------Health bar
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderFillRect(renderer, &health_bar);
		//------------Mana bar
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		SDL_RenderFillRect(renderer, &mana_bar);

		//SDL_RenderCopy(renderer, text, NULL, &textRect);

		for (int i = 0; i < max_enemies; i++)
		{
			
			if (enemies[i].alive == 1)
			{
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_RenderFillRect(renderer, &badguys[i]);

			}
			
			
		}

		if (player.x > 700)
		{

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			//----player box
			SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
			SDL_RenderFillRect(renderer, &player);

			//---------Text_walls
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRect(renderer, &left_text_wall);

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRect(renderer, &top_text_wall);

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRect(renderer, &right_text_wall);

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRect(renderer, &bottom_text_wall);

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRect(renderer, &text_wall4);
		}

		if (health_bar.w <= 0)
		{
			if (current_time - last_text_change_time > 1000)
			{
				last_text_change_time = current_time;
				random_text_size = 16 + rand() % 64;
				//refresh random text
				for (int i = 0; i < 16; i++)
				{
					random_text[i] = 'a' + rand() % 26;
				}
				random_text[16] = 0;
			}

			float text_x = 50;
			float text_y = screen_height / 2;

			//draw one character at a time
			for (int i = 0; i < 16; i++)
			{
				//define the rectangle you are copying FROM the texture
				//in this case, the font sheet is 1024 by 1024 pixel box and it contains 256 characters
				//characters are laid out on a 16 by 16 grid
				//each character is 64 by 64. This mean you need to divide the ascii character by 16 for row and modulus 16 for column.
				SDL_Rect src;
				src.x = 64 * (random_text[i] % 16);//column
				src.y = 64 * (random_text[i] / 16);//row
				src.w = 64;
				src.h = 64;

				//define the rectangle you are copying TO on the SCREENw
				//you can set width and height to anything you want, the GPU will properly scale your tile
				SDL_Rect dest;
				dest.x = text_x;
				dest.y = text_y;
				dest.w = random_text_size;
				dest.h = dest.w;

				//set transparency of the texture.
				//0 to 255, 255 is opaque
				SDL_SetTextureAlphaMod(font_texture, 255);

				//copy from source texture to destination screen.
				//you can change the rotation angle and provide a pivot point for rotations.
				//SDL_FLIP_XXX enumeration allows you to mirror the image
				SDL_RenderCopyEx(renderer, font_texture, &src, &dest, 0, NULL, SDL_FLIP_NONE);

				//increment text_x!
				text_x += dest.w;
			}
			
			sprintf(random_text, "You're Dead  ");

			//float text_x = 320;
			//float text_y = screen_height - 50;

			
		}

		SDL_RenderPresent(renderer);
	}

	return 0;
}