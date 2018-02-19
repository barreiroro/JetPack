
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include <esat/window.h>
#include <esat/draw.h>
#include <esat/sprite.h>
#include <esat/input.h>
#include <esat/time.h>
#include <esat/math.h>

#include <esat_extra/soloud/soloud_wav.h>
#include <esat_extra/soloud/soloud.h>


const int kgravity = 2;
const int kwindow_x=800;
const int kwindow_y=600;

//VARIABLES GLOBALES
bool g_player_one_turn = true;
bool g_start = false, g_next_level = false;;
int g_high_score = 0, player1_level = 1, player1_lives = 5, player1_score = 0,
player2_level = 1, player2_lives = 5, player2_score = 0, die_counter = 0;
char *g_cadena1, *g_cadena2, *g_cadena3, *g_cadena4, *g_cadena5;

int g_firecount;//variable nueva que controla el sprite del fuego de la nave durante la animacion

bool fuelcounter;//Variable para controlar que solo haya 1 fuel a la vez

bool collecting1 = false, dropping1 = false; //Gestionan la recogida y colocacion de la pieza 1
bool collecting2 = false, dropping2 = false; //Gestionan la recogida y colocacion de la pieza 2
bool collecting3 = false, dropping3 = false; //Gestionan la recogida y colocacion del fuel

int g_counter_map=0;
esat::SpriteHandle sheet, sheet_verde, sheet_azul, sheet_amarillo, sheet_rosa, sheet_rojo;//He añadido las hojas de sprites de los distintos colores
esat::SpriteHandle fire_small;//una extra para el fuego de la nave
esat::SpriteHandle hud_lives;

struct Player
{
  float weight;
  float x,y;
  float velocity;
  int level;
  int lives;
  int score;
  int dir;
  int steps;
  bool alive;
  bool platform;
  //true right, false left
  bool check_dir;
  bool can_up;
  bool has_item1;
  bool has_item2;
  bool has_fuel;
  esat::SpriteHandle *animation;
};
Player player;

struct Animation
{
 esat::SpriteHandle A1,A2;
};
Animation sprite_pink,sprite_green,sprite_blue,sprite_red;


struct Enemy
{
  int type;
  float x,y;
  float vec_dir_x,vec_dir_y;
  int patron;
  int score = 1;
  bool alive;
  int color;
  int indice_sprite;
  int Dirx;
  int Diry;
  bool Move = true,Go = false,impact=false,platform=false;
  int En_contador = 0, En_contador2 = 0;
  Animation animation;
};

Enemy *p_enemy,aux;


struct SpaceShip
{
  float x,y;
  int num_parts;
  int num_fuel;
  bool do_animation;
};
SpaceShip spaceship;

struct Items
{
  float weight;
  float x,y;
  int num_part;
  int score;
  bool is_fuel;
  bool is_part;
  esat::SpriteHandle sprite;
};
Items *spaceshipparts, fuel;

struct Map
{
	float init,end;
	float x;
	float y;
	int type;
	esat::SpriteHandle *sprite;
};
Map map, *all_map;

//Añadir variables de cada tipo. Comunicar a el grupo.

//LOADS
void LoadSheet(){
	sheet = esat::SpriteFromFile("./Recursos/Sprites/sheet.jpg");
	sheet_verde = esat::SpriteFromFile("./Recursos/Sprites/sheetverde.jpg");
	sheet_azul = esat::SpriteFromFile("./Recursos/Sprites/sheetazul.jpg");
	sheet_amarillo = esat::SpriteFromFile("./Recursos/Sprites/sheetamarillo.jpg");
	sheet_rosa = esat::SpriteFromFile("./Recursos/Sprites/sheetrosa.jpg");
	sheet_rojo = esat::SpriteFromFile("./Recursos/Sprites/sheetrojo.jpg");
  hud_lives = esat::SubSprite(sheet, 438, 45, 24, 30);
}
void LoadSprites(){
	map.sprite = (esat::SpriteHandle*)malloc(6*sizeof(esat::SpriteHandle));
	*(map.sprite + 0) = esat::SubSprite(sheet_amarillo,396,321,24,24);
	*(map.sprite + 1) = esat::SubSprite(sheet_amarillo,435,321,24,24);
	*(map.sprite + 2) = esat::SubSprite(sheet_amarillo,475,321,24,24);
	*(map.sprite + 3) = esat::SubSprite(sheet_verde,396,321,24,24);
	*(map.sprite + 4) = esat::SubSprite(sheet_verde,435,321,24,24);
	*(map.sprite + 5) = esat::SubSprite(sheet_verde,475,321,24,24);

	player.animation =  (esat::SpriteHandle*)malloc(16*sizeof(esat::SpriteHandle));
	//Sin volar izquierda
	*(player.animation + 0) = esat::SubSprite(sheet,68,101,44,68);
	*(player.animation + 1) = esat::SubSprite(sheet,139,101,44,68);
	*(player.animation + 2) = esat::SubSprite(sheet,210,101,44,68);
	*(player.animation + 3) = esat::SubSprite(sheet,281,101,44,68);
	//Sin volar derecha
	*(player.animation + 4) = esat::SubSprite(sheet,74,195,44,66);
	*(player.animation + 5) = esat::SubSprite(sheet,145,195,44,66);
	*(player.animation + 6) = esat::SubSprite(sheet,216,195,44,66);
	*(player.animation + 7) = esat::SubSprite(sheet,287,195,44,66);
	//Volando izquierda
	*(player.animation + 8) = esat::SubSprite(sheet,574,100,48,76);
	*(player.animation + 9) = esat::SubSprite(sheet,650,100,48,76);
	*(player.animation + 10) = esat::SubSprite(sheet,723,98,51,76);
	*(player.animation + 11) = esat::SubSprite(sheet,795,98,53,76);
	//Volando derecha
	*(player.animation + 12) = esat::SubSprite(sheet,576,185,46,74);
	*(player.animation + 13) = esat::SubSprite(sheet,647,189,50,73);
	*(player.animation + 14) = esat::SubSprite(sheet,721,185,51,74);
	*(player.animation + 15) = esat::SubSprite(sheet,791,185,53,74);


	fire_small = esat::SubSprite(sheet_rojo,438,636,54,51);
}

//INITS
void SavePartsMap(float init, float end, float y){
	/*
		Tipos: 0 parte izquierda de la plataforma; 1 medio; 2 parte derecha
	*/
	for (float i = init; i < end; i+=24){
		map.x = i;

		if(i == init){
			map.type = 0;
		}
		if(i > init){
			map.type = 1;
		}
		if(i == end - 24){
			map.type = 2;
		}

		*(all_map + g_counter_map) = map;
		g_counter_map++;
		all_map = (Map*)realloc(all_map, (g_counter_map + 1)*sizeof(Map));
	}
}
void InitMap(){

	/*
	Mapa del juego en X : 1X24 32X24
	De izquierda a derecha, las plataformas: 5X24 11X24; 15X24 19X24; 24X24 30X24
	*/
	all_map = (Map*)malloc(1*sizeof(Map));
	// X
	map.init = 24;
	map.end = 768;
	map.y = 570;
	SavePartsMap(map.init,map.end,map.y);

	map.init = 120;
	map.end = 264;
	map.y = 216;
	SavePartsMap(map.init,map.end,map.y);

	map.init = 360;
	map.end = 456;
	map.y = 321;
	SavePartsMap(map.init,map.end,map.y);

	map.init = 576;
	map.end = 720;
	map.y = 168;
	SavePartsMap(map.init,map.end,map.y);
}
void InitPlayer(){
	player.weight = 5.0f;
	player.x = 300;
	player.y = 200;
	player.level = 1;
	player.lives = 5;
	player.score = 0;
	player.dir = 0;
	player.velocity = 3;
	player.steps = 0;
	player.alive = true;
	player.check_dir = true;
	player.platform = false;
	player.can_up = true;
	player.has_item1 = false;
	player.has_item2 = false;
	player.has_fuel = false;
}
void Reset_Player(){
	player.weight = 5.0f;
	player.x = 200;
	player.y = 200;
	player.dir = 0;
	player.velocity = 3;
	player.alive = true;
	player.check_dir=true;
}
void InitShipParts(int level){
	//llamarla dentro del checklevel pasandole el level del jugador como parametro
	//Inicializa las piezas de la nave segun el nivel en el que se encuentre el jugador
	if(level ==1){
		(*(spaceshipparts + 0)).x = 495;
		(*(spaceshipparts + 0)).y = 517;
		(*(spaceshipparts + 0)).num_part = 0;
		(*(spaceshipparts + 0)).score = 100;
		(*(spaceshipparts + 0)).is_fuel = false;
		(*(spaceshipparts + 0)).is_part = true;
		(*(spaceshipparts + 0)).sprite = esat::SubSprite(sheet,237,555,54,54);

		(*(spaceshipparts + 1)).x = 382;
		(*(spaceshipparts + 1)).y = 267;
		(*(spaceshipparts + 1)).num_part = 1;
		(*(spaceshipparts + 1)).score = 100;
		(*(spaceshipparts + 1)).is_fuel = false;
		(*(spaceshipparts + 1)).is_part = true;
		(*(spaceshipparts + 1)).sprite = esat::SubSprite(sheet,237,486,54,54);

		(*(spaceshipparts + 2)).x = 180;
		(*(spaceshipparts + 2)).y = 162;
		(*(spaceshipparts + 2)).num_part = 2;
		(*(spaceshipparts + 2)).score = 100;
		(*(spaceshipparts + 2)).is_fuel = false;
		(*(spaceshipparts + 2)).is_part = true;
		(*(spaceshipparts + 2)).sprite = esat::SubSprite(sheet,249,420,30,54);

		spaceship.num_parts = 1;
	}
	if(level ==5){
		(*(spaceshipparts + 0)).x = 495;
		(*(spaceshipparts + 0)).y = 517;
		(*(spaceshipparts + 0)).num_part = 0;
		(*(spaceshipparts + 0)).score = 100;
		(*(spaceshipparts + 0)).is_fuel = false;
		(*(spaceshipparts + 0)).is_part = true;
		(*(spaceshipparts + 0)).sprite = esat::SubSprite(sheet,330,555,54,54);

		(*(spaceshipparts + 1)).x = 387;
		(*(spaceshipparts + 1)).y = 267;
		(*(spaceshipparts + 1)).num_part = 1;
		(*(spaceshipparts + 1)).score = 100;
		(*(spaceshipparts + 1)).is_fuel = false;
		(*(spaceshipparts + 1)).is_part = true;
		(*(spaceshipparts + 1)).sprite = esat::SubSprite(sheet,342,489,39,54);

		(*(spaceshipparts + 2)).x = 173;
		(*(spaceshipparts + 2)).y = 162;
		(*(spaceshipparts + 2)).num_part = 2;
		(*(spaceshipparts + 2)).score = 100;
		(*(spaceshipparts + 2)).is_fuel = false;
		(*(spaceshipparts + 2)).is_part = true;
		(*(spaceshipparts + 2)).sprite = esat::SubSprite(sheet,342,420,39,54);

		spaceship.num_parts = 1;
	}
	if(level ==9){
		(*(spaceshipparts + 0)).x = 495;
		(*(spaceshipparts + 0)).y = 517;
		(*(spaceshipparts + 0)).num_part = 0;
		(*(spaceshipparts + 0)).score = 100;
		(*(spaceshipparts + 0)).is_fuel = false;
		(*(spaceshipparts + 0)).is_part = true;
		(*(spaceshipparts + 0)).sprite = esat::SubSprite(sheet,468,558,54,54);

		(*(spaceshipparts + 1)).x = 387;
		(*(spaceshipparts + 1)).y = 267;
		(*(spaceshipparts + 1)).num_part = 1;
		(*(spaceshipparts + 1)).score = 100;
		(*(spaceshipparts + 1)).is_fuel = false;
		(*(spaceshipparts + 1)).is_part = true;
		(*(spaceshipparts + 1)).sprite = esat::SubSprite(sheet,468,489,54,54);

		(*(spaceshipparts + 2)).x = 173;
		(*(spaceshipparts + 2)).y = 162;
		(*(spaceshipparts + 2)).num_part = 2;
		(*(spaceshipparts + 2)).score = 100;
		(*(spaceshipparts + 2)).is_fuel = false;
		(*(spaceshipparts + 2)).is_part = true;
		(*(spaceshipparts + 2)).sprite = esat::SubSprite(sheet,462,420,48,54);

		spaceship.num_parts = 1;
	}
	if(level ==13){
		(*(spaceshipparts + 0)).x = 495;
		(*(spaceshipparts + 0)).y = 517;
		(*(spaceshipparts + 0)).num_part = 0;
		(*(spaceshipparts + 0)).score = 100;
		(*(spaceshipparts + 0)).is_fuel = false;
		(*(spaceshipparts + 0)).is_part = true;
		(*(spaceshipparts + 0)).sprite = esat::SubSprite(sheet,585,555,54,54);

		(*(spaceshipparts + 1)).x = 382;
		(*(spaceshipparts + 1)).y = 267;
		(*(spaceshipparts + 1)).num_part = 1;
		(*(spaceshipparts + 1)).score = 100;
		(*(spaceshipparts + 1)).is_fuel = false;
		(*(spaceshipparts + 1)).is_part = true;
		(*(spaceshipparts + 1)).sprite = esat::SubSprite(sheet,585,489,54,54);

		(*(spaceshipparts + 2)).x = 173;
		(*(spaceshipparts + 2)).y = 162;
		(*(spaceshipparts + 2)).num_part = 2;
		(*(spaceshipparts + 2)).score = 100;
		(*(spaceshipparts + 2)).is_fuel = false;
		(*(spaceshipparts + 2)).is_part = true;
		(*(spaceshipparts + 2)).sprite = esat::SubSprite(sheet,594,420,36,54);

		spaceship.num_parts = 1;
	}
	(*(spaceshipparts + 3)).x = 495;
	(*(spaceshipparts + 3)).y = 577;
	(*(spaceshipparts + 3)).num_part = 0;
	(*(spaceshipparts + 3)).score = 0;
	(*(spaceshipparts + 3)).is_fuel = false;
	(*(spaceshipparts + 3)).is_part = false;
	(*(spaceshipparts + 3)).sprite = esat::SubSprite(sheet_rojo,348,636,54,51);
}
void InitFuel(){
	fuel.score = 100;
	fuel.is_fuel = true;
	fuel.is_part = false;
	fuel.sprite = esat::SubSprite(sheet_rosa,66,318,54,39);
}
void CheckScreen(float *pos_x, float *pos_y, bool *impact, bool *platform){
	if(*pos_x < -11){
		*pos_x = 800;
	}
	if(*pos_x > 811){
		*pos_x = 0;
	}
	if(*pos_y <= 60){
		*impact = true;
	}
	if((*pos_x >= -45 && *pos_x + 45 <= 845) && *pos_y>= 500){
		*platform = true;
	}else if((*pos_x >= 95 && *pos_x <= 250) && (*pos_y>= 145 && *pos_y<= 160) ){
		*platform= true;

	}else if((*pos_x >= 310 && *pos_x <= 460) && (*pos_y>= 250 && *pos_y<= 260)){
		*platform = true;
	}else if((*pos_x >= 531 && *pos_x <= 720) && (*pos_y>= 98 && *pos_y<= 105)){
		*platform = true;
	}else{
		*platform = false;
	}
}
Enemy CheckEnemyCollide(Enemy enemy){

	if(enemy.y >= 550 || enemy.impact){
		enemy.alive = false;
	}

	return enemy;
}
Enemy PatronSimple(Enemy enemy,int (*Vx),int (*Vy)){

	printf("Enemy.color = %d\n",enemy.color );
	switch(enemy.color){

		case 0: (*Vx) = 5 * enemy.Dirx; ;break;//rojos y verdes

		case 1: (*Vx) = 5 * enemy.Dirx; ;break;//rojos y verdes

		case 2: (*Vx) = 5 * enemy.Dirx; (*Vy) = 2 * enemy.Diry; ;break;//azules y rosas

		case 3: (*Vx) = 5 * enemy.Dirx; (*Vy) = 2 * enemy.Diry; ;break;//azules y rosas


	}
	enemy.vec_dir_y = (*Vy);
	enemy.vec_dir_x = (*Vx);
	printf("Set Vector\n");

	return enemy;
}
Enemy PatronRebotar(Enemy enemy,int (*Vx),int (*Vy)){
	printf("REBOTAR\n");
	switch(enemy.color){

		case 0: (*Vy) = 5 * enemy.Diry; ;break;//rojos y verdes

		case 1: (*Vy) = 5 * enemy.Diry; ;break;//rojos y verdes

		case 2: (*Vx) = 5 * enemy.Dirx; (*Vy) = 2 * enemy.Diry; ;break;//azules y rosas

		case 3: (*Vx) = 5 * enemy.Dirx; (*Vy) = 2 * enemy.Diry; ;break;//azules y rosas


	}
	enemy.vec_dir_y = (*Vy);
	enemy.vec_dir_x = (*Vx);

 return enemy;
}
Enemy PatronBolas(Enemy enemy,int (*Vx),int (*Vy)){
	//NO ACABADO
	int Random,Random1 = 0;
	Random = rand()%50;

	
		Random1 = rand()%2;
	
	if(Random1 == 1){
		aux = enemy;
		enemy = PatronSimple(aux,&(*Vx),&(*Vy));
	}
	enemy.vec_dir_y = (*Vy);
	enemy.vec_dir_x = (*Vx);

	return enemy;
}
Enemy PatronStatic(Enemy enemy,int (*Vx),int (*Vy)){
	int Random,x,y,M_Vd;//M_Vd = modulo del vector director;
	Random = rand()%30;

	if(enemy.Go && enemy.En_contador2%10 == 0){
	x = esat::MousePositionX();
	y = esat::MousePositionY();

	(*Vy) = enemy.y - y;
	(*Vx) = enemy.x - x;

	M_Vd = sqrt(pow((*Vx),2)+pow((*Vy),2));

	(*Vy) = y/M_Vd + 4;
	(*Vx) = x/M_Vd + 6;

	if(enemy.y < esat::MousePositionY()){
		(*Vy) = (*Vy) * 1;
	}

	if(enemy.y > esat::MousePositionY()){
		(*Vy) = (*Vy) * -1;
	}

	enemy.vec_dir_y = (*Vy);
	enemy.vec_dir_x = (*Vx);
	enemy.En_contador2++;

	}


	if(!enemy.Go){
		if(Random == 5){
		enemy.Go = true;
		}

		if(enemy.Move){


			(*Vy) = -5;
			(*Vx) = 0;
			if(enemy.En_contador == 3){
				enemy.Move = false;
				enemy.En_contador = 0;
			}else{
				enemy.En_contador++;
			}
		}else{

			(*Vy) = 5;
			(*Vx) = 0;
			if(enemy.En_contador == 3){
					enemy.Move = true;
					enemy.En_contador = 0;
			}else{
				enemy.En_contador++;
			}
		}
	}


	return enemy;
}
Enemy PatronFollow(Enemy enemy,int (*Vx),int (*Vy)){
	int x,y,M_Vd;//M_Vd = modulo del vector director
	float Vec_UniX,Vec_UniY;



	x = esat::MousePositionX();
	y = esat::MousePositionY();

	(*Vy) = enemy.y - y;
	(*Vx) = enemy.x - x;

	M_Vd = sqrt(pow((*Vx),2)+pow((*Vy),2));

	Vec_UniY = y/M_Vd + 4;
	Vec_UniX = x/M_Vd + 4;

	if(enemy.y < esat::MousePositionY()){
		Vec_UniY = Vec_UniY * 1;
	}

	if(enemy.y > esat::MousePositionY()){
		Vec_UniY = Vec_UniY * -1;
	}

	if(enemy.x < esat::MousePositionX()){
		Vec_UniX = Vec_UniX * 1;
	}

	if(enemy.x > esat::MousePositionX()){
		Vec_UniX = Vec_UniX * -1;
	}

	enemy.vec_dir_y = Vec_UniY;
	enemy.vec_dir_x = Vec_UniX;



	return enemy;
}
Enemy CheckEnemyDir(Enemy enemy){

	int Vx = 0,Vy = 0;
	Enemy aux3;



	aux3 = enemy;


	switch(aux3.patron){

		case 1: enemy = PatronSimple(aux3,&Vx,&Vy);break;
		case 2: Vx = 2;
		 enemy = PatronRebotar(aux3,&Vx,&Vy);break;
		case 3: enemy = PatronBolas(aux3,&Vx,&Vy);break;
		case 4: enemy = PatronStatic(aux3,&Vx,&Vy);break;
		case 5: enemy = PatronFollow(aux3,&Vx,&Vy);break;
	}
	printf("%d %d\n",Vx,Vy );

	enemy.x += enemy.vec_dir_x;
	enemy.y += enemy.vec_dir_y;

	return enemy;
}
Enemy CheckEnemy(Enemy enemy){
 Enemy aux2;

 	aux2 = enemy;
	enemy = CheckEnemyDir(aux2);// lleva CheckCollide

	return enemy;
}
Enemy InitLevel(Enemy enemy,int j = 8){
	player.level = 1;

	switch(player.level){

		case 1:	enemy.patron = 1;//meteoritos
				enemy.score = 25;
				enemy.alive = true;
				enemy.y = 150 + rand()%400;
				enemy.x = 0;

				//if(enemy.color == 2 || enemy.color == 3){
					enemy.Diry = 1;
				//}
				enemy.Dirx = - 1 + rand()%2;
				if(enemy.Dirx == 0){
				enemy.Dirx = 1;
				}

				if( j < 8){

					if(j == 0 || j == 1){//VERDE
						enemy.color = 0;
						enemy.animation.A1 = esat::SubSprite(sheet_verde,744,327,48,33);//meteorito 1
						enemy.animation.A2 = esat::SubSprite(sheet_verde,816,327,48,33);//meteorito 2
						printf("Cargado de Sprites\n");
						//esat::DrawSprite(sprite_green.A1,100.0f,100.0f);
					}

					if(j == 2 || j == 3){//ROJO
						enemy.color = 1;
						enemy.animation.A1 = esat::SubSprite(sheet_rojo,744,327,48,33);//meteorito 1
						enemy.animation.A2 = esat::SubSprite(sheet_rojo,816,327,48,33);//meteorito 2
						printf("Cargado de Sprites 2\n");
					}

					if(j == 4 || j == 5){//AZUL
						enemy.color = 2;
						enemy.animation.A1 = esat::SubSprite(sheet_azul,744,327,48,33);//meteorito 1
						enemy.animation.A2 = esat::SubSprite(sheet_azul,816,327,48,33);//meteorito 2
						printf("Cargado de Sprites 3\n");
					}

					if(j == 6 || j == 7){//ROSA
						enemy.color = 3;
						enemy.animation.A1 = esat::SubSprite(sheet_rosa,744,327,48,33);//meteorito 1
						enemy.animation.A2 = esat::SubSprite(sheet_rosa,816,327,48,33);//meteorito 2
						printf("Cargado de Sprites 4\n");
					}
				}
				printf("Level Loaded\n");
				printf("%f  %f  %d  %d  %d\n",enemy.x,enemy.y,enemy.color,enemy.Dirx,enemy.Diry );
				;break;

		case 2: enemy.patron = 2;//Pompones
				enemy.score = 80;
				enemy.alive = true;
				enemy.y = 150 + rand()%400;
				enemy.x = 0;
				enemy.Dirx = rand()%2 - 1;
				if(enemy.Dirx == 0){
				enemy.Dirx = 1;
				}
				if( j < 8){

					if(j == 0 || j == 1){//VERDE
						enemy.color = 0;
						enemy.animation.A1 = esat::SubSprite(sheet_verde,747,375,54,48);//Pompom 1
						enemy.animation.A2 = esat::SubSprite(sheet_verde,816,375,54,48);//Pompom 2
					}

					if(j == 2 || j == 3){//ROJO
						enemy.color = 1;
						enemy.animation.A1 = esat::SubSprite(sheet_rojo,747,375,54,48);//Pompom 1
						enemy.animation.A2 = esat::SubSprite(sheet_rojo,816,375,54,48);//Pompom 2
					}

					if(j == 4 || j == 5){//AZUL
						enemy.color = 2;
						enemy.animation.A1 = esat::SubSprite(sheet_azul,747,375,54,48);//Pompom 1
						enemy.animation.A2 = esat::SubSprite(sheet_azul,816,375,54,48);//Pompom 2
					}

					if(j == 6 || j == 7){//ROSA
						enemy.color = 3;
						enemy.animation.A1 = esat::SubSprite(sheet_rosa,747,375,54,48);//Pompom 1
						enemy.animation.A2 = esat::SubSprite(sheet_rosa,816,375,54,48);//Pompom 2
					}
				}
				;break;

		case 3: enemy.patron = 3;//Bolas
				enemy.score = 40;
				enemy.alive = true;
				enemy.y = 150 + rand()%400;
				enemy.x = 0;
				enemy.Dirx = rand()%2 - 1;
				if(enemy.Dirx == 0){
				enemy.Dirx = 1;
				}
				if( j < 8){

					if(j == 0 || j == 1){//VERDE
						enemy.color = 0;
						enemy.animation.A1 = esat::SubSprite(sheet_verde,744,435,54,39);//Balon 1
						enemy.animation.A2 = esat::SubSprite(sheet_verde,816,375,54,39);//Balon 2
					}

					if(j == 2 || j == 3){//ROJO
						enemy.color = 1;
						enemy.animation.A1 = esat::SubSprite(sheet_rojo,744,435,54,39);//Balon 1
						enemy.animation.A2 = esat::SubSprite(sheet_rojo,816,375,54,39);//Balon 2
					}

					if(j == 4 || j == 5){//AZUL
						enemy.color = 2;
						enemy.animation.A1 = esat::SubSprite(sheet_azul,744,435,54,39);//Balon 1
						enemy.animation.A2 = esat::SubSprite(sheet_azul,816,375,54,39);//Balon 2
					}

					if(j == 6 || j == 7){//ROSA
						enemy.color = 3;
						enemy.animation.A1 = esat::SubSprite(sheet_rosa,744,435,54,39);//Balon 1
						enemy.animation.A2 = esat::SubSprite(sheet_rosa,816,375,54,39);//Balon 2
					}
				}
				;break;

		case 4: enemy.patron = 4;//Aviones/pajaros
				enemy.score = 55;
				enemy.alive = true;
				enemy.y = 150 + rand()%400;
				enemy.x = 0;
				enemy.Dirx = rand()%2 - 1;
				if(enemy.Dirx == 0){
				enemy.Dirx = 1;
				}
				if( j < 8){

					if(j == 0 || j == 1){//VERDE
						enemy.color = 0;
						enemy.animation.A1 = esat::SubSprite(sheet_verde,744,513,54,27);//Pajaro / Avion
					}

					if(j == 2 || j == 3){//ROJO
						enemy.color = 1;
						enemy.animation.A1 = esat::SubSprite(sheet_rojo,744,513,54,27);//Pajaro / Avion
					}

					if(j == 4 || j == 5){//AZUL
						enemy.color = 2;
						enemy.animation.A1 = esat::SubSprite(sheet_azul,744,513,54,27);//Pajaro / Avion
					}

					if(j == 6 || j == 7){//ROSA
						enemy.color = 3;
						enemy.animation.A1 = esat::SubSprite(sheet_rosa,744,513,54,27);//Pajaro / Avion
					}
				}
				;break;

		case 5: enemy.patron = 5;//Ovnis
				enemy.score = 50;
				enemy.alive = true;
				enemy.y = 150 + rand()%400;
				enemy.x = 0;
				enemy.Dirx = rand()%2 - 1;
				if(enemy.Dirx == 0){
				enemy.Dirx = 1;
				}
				if( j < 8){

					if(j == 0 || j == 1){//VERDE
						enemy.color = 0;
						enemy.animation.A1 = esat::SubSprite(sheet_verde,744,558,54,30);//Ovni
					}

					if(j == 2 || j == 3){//ROJO
						enemy.color = 1;
						enemy.animation.A1 = esat::SubSprite(sheet_rojo,744,558,54,30);//Ovni
					}

					if(j == 4 || j == 5){//AZUL
						enemy.color = 2;
						enemy.animation.A1 = esat::SubSprite(sheet_azul,744,558,54,30);//Ovni
					}

					if(j == 6 || j == 7){//ROSA
						enemy.color = 3;
						enemy.animation.A1 = esat::SubSprite(sheet_rosa,744,558,54,30);//Ovni
					}
				}
				;break;

		case 6: enemy.patron = 2;//Spinners
				enemy.score = 60;
				enemy.alive = true;
				enemy.y = 150 + rand()%400;
				enemy.x = 0;
				enemy.Dirx = rand()%2 - 1;
				if(enemy.Dirx == 0){
				enemy.Dirx = 1;
				}
				if( j < 8){

					if(j == 0 || j == 1){//VERDE
						enemy.color = 0;
						enemy.animation.A1 = esat::SubSprite(sheet_verde,744,609,51,51);//Spinner
					}

					if(j == 2 || j == 3){//ROJO
						enemy.color = 1;
						enemy.animation.A1 = esat::SubSprite(sheet_rojo,744,609,51,51);//Spinner
					}

					if(j == 4 || j == 5){//AZUL
						enemy.color = 2;
						enemy.animation.A1 = esat::SubSprite(sheet_azul,744,609,51,51);//Spinner
					}

					if(j == 6 || j == 7){//ROSA
						enemy.color = 3;
						enemy.animation.A1 = esat::SubSprite(sheet_rosa,744,609,51,51);//Spinner
					}
				}
				;break;

		case 7: enemy.patron = 1;//Halcon Milenario
				enemy.score = 25;
				enemy.alive = true;
				enemy.y = 150 + rand()%400;
				enemy.x = 0;
				if(enemy.color == 2 || enemy.color == 3){
					enemy.Diry = 1;
				}
				enemy.Dirx = rand()%2 - 1;
				if(enemy.Dirx == 0){
				enemy.Dirx = 1;
				}
				if( j < 8){

					if(j == 0 || j == 1){//VERDE
						enemy.color = 0;
						enemy.animation.A1 = esat::SubSprite(sheet_verde,744,669,54,48);//Halcon
					}

					if(j == 2 || j == 3){//ROJO
						enemy.color = 1;
						enemy.animation.A1 = esat::SubSprite(sheet_rojo,744,669,54,48);//Halcon
					}

					if(j == 4 || j == 5){//AZUL
						enemy.color = 2;
						enemy.animation.A1 = esat::SubSprite(sheet_azul,744,669,54,48);//Halcon
					}

					if(j == 6 || j == 7){//ROSA
						enemy.color = 3;
						enemy.animation.A1 = esat::SubSprite(sheet_rosa,744,669,54,48);//Halcon
					}
				}
				;break;

		case 8: enemy.patron = 5;//ranas
				enemy.score = 50;
				enemy.alive = true;
				enemy.y = 150 + rand()%400;
				enemy.x = 0;
				enemy.Dirx = rand()%2 - 1;
				if(enemy.Dirx == 0){
				enemy.Dirx = 1;
				}
				if( j < 8){

					if(j == 0 || j == 1){//VERDE
						enemy.color = 0;
						enemy.animation.A1 = esat::SubSprite(sheet_verde,744,735,54,48);//Rana
					}

					if(j == 2 || j == 3){//ROJO
						enemy.color = 1;
						enemy.animation.A1 = esat::SubSprite(sheet_rojo,744,735,54,48);//Rana
					}

					if(j == 4 || j == 5){//AZUL
						enemy.color = 2;
						enemy.animation.A1 = esat::SubSprite(sheet_azul,744,735,54,48);//Rana
					}

					if(j == 6 || j == 7){//ROSA
						enemy.color = 3;
						enemy.animation.A1 = esat::SubSprite(sheet_rosa,744,735,54,48);//Rana
					}
				}
				;break;
	}
	return enemy;
}
void InitEnemies(){
	for (int i = 0; i < 8; ++i)
	{
  	*(p_enemy + i) = InitLevel(aux,i);
  	printf("%d\n",i );
	}
}

//CHECKS
void InputPlayer(){
	if(esat::IsSpecialKeyPressed(esat::kSpecialKey_Up) || esat::IsSpecialKeyPressed(esat::kSpecialKey_Left) || esat::IsSpecialKeyPressed(esat::kSpecialKey_Right)){
		player.steps++;
		//right
		 if(esat::IsSpecialKeyPressed(esat::kSpecialKey_Right) && !esat::IsSpecialKeyPressed(esat::kSpecialKey_Up) ){
			player.dir = 0;
			player.check_dir = true;
		}
		//Right up
		if(esat::IsSpecialKeyPressed(esat::kSpecialKey_Right) && esat::IsSpecialKeyPressed(esat::kSpecialKey_Up)){
			player.dir = 1;

		}
		//left
		 if(esat::IsSpecialKeyPressed(esat::kSpecialKey_Left) && !esat::IsSpecialKeyPressed(esat::kSpecialKey_Up) &&  !esat::IsSpecialKeyPressed(esat::kSpecialKey_Right)){
			player.dir = 2;
			player.check_dir = false;
		}
		//Left up
		if(esat::IsSpecialKeyPressed(esat::kSpecialKey_Left) && esat::IsSpecialKeyPressed(esat::kSpecialKey_Up)){
			player.dir = 3;
		}
		//up down
		 if(esat::IsSpecialKeyPressed(esat::kSpecialKey_Up) && !esat::IsSpecialKeyPressed(esat::kSpecialKey_Left) && !esat::IsSpecialKeyPressed(esat::kSpecialKey_Right)){
			player.dir = 4;
		}
	}	else if(!player.platform){
		player.steps++;
		player.dir = 6;
	} else{

		player.dir = 8;
	}
}
void CheckPlayer(){
	if( ((player.x >= 95 && player.x <= 250) && (player.y>= 230 && player.y<= 235)) || (player.x >= 310 && player.x <= 460) && (player.y>= 340 && player.y<= 345) || (player.x >= 531 && player.x <= 720) && (player.y>= 190 && player.y<= 195)){
		player.can_up = false;
	}else{
		player.can_up = true;
	}
}
void Load_Level(int level_){

	if(g_player_one_turn == true){		//player 1
		player1_level++;
		player1_score += 250;
		//Reset_Player(player1);

	}else{								//player 2
		player2_level++;
		player2_score += 250;
		//Reset_Player(player2);

	}
	if(g_player_one_turn == true){	//checks player
			InitShipParts(player1_level);
		}else{
			InitShipParts(player2_level);
		}
}
void Check_Level(){
	if(spaceship.y == 50){		//ship leaves, next level
		g_next_level = true;
		if(g_player_one_turn == true){	//checks player
			Load_Level(player1_level);
		}else{
			Load_Level(player2_level);
		}
	}
}
void EnemyChecking(){
	for (int i = 0; i < 8; ++i)
	{
		aux = *(p_enemy + i);
		aux = CheckEnemy(aux);
		aux = CheckEnemyCollide(aux);
		printf("CheckEnemy %d\n",i);

		CheckScreen(&aux.x,&aux.y,&aux.impact,&aux.platform);


		if(!aux.alive){
			aux = InitLevel(aux);
		}


	*(p_enemy + i) = aux;
	}

}
//UPDATE
void UpdatePlayer(){
	CheckScreen(&player.x,&player.y,&player.can_up,&player.platform);
	switch(player.dir){
		case 0:
			
				player.x += player.velocity;
			

		break;
		case 1:
		if(player.can_up){
			player.x += player.velocity;
			player.y -= player.weight;
		}
		break;
		case 2:
			
				player.x -= player.velocity;
			
		break;
		case 3:
			if(player.can_up){
			 player.x -= player.velocity;
			 player.y -= player.weight;
			}
		break;
		case 4:
			if(player.can_up){
				player.y -= player.weight;
			}
		break;
		case 6:
			if(!player.platform){
				player.y += player.weight * kgravity;
			}

		break;
	}
}
void Part1SPMovement(){
	//Gestion del movimiento y colocacion de la pieza 1 por el jugador
	//Esta es la pieza del medio, la mediana (plataforma del medio)
	Items Part1;
	Part1 = *(spaceshipparts + 1);
		if((player.x <= (Part1.x) + esat::SpriteWidth((Part1.sprite))) &&
		  (player.x + esat::SpriteWidth(*player.animation) >= (Part1.x)) &&
		  (player.y  + esat::SpriteHeight(*player.animation) /2 <= (Part1.y) + esat::SpriteHeight((Part1.sprite))) &&
		  (player.y  + esat::SpriteHeight(*player.animation) >= (Part1.y) + esat::SpriteHeight((Part1.sprite)) / 2) &&
		  ((Part1.is_fuel) == true || (Part1.is_part) == true) && collecting1 == false && dropping1 == false && spaceship.num_parts == 1){
			player.has_item1 = true;//Variable nueva
			collecting1 = true;//Variable nueva
			printf("Part1\n");
		}
		if(player.has_item1 == true && collecting1 == true){
			(Part1.x) = player.x;
			(Part1.y) = player.y + esat::SpriteHeight(*player.animation) - esat::SpriteHeight((Part1.sprite));//controlar posicion correcta
		}
		if((Part1.x) >= 495 && (Part1.x)<=500 && collecting1 == true){
			collecting1 = false;
			player.has_item1 = false;
			dropping1 = true;//Variable nueva
			if(player1_level != 5 || player2_level != 5){
				(Part1.x) = 495;
			}
			if(player1_level == 5 || player2_level == 5){
				(Part1.x) = 507;
			}
		}
		if(dropping1 == true){
			(Part1.y) +=5;
		}
		if((Part1.y) >= 463 && (Part1.num_part) == 1 && dropping1 == true){
			dropping1 = false;
			if(player1_level != 5 || player2_level != 5){
				(Part1.x) = 495;
			}
			if(player1_level == 5 || player2_level == 5){
				(Part1.x) = 507;
			}
			(Part1.y) = 467;
			spaceship.num_parts=2;
      if(g_player_one_turn == true){
      player1_score += 100;
      }else{
      player2_score += 100;
      }
		}
	*(spaceshipparts + 1) = Part1;
}
void Part2SPMovement(){
	//Gestion del movimiento y colocacion de la pieza 1 por el jugador
	//Esta es la pieza de arriba del todo, la pequeña(plataforma de la izquierda)
	Items Part2;
	Part2 = *(spaceshipparts + 2);

		if((player.x <= (Part2.x) + esat::SpriteWidth((Part2.sprite))) &&
		  (player.x + esat::SpriteWidth(*player.animation) >= (Part2.x)) &&
		  (player.y  + esat::SpriteHeight(*player.animation) /2 <= (Part2.y) + esat::SpriteHeight((Part2.sprite))) &&
		  (player.y  + esat::SpriteHeight(*player.animation) >= (Part2.y) + esat::SpriteHeight((Part2.sprite)) / 2) &&
		  ((Part2.is_fuel) == true || (Part2.is_part) == true) && collecting2 == false && dropping2 == false && spaceship.num_parts == 2){
			player.has_item2 = true;//Variable nueva
			collecting2 = true;//Variable nueva
			printf("Part1\n");
		}
		if(player.has_item2 == true && collecting2 == true){
			(Part2.x) = player.x;
			(Part2.y) = player.y + esat::SpriteHeight(*player.animation) - esat::SpriteHeight((Part2.sprite));//controlar posicion correcta
		}
		if((Part2.x) >=495 && (Part2.x)<=500 && collecting2 == true){
			collecting2 = false;
			player.has_item2 = false;
			dropping2 = true;//Variable nueva
			if((player1_level == 1 || player1_level == 5) || (player2_level == 1 || player2_level == 5)){
				(Part2.x) = 507;
			}
						if(player1_level == 9 || player2_level == 9){
				(Part2.x) = 493;
			}
				if(player1_level == 13 || player2_level == 13){
				(Part2.x) = 504;
			}
		}
		if(dropping2 == true){
			(Part2.y) +=5;
		}
		if((Part2.y) >= 409 && (Part2.num_part) == 2 && dropping2 == true){
			dropping2 = false;
		if((player1_level == 1 || player1_level == 5) || (player2_level == 1 || player2_level == 5)){
				(Part2.x) = 507;
			}
			if(player1_level == 9 || player2_level == 9){
				(Part2.x) = 493;
			}
			if(player1_level == 13 || player2_level == 13){
				(Part2.x) = 504;
			}
			(Part2.y) = 417;
			spaceship.num_parts=3;
      if(g_player_one_turn == true){
      player1_score += 100;
      }else{
      player2_score += 100;
      }
		}
	*(spaceshipparts + 2) = Part2;
}
void FuelGeneration(){
	int aux;
	if(spaceship.num_parts == 3 && fuelcounter == false){
		aux = rand()%100;
		if(aux <= 10){
			fuelcounter = true;
			fuel.x = rand()%(kwindow_x-esat::SpriteWidth(fuel.sprite));
			fuel.y = 0;
		}
	}	
}
void FuelMovement(){
	if(fuelcounter == true){

		fuel.y += 5;

		if((fuel.y >= 175 && fuel.y <= 178) && (fuel.x >= 95 && fuel.x <= 250)){
			fuel.y = 175;//izquierda
		}
		if((fuel.y >= 282 && fuel.y <= 285) && (fuel.x >= 310 && fuel.x <= 460)){
			fuel.y = 282;//medio
		}
		if(fuel.y <= 100 && (fuel.x >= 531 && fuel.x <= kwindow_x)){
			fuel.y = 100;//derecha
		}
		if(fuel.y >= 532 && (fuel.x >= 0 && fuel.x <= kwindow_x - esat::SpriteWidth(fuel.sprite))){
			fuel.y = 532;//suelo abajo
		}
	}
}
void CollectingFuel(){
	if(fuelcounter == true){
		if((player.x <= (fuel.x) + esat::SpriteWidth((fuel.sprite))) &&
		  (player.x + esat::SpriteWidth(*player.animation) >= (fuel.x)) && 
		  (player.y  + esat::SpriteHeight(*player.animation) /2 <= (fuel.y) + esat::SpriteHeight((fuel.sprite))) && 
		  (player.y  + esat::SpriteHeight(*player.animation) >= (fuel.y) + esat::SpriteHeight((fuel.sprite)) / 2) &&
		  (fuel.is_fuel) == true){
			player.has_fuel = true;
			collecting3 = true;
		}
		if(player.has_fuel == true && collecting3 == true){
			(fuel.x) = player.x;
			(fuel.y) = player.y + esat::SpriteHeight(*player.animation) - esat::SpriteHeight((fuel.sprite));//controlar posicion correcta
		}
		if((fuel.x) >= 495 && (fuel.x)<=500 && collecting3 == true){
			collecting3 = false;
			player.has_fuel = false;
			dropping3 = true;
			fuel.x = 495;
		}
		if(dropping3 == true){
			(fuel.y) +=5;
		}
		if((fuel.y) >= 517){
			dropping3 = false;
			fuelcounter = false;
			fuel.y = 517;
			spaceship.num_fuel++;
		}
	}
}

//DRAW
void DrawBackground(){
	for(int i = 0; i < g_counter_map; ++i){
		map = *(all_map + i);
		if(map.init == 24){
			esat::DrawSprite(*(map.sprite + map.type), map.x,map.y);
		}else{
			esat::DrawSprite(*(map.sprite + map.type + 3), map.x,map.y);
		}

	}
}
void DrawPlayerUpDownRight(){
	if(player.steps < 4){
					esat::DrawSprite(*(player.animation + 12), player.x,player.y);
				}
				if (player.steps >= 4 && player.steps  < 8){
					esat::DrawSprite(*(player.animation + 13), player.x,player.y);
				}
				if (player.steps >= 8 && player.steps  < 12){
					esat::DrawSprite(*(player.animation + 14), player.x,player.y);
				}
				if (player.steps >= 12 && player.steps  < 16){
					esat::DrawSprite(*(player.animation + 15), player.x,player.y);
				}
				if (player.steps >= 16 ){
					player.steps = 0;
					esat::DrawSprite(*(player.animation + 12), player.x,player.y);
				}
}
void DrawPlayerUpDownLeft(){
	if(player.steps < 4){
					esat::DrawSprite(*(player.animation + 8), player.x,player.y);
				}
				if (player.steps >= 4 && player.steps  < 8){
					esat::DrawSprite(*(player.animation + 9), player.x,player.y);
				}
				if (player.steps >= 8 && player.steps  < 12){
					esat::DrawSprite(*(player.animation + 10), player.x,player.y);
				}
				if (player.steps >= 12 && player.steps < 16){
					esat::DrawSprite(*(player.animation + 11), player.x,player.y);
				}
				if (player.steps >= 16 ){
					player.steps = 0;
					esat::DrawSprite(*(player.animation + 8), player.x,player.y);
				}
}
void DrawPlayer(){
	switch(player.dir){

		case 0:
			if(player.platform){
				if(player.steps < 4){
				esat::DrawSprite(*(player.animation + 4), player.x,player.y);
			}
			if (player.steps >= 4 && player.steps  < 8){
				esat::DrawSprite(*(player.animation + 5), player.x,player.y);
			}
			if (player.steps >= 8 && player.steps  < 12){
				esat::DrawSprite(*(player.animation + 6), player.x,player.y);
			}
			if (player.steps >= 12 && player.steps < 16){
				esat::DrawSprite(*(player.animation + 7), player.x,player.y);
			}
			if (player.steps >= 16 ){
				player.steps = 0;
				esat::DrawSprite(*(player.animation + 4), player.x,player.y);
			}
			}else{
				DrawPlayerUpDownRight();
			}
			
		break;
		case 1:
			DrawPlayerUpDownRight();
		break;
		case 2:
		if(player.platform){
				if(player.steps < 4){
				esat::DrawSprite(*(player.animation + 0), player.x,player.y);
			}
			if (player.steps >= 4 && player.steps  < 8){
				esat::DrawSprite(*(player.animation + 1), player.x,player.y);
			}
			if (player.steps >= 8 && player.steps  < 12){
				esat::DrawSprite(*(player.animation + 2), player.x,player.y);
			}
			if (player.steps >= 12 && player.steps < 16){
				esat::DrawSprite(*(player.animation + 3), player.x,player.y);
			}
			if (player.steps >= 16 ){
				player.steps = 0;
				esat::DrawSprite(*(player.animation + 0), player.x,player.y);
			}
		}else{
			DrawPlayerUpDownLeft();
		}
	

		break;
		case 3:
			DrawPlayerUpDownLeft();
		break;
		case 4:
			if(!player.check_dir){
				DrawPlayerUpDownLeft();
			}else{
				DrawPlayerUpDownRight();
			}

		break;
		case 6:
			if(!player.check_dir){
				DrawPlayerUpDownLeft();
			}else{
				DrawPlayerUpDownRight();
			}
		break;
		case 8:
			if(!player.check_dir){
				esat::DrawSprite(*(player.animation + 0), player.x,player.y);
				player.steps = 0;
			}else{
					esat::DrawSprite(*(player.animation + 4), player.x,player.y);
					player.steps = 0;
			}
	}
}
void DrawShipParts(){
	//Dibuja las piezas de la nave durante el gameplay
	esat::DrawSprite((*(spaceshipparts + 0)).sprite, (*(spaceshipparts + 0)).x,(*(spaceshipparts + 0)).y);
	esat::DrawSprite((*(spaceshipparts + 1)).sprite, (*(spaceshipparts + 1)).x,(*(spaceshipparts + 1)).y);
	esat::DrawSprite((*(spaceshipparts + 2)).sprite, (*(spaceshipparts + 2)).x,(*(spaceshipparts + 2)).y);
}
void DrawFuel(){
	if(fuelcounter == true){
		esat::DrawSprite(fuel.sprite, fuel.x, fuel.y);
	}
}
void DrawShipPartsAnimation(){
	//Se encarga de dibujar los sprites durante la animacion
	esat::DrawSprite((*(spaceshipparts + 0)).sprite, (*(spaceshipparts + 0)).x,(*(spaceshipparts + 0)).y);
	esat::DrawSprite((*(spaceshipparts + 1)).sprite, (*(spaceshipparts + 1)).x,(*(spaceshipparts + 1)).y);
	esat::DrawSprite((*(spaceshipparts + 2)).sprite, (*(spaceshipparts + 2)).x,(*(spaceshipparts + 2)).y);
	if((*(spaceshipparts + 3)).y <= 510){
		if(g_firecount%2==0){
			esat::DrawSprite((*(spaceshipparts + 3)).sprite, (*(spaceshipparts + 3)).x, (*(spaceshipparts + 3)).y);
		}else{
			esat::DrawSprite(fire_small, (*(spaceshipparts + 3)).x, (*(spaceshipparts + 3)).y);
		}
	}
}
void ShipAnimation(){
	//Se llamará dentro del CheckLevel() y se encarga de hacer la animacion de cambio de nivel
	bool up = true;
	bool down = false;

	if(spaceship.num_parts == 3 && spaceship.num_fuel == 6 &&
		player.x + esat::SpriteWidth(*(player.animation)) >= (*(spaceshipparts + 0)).x &&
		player.x <= (*(spaceshipparts + 0)).x + esat::SpriteWidth((*(spaceshipparts + 0)).sprite) &&
		player.y >  (*(spaceshipparts + 2)).y && player.y < (*(spaceshipparts + 0)).y){
		spaceship.do_animation = true;
		g_firecount = 0;
		while(spaceship.do_animation == true && up == true && down == false){
			(*(spaceshipparts + 0)).y -= 5;
			(*(spaceshipparts + 1)).y -= 5;
			(*(spaceshipparts + 2)).y -= 5;
			(*(spaceshipparts + 3)).y -= 5;
			if((*(spaceshipparts + 2)).y <= 150){
				(*(spaceshipparts + 0)).y = 162;
				(*(spaceshipparts + 1)).y = 267;
				(*(spaceshipparts + 2)).y = 517;
				(*(spaceshipparts + 3)).y = 577;
				up = false;
				down = true;
			}
			DrawBackground();
			DrawShipPartsAnimation();
			g_firecount ++;
		}
		while(spaceship.do_animation == true && up == false && down == true){
			(*(spaceshipparts + 0)).y += 5;
			(*(spaceshipparts + 1)).y += 5;
			(*(spaceshipparts + 2)).y += 5;
			(*(spaceshipparts + 3)).y += 5;
			if((*(spaceshipparts + 0)).y  >= 517){
				(*(spaceshipparts + 0)).y = 162;
				(*(spaceshipparts + 1)).y = 267;
				(*(spaceshipparts + 2)).y = 517;
				(*(spaceshipparts + 3)).y = 577;
				up = false;
				down = true;
			}
			DrawBackground();
			DrawShipPartsAnimation();
			g_firecount ++;
		}
	}
}
void HUD(){

	/**g_cadena1 = &player1.score;
	*g_cadena2 = &player2.score;
	*g_cadena3 = &g_high_score;*/

	if(g_player_one_turn == true){				//high-score
		if(g_high_score < player1_score){
		g_high_score = player1_score;
	}else{
		if(g_high_score < player2_score){
			g_high_score = player2_score;
		}
	}
	}
  esat::DrawSetTextSize(30);
	esat::DrawSetFillColor(255, 255, 255);	//white color
	esat::DrawText(40, 30, "1UP");
	esat::DrawText(700, 30, "2UP");
	esat::DrawText(350, 60, g_cadena3);		//high-score

	if(g_player_one_turn == true){
		esat::DrawSprite(hud_lives, 150, 5);
		esat::DrawText(120, 30, g_cadena4);		//player 1 lives
	}else{
		esat::DrawSprite(hud_lives, 610, 5);
		esat::DrawText(650, 30, g_cadena5);		//player 2 lives
	}

 	esat::DrawSetFillColor(255, 255, 0);	//yellow color
	esat::DrawText(20, 60, g_cadena1);		//player 1 score
	esat::DrawText(700, 60, g_cadena2);		//player 2 score

	esat::DrawSetFillColor(37, 109, 123);	//blue/green color
	esat::DrawText(330, 30, "High-Score");
}
void DrawEnemies(){
		for (int i = 0; i < 8; ++i)
	{
		aux = *(p_enemy + i);

		esat::DrawSprite(aux.animation.A1,aux.x,aux.y);

	}
}
void Player_dead(){
	if(player.alive == false){

		if(die_counter < 76){
			player.x = -100;
			player.y = -100;
		die_counter++;
		printf("%d\n", die_counter);

		}

		if(die_counter > 75){
			die_counter = 0;
			if(g_player_one_turn == true){
				player1_lives--;
				g_player_one_turn = false;
			}else{
				player2_lives--;
				g_player_one_turn = true;

			}

			Reset_Player();
		}

	}
}
void Memory_Allocation(){
	g_cadena1 = (char*)malloc(6 * sizeof(char));
	g_cadena2 = (char*)malloc(6 * sizeof(char));
	g_cadena3 = (char*)malloc(6 * sizeof(char));
	g_cadena4 = (char*)malloc(1 * sizeof(char));
	g_cadena5 = (char*)malloc(1 * sizeof(char));
}


	


//FREE
void FreePointers(){
	free(map.sprite);
	free(all_map);
	free(player.animation);
	free(spaceshipparts);
  	free(g_cadena1);
	free(g_cadena2);
	free(g_cadena3);
	free(g_cadena4);
	free(g_cadena5);
}
void ReleaseSprites(){
	esat::SpriteRelease(sheet);
	esat::SpriteRelease(sheet_verde);
	esat::SpriteRelease(sheet_amarillo);
	esat::SpriteRelease(sheet_azul);
	esat::SpriteRelease(sheet_rojo);
	esat::SpriteRelease(sheet_rosa);
}
int esat::main(int argc, char **argv) {

  double current_time,last_time;
  unsigned char fps=25;

  esat::WindowInit(kwindow_x,kwindow_y);
  WindowSetMouseVisibility(true);
  srand(time(NULL));
  esat::DrawSetTextFont("./Recursos/Fuente/JetpacFont.ttf");
     ///////////////LOADS/////////////
  LoadSheet();
  LoadSprites();
  p_enemy = (Enemy*) malloc(8*sizeof(Enemy));




  sheet_verde = esat::SpriteFromFile("./Recursos/Sprites/sheetverde.jpg");
  sheet_rojo = esat::SpriteFromFile("./Recursos/Sprites/sheetrojo.jpg");
  sheet_azul = esat::SpriteFromFile("./Recursos/Sprites/sheetazul.jpg");
  sheet_rosa = esat::SpriteFromFile("./Recursos/Sprites/sheetrosa.jpg");
  printf("Sheet Loaded\n");

  
  Memory_Allocation();
     ///////////////INITS/////////////
  spaceshipparts = (Items*) malloc(4*sizeof(Items));
  InitMap();
  InitPlayer();
  InitEnemies();
  InitFuel();
  InitShipParts(player.level);
  while(esat::WindowIsOpened() && !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {
	last_time = esat::Time();
  itoa(player1_score, g_cadena1, 10);		//player 1 points
  itoa(player2_score, g_cadena2, 10);		//player 2 points
  itoa(g_high_score, g_cadena3, 10);		//high-score
  itoa(player1_lives, g_cadena4, 10);		//player 1 lives
  itoa(player2_lives, g_cadena5, 10);		//player 2 lives

	esat::DrawBegin();

   ///////////////CHECKS/////////////
	InputPlayer();
	CheckPlayer();
	EnemyChecking();
  	Check_Level();
  	Player_dead();
  	FuelGeneration();

   ///////////////UPDATES/////////////
	UpdatePlayer();
  	Part1SPMovement();
	Part2SPMovement();
	FuelMovement();
	HUD();




   ///////////////DRAWS/////////////
	DrawBackground();
	DrawPlayer();
	DrawShipParts();
	DrawFuel();
	DrawEnemies();

    esat::DrawClear(0,0,0);


    esat::DrawEnd();

	do{
    current_time = esat::Time();
    }while((current_time-last_time)<=1000.0/fps);
    esat::WindowFrame();
  }

  ///////////////FREE/////////////
  FreePointers();
  ReleaseSprites();
  esat::WindowDestroy();
  return 0;
}
