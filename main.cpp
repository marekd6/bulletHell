#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	888//width of the window888
#define SCREEN_HEIGHT	666//height of the window585
#define MARGIN			44//size of ther player
#define STAGE_WIDTH		SCREEN_WIDTH*3//width of the expanded stage
#define STAGE_HEIGHT	SCREEN_HEIGHT*3//height of the expanded stage
#define ENEMY_X			244//initial x coordinate of the enemy 1  544
#define ENEMY_Y			144//initial y coordinate of the enemy 1
#define LOW_SPEED		1.412
#define USUAL_SPEED		3.141592*LOW_SPEED
#define HIGH_SPEED		2.7182818*LOW_SPEED*USUAL_SPEED
#define FPS_CONST		33//desired constant FPS value
#define ARROW_JUMP		8//position jump after pressing an arrow
#define PLAYER_X		SCREEN_WIDTH / 2//initial x coordinate of the player
#define PLAYER_Y		SCREEN_HEIGHT / 2//initial y coordinate of the player
#define NB_OF_LEVELS	3//nuber of levels in the game
#define NB_OF_BULLETS	4//number of bullets
#define FULL_CIRCLE		360//full angle in degrees
#define DEG_JUMP		FULL_CIRCLE / NB_OF_BULLETS
#define FACT_PART		33
#define FACTOR			FACT_PART/(FACT_PART+1)
#define EVTIME			12//time after which eti becomes evil
#define NB_OF_DIRECTIONS 4//four directions in the 2D world
#define MARGINUP		77//margin for text box
#define CAMERA_SIZE		444//useless now
#define BACK_X			111//x dimension of background file
#define BACK_Y			83//y dimension of background file
//#define STAGE_WIDTH		(FULL_STAGE_WIDTH-SCREEN_WIDTH)/2//help
//#define STAGE_HEIGHT	(FULL_STAGE_HEIGHT-SCREEN_HEIGHT)/2
#define nSTAGE_WIDTH	STAGE_WIDTH+SCREEN_WIDTH/4
#define nSTAGE_HEIGHT	STAGE_HEIGHT+SCREEN_HEIGHT/4

enum colors {
	BLACK,
	GREEN,
	RED,
	BLUE
};


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
	int cox, coy;//movement coefficients
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


//check if sth belongs to an interval: 1-yes 0-no
int isIn(double a, double b, double x) {
	if (x >= a && x <= b) {
		return 1;
	}
	return 0;
}


//move - change coordinates
void moveCoord(direction dir, double screen_borders[], double* horizontal_shift, double* vertical_shift, spirits* player) {
	switch (dir)
	{
	case LEFT:
		if (player->x > screen_borders[LEFT]) {
			player->x -= ARROW_JUMP;
		}
		else if (player->x + *horizontal_shift + MARGIN + ARROW_JUMP < STAGE_WIDTH) {
			*horizontal_shift += ARROW_JUMP;
		}
		else if (player->x > MARGIN) {
			player->x -= ARROW_JUMP;
		}
		player->side = LEFT;
		break;
	case RIGHT:
		if (player->x < screen_borders[RIGHT]) {
			player->x += ARROW_JUMP;
		}
		else if (player->x - *horizontal_shift + MARGIN + ARROW_JUMP < STAGE_WIDTH) {
			*horizontal_shift -= ARROW_JUMP;
		}
		else if (player->x + MARGIN < SCREEN_WIDTH) {
			player->x += ARROW_JUMP;
		}
		player->side = RIGHT;
		break;
	case UP:
		if (player->y > screen_borders[UP]) {
			player->y -= ARROW_JUMP;
		}
		else if (player->y + *vertical_shift + MARGIN + ARROW_JUMP < STAGE_HEIGHT) {
			*vertical_shift += ARROW_JUMP;
		}
		else if (player->y > MARGINUP) {
			player->y -= ARROW_JUMP;
		}
		player->side = UP;
		break;
	case DOWN:
		if (player->y + MARGIN < screen_borders[DOWN]) {
			player->y += ARROW_JUMP;
		}
		else if (player->y - *vertical_shift - MARGIN - ARROW_JUMP < STAGE_HEIGHT) {
			*vertical_shift -= ARROW_JUMP;
		}
		else if (player->y + MARGIN < SCREEN_HEIGHT) {
			player->y += ARROW_JUMP;
		}
		player->side = DOWN;
		break;
	default:
		break;
	}
}


// obs≥uga zdarzeÒ (o ile jakieú zasz≥y) / handling of events (if there were any)
void handlingEvents(int* quit, int* new_game, int* level, double* etiSpeed, SDL_Event& event,
	spirits* player, double* horizontal_shift, double* vertical_shift, double screen_borders[]) {
	double changeR = 0;
	double changeL = 0;
	double expression = player->x - MARGIN - ARROW_JUMP - PLAYER_X;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) *quit = 1;
			else if (event.key.keysym.sym == SDLK_1) {
				*level = 1;
				*new_game = true;
			}
			else if (event.key.keysym.sym == SDLK_2) {
				*level = 2;
				*new_game = true;
			}
			else if (event.key.keysym.sym == SDLK_n) *new_game = true;
			else if (event.key.keysym.sym == SDLK_f) *etiSpeed = HIGH_SPEED;
			else if (event.key.keysym.sym == SDLK_s) *etiSpeed = LOW_SPEED;
			else if (event.key.keysym.sym == SDLK_LEFT) {
				/*if (player->x > screen_borders[LEFT]) {
					player->x -= ARROW_JUMP;
				}
				else if (player->x + *horizontal_shift + MARGIN + ARROW_JUMP < STAGE_WIDTH) {
					*horizontal_shift += ARROW_JUMP;
				}
				else if (player->x > MARGIN) {
					player->x -= ARROW_JUMP;
				}
				player->side = LEFT;*/
				moveCoord(LEFT, screen_borders, horizontal_shift, vertical_shift, player);
			}
			else if (event.key.keysym.sym == SDLK_RIGHT) {
				/*				if (player->x < screen_borders[RIGHT]) {
									player->x += ARROW_JUMP;
								}
								else if (player->x - *horizontal_shift + MARGIN + ARROW_JUMP < STAGE_WIDTH) {
									*horizontal_shift -= ARROW_JUMP;
								}
								else if (player->x + MARGIN < SCREEN_WIDTH) {
									player->x += ARROW_JUMP;
								}
								player->side = RIGHT;*/
				moveCoord(RIGHT, screen_borders, horizontal_shift, vertical_shift, player);
			}
			else if (event.key.keysym.sym == SDLK_UP) {
				moveCoord(UP, screen_borders, horizontal_shift, vertical_shift, player);
			}
			else if (event.key.keysym.sym == SDLK_DOWN) {
				moveCoord(DOWN, screen_borders, horizontal_shift, vertical_shift, player);
			}
			break;
		case SDL_KEYUP:
			*etiSpeed = USUAL_SPEED;
			player->side = RIGHT;
			break;
		case SDL_QUIT:
			*quit = 1;
			break;
		}
	}
	//printf("x: %f, y: %f, v: %f, h: %f\n", player->x, player->y, *vertical_shift, *horizontal_shift);
	//printf("x: %f, h: %f, L:%f, R:%f\n", player->x, *horizontal_shift, screen_borders[UP], screen_borders[DOWN]);
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
	if ((1000 / FPS_CONST) > delta) {
		SDL_Delay(1000 / FPS_CONST - delta);
	}
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


//check if object is outside the stage
int bulletOutOfStage(spirits bullet) {
	if (fabs(bullet.x) >= nSTAGE_WIDTH || fabs(bullet.y) >= nSTAGE_HEIGHT) {
		return 1;
	}
	return 0;
}


//euklidean distance
double dist(point a, point b) {
	double p = a.x - b.x;
	double q = a.y - b.y;
	return sqrt((p * p) + (q * q));
}


//linear move of bullets
void moveObject(spirits* bullet, point initial, double speed) {
	if (bulletOutOfStage(*bullet)) {
		bullet->x = initial.x;
		bullet->y = initial.y;
	}
	else {
		double dx = bullet->cox * speed;//x++ or x--
		bullet->x += dx;
		bullet->y += bullet->coy * dx;//y-- or y++
	}
}


//linear move of bullets
void moveObject2(spirits* bullet, point initial, double speed, int level, double wt) {
	int a = 5;
	if (bulletOutOfStage(*bullet)) {
		printf("% f - % f\n\n", bullet->x, bullet->y);
		bullet->x = initial.x;
		bullet->y = initial.y;
		if (level == 2) {
			bullet->coy *= -1;
			bullet->cox *= -1;
		}
	}
	else {
		double dx = bullet->cox * speed;//x++ or x--
		bullet->x += dx;
		switch (level)
		{
		case 1:
			bullet->y += bullet->coy * dx;//y-- or y++
			break;
		case 2:
			bullet->y += bullet->coy * dx * a / bullet->x * cos(wt) + sin(wt) * dx;
			break;
		default:
			break;
		}
	}
}


//draw border lines
void drawBorders(double horizontal_shift, double vertical_shift, int czerwony, SDL_Surface* screen, spirits player) {
	//RIGHT
	if (player.x - horizontal_shift + MARGIN + ARROW_JUMP >= STAGE_WIDTH) {
		DrawLine(screen, SCREEN_WIDTH - 1, 1, SCREEN_HEIGHT - 1, 0, 1, czerwony);
	}
	//LEFT
	if (player.x + horizontal_shift + MARGIN + ARROW_JUMP >= STAGE_WIDTH) {
		DrawLine(screen, 1, 1, SCREEN_HEIGHT - 1, 0, 1, czerwony);
	}
	//BOTTOM
	if (player.y - vertical_shift + MARGIN >= STAGE_HEIGHT) {
		DrawLine(screen, 1, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, 1, 0, czerwony);
	}
}


//run 1. level
void firstLevel(SDL_Surface* screen, double horizontal_shift, double vertical_shift, double worldTime, double etiSpeed, SDL_Surface* etis[],
	SDL_Surface* blue_dot, SDL_Surface* pink_dot, point initial, spirits bullets[], spirits* first_en, double screen_borders[]) {
	//static enemy
	DrawSurface(screen, etis[first_en->side], first_en->x + horizontal_shift, first_en->y + vertical_shift);
	//enemy - bullets
	for (int i = 0; i < NB_OF_BULLETS; i++) {
		//draw bullet
		DrawSurface(screen, blue_dot, bullets[i].x + horizontal_shift, bullets[i].y + vertical_shift);
		DrawSurface(screen, blue_dot, bullets[i].x * FACTOR + horizontal_shift, bullets[i].y * FACTOR + vertical_shift);
		DrawSurface(screen, blue_dot, bullets[i].x + horizontal_shift, bullets[i].y * FACTOR - 3 * 2 * FACTOR + vertical_shift);
		DrawSurface(screen, pink_dot, ENEMY_X + horizontal_shift, bullets[i].y + vertical_shift);
		DrawSurface(screen, pink_dot, ENEMY_X + horizontal_shift - FACT_PART, bullets[i].y + vertical_shift);
		DrawSurface(screen, pink_dot, bullets[i].x + horizontal_shift, ENEMY_Y + vertical_shift);
		DrawSurface(screen, pink_dot, bullets[i].x + horizontal_shift, ENEMY_Y + vertical_shift - FACT_PART);
		//move bullet
		moveObject2(&bullets[i], initial, etiSpeed, 1, worldTime);
	}
	if (worldTime > EVTIME) {
		int wt = worldTime;
		if (wt % 2) {
			first_en->side = RIGHT;
		}
		else {
			first_en->side = EVIL;
		}
	}
}


//run 2. level
void secondLevel(SDL_Surface* screen, double horizontal_shift, double vertical_shift, double worldTime, double etiSpeed, SDL_Surface* etis[],
	SDL_Surface* blue_dot, SDL_Surface* pink_dot, point initial, spirits bullets[], spirits* first_en, double screen_borders[]) {
	//static enemy
	DrawSurface(screen, etis[first_en->side], first_en->x + horizontal_shift, first_en->y + vertical_shift);
	//enemy - bullets
	for (int i = 0; i < NB_OF_BULLETS; i++) {
		//draw bullet
		DrawSurface(screen, blue_dot, bullets[i].x + horizontal_shift, bullets[i].y + vertical_shift);
		DrawSurface(screen, blue_dot, bullets[i].x * FACTOR + horizontal_shift, bullets[i].y * FACTOR + vertical_shift);
		DrawSurface(screen, blue_dot, bullets[i].x + horizontal_shift, bullets[i].y * FACTOR - 3 * 2 * FACTOR + vertical_shift);
		DrawSurface(screen, blue_dot, bullets[i].x + horizontal_shift, ENEMY_Y + vertical_shift);
		DrawSurface(screen, blue_dot, bullets[i].x + horizontal_shift, ENEMY_Y + vertical_shift - 24);
		//move bullet
		moveObject2(&bullets[i], initial, etiSpeed, 2, worldTime);
	}
	if (worldTime > EVTIME) {
		int wt = worldTime;
		if (wt % 2) {
			first_en->side = RIGHT;
			etiSpeed *= HIGH_SPEED;
		}
		else {
			first_en->side = EVIL;
			etiSpeed /= LOW_SPEED;
		}
	}
}


//new game = if "n" pressed - set up initial values
void setUpNewGame(double* horizontal_shift, double* vertical_shift, double* worldTime, int* new_game, spirits enemies[], spirits* player,
	double screen_borders[], spirits bullets[][NB_OF_BULLETS], int level) {
	*worldTime = 0;
	*new_game = false;

	//set bullets initial positions and direction of movement
	for (int i = 0; i < NB_OF_LEVELS; i++) {
		int a, b;
		a = b = 1;
		enemies[i].x = ENEMY_X;
		enemies[i].y = ENEMY_Y;
		for (int j = 0; j < NB_OF_BULLETS; j++) {
			bullets[i][j].x = ENEMY_X;
			bullets[i][j].y = ENEMY_Y;
			bullets[i][j].cox = a;
			bullets[i][j].coy = b;
			b *= -a;
			a *= -1;
		}
	}

	enemies[level].side = RIGHT;
	*player = { PLAYER_X , PLAYER_Y , 1, 1, RIGHT };
	*horizontal_shift = *vertical_shift = 0;
}


//draw stage - background and borders
void drawStage(SDL_Surface* screen, int colours[], SDL_Surface* texture, double horizontal_shift, double vertical_shift, spirits player) {
	SDL_FillRect(screen, NULL, colours[BLACK]);
	for (int j = -STAGE_HEIGHT / BACK_Y; j < STAGE_HEIGHT / BACK_Y * 2; j++) {//for every row
		for (int i = -STAGE_WIDTH / BACK_X; i < STAGE_WIDTH / BACK_X * 2; i++) {//for every 'column' in a row 
			DrawSurface(screen, texture, -0 + horizontal_shift + BACK_X * i, MARGINUP + vertical_shift + BACK_Y * j);
		}
	}
	drawBorders(horizontal_shift, vertical_shift, colours[RED], screen, player);
}


//choose correct level function
void levelChoice(int level, SDL_Surface* screen, double horizontal_shift, double vertical_shift, double worldTime, double etiSpeed,
	SDL_Surface* etis[], SDL_Surface* blue_dot, SDL_Surface* pink_dot, point initial, spirits bullets[][NB_OF_BULLETS], spirits* enemies,
	double screen_borders[]) {
	if (level == 1) {
		firstLevel(screen, horizontal_shift, vertical_shift, worldTime, etiSpeed, etis, blue_dot, pink_dot, initial,
			bullets[0], &enemies[0], screen_borders);
	}
	else {
		//etiSpeed = LOW_SPEED;
		secondLevel(screen, horizontal_shift, vertical_shift, worldTime, etiSpeed, etis, blue_dot, pink_dot, initial,
			bullets[0], &enemies[0], screen_borders);
	}
}


//looping through frames and events
void playControl(int* quit, int* new_game, int* x_pos, int* y_pos, int* t1, int* t2, double* worldTime, double* distance, double* etiSpeed,
	SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* blue_dot, SDL_Surface* pink_dot, SDL_Surface* charset, SDL_Texture* scrtex, int colours[],
	double* fpsTimer, int* frames, double* fps, char text[], double* delta, SDL_Event& event, SDL_Surface* etis[], SDL_Surface* textures[]) {

	double horizontal_shift = 0;
	double vertical_shift = 0;
	double screen_borders[NB_OF_DIRECTIONS] = { SCREEN_WIDTH / 3 , SCREEN_WIDTH * 2 / 3 , MARGINUP * 3, SCREEN_HEIGHT * 4 / 5 };
	int level = 1;
	spirits enemies[NB_OF_LEVELS];
	point initial = { ENEMY_X , ENEMY_Y };
	spirits bullets[NB_OF_LEVELS][NB_OF_BULLETS];
	enemies[0] = bullets[0][0];
	spirits player = { PLAYER_X , PLAYER_Y , 1,1, RIGHT };

	while (!*quit) {
		*t2 = SDL_GetTicks();
		timeFlow(delta, t1, *t2, worldTime, *fps);

		//if n pressed then new game
		if (*new_game) {
			setUpNewGame(&horizontal_shift, &vertical_shift, worldTime, new_game, enemies, &player, screen_borders, bullets, level - 1);
		}

		//stage
		drawStage(screen, colours, textures[level - 1], horizontal_shift, vertical_shift, player);

		//player
		DrawSurface(screen, etis[player.side], player.x, player.y);

		levelChoice(level, screen, horizontal_shift, vertical_shift, *worldTime, *etiSpeed, etis, blue_dot, pink_dot, initial, bullets,
			enemies, screen_borders);

		fpsTiming(*delta, fpsTimer, frames, fps);
		textWriting(colours[RED], colours[BLUE], text, screen, charset, *worldTime, *fps);
		updatingCanva(renderer, scrtex, screen);//renderer, scrtex, screen
		handlingEvents(quit, new_game, &level, etiSpeed, event, &player, &horizontal_shift, &vertical_shift, screen_borders);
		makeFPSconstant(*delta);
		(*frames)++;
	}
}


//load images
int loadBMPs(SDL_Surface* etis[], SDL_Surface** blue_dot, SDL_Surface** pink_dot, SDL_Surface* background[],
	SDL_Renderer* renderer, SDL_Window* window, SDL_Texture* scrtex, SDL_Surface* screen, SDL_Surface** charset) {
	// wczytanie obrazka cs8x8.bmp
	*charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 1;
	}

	etis[RIGHT] = SDL_LoadBMP("./eti.bmp");
	if (etis[RIGHT] == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 0;
	}

	etis[EVIL] = SDL_LoadBMP("./devileti.bmp");
	if (etis[EVIL] == NULL) {
		printf("SDL_LoadBMP(devileti.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 0;
	}

	etis[LEFT] = SDL_LoadBMP("./etiL.bmp");
	if (etis[LEFT] == NULL) {
		printf("SDL_LoadBMP(etiL.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 0;
	}

	etis[UP] = SDL_LoadBMP("./etiU.bmp");
	if (etis[UP] == NULL) {
		printf("SDL_LoadBMP(etiL.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 0;
	}

	etis[DOWN] = SDL_LoadBMP("./etiD.bmp");
	if (etis[DOWN] == NULL) {
		printf("SDL_LoadBMP(etiL.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 0;
	}

	*blue_dot = SDL_LoadBMP("./blue.bmp");
	if (*blue_dot == NULL) {
		printf("SDL_LoadBMP(blue.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 0;
	}

	*pink_dot = SDL_LoadBMP("./pink.bmp");
	if (*pink_dot == NULL) {
		printf("SDL_LoadBMP(pink.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 0;
	}

	background[0] = SDL_LoadBMP("./backgr.bmp");
	if (background[0] == NULL)
	{
		printf("SDL_LoadBMP(backgr.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 0;
	}

	background[1] = SDL_LoadBMP("./backgr1.bmp");
	if (background[1] == NULL)
	{
		printf("SDL_LoadBMP(backgr1.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 0;
	}

	background[2] = SDL_LoadBMP("./backgr2.bmp");
	if (background[2] == NULL)
	{
		printf("SDL_LoadBMP(backgr2.bmp) error: %s\n", SDL_GetError());
		freeingFun(renderer, window, scrtex, screen, *charset);
		return 0;
	}

	return 1;
}


// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	int t1, t2, quit, new_game, frames, rc, x_pos, y_pos;
	double delta, worldTime, fpsTimer, fps, distance, etiSpeed;
	SDL_Event event;
	SDL_Surface* screen, * charset = NULL;
	SDL_Surface* etis[6], * blue_dot = NULL, * pink_dot = NULL, * backgrounds[NB_OF_LEVELS];
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;

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
	//SDL_RenderSetLogicalSize(renderer, STAGE_WIDTH, STAGE_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Bullet Hell MD");

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);

	// wy≥πczenie widocznoúci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	if (loadBMPs(etis, &blue_dot, &pink_dot, backgrounds, renderer, window, scrtex, screen, &charset)) {
		SDL_SetColorKey(charset, true, 0x000000);

		char text[128];
		int rgb[] = { SDL_MapRGB(screen->format, 0x00, 0x00, 0x00),
			SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00),
			SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00),
			SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC) };
		int moj_kolor = SDL_MapRGB(screen->format, 0x01, 0x44, 0x44);

		t1 = SDL_GetTicks();
		frames = 0;
		fpsTimer = 0;
		fps = 0;
		quit = 0;
		etiSpeed = USUAL_SPEED;
		new_game = 1;

		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		playControl(&quit, &new_game, &x_pos, &y_pos, &t1, &t2, &worldTime, &distance, &etiSpeed, screen, renderer,
			blue_dot, pink_dot, charset, scrtex, rgb, &fpsTimer, &frames, &fps, text, &delta, event, etis, backgrounds);

		freeingFun(renderer, window, scrtex, screen, charset);
	}
	SDL_Quit();
	return 0;
}
