#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	1111//777
#define SCREEN_HEIGHT	585
#define ENEMY_X 544
#define ENEMY_Y 144
#define LOW_SPEED 1.412
#define HIGH_SPEED 3.141592*LOW_SPEED
#define USUAL_SPEED 2.7182818
#define FPS_CONST 44//desired constant FPS value
#define ARROW_JUMP 12//position jum after pressing an arrow
#define PLAYER_X SCREEN_WIDTH / 2
#define PLAYER_Y SCREEN_HEIGHT / 2

enum direction {
	LEFT,
	RIGHT,
	UP,
	DOWN,
	EVIL
};

struct point {
	double x, y;
};

struct spirits {
	double x, y;
	int a;//movement coeficient
	direction side;
};


// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	}
}


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
}


// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
}


// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	}
}


// rysowanie prostokπta o d≥ugoúci bokÛw l i k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
}


// zwolnienie powierzchni / freeing all surfaces
void freeingFun(SDL_Renderer* renderer, SDL_Window* window, SDL_Texture* scrtex, SDL_Surface* screen, SDL_Surface* charset) {
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}


// obs≥uga zdarzeÒ (o ile jakieú zasz≥y) / handling of events (if there were any)
void handlingEvents(int* quit, int* new_game, double* etiSpeed, SDL_Event& event, spirits* player) {
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) *quit = 1;
			else if (event.key.keysym.sym == SDLK_n) *new_game = true;
			else if (event.key.keysym.sym == SDLK_f) *etiSpeed = HIGH_SPEED;
			else if (event.key.keysym.sym == SDLK_s) *etiSpeed = LOW_SPEED;
			else if (event.key.keysym.sym == SDLK_LEFT) {
				player->x -= ARROW_JUMP;
				player->side = LEFT;
			}
			else if (event.key.keysym.sym == SDLK_RIGHT) {
				player->x += ARROW_JUMP;
				player->side = RIGHT;
			}
			else if (event.key.keysym.sym == SDLK_UP) {
				player->y -= ARROW_JUMP;
				player->side = UP;
			}
			else if (event.key.keysym.sym == SDLK_DOWN) {
				player->y += ARROW_JUMP;
				player->side = DOWN;
			}
			break;
		case SDL_KEYUP:
			*etiSpeed = USUAL_SPEED;
			break;
		case SDL_QUIT:
			*quit = 1;
			break;
		}
	}
}


//adjusting fps
void fpsTiming(double delta, double* fpsTimer, int* frames, double* fps) {
	*fpsTimer += delta;
	if (*fpsTimer > 0.5) {
		*fps = (*frames) * 2;
		*frames = 0;
		*fpsTimer -= 0.5;
	}
}


// w tym momencie t2-t1 to czas w milisekundach,
// jaki uplyna≥ od ostatniego narysowania ekranu
// delta to ten sam czas w sekundach
void timeFlow(double* delta, int* t1, int t2, double* worldTime, double fps) {
	*delta = (t2 - *t1) * 0.001;
	*t1 = t2;
	*worldTime += *delta;
}


void makeFPSconstant(double delta) {
	/*if ((1000 / FPS_CONST) > delta) {
		SDL_Delay(1000 / FPS_CONST - delta);
		//printf("%f\n", delta);
	}*/
	SDL_Delay(1000 / FPS_CONST);
}


//outputs text info
void textWriting(int czerwony, int niebieski, char text[], SDL_Surface* screen, SDL_Surface* charset, double worldTime, double fps) {
	// tekst informacyjny / info text
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
	sprintf(text, "Bullet Hell game, czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	//	      "Esc - exit, \030 - faster, \031 - slower"
	sprintf(text, "Esc - exit, f - faster, s - slower, n - new game, control: \030, \031, \032, \033");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
}


void updatingCanva(SDL_Renderer* renderer, SDL_Texture* scrtex, SDL_Surface* screen) {
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	//		SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}


//check if sth belongs to an interval
int isIn(int a, int b, double x) {
	if (x > a && x < b) {
		return true;
	}
	return 0;
}


//check if object is outside the sreen
int outOfScreen(double x, double y) {
	if ((isIn(0, SCREEN_HEIGHT, y) && isIn(0, SCREEN_WIDTH, x)) == 0) {
		return 1;
	}
	return 0;
}


//linear move of bullets
void movem(spirits* bullet, point initial, double speed) {
	if (outOfScreen(bullet->x, bullet->y)) {
		bullet->x = initial.x + speed;
		bullet->y = initial.y;
		bullet->a *= -1;
	}
	else {
		bullet->x += bullet->a * speed;
		bullet->y += speed;
	}
}


//looping through frames and events
void playControl(int* quit, int* new_game, int* x_pos, int* y_pos, int* t1, int* t2, double* worldTime, double* distance, double* etiSpeed,
	SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* eti, SDL_Surface* etiL, SDL_Surface* yellow_dot, SDL_Surface* pink_dot, SDL_Surface* charset, SDL_Texture* scrtex, int moj_kolor, int niebieski,
	int czerwony, double* fpsTimer, int* frames, double* fps, char text[], double* delta, SDL_Event& event, SDL_Surface* etis[]) {
	point initial = { ENEMY_X , ENEMY_Y };

	spirits bullet = { ENEMY_X , ENEMY_Y , 1, RIGHT };
	spirits bullet2 = { ENEMY_X , ENEMY_Y , -1, RIGHT };
	spirits first = bullet;
	spirits player = { PLAYER_X , PLAYER_Y , -1, RIGHT };
	double en_x = ENEMY_X;
	double en_y = ENEMY_Y;
	double moving_en_x = ENEMY_X;
	double moving_en_y = ENEMY_Y;
	double moving_bullet_x = ENEMY_X;
	double moving_bullet_y = ENEMY_Y;
	while (!*quit) {
		*t2 = SDL_GetTicks();
		//if n pressed
		if (*new_game) {
			*worldTime = 0;
			*new_game = false;
			*distance = 0;
			*x_pos = PLAYER_X;
			*y_pos = PLAYER_Y;
			en_y = ENEMY_Y;
			en_x = ENEMY_X;
			moving_en_x = ENEMY_X;
			moving_en_y = ENEMY_Y;
			moving_bullet_x = ENEMY_X;
			moving_bullet_y = ENEMY_Y;

			bullet.x = bullet2.x = first.x = initial.x;
			bullet.y = bullet2.y = first.y = initial.y;
			first.side = RIGHT;
			player = { PLAYER_X , PLAYER_Y , -1, RIGHT };
		}

		timeFlow(delta, t1, *t2, worldTime, *fps);
		*distance += *etiSpeed * *delta;

		SDL_FillRect(screen, NULL, moj_kolor);
		//player
		DrawSurface(screen, etis[player.side], player.x, player.y);

		//static enemy
		DrawSurface(screen, etis[first.side], first.x, first.y);
		//enemy - bullet
		DrawSurface(screen, yellow_dot, bullet.x, bullet.y);
		movem(&bullet, initial, *etiSpeed);
		DrawSurface(screen, yellow_dot, bullet2.x, bullet2.y);
		movem(&bullet2, initial, LOW_SPEED);

		DrawSurface(screen, pink_dot, bullet2.y, bullet2.x);
		//movem(&bullet3, initial, LOW_SPEED);
		if (*worldTime > 24) {
			movem(&bullet2, initial, HIGH_SPEED);
			int wt = *worldTime;
			if (wt % 2) {
				first.side = RIGHT;
			}
			else {
				first.side = EVIL;
			}
		}

		fpsTiming(*delta, fpsTimer, frames, fps);
		textWriting(czerwony, niebieski, text, screen, charset, *worldTime, *fps);
		updatingCanva(renderer, scrtex, screen);

		handlingEvents(quit, new_game, etiSpeed, event, &player);
		makeFPSconstant(*delta);
		(*frames)++;
	}
}


// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	int t1, t2, quit, new_game, frames, rc, x_pos, y_pos;
	double delta, worldTime, fpsTimer, fps, distance, etiSpeed;
	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Surface* eti, * yellow_dot, * pink_dot, * etiL, * devil;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* etis[5];

	// okno konsoli nie jest widoczne, jeøeli chcemy zobaczyÊ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniÊ na "Console"
	printf("wyjscie printfa trafia do tego okienka\n");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	// tryb pe≥noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Bullet Hell MD");

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);

	// wy≥πczenie widocznoúci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, charset);
		return 1;
	}
	SDL_SetColorKey(charset, true, 0x000000);

	etis[RIGHT] = SDL_LoadBMP("./eti.bmp");
	if (etis[RIGHT] == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, charset);
		return 1;
	}

	etis[EVIL] = SDL_LoadBMP("./devileti.bmp");
	if (etis[EVIL] == NULL) {
		printf("SDL_LoadBMP(devileti.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, charset);
		return 1;
	}

	etis[LEFT] = SDL_LoadBMP("./etiL.bmp");
	if (etis[LEFT] == NULL) {
		printf("SDL_LoadBMP(etiL.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, charset);
		return 1;
	}

	etis[UP] = SDL_LoadBMP("./etiU.bmp");
	if (etis[UP] == NULL) {
		printf("SDL_LoadBMP(etiL.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, charset);
		return 1;
	}

	etis[DOWN] = SDL_LoadBMP("./etiD.bmp");
	if (etis[DOWN] == NULL) {
		printf("SDL_LoadBMP(etiL.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, charset);
		return 1;
	}

	yellow_dot = SDL_LoadBMP("./yellow.bmp");
	if (yellow_dot == NULL) {
		printf("SDL_LoadBMP(yellow.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, charset);
		return 1;
	}

	pink_dot = SDL_LoadBMP("./pink.bmp");
	if (yellow_dot == NULL) {
		printf("SDL_LoadBMP(pink.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, charset);
		return 1;
	}

	//SDL_Surface* pictures[] = { eti, devil, etiL, yellow_dot, pink_dot };

	char text[128];
	int moj_kolor = SDL_MapRGB(screen->format, 0x01, 0x44, 0x44);
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();
	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	etiSpeed = 1;
	new_game = 1;

	playControl(&quit, &new_game, &x_pos, &y_pos, &t1, &t2, &worldTime, &distance, &etiSpeed, screen, renderer, etis[RIGHT], etis[LEFT], yellow_dot, pink_dot, charset, scrtex,
		czarny, niebieski, czerwony, &fpsTimer, &frames, &fps, text, &delta, event, etis);

	freeingFun(renderer, window, scrtex, screen, charset);
	SDL_Quit();
	return 0;
}
