//
//  main.mm
//  S.C.Out
//
//  Created by Paul MERCAT on 02/05/08.
//  Copyright 2008. All rights reserved.
//

#define MAC	true

#define TTF	true

#define HSON	true

#define NI	420 //number of images in the game (number of objects)

#define EDIT	false

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#if MAC
const char *RESOURCES = "S.C.Out.app/Contents/Resources/"; //resource folder in Mac apps
const char *DSONS = ""; //sounds folder
const char *TAB = "Tableaux/"; //tables folder
#import <Cocoa/Cocoa.h>
#import <SDL/SDL.h>
#if TTF
#import <SDL_ttf/SDL_ttf.h>
#endif
#else
const char *RESOURCES = ""; //resource folder
const char *DSONS = "Sons/"; //sounds folder
const char *TAB = "Tableaux/"; //tables folder
#include <SDL/SDL.h>
#if TTF
#include <SDL/SDL_ttf.h>
#endif
#endif
#include "read.h"
#include "sound.h"
#include "tab.h"
#include "regles.h"
#include "anims.h"

#define FINAL	true
#define TRICHE	false
bool ROUGE = true;

#if FINAL && !EDIT
#define SCREENX 512 //1280 //640 //window dimensions
#define SCREENY	380 //800 //480

#define NX	10 //40 //10 //10 //number of visible boxes
#define NY	8 //24 //8 //8
#else
#define SCREENX 1280 //640 //window dimensions
#define SCREENY	800 //480

#define NX	40 //10 //10 //number of visible boxes
#define NY	24 //8 //8
#endif

#define CX	32 //box size in pixels
#define CY	25

#import "edit.h"

#define DHEAR	10 //sound distance in boxes
#define DHEAR2	(DHEAR*DHEAR)

#define TANIM	30 //minimum time between two frame animations (in milliseconds)

#if FINAL
#define TVUE	20 //time between displacements of sight?? (temps entre deux deplacements de la vue)
#define NVUE	10 //field of view picture number?? (nombre d'image d'un deplacement du champ de vision)
#define NMUS0	5	//number of images of a displacement?? (nombre d'images d'un deplacement de nous)
Uint32 NMUS	 = NMUS0;	//field of view picture number?? (nombre d'image d'un deplacement du champ de vision)
#define TMUS	30	//time between two images of displacement?? (temps entre deux images d'un deplacement de nous)
#else
#define TVUE	5 //time between displacement of sight (temps entre deux deplacements de la vue)
#define NVUE	24 //field of view picture number?? (nombre d'image d'un deplacement du champ de vision)
#define NMUS0	5	//number of images of a displacement?? (nombre d'images d'un deplacement de nous)
Uint32 NMUS	 = NMUS0;	//field of view picture number?? (nombre d'image d'un deplacement du champ de vision)
#define TMUS	20	//time between two images of displacement?? (temps entre deux images d'un deplacement de nous)
#endif
#define tNMUS0	10 //remaining before the end of the acceleration (in many cases) time?? (temps restant avant fin de l'acceleration (en nombre de cases))
Uint32 tNMUS = 0; //time remaining before the end of the acceleration (temps restant avant fin de l'acceleration)

#define NBMAX	1000 //maximum number of balls
#define TBALLS	20	//time between two images for bullets
#define VBALL	3	//displacement every time (number of pixels) - movement?
#define TTAPIS_ROULANT	2 //time between two images? (temps entre deux images)

#define INVINCIBLE	300 //invincible time (temps d'invincibilite)

Uint8 GetSens (int dx, int dy);
Uint8 GetSens2 (int dx, int dy);
char inline deplx (Uint8 sens);
char inline deply (Uint8 sens);
Sint8 signe (Sint8 n);
void pause (Uint32 t);
Pos GetRailSens (Uint16 c, Uint8 sens); //gives meaning to the rail which is depending on the direction of entry (donne le sens vers lequel va le rail en fonction du sens d'entree)
Pos GetRailSens (Uint16 c); //gives meaning to which the rail will?? (donne le sens vers lequel va le rail)

struct Ball
{
	Uint16 x,y;	//position
	Sint8	vx,vy; //speed
	//SDL_Surface s;
	bool used; //is ball active
	Uint8 react; //did the ball react with a box? // 0 = no, 1 = between in box, 2 = middle of the box (la balle a deja reagit avec a case ?) //0 = non, 1 = entre dans la case, 2 = milieu de la case
	
	void init ();
};

enum
{
	wmur = 0,
	wplante = 1,
	wfleche = 2
};

enum
{
	whaut = 1 << 0,
	wbas = 1 << 1,
	wdroite = 1 << 2,
	wgauche = 1 << 3
};

struct Well
{
	Uint16 t[2*2*2*2];
	
	void init (Uint8 which);
};

//win
enum
{
	perdu = 0,
	gagne = 1,
	kabort = 2,
	ingame = 3
};

//touches
#define NTOUCHES	8
enum
{
	thaut = 0,
	tbas  = 1,
	tdroite = 2,
	tgauche = 3,
	ttirer = 4,
	tprendre  = 5,
	tteleporter = 6,
	tsuicide = 7
};

struct Jeu //Jeu == Game
{
	Sons s; //sound management
	Tab t; //current table (table = level?)
	Pos pr; //our position in pixels, relative to the case (case == frame?)
	Regles r[NI]; //rules of each object
	Anim a[NA]; //list of events
	Ball b[NBMAX]; //list of bullets
	SDL_Surface *ecran; //screen
	SDL_Surface *im; //game images
	SDL_Surface *jeu; //image of full level??
	SDL_Surface *ic; //image of frame??
	Well w[3]; //used by the editor to improve?? (utilise par l'editeur pour ameliorer)
#if TTF
	TTF_Font *police;
#endif
	SDLKey key[NTOUCHES]; //list of control keys
	SDL_Rect vue; //image area
	SDL_Rect pvue; //view position on screen?
	bool quit;
	bool quitanim;
	Uint8 win; //was gained?? (on a gagné ?)
	bool moveus; //we must move forward?
	bool canmove; //can change direction
	Uint16 invincible; //remaining invincible time
	Uint16 vposx;
	Uint16 vposy;
	
	bool dovuerun;	//used by DoVue
	bool updatevue; //used by DoVue
	bool paused;
	
	void init ();
	void end ();
	void Play (Uint32 niveau); //start the game in the current level (the argument is only used for cheating)
	//
	void Draw (); //redraw all
	inline void DrawSol (Uint16 x, Uint16 y); //draws the floor of box x,y
	inline void Draw (Uint16 x, Uint16 y); //redraws the box x,y
	inline void DrawUs ();
	void DrawBall (Uint16 x, Uint16 y);
	void MoveUs (Uint8 sens);
	void Update (); //redraw visible area
	void Anime (Uint16 x, Uint16 y, Uint16 n); //lance animation?? (lance l'animation n en (x,y))
	void DoBall (Uint16 x, Uint16 y); //action of a bullet explosion x,y
	void DoExplosif (Uint16 x, Uint16 y); //start explosive at x,y
	void DoGrenade (Uint16 x, Uint16 y); //start grenade x,y
	void DoPorte (Uint16 x, Uint16 y); //start an object that react when we pass by (demarre un objet reagissant quand on passe a cote)
	void UpdateVue (Uint16 x, Uint16 y); //day makes the field of view?? (met à jour le champ de vue)
	void Perdu ();
	void ReactOn (Uint16 x, Uint16 y);
	void OpenPortes ();
	void OuvrePortes (Uint16 w);
	void OpenBarriere ();
	void CloseBarriere ();
	void InitBalls ();
	void Balle(Uint16 x,Uint16  y,int dx,int dy); //shoot bullet from x,y in direction dx,dy
	//void Balle2(Uint16 x,Uint16  y,int dx,int dy); //shoot bullet from x,y in direction dx,dy from center??
	void StartVue (); //instead the field of view or must?? (place le champ de vision où il faut)
	Pos GoodVue (); //should be the field of view?? (là ou devrait être le champ de vision !)
	void ActiveRail (Uint16 x, Uint16 y); //activates a rail (rail == ??)
	void DesactiveRail (Uint16 x, Uint16 y); //disables a rail
	void Teleporte (Uint16 w);
	void DoOrdi (Uint16 w);
	void DoPastille (Uint16 x, Uint16 y, Uint16 w);
	void Vue (); //loupe
	void MoveVueTo (SDL_Rect nvue);
	void MoveVueTo2 (SDL_Rect nvue);
	void PutRed (Uint8 red);
	void Main (); //page de depart
	int EntreCode ();
	void EditKeys (); //control settings
	void EditSon (); //sound setting
	void EditGraph (); //graphic settings
	void EditParams (); //game settings
	void ChooseTab (Uint16 *niveau); //level select
	void SavePrefs (); 
	void LoadPrefs (); 
	void DispInfo (Uint16 niveau); //display information (level + code)
	void Recadre (); //puts things in place
	void Pause (); //
	void DoPause (); //thread break??
 	//Train
	void initRail (Uint16 x, Uint16 y, Uint16 t, Uint8 sens); //initializes the rail (x, y) and the following (initialise le rail en (x,y) et les suivants)
	void initTrain (); //initializes automatic rails
	void DesactiveRails (); //disable all rails
	Uint16 ActiveRail (Uint16 rail); //activates a rail
	Uint16 DesactiveRail (Uint16 rail); //disables a rail
	//
	void Load (Uint16 n); //load level
	void Save (Uint16 n); //save
	void Update_tsens (); //??? (met à jour tsens)
	//
	void Edit (Uint32 niveau);
	//void LoadTab (Uint32 n); //load user level
	//void SaveTab (Uint32 n); //save user level
	Uint16 WellTruc (Uint16 x, Uint16 y, Uint32 truc, Uint32 wtruc);
	void PutWellTruc (Uint16 x, Uint16 y, Uint32 truc, Uint32 wtruc);
	void PutWell (Uint16 x, Uint16 y, Uint16 what);
};

int max (int a, int b)
{
	if (a < b)
		return b;
	return a;
}

void Well::init (Uint8 which)
{
	switch (which)
	{
		case wmur: //walls
			t[0] = 28;
			t[wgauche] = 27;
			t[wdroite] = 16;
			t[wgauche+wdroite] = 26;
			t[wbas] = 24;
			t[wbas+wgauche] = 18;
			t[wbas+wdroite] = 19;
			t[wbas+wdroite+wgauche] = 72;
			t[whaut] = 25;
			t[whaut+wgauche] = 14;
			t[whaut+wdroite] = 29;
			t[whaut+wdroite+wgauche] = 75;
			t[whaut+wbas] = 17;
			t[whaut+wbas+wgauche] = 73;
			t[whaut+wbas+wdroite] = 74;
			t[whaut+wbas+wdroite+wgauche] = 15;
			break;
		case wplante: //plants
			t[0] = 68;
			t[wgauche] = 76;
			t[wdroite] = 77;
			t[wgauche+wdroite] = 54;
			t[wbas] = 78;
			t[wbas+wgauche] = 57;
			t[wbas+wdroite] = 56;
			t[wbas+wdroite+wgauche] = 64;
			t[whaut] = 79;
			t[whaut+wgauche] = 59;
			t[whaut+wdroite] = 58;
			t[whaut+wdroite+wgauche] = 65;
			t[whaut+wbas] = 55;
			t[whaut+wbas+wgauche] = 66;
			t[whaut+wbas+wdroite] = 67;
			t[whaut+wbas+wdroite+wgauche] = 69;
			break;
		case wfleche: //arrows
			t[0] = 0;
			t[wgauche] = 0;
			t[wdroite] = 0;
			t[wgauche+wdroite] = 47;
			t[wbas] = 0;
			t[wbas+wgauche] = 51;
			t[wbas+wdroite] = 50;
			t[wbas+wdroite+wgauche] = 0;
			t[whaut] = 0;
			t[whaut+wgauche] = 42;
			t[whaut+wdroite] = 40;
			t[whaut+wdroite+wgauche] = 0;
			t[whaut+wbas] = 46;
			t[whaut+wbas+wgauche] = 0;
			t[whaut+wbas+wdroite] = 0;
			t[whaut+wbas+wdroite+wgauche] = 0;
			break;
	}
}

void Ball::init ()
{
	used = false;
	react = false;
}

void Jeu::SavePrefs ()
{
	FILE *f = fopen("prefs.dat","w+");
	if (f)
	{
		fwrite(&SON, sizeof(SON), 1, f);
		fwrite(&ROUGE, sizeof(ROUGE), 1, f);
		Uint32 i;
		for (i=0;i<NTOUCHES;i++)
		{
			fwrite(&key[i], sizeof(key[i]), 1, f);
		}
		fclose(f);
	}else
	{
		fprintf(stderr, "Error: Unable to open (or create) the preferences file!\n");
	}
}

void Jeu::LoadPrefs ()
{
	FILE *f = fopen("prefs.dat","r");
	if (f)
	{
		fread(&SON, sizeof(SON), 1, f);
		fread(&ROUGE, sizeof(ROUGE), 1, f);
		Uint32 i;
		for (i=0;i<NTOUCHES;i++)
		{
			fread(&key[i], sizeof(key[i]), 1, f);
		}
		fclose(f);
	}else
	{
		//fprintf(stderr, "Error: Unable to open the preferences file!\n");
	}
}

void Jeu::Load (Uint16 n)
{
	t.Load(n);
	initTrain();
}

void Jeu::Save (Uint16 n)
{
	t.Save(n);
}

void FadeIn (SDL_Surface *s, SDL_Surface *ecran)
{
	SDL_Event event;
	Uint32 ti,rti;
	SDL_Rect dest;
	rti = SDL_GetTicks();
	int i;
	for (i=0;i<=12;i++)
	{
		ti = SDL_GetTicks();
		if (ti-rti < 50)
			SDL_Delay(50-(ti-rti));
		rti = SDL_GetTicks();
		SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
		dest.x = (ecran->w - s->w)/2;
		dest.y = (ecran->h - s->h)/2;
		SDL_SetAlpha(s, SDL_SRCALPHA, 20*i+15);
		SDL_BlitSurface(s, NULL, ecran, &dest);
		SDL_Flip(ecran);
		SDL_PollEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				i = 100;
				SDL_PushEvent(&event);
				break;
			case SDL_KEYDOWN:
				i = 100;
				SDL_PushEvent(&event);
				break;
		}
	}
	SDL_SetAlpha(s, SDL_SRCALPHA, 255);
	dest.x = (ecran->w - s->w)/2;
	dest.y = (ecran->h - s->h)/2;
	SDL_BlitSurface(s, NULL, ecran, &dest);
	SDL_Flip(ecran);
}

void FadeOut (SDL_Surface *s, SDL_Surface *ecran)
{
	SDL_Event event;
	Uint32 ti,rti;
	rti = SDL_GetTicks();
	int i;
	SDL_Rect dest;
	for (i=0;i<12;i++)
	{
		ti = SDL_GetTicks();
		if (ti-rti < 50)
			SDL_Delay(50-(ti-rti));
		rti = SDL_GetTicks();
		SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
		dest.x = (ecran->w - s->w)/2;
		dest.y = (ecran->h - s->h)/2;
		SDL_SetAlpha(s, SDL_SRCALPHA, 255 - 20*i);
		SDL_BlitSurface(s, NULL, ecran, &dest);
		SDL_Flip(ecran);
		SDL_PollEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				i = 100;
				SDL_PushEvent(&event);
				break;
			case SDL_KEYDOWN:
				i = 100;
				SDL_PushEvent(&event);
				break;
		}
	}
	SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
	SDL_Flip(ecran);
}

int GetNiveau (const char *txt)
{
	char tamp[256];
	MakeName("codes.txt", tamp);
	FILE *f = fopen(tamp, "r");
	int niveau = -1;
	Uint16 k;
	int i;
	if (f)
	{
		char tamp[256];
		const char *ptr,*ptr2;
		for (;;)
		{
			if (!readline(f, tamp, 256))
				break;
			ptr = tamp;
			ptr = findnum (ptr, &k);
			//printf("k = %d ",k);
			if (!ptr)
				continue;
			ptr = pass(ptr); //forgot spaces ?? (passe les espaces)
			//printf(ptr);
			//printf("\n");
			ptr2 = txt;
			for (i=0;i<6;i++)
			{
				if (*ptr != *ptr2)
				{
					i = 0;
					break;
				}
				ptr++;
				ptr2++;
			}
			if (i)
			{
				niveau = k-1;
				break;
			}
		}
		fclose(f);
	}else
	{
		fprintf(stderr, "Unable to open the code file!\n");
	}
	return niveau;
}

bool GetNiveau (int niveau, char *txt)
{
	char tamp[256];
	MakeName("codes.txt", tamp);
	FILE *f = fopen(tamp, "r");	
	Uint16 k;
	int i;
	if (f)
	{
		char tamp[256];
		const char *ptr;
		char *ptr2;
		for (;;)
		{
			if (!readline(f, tamp, 256))
				break;
			ptr = tamp;
			ptr = findnum (ptr, &k);
			if (!ptr)
				continue;
			if (k == niveau)
			{
				ptr = pass(ptr);
				ptr2 = txt;
				for (i=0;i<6;i++)
				{
					*ptr2 = *ptr;
					ptr2++;
					ptr++;
				}
				*ptr2 = 0;
				return true;
			}
		}
		fclose(f);
	}else
	{
		fprintf(stderr, "Unable to open the code file !!!\n");
	}
	return false;
}

int Jeu::EntreCode ()
{
#if TTF
	SDL_Color blanc = {255, 255, 255};
	SDL_Rect dest;
	SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
	SDL_Surface *txt = TTF_RenderText_Blended(police, "Between code from one level :", blanc); //(Entre le code d'un niveau)
	dest.x = (ecran->w - txt->w)/2;
	dest.y = 100;
	SDL_BlitSurface(txt, NULL, ecran, &dest);
	SDL_Flip(ecran);
	SDL_FreeSurface(txt);
	bool cont = true;
	SDL_Event event;
	char tamp[255]; //text between
	Uint16 pos = 0; //cursor position (number of letters)
	bool update = false;
	bool wrong = false;
	SDL_EnableUNICODE(1);
	Uint16 c;
	int niveau = -1;
	do
	{
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				cont = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						cont = false;
						break;
					case SDLK_BACKSPACE:
						if (pos)
						{
							s.Play(16); //bomb explosion
							pos--;
							tamp[pos] = 0;
							update = true;
						}
						break;
					case SDLK_RETURN:
						niveau = GetNiveau(tamp);
						if (niveau != -1)
						{
							s.Play(4);
							cont = false;
						}else
						{
							wrong = true;
						}
						break;
				}
				c = toupper(event.key.keysym.unicode); //event.key.keysym.sym; //event.key.keysym.unicode);
				if (c >= 'A' && c <= 'Z') //SDLK_a && c <= SDLK_z)
				{
					if (pos < 254)
					{
						s.Play(13);
						tamp[pos] = c;
						pos++;
						tamp[pos] = 0;
						update = true;
					}
				}
				break;
		}
		if (update || wrong)
		{
			dest.x = 200;
			dest.y = 200;
			dest.w = 400;
			dest.h = 50;
			SDL_FillRect(ecran, &dest, SDL_MapRGB(ecran->format, 0, 0, 0));
			txt = TTF_RenderText_Blended(police, tamp, blanc);
			dest.x = 200;
			dest.y = 200;
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			SDL_Flip(ecran);
			SDL_FreeSurface(txt);
			if (wrong)
			{
				SDL_FillRect(ecran, &dest, SDL_MapRGB(ecran->format, 0, 0, 0));
				txt = TTF_RenderText_Blended(police, "Bad code!", blanc);
				dest.x = 200;
				dest.y = 300;
				SDL_BlitSurface(txt, NULL, ecran, &dest);
				SDL_Flip(ecran);
				SDL_FreeSurface(txt);
			}else
			{
				dest.x = 200;
				dest.y = 300;
				dest.w = 400;
				dest.h = 50;
				SDL_FillRect(ecran, &dest, SDL_MapRGB(ecran->format, 0, 0, 0));
			}
			update = false;
			wrong = false;
		}
	}while(cont);
	SDL_EnableUNICODE(0);
	return niveau;
#endif
}
/*
void DrawTxt (const char *texte, Uint16 x, Uint16 y, SDL_Surface *ecran)
{
	SDL_Color blanc = {255, 255, 255};
	SDL_Surface *txt = TTF_RenderText_Blended(police, texte, blanc);
	dest.x = x;
	dest.y = y;
	SDL_BlitSurface(txt, NULL, ecran, &dest);
}
*/

void Jeu::EditSon ()
{
	SDL_Rect dest;
	char tamp[255];
	SDL_Color blanc = {255, 255, 255};
	SDL_Surface *txt;
	Uint32 c = 0;
	bool cont = true;
	bool update = true;
	SDL_Event event;
	Uint32 touche;
	do
	{
		if (update)
		{
			update = false;
			SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
#if TTF
			if (SON)
				txt = TTF_RenderText_Blended(police, "Remove the F1", blanc);
			else
				txt = TTF_RenderText_Blended(police, "F1 Unmute", blanc);
			dest.x = 20;
			dest.y = 20;
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			SDL_FreeSurface(txt);
			txt = TTF_RenderText_Blended(police, "ESC Back", blanc); //ESC Retour
			dest.x = 20;
			dest.y = 50;
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			SDL_FreeSurface(txt);
#endif
			SDL_Flip (ecran);
		}
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				cont = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						cont = false;
						break;
					case SDLK_F1:
						SON = !SON;
						s.Play(4);
						cont = false;
						break;
				}
				break;
		}
	}while(cont);
}

void Jeu::EditGraph ()
{
	SDL_Rect dest;
	char tamp[255];
	SDL_Color blanc = {255, 255, 255};
	SDL_Surface *txt;
	Uint32 c = 0;
	bool cont = true;
	bool update = true;
	SDL_Event event;
	Uint32 touche;
	do
	{
		if (update)
		{
			update = false;
			SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
#if TTF
			if (ROUGE)
				txt = TTF_RenderText_Blended(police, "Remove the red flashing F1", blanc);
			else
				txt = TTF_RenderText_Blended(police, "Return the flashing red F1", blanc);
			dest.x = 20;
			dest.y = 20;
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			SDL_FreeSurface(txt);
			txt = TTF_RenderText_Blended(police, "ESC Back", blanc);
			dest.x = 20;
			dest.y = 50;
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			SDL_FreeSurface(txt);
#endif
			SDL_Flip (ecran);
		}
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				cont = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						cont = false;
						break;
					case SDLK_F1:
						ROUGE = !ROUGE;
						s.Play(4);
						cont = false;
						break;
				}
				break;
		}
	}while(cont);
}

void Jeu::EditKeys () //allows the user to change the control keys
{
	SDL_Rect dest;
	char tamp[255];
	SDL_Color blanc = {255, 255, 255};
	SDL_Surface *txt;
	Uint32 c = 0;
	bool cont = true;
	bool update = true;
	SDL_Event event;
	Uint32 touche;
	do
	{
		if (update)
		{
			update = false;
			SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
#if TTF
			switch (c)
			{
				case 0:
					txt = TTF_RenderText_Blended(police, "Key to move left ?", blanc);
					break;
				case 1:
					txt = TTF_RenderText_Blended(police, "Key to move right?", blanc);
					break;
				case 2:
					txt = TTF_RenderText_Blended(police, "Key to move up?", blanc);
					break;
				case 3:
					txt = TTF_RenderText_Blended(police, "Key to move down?", blanc);
					break;
				case 4:
					txt = TTF_RenderText_Blended(police, "Key to take/deposit ?", blanc);
					break;
				case 5:
					txt = TTF_RenderText_Blended(police, "Key to shoot?", blanc);
					break;
				case 6:
					txt = TTF_RenderText_Blended(police, "Key to teleport?", blanc);
					break;
				case 7:
					txt = TTF_RenderText_Blended(police, "Key to give up?", blanc);
					break;
			}
			dest.x = 20;
			dest.y = 20;
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			dest.x += txt->w;
			SDL_FreeSurface(txt);
#endif
			SDL_Flip (ecran);
		}
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				cont = false;
				break;
			case SDL_KEYDOWN:
				switch (c)
				{
					case 0:
						touche = tgauche;
						break;
					case 1:
						touche = tdroite;
						break;
					case 2:
						touche = thaut;
						break;
					case 3:
						touche = tbas;
						break;
					case 4:
						touche = tprendre;
						break;
					case 5:
						touche = ttirer;
						break;
					case 6:
						touche = tteleporter;
						break;
					case 7:
						touche = tsuicide;
						break;
				}
				key[touche] = event.key.keysym.sym;
				s.Play(4); //serrure
				c++;
				if (c > 7)
					return;
					update = true;
		}
	}while(cont);
}

void Jeu::EditParams ()
{
	SDL_Rect dest;
	char tamp[255];
	SDL_Color blanc = {255, 255, 255};
	SDL_Surface *txt;
	Uint32 c = 0;
	bool cont = true;
	bool update = true;
	SDL_Event event;
	Uint32 touche;
	do
	{
		if (update)
		{
			update = false;
			SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
#if TTF
			dest.x = 20;
			dest.y = 20;
			txt = TTF_RenderText_Blended(police, "Parameters", blanc);
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			dest.x += txt->w;
			SDL_FreeSurface(txt);
			dest.x = 20;
			dest.y = 90;
			txt = TTF_RenderText_Blended(police, "F1 Changes Keys", blanc);
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			dest.x += txt->w;
			SDL_FreeSurface(txt);
			dest.x = 20;
			dest.y = 140;
			txt = TTF_RenderText_Blended(police, "F2 Adjust Sound", blanc);
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			dest.x += txt->w;
			SDL_FreeSurface(txt);
			dest.x = 20;
			dest.y = 190;
			txt = TTF_RenderText_Blended(police, "F3 Graphics", blanc);
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			dest.x += txt->w;
			SDL_FreeSurface(txt);
			dest.x = 20;
			dest.y = 240;
			txt = TTF_RenderText_Blended(police, "ESC Back", blanc);
			SDL_BlitSurface(txt, NULL, ecran, &dest);
			dest.x += txt->w;
			SDL_FreeSurface(txt);
#endif
			SDL_Flip (ecran);
			SavePrefs ();
		}
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				cont = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						cont = false;
						break;
					case SDLK_F1:
						EditKeys();
						update = true;
						break;
					case SDLK_F2:
						EditSon();
						update = true;
						break;
					case SDLK_F3:
						EditGraph();
						update = true;
						break;
				}
		}
	}while(cont);
}

void Jeu::DispInfo (Uint16 niveau)
{
	niveau++;
	
	SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
	SDL_Rect dest;
	char tamp[255];
	SDL_Color blanc = {255, 200, 100};
	SDL_Surface *txt;
#if TTF
	txt = TTF_RenderText_Blended(police, "Level ", blanc);
	dest.x = 20;
	dest.y = 20;
	SDL_BlitSurface(txt, NULL, ecran, &dest);
	dest.x += txt->w;
	SDL_FreeSurface(txt);
	tamp[2] = '0' + (niveau%10);
	tamp[1] = '0' + ((niveau/10)%10);
	tamp[0] = '0' + (niveau/100);
	tamp[3] = 0;
	if (tamp[0] == '0' && tamp[1] == '0')
		tamp[1] = ' ';
	if (tamp[0] == '0')
		tamp[0] = ' ';
	txt = TTF_RenderText_Blended(police, tamp, blanc);
	SDL_BlitSurface(txt, NULL, ecran, &dest);
	SDL_FreeSurface(txt);
	
	if (GetNiveau(niveau, tamp))
	{
		txt = TTF_RenderText_Blended(police, "Code ", blanc);
		dest.x = 20;
		dest.y = 40;
		SDL_BlitSurface(txt, NULL, ecran, &dest);
		dest.x += txt->w;
		SDL_FreeSurface(txt);
		txt = TTF_RenderText_Blended(police, tamp, blanc);
		SDL_BlitSurface(txt, NULL, ecran, &dest);
		SDL_FreeSurface(txt);
	}
#endif
}

void Jeu::Main ()
{
	//affichage de l'image de Kalisto
	char tamp[256];
	MakeName("Kalisto.bmp", tamp);
	SDL_Surface *s = SDL_LoadBMP (tamp);
	//SDL_Surface *sf = SDL_CreateRGBSurface(SDL_HWSURFACE, s->w, s->h, 32, 0, 0, 0, 0); //fond
	if (!s)
		fprintf(stderr, "Kalisto image missing\n");
	int i;
	Uint32 ti,rti;
	SDL_Event event;
	SDL_Rect src, dest;
	FadeIn(s, ecran);
	bool cont = true;
	rti = SDL_GetTicks();
	do
	{
		ti = SDL_GetTicks();
		if (ti-rti > 2000)
			break;
		SDL_Delay(10);
		SDL_PollEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				SDL_PushEvent(&event);
				cont = false;
				break;
			case SDL_KEYDOWN:
				SDL_PushEvent(&event);
				cont = false;
				break;
		}
	}while(cont);
	FadeOut(s, ecran);
	SDL_FreeSurface(s);
	
	//affichage de l'ecran de depart
	MakeName("S.C.OUT.bmp",tamp);
	s = SDL_LoadBMP (tamp);
	SDL_Color blanc = {255, 200, 100};
#if TTF
	SDL_Surface *txt = TTF_RenderText_Blended(police, "F1 Play", blanc);
	dest.x = 20;
	dest.y = 300;
	SDL_BlitSurface(txt, NULL, s, &dest);
	SDL_FreeSurface(txt);
	txt = TTF_RenderText_Blended(police, "F2 Enter Code", blanc);
	dest.x = 20;
	dest.y = 320;
	SDL_BlitSurface(txt, NULL, s, &dest);
	SDL_FreeSurface(txt);
	txt = TTF_RenderText_Blended(police, "F3 Settings", blanc);
	dest.x = 20;
	dest.y = 340;
	SDL_BlitSurface(txt, NULL, s, &dest);
	SDL_FreeSurface(txt);
	txt = TTF_RenderText_Blended(police, "ESC Quit", blanc);
	dest.x = 350;
	dest.y = 340;
	SDL_BlitSurface(txt, NULL, s, &dest);
	SDL_FreeSurface(txt);
#endif
	FadeIn(s, ecran);
	cont = true;
	Uint16 niveau = 0;
	int niv;
	do
	{
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				cont = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						cont = false;
						break;
					case SDLK_F1:
						FadeOut(s, ecran);
						for (;;)
						{
							DispInfo(niveau);
							Load(niveau);
							Play(niveau);
							if (win == gagne)
								niveau++;
							if (win == kabort)
								break;
						}
						FadeIn(s, ecran);
						break;
					case SDLK_F2:
						FadeOut(s, ecran);
						niv = EntreCode();
						if (niv != -1)
							niveau = niv;
						FadeIn(s, ecran);
						break;
					case SDLK_F3:
						FadeOut(s, ecran);
						EditParams ();
						FadeIn(s, ecran);
						break;
#if !FINAL || TRICHE
					case SDLK_F4:
						FadeOut(s, ecran);
						ChooseTab (&niveau);
						printf("level = %d\n",niveau);
						FadeIn(s, ecran);
						break;
#endif
#if EDIT || !FINAL
					case SDLK_F5:
						FadeOut(s, ecran);
						Load(niveau);
						Edit(niveau);
						FadeIn(s, ecran);
						break;
#endif
				}
		}
	}while(cont);
	SDL_FreeSurface(s);
}

/*
void Jeu::SaveTab (Uint32 n) //records a table created by the user
{
	char txt[40];
	sprintf(txt,"tableau%d",n);
	t.Save(txt);
}
*/

Uint16 Jeu::WellTruc (Uint16 x, Uint16 y, Uint32 truc, Uint32 wtruc)
{
	Uint32 m = 0;
	if ((Uint16)(x-1) < DIMX)
		if ((r[t.t[x-1][y]].is(truc) && wtruc != wfleche) || (r[t.sol[x-1][y]].is(truc) && wtruc == wfleche))
			m+=wgauche;
	if ((Uint16)(x+1) < DIMX)
		if ((r[t.t[x+1][y]].is(truc) && wtruc != wfleche) || (r[t.sol[x+1][y]].is(truc) && wtruc == wfleche))
			m+=wdroite;
	if ((Uint16)(y-1) < DIMY)
		if ((r[t.t[x][y-1]].is(truc) && wtruc != wfleche) || (r[t.sol[x][y-1]].is(truc) && wtruc == wfleche))
			m+=whaut;
	if ((Uint16)(y+1) < DIMY)
		if ((r[t.t[x][y+1]].is(truc) && wtruc != wfleche) || (r[t.sol[x][y+1]].is(truc) && wtruc == wfleche))
			m+=wbas;
	//printf("m = %d\n",m);
	return w[wtruc].t[m];
}

void Jeu::PutWellTruc (Uint16 x, Uint16 y, Uint32 truc, Uint32 wtruc)
{
	Uint16 c;
	int i,j;
	for (i=-1;i<=1;i++)
	{
		for (j=-1;j<=1;j++)
		{
			if (!i || !j)
			{
				if ((Uint16)(x+i) < DIMX && (Uint16)(y+j) < DIMY)
				{
					if ((r[t.t[x+i][y+j]].is(truc) && wtruc != wfleche) || (r[t.sol[x+i][y+j]].is(truc) && wtruc == wfleche))
					{
						//printf("(%d,%d) : %d\n",i,j,t.t[x+i][y+j]);
						c = WellTruc(x+i, y+j, truc, wtruc);
						if (c)
						{
							//printf("c = %d\n",c);
							if (wtruc == wfleche)
								t.sol[x+i][y+j] = c;
							else
								t.t[x+i][y+j] = c;
							Draw(x+i, y+j);
						}
					}
				}
			}
		}
	}
}

void Jeu::PutWell (Uint16 x, Uint16 y, Uint16 what)
{
	if (r[what].is(wellable) || t.t[x][y] == 0)
	{
		if (r[what].is(vraimur))
		{
			//printf("put well truc...\n");
			PutWellTruc (x, y, vraimur, wmur);
		}
		if (r[what].is(vraiplante))
		{
			PutWellTruc (x, y, vraiplante, wplante);
		}
		if (r[what].is(fleche_sol))
		{
			PutWellTruc (x, y, fleche_sol, wfleche);
		}
		if (r[what].is(rail))
		{
			//////////////
		}
	}
}

void Jeu::Edit (Uint32 niveau)
{
	struct Edit e;
	printf("loading...\n");
	e.Load();
	pvue.y = 0;
	printf("drawing...\n");
	Draw();
	SDL_BlitSurface(jeu, &vue, ecran, &pvue);
	e.Draw(im, ecran);
	SDL_Flip(ecran);
	Update(); //
	SDL_Event event;
	bool cont = true;
	printf("curly...\n"); //(boucle)
	bool fill = false;
	Uint16 x,y;
	Uint16 rt;
	do
	{
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				cont = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						cont = false;
						break;
					case SDLK_d:
						Draw();
						Update();
						break;
					case SDLK_e:
						Save(niveau);
						break;
					case SDLK_c:
						Load(niveau);
						break;
				}
			case SDL_MOUSEBUTTONDOWN:
				x = event.motion.x/CX;
				y = event.motion.y/CY;
				if (x < DIMX && y < DIMY)
				{
					//fill = true;
				}else
				{
					e.Select(x,y-DIMY);
				}
			case SDL_MOUSEMOTION:
				fill = (SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1)); //(event.motion.state == SDL_PRESSED);
				x = event.motion.x/CX;
				y = event.motion.y/CY;
				if (fill)
				{
					if (x < DIMX && y < DIMY)
					{
						if (r[e.sel].is(sol))
						{
							rt = t.sol[x][y];
							t.sol[x][y] = e.sel;
						}else
						{
							rt = t.t[x][y];
							t.t[x][y] = e.sel;
						}
						if (!e.sel)
							PutWell(x,y,rt);
						else
							PutWell(x,y,e.sel);
						Draw(x,y);
						//SDL_BlitSurface(jeu, &vue, ecran, &pvue);
						//SDL_Flip(ecran);
						Update();
					}
				}
				break;
		}
	}while(cont);
}

void Jeu::DoPastille (Uint16 x, Uint16 y, Uint16 w)
{
	int dx,dy;
	for (dx=-1;dx<=1;dx++)
	{
		for (dy=-1;dy<=1;dy++)
		{
			if ((Uint16)(x+dx) < DIMX && (Uint16)(y+dy) < DIMY)
			{
				if (!t.t[x+dx][y+dy] || (!dx && !dy))
				{
					if (w == 164)//pastille verte
						Anime(x+dx, y+dy, 14);
					else
						Anime(x+dx, y+dy, 15);
				}
			}
		}
	}
}

void Jeu::DoOrdi (Uint16 w)
{
	Uint16 x,y;
	Uint16 na = 0;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (t.t[x][y] == 168 || t.t[x][y] == 218) //door open or closed computer?? (porte d'ordi ouverte ou fermée)
			{
				na = 0;
				switch (w)
				{
					case 197:
						if (t.t[x][y] == 218) //open
							na = 13; //solid/hard/definite (ferme)
						break;
					case 207:
						if (t.t[x][y] == 168) //solid/hard/definite (ferme) 
							na = 12; //open
						else
							na = 13; //solid (ferme)
						break;
					case 217:
						if (t.t[x][y] == 168) //solid (ferme)
							na = 12; //open
						break;
				}
				if (na)
					Anime(x, y, na);
			}
		}
	}
}

Uint16 Jeu::ActiveRail (Uint16 wrail) //activates a rail
{
	if (r[wrail].is(rail))
	{
		if (wrail%10 < 2)
		{
			if (((int)(wrail/10))%2 == 0)
				wrail += 10;
		}
	}
	return wrail;
}

Uint16 Jeu::DesactiveRail (Uint16 wrail) //deactivates a rail
{
	if (r[wrail].is(rail))
	{
		if (wrail%10 < 2)
		{
			if (((int)(wrail/10))%2 == 1)
				wrail -= 10;
		}
	}
	return wrail;
}

Pos GetRailSens (Uint16 c, Uint8 sens) //defines which is the rail with the direction of entry (donne le sens vers lequel va le rail en fonction du) sens d'entree
{
	Pos p;
	p.x = p.y = 0;
	if ((c == 170 || c == 210 || c == 160 || c == 200)) // && sens == gauche) //left
		p.x = -1;
	if ((c == 190 || c == 230 || c == 180 || c == 220)) // && sens == droite) //right
		p.x = 1;
	if ((c == 171 || c == 211 || c == 161 || c == 201)) // && sens == haut) //up
		p.y = -1;
	if ((c == 191 || c == 231 || c == 181 || c == 221)) // && sens == bas) //down
		p.y = 1;
	if (c == 162 && sens == droite) //right rail -> down
		p.y = 1;
	if (c == 172 && sens == gauche) //left rail -> down
		p.y = 1;
	if (c == 182 && sens == bas) //bottom rail -> right
		p.x = 1;
	if (c == 192 && sens == droite) //right rail -> up
		p.y = -1;
	if (c == 202 && sens == haut) //top rail  -> left
		p.x = -1;
	if (c == 212 && sens == haut) //top rail -> right
		p.x = 1;
	if (c == 222 && sens == gauche) //left rail -> up
		p.y = -1;
	if (c == 232 && sens == bas) //bottom rail -> left
		p.x = -1;
	return p;
}

Pos GetRailSens2 (Uint16 c, Uint8 sens) //gives meaning to which is the rail with the direction of entry (donne le sens vers lequel va le rail en fonction du sens d'entree)
{
	Pos p;
	p.x = p.y = 0;
	if ((c == 170 || c == 210 || c == 160 || c == 200) && sens == gauche) //left
		p.x = -1;
	if ((c == 190 || c == 230 || c == 180 || c == 220) && sens == droite) //right
		p.x = 1;
	if ((c == 171 || c == 211 || c == 161 || c == 201) && sens == haut) //up
		p.y = -1;
	if ((c == 191 || c == 231 || c == 181 || c == 221) && sens == bas) //down
		p.y = 1;
	if (c == 162 && sens == droite) //right rail -> down
		p.y = 1;
	if (c == 172 && sens == gauche) //left rail -> down
		p.y = 1;
	if (c == 182 && sens == bas) //bottom rail -> right
		p.x = 1;
	if (c == 192 && sens == droite) //right rail  -> up
		p.y = -1;
	if (c == 202 && sens == haut) //top rail -> left
		p.x = -1;
	if (c == 212 && sens == haut) //top rail -> right
		p.x = 1;
	if (c == 222 && sens == gauche) //left rail -> up
		p.y = -1;
	if (c == 232 && sens == bas) //bottom rail -> left
		p.x = -1;
	return p;
}

Pos GetRailSens (Uint16 c) //gives meaning to which is the rail with the direction of entry (donne le sens vers lequel va le rail en fonction du sens d'entree)
{
	Pos p;
	p.x = p.y = 0;
	if (c == 170 || c == 210 || c == 160 || c == 200) //left
		p.x = -1;
	if (c == 190 || c == 230 || c == 180 || c == 220) //right
		p.x = 1;
	if (c == 171 || c == 211 || c == 161 || c == 201) //up
		p.y = -1;
	if (c == 191 || c == 231 || c == 181 || c == 221) //down
		p.y = 1;
	if (c == 162) //right rail -> down
		p.y = 1;
	if (c == 172) //left rail -> down
		p.y = 1;
	if (c == 182) //bottom rail -> right
		p.x = 1;
	if (c == 192) //right rail -> up
		p.y = -1;
	if (c == 202) //top rail -> left
		p.x = -1;
	if (c == 212) //top rail -> right
		p.x = 1;
	if (c == 222) //left rail -> up
		p.y = -1;
	if (c == 232) //bottom rail -> left
		p.x = -1;
	return p;
}

void Jeu::ActiveRail (Uint16 x, Uint16 y) //activate rail
{
	t.t[x][y] = ActiveRail(t.t[x][y]);
}

void Jeu::DesactiveRail (Uint16 x, Uint16 y) //deactivate rail
{
	t.t[x][y] = DesactiveRail(t.t[x][y]);
}

void Jeu::OuvrePortes (Uint16 c) //opens all doors color c (after pulling in a colored circle)?? (ouvre toutes les portes de couleur c (apres avoir tire dans un rond colore))
{
	Uint16 x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (t.t[x][y] == 80 + (c%10)) //door color
			{
				//printf("ouverture d'une porte en (%d,%d)...\n",x,y);
				Anime(x, y, c%10);
			}
		}
	}
}

void Jeu::OpenBarriere ()
{
	//printf("ouverture barriere...\n");
	Uint16 x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (t.t[x][y] == 139 || t.a[x][y].n == 10) //close door
			{
				//printf("ouverture d'une porte en (%d,%d)...\n",x,y);
				Anime(x, y, 9);
			}
		}
	}
}

void Jeu::CloseBarriere ()
{
	//printf("fermeture barriere...\n");
	Uint16 x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (t.t[x][y] == 89 || t.a[x][y].n == 9) //open door
			{
				//printf("ouverture d'une porte en (%d,%d)...\n",x,y);
				Anime(x, y, 10);
			}
		}
	}
}

void Jeu::OpenPortes () //opens every door (after having put the key in the lock)
{
	Uint16 x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (t.t[x][y] == 80) //porte
			{
				//printf("ouverture d'une porte en (%d,%d)...\n",x,y);
				Anime(x, y, 31);
			}
		}
	}
}

void Jeu::DoPorte (Uint16 x, Uint16 y) //open one-way door
{
	if (!t.a[x][y].n)
		Anime(x, y, t.t[x][y]%10);
}

Pos Jeu::GoodVue () //field of view? (là ou devrait être le champ de vision !)
{
	Pos p;
	p.x = t.posx - NX/2;
	p.y = t.posy - NY/2;
	if (p.x < 0)
		p.x = 0;
	if (p.y < 0)
		p.y = 0;
	if (p.x+NX > DIMX)
		p.x = DIMX-NX;
	if (p.y+NY > DIMY)
		p.y = DIMY-NY;
	p.x = p.x*CX;
	p.y = p.y*CY;
	return p;
};

void Jeu::StartVue ()
{
	Pos p;
	p = GoodVue();
	vue.x = p.x;
	vue.y = p.y;
}

Sint8 signe (Sint8 n)
{
	if (n > 0)
		return 1;
	if (n < 0)
		return -1;
	return 0;
}

void Jeu::Balle(Uint16 x,Uint16  y,int dx,int dy) //throws a ball from (x, y) in the direction (dx, dy)
{
	int i;
	if ((Uint16)(x+dx) < DIMX)
	{
		if (r[t.t[x+dx][y]].is(mur))
		{
			if ((Uint16)(y+dy) < DIMY)
			{
				if (r[t.t[x][y+dy]].is(mur))
				{
					return;
				}
			}
		}
	}
	for (i=0;i<NBMAX;i++)
	{
		if (!b[i].used)
		{
			b[i].x = (2*x+1)*CX/2;
			b[i].y = (2*y+1)*CY/2;
			b[i].x += dx*(CX/2);
			b[i].y += dy*(CY/2);
			b[i].vx = dx*VBALL;
			b[i].vy = dy*VBALL;
			b[i].used = true;
			b[i].react = 0;
			break;
		}
	}
}
/*
void Jeu::Balle2(Uint16 x,Uint16  y,int dx,int dy) //throws a ball from (x, y) in the direction (dx, dy)
{
	int i;
	for (i=0;i<NBMAX;i++)
	{
		if (!b[i].used)
		{
			b[i].x = (2*x+1)*CX/2;
			b[i].y = (2*y+1)*CY/2;
			b[i].vx = dx*VBALL;
			b[i].vy = dy*VBALL;
			b[i].used = true;
			b[i].react = 0;
			break;
		}
	}
}
*/
void Jeu::MoveVueTo (SDL_Rect nvue)
{
	Uint32 ti,rti;
	rti = SDL_GetTicks();
	SDL_Rect rvue = vue;
	Uint16 i;
	for (i = 0;i < 20; i++)
	{
		SDL_PumpEvents();
		ti = SDL_GetTicks();
		if (ti-rti < 50)
			SDL_Delay(50 - (ti-rti));
		rti = SDL_GetTicks();
		
		vue.x = (rvue.x*(20-i) + nvue.x*i)/20;
		vue.y = (rvue.y*(20-i) + nvue.y*i)/20;
		//Update();
	}
}

void Jeu::MoveVueTo2 (SDL_Rect nvue)
{
	Uint32 ti,rti;
	rti = SDL_GetTicks();
	SDL_Rect rvue = vue;
	Uint16 i;
	for (i = 0;i <= NVUE; i++)
	{
		ti = SDL_GetTicks();
		if (ti-rti < TVUE)
			SDL_Delay(TVUE - (ti-rti));
		rti = SDL_GetTicks();
		
		vue.x = (rvue.x*(NVUE-i) + nvue.x*i)/NVUE;
		vue.y = (rvue.y*(NVUE-i) + nvue.y*i)/NVUE;
		//Update();
	}
}

void Jeu::Teleporte (Uint16 w)
{
	Uint16 x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (t.t[x][y] == w+10) //arrive !
			{
				Draw(t.posx, t.posy);
				t.posx = x;
				t.posy = y;
				s.Play(10);
				SDL_Rect nvue;
				Pos p = GoodVue();
				nvue.x = p.x;
				nvue.y = p.y;
				nvue.w = vue.w;
				nvue.h = vue.h;
				MoveVueTo(nvue);
				s.Play (12);
				DrawUs();
				return;
			}
		}
	}
}

void Jeu::Vue () //zoom
{
	SDL_Event event;
	SDL_Rect rvue = vue; //retains the old position
	bool cont = true;
	Uint32 time = 5000; //time view
	Uint32 ti,rti;
	rti = SDL_GetTicks();
	Uint8 *keystate;
	do
	{
		ti = SDL_GetTicks();
		if (ti-rti < 40)
			SDL_Delay(40 - (ti-rti));
		rti = SDL_GetTicks();
		time -= 40;
		if (time < 40)
			break;
		SDL_PollEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				win = kabort;
				quit = true;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						win = kabort;
						quit = true;
						break;
				}
				break;
		}
		keystate = SDL_GetKeyState (NULL);
		if (keystate[SDLK_UP])
		{
			vue.y -= 10;
		}
		if (keystate[SDLK_DOWN])
		{
			vue.y += 10;
		}
		if (keystate[SDLK_RIGHT])
		{
			vue.x += 10;
		}
		if (keystate[SDLK_LEFT])
		{
			vue.x -= 10;
		}
		if (vue.x < 0)
			vue.x = 0;
		if (vue.x + vue.w >= DIMX*CX)
			vue.x = DIMX*CX-vue.w;
		if (vue.y < 0)
			vue.y = 0;
		if (vue.y + vue.h >= DIMY*CY)
			vue.y = DIMY*CY-vue.h;
		//Update();
	}while(cont && !quit);
	if (!quit)
	{
		//returns to the initial position
		MoveVueTo(rvue);
	}
	t.t[t.posx][t.posy] = 0;
}

void Jeu::ReactOn (Uint16 x, Uint16 y)
{
	Uint16 c = t.t[x][y];
	//Uint16 c2 = t.t[x][y];
	int dx, dy;
	if (c == 285 || c == 295 || c == 305 || c == 315) //shooter
	{
		dx = dy = 0;
		if (c == 285)
			dx = -1;
		if (c == 295)
			dx = 1;
		if (c == 305)
			dy = 1;
		if (c == 315)
			dy = -1;
		s.Play(5);
		//printf("Lancement d'une balle en (%d,%d), vers (%d,%d)..\n",x,y,dx,dy);
		Balle(x, y, dx, dy); //throws a ball from (x, y) in the direction (dx, dy)
	}
	if (c == 310) //vue
	{
		s.Play(15);
		Vue ();
	}
	if (c == 307) //accelerator
	{
		tNMUS = tNMUS0;
		NMUS = NMUS0/2;
		s.Play(1);
		t.t[x][y] = 0;
	}
	if (c == 281) //invincibility
	{
		s.Play(1);
		t.t[x][y] = 0;
		invincible = INVINCIBLE;
	}
	//printf("react_on !\n");
	if (c == 283 || c == 297) //trick or it reappears?? (truc ou on reapparait)
	{
		t.sx = x;
		t.sy = y;
		t.so = t.objet;
		//printf("sauvegarde de la position...\n");
	}
}

void Jeu::Perdu ()
{
	if (quit)
		return;
	if (t.t[t.posx][t.posy] != 317 && !invincible) //box of invincibility? (case d'invincibilite ou invincible)
	{
		Anime(t.posx, t.posy, 27);
		if (win != gagne)
			win = perdu;
		quit = true;
	}else
	{
		if (invincible)
		{
			printf("invincible (%d)\n",invincible);
			DoBall(t.posx, t.posy);
		}
	}
}

void Jeu::DoBall (Uint16 x, Uint16 y) //bullet or explosion (x,y)
{
	int dx,dy;
	Uint16 c = t.t[x][y];
	if (x == t.posx && y == t.posy)
	{
		if (!invincible)
			Perdu ();
	}
	if (r[c].is(rail_tourne)) //rail turning
	{
		switch (t.t[x][y])
		{
			case 162: //bottom left
				c = 192; //top left
				break;
			case 192: //top left
				c = 162; //bottom left
				break;
			case 172: //bottom right
				c = 222; //top right
				break;
			case 222: //top right
				c = 172; //bottom right
				break;
			case 182: //top right
				c = 232; //top left
				break;
			case 232: //top left
				c = 182; //bottom right
				break;
			case 202: //bottom left
				c = 212; //bottom right
				break;
			case 212: //bottom right
				c = 202; //bottom left
				break;
		}
		t.t[x][y] = c;
		Draw(x, y);
		//printf("Changement de rail...\n");
	}
	if (c == 233) //destroy after 10 balls?? (truc qui se detruit au bout de 10 balles)
	{
		t.a[x][y].t++;
		if (t.a[x][y].t >= 10)
		{
			s.Play(16);
			Anime(x, y, 28);
		}
	}
	if (r[c].is(onda_mania_bouge))
	{
		dx = dy = 0;
		if (t.a[x][y].n == 19 || t.a[x][y].n == 23) //onda-mania case left
			dx = 1;
		if (t.a[x][y].n == 20 || t.a[x][y].n == 24) //onda-mania case right
			dx = -1;
		if (t.a[x][y].n == 21 || t.a[x][y].n == 25) //onda-mania case top
			dy = 1;
		if (t.a[x][y].n == 22 || t.a[x][y].n == 26) //onda-mania case bottom
			dy = -1;
		if (dx || dy)
		{
			//printf("dx = %d, dy = %d\n",dx,dy);
			if ((Uint16)(x+dx) < DIMX && (Uint16)(y+dy) < DIMY)
			{
				Anime(x+dx, y+dy, 28); //explosion
			}
		}
		s.Play(6);
		Anime(x, y, 28); //explosion
	}
	if (c == 285 || c == 295 || c == 305 || c == 315) //shooter
	{
		dx = dy = 0;
		if (c == 285)
			dx = -1;
		if (c == 295)
			dx = 1;
		if (c == 305)
			dy = 1;
		if (c == 315)
			dy = -1;
		s.Play(5);
		//printf("Lancement d'une balle en (%d,%d), vers (%d,%d)..\n",x,y,dx,dy);
		Balle(x, y, dx, dy); //throws a ball from (x, y) in the direction (dx, dy)
	}
	if (c == 287) //shooter down special
	{
		s.Play(5);
		Balle(x, y, 0, 1);
	}
	if (r[t.t[x][y]].is(explosif)) //explosive
	{
		DoExplosif(x, y);
	}
	if (t.t[x][y] == 164) //green patch
	{
		DoPastille(x,y,164);
	}
	if (t.t[x][y] == 165) //gray patch
	{
		DoPastille(x,y,165);
	}
	if (t.t[x][y] == 169) //computer
	{
		dx = dy = 0;
		for (dy=-1;dy<=1;dy++)
		{
			for (dx=-1;dx<=1;dx++)
			{
				if ((Uint16)(x+dx) < DIMX && (Uint16)(y+dy) < DIMY)
				{
					if (t.sol[x+dx][y+dy] == 187)
					{
						if (r[t.t[x+dx][y+dy]].is(truc_ordi))
						{
							DoOrdi (t.t[x+dx][y+dy]);
							return;
						}
					}
				}
			}
		}
	}
	if (r[t.t[x][y]].is(fleche)) //arrows
	{
		Anime(x, y, 28); //explosion
		switch (t.t[x][y])
		{
			case 286: //top
				t.sens = haut;
				//MoveUs(haut);
				break;
			case 296: //right
				t.sens = droite;
				//MoveUs(droite);
				break;
			case 306: //bottom
				t.sens = bas;
				//MoveUs(bas);
				break;
			case 316: //left
				t.sens = gauche;
				//MoveUs(gauche);
				break;
		}
		canmove = true;
		moveus = true;
	}
	if (t.t[x][y] == 166) //virus
	{
		for (dx=-1;dx<=1;dx++)
		{
			for (dy=-1;dy<=1;dy++)
			{
				if (!dx || !dy)
				{
					if ((Uint16)(x+dx) < DIMX && (Uint16)(y+dy) < DIMY)
					{
						if (!t.t[x+dx][y+dy] || (t.t[x][y] == 166 && !dx && !dy))
							Anime(x+dx, y+dy, 16);
					}
				}
			}
		}
	}
	if (t.t[x][y] == 144) //grenade
	{
		DoGrenade(x, y);
	}
	if (r[t.t[x][y]].is(plante)) //plant
	{
		s.Play(16);
		Anime(x, y, 28); //explosion
	}
	if (r[t.t[x][y]].is(rond_colore)) //round color
	{
		s.Play(16);
		Anime(x, y, 28); //explosion
		OuvrePortes(t.t[x][y]);
	}
	if (t.t[x][y] == 234) //multi-missiles
	{
		s.Play(16);
		Anime(x, y, 28); //explosion
		if (x+1 < DIMX)
		{
			if (t.t[x+1][y] == 0)
			{
				t.t[x+1][y] = 61; //missile right
				Draw(x+1, y);
				if (x+1 == t.posx && y == t.posy)
					DrawUs();
			}
		}
		if ((int)x-1 >=0)
		{
			if (t.t[x-1][y] == 0)
			{
				t.t[x-1][y] = 62; //missile left
				Draw(x-1, y);
				if (x-1 == t.posx && y == t.posy)
					DrawUs();
			}
		}
		if (y+1 < DIMY)
		{
			if (t.t[x][y+1] == 0)
			{
				t.t[x][y+1] = 70; //missile bottom
				Draw(x, y+1);
				if (x == t.posx && y+1 == t.posy)
					DrawUs();
			}
		}
		if ((int)y-1 >=0)
		{
			if (t.t[x][y-1] == 0)
			{
				t.t[x][y-1] = 63; //missile top
				Draw(x, y-1);
				if (x == t.posx && y-1 == t.posy)
					DrawUs();
			}
		}
	}
}

void Jeu::DoGrenade (Uint16 x, Uint16 y)
{
	if ((x-t.posx)*(x-t.posx) + (y-t.posy)*(y-t.posy) < DHEAR2)
	{
		s.Play(20);
	}
	int dx,dy;
	t.t[x][y] = 224; //hole
	for (dx=-1;dx<=1;dx++)
	{
		for (dy=-1;dy<=1;dy++)
		{
			if (x+dx >= 0 && x+dx < DIMX && y+dy >= 0 && y+dy < DIMY)
			{
				if (t.t[x+dx][y+dy] == 300) //on gagne !!!
				{
					Anime(x+dx, y+dy, 27);
					win = gagne;
					quit = true;
				}else
				{
					if (!t.t[x+dx][y+dy] || (!dx && !dy))
					{
						//printf("action de la grenade en (%d,%d)...\n",x+dx,y+dy);
						Anime(x+dx, y+dy, 29);
					}
					DoBall(x+dx, y+dy);
				}
			}
		}
	}
}

void Jeu::DoExplosif (Uint16 x, Uint16 y) //Start an explosive located at (x, y)
{
	if (t.t[x][y] == 151) //time bomb
	{
		Anime(x, y, 11);
		return;
	}
	//printf("Lancement d'un explosif en (%d,%d)...\n",x,y);
	if ((x-t.posx)*(x-t.posx) + (y-t.posy)*(y-t.posy) < DHEAR2)
	{
		s.Play(16); //plays the sound
	}
	int dx,dy;
	Anime(x , y, 33); //center of explosion
	for (dx=-1;dx<=1;dx++)
	{
		for (dy=-1;dy<=1;dy++)
		{
			if (x+dx >= 0 && x+dx < DIMX && y+dy >= 0 && y+dy < DIMY)
			{
				if (t.t[x+dx][y+dy] == 300) //we win
				{
					Anime(x+dx, y+dy, 27);
					win = gagne;
					quit = true;
				}else
				{
					if (!t.t[x+dx][y+dy])
					{
						//printf("explosion en (%d,%d)\n",x+dx,y+dy);
						Anime(x+dx, y+dy, 28);
					}
					/*else
					{
						if (r[t.t[x+dx][y+dy]].is(explosif))
							DoBall(x+dx, y+dy);
					}*/
				}
			}
		}
	}
}

void Jeu::DrawBall (Uint16 x, Uint16 y)
{
	SDL_Rect r;
	r.x = x-1 - vue.x + pvue.x;
	r.y = y-1 - vue.y + pvue.y;
	r.w = 3;
	r.h = 3;
	if (r.x >= pvue.x-3 && r.y >= pvue.y-3 && r.x < pvue.x + pvue.w + 3 && r.y < pvue.y + pvue.h + 3)
	{
		if (r.x < pvue.x)
			r.x = pvue.x;
		if (r.y < pvue.y)
			r.y = pvue.y;
		if (r.x + r.w >= pvue.x + pvue.w)
			r.w = pvue.x + pvue.w - r.x - 1;
		if (r.y + r.h >= pvue.y + pvue.h)
			r.h = pvue.y + pvue.h - r.y - 1;
		if ((Uint16)r.w > 5 || (Uint16)r.h > 5)
			return;
		SDL_FillRect (ecran, &r, SDL_MapRGB(ecran->format, 250, 200, 0));
	}
}

void Jeu::PutRed (Uint8 red)
{
	if (!red)
		red = 1;
	SDL_Surface *s = ecran;
	Uint32 *ptr = (Uint32 *)s->pixels+pvue.y*s->pitch/4+pvue.x;
	int i,m;
	Uint8 r,g,b;
	m = s->pitch/4;
	Uint32 x,y;
	for (y=0;y<NY*CY;y++)
	{
		for (x=0;x<NX*CX;x++)
		{
			SDL_GetRGB(*ptr,s->format,&r,&g,&b);
			if ((g == 0 && b == 0 && r != 0) || (g == 0 && b == 1 && r == 0))
			{
				*ptr = SDL_MapRGB(s->format, red, 0, 0);
				SDL_GetRGB(*ptr,s->format,&r,&g,&b);
			}
			ptr++;
		}
		ptr += (m-CX*NX);
	}
}
/*
 void Jeu::PutRed (Uint8 red)
 {
	 if (!red)
		 red = 1;
	 SDL_Surface *s = ecran;
	 Uint8 *ptr = (Uint8 *)s->pixels+pvue.y*s->pitch+pvue.x*4;
	 int i,m;
	 Uint8 r,g,b;
	 m = s->pitch/4;
	 Uint32 x,y;
	 for (y=0;y<NY*CY;y++)
	 {
		 for (x=0;x<NX*CX;x++)
		 {
			 //SDL_GetRGB(*ptr,s->format,&r,&g,&b);
			 r = *ptr; ptr++;
			 g = *ptr; ptr++;
			 b = *ptr; ptr++;
			 if ((g == 0 && b == 0 && r != 0) || (g == 0 && b == 1 && r == 0))
			 {
				 *ptr = SDL_MapRGB(s->format, red, 0, 0);
				 SDL_GetRGB(*ptr,s->format,&r,&g,&b);
			 }
			 ptr++;
		 }
		 ptr += (m-CX*NX)*4;
	 }
 }
 */

//thread that manage the balls, flashing red and the update of the screen
int DoBalls (void *data)
{
	int i;
	Jeu *j = (Jeu *)data;
	Uint32 t,rt;
	int x,y,dx,dy;
	rt = SDL_GetTicks();
	Sint8 tamp;
	j->InitBalls ();
	int red = 0;
	int vred = 10;
	Uint32 retard = 0;
	do
	{
		if (j->paused)
		{
			j->DoPause();
			rt = SDL_GetTicks();
		}
		t = SDL_GetTicks();
		if ((int)t-rt < TBALLS)
		{
			SDL_Delay(TBALLS-(t-rt));
		}else
		{
			if (retard)
				printf("retard cumule !!! (retard = %d)...\n",retard);
			retard = (t-rt)/TBALLS;
		}
		rt = SDL_GetTicks();//t;//+TBALLS;
		if (!retard)
		{
			//met à jour ce qui est a l'ecran
			SDL_BlitSurface(j->jeu, &j->vue, j->ecran, &j->pvue);
		}
		// draw balls
		for (i=0;i<NBMAX;i++)
		{
			if (j->b[i].used)
			{
				//see if the ball lands on a new box
				x = (int)((j->b[i].x+j->b[i].vx)/CX);
				y = (int)((j->b[i].y+j->b[i].vy)/CY);
				if ((Uint16)x >= DIMX || (Uint16)y >= DIMY)
				{
					j->b[i].used = false;
					continue;
				}
				if (x == j->t.posx && y == j->t.posy)
				{//ball hits us
					j->Perdu ();
				}
				if ((int)(j->b[i].x/CX) != x || (int)(j->b[i].y/CY) != y || j->b[i].react == 0) //new box
				{
					j->b[i].react = 1;
					if (x < 0 || y < 0 || x >= DIMX || y >= DIMY)
					{//ball out of field
						//printf("sortie !\n");
						j->b[i].used = false;
					}else
					{
						if (j->r[j->t.t[x][y]].is(react_ball))
						{//ball hits object
							j->DoBall(x, y);
							j->b[i].used = false;
						}
						if (j->r[j->t.t[x][y]].is(stop_ball))
						{//ball stops
							j->b[i].used = false;
							//printf("vx=%d, vy=%d.\n",j->b[i].vx,j->b[i].vy);
							if (j->b[i].vx != 0 && j->b[i].vy != 0) //diagonal ball
							{
								//printf("balle diagonale.../n");
								if (j->r[j->t.t[x][y]].is(mur))
								{
									//printf("...et mur ...\n");
									int rx,ry;
									rx = (int)(j->b[i].x/CX);
									ry = (int)(j->b[i].y/CY);
									//bounce
									if (x!=rx && y!=ry)
									{
										if (j->r[j->t.t[rx][y]].is(mur))
											j->b[i].vy = -j->b[i].vy;
										else
										{
											j->b[i].vx = -j->b[i].vx;
											if (j->r[j->t.t[rx][y]].is(mur))
												j->b[i].vy = -j->b[i].vy;
										}
									}else
									{
										if (x!=rx)
											j->b[i].vx = -j->b[i].vx;
										else
											j->b[i].vy = -j->b[i].vy;
									}
									/*
									j->b[i].vx = (2*(rx-x)-1)*VBALL;
									j->b[i].vy = (2*(ry-y)-1)*VBALL;
									 */
									j->b[i].used = true;
								}
							}
						}
					}
				}
				if (((int)((j->b[i].x+signe(j->b[i].vx)*CX/2)/CX) != x || (int)((j->b[i].y+signe(j->b[i].vy)*CY/2)/CY) != y) && j->b[i].react != 2) //middle box
				{
					j->b[i].react = 2;
					if (j->r[j->t.t[x][y]].is(act_ball))
					{//object is on the ball
						switch (j->t.t[x][y])
						{
							case 284: //anti-diagonal mirror
							case 203:
								tamp = j->b[i].vy;
								j->b[i].vy = -j->b[i].vx;
								j->b[i].vx = -tamp;
								break;
							case 294: //diagonal mirror
							case 213:
								tamp = j->b[i].vy;
								j->b[i].vy = j->b[i].vx;
								j->b[i].vx = tamp;
								break;
							case 304: //horizontal mirror
							case 193:
								j->b[i].vx = -j->b[i].vx;
								break;
							case 314: //vertical mirror
							case 183:
								j->b[i].vy = -j->b[i].vy;
								break;
							case 291: //horizontal and vertical mirror
								j->b[i].vx = -j->b[i].vx;
								j->b[i].vy = -j->b[i].vy;
								break;
							case 301: //multiple shooter
								j->b[i].used = false;
								//j->s.Play(5);
								j->Balle(x, y, 1, 0);
								j->Balle(x, y, -1, 0);
								j->Balle(x, y, 0, 1);
								j->Balle(x, y, 0, -1);
								break;
							case 285: //cannon left
								j->s.Play(5);
								j->Balle(x, y, -1, 0);
								break;
							case 295: //cannon right
								j->s.Play(5);
								j->Balle(x, y, 1, 0);
								break;
							case 305: //cannon bottom
								j->s.Play(5);
								j->Balle(x, y, 0, 1);
								break;
							case 315: //cannon top
								j->s.Play(5);
								j->Balle(x, y, 0, -1);
								break;
							case 287: //cannon bottom special
								j->s.Play(5);
								j->Balle(x, y, 0, 1);
								j->b[i].used = false;
								break;
							case 280: //green diagonal shooter (shooting in all directions)
								j->s.Play(5);
								j->Balle(x, y, -1, -1);
								j->Balle(x, y, -1, 1);
								j->Balle(x, y, 1, -1);
								j->Balle(x, y, 1, 1);
								j->b[i].used = false;
								break;
							case 290: //shooter yellow diagonal (shooting a bullet at us)
								if (x < j->t.posx)
									dx = 1;
								else
									dx = -1;
								if (y < j->t.posy)
									dy = 1;
								else
									dy = -1;
								j->Balle(x, y, dx, dy);
								j->b[i].used = false;
								break;
						}
					}
				}
				//move the ball
				j->b[i].x += j->b[i].vx;
				j->b[i].y += j->b[i].vy;
				//redraws the ball
				j->DrawBall(j->b[i].x, j->b[i].y);
			}
		}
		//put red?? (met le rouge)
		if (!retard)
		{
			if (red > 255)
			{
				red = 255;
				vred = -abs(vred);
			}
			if (red < -300)
			{
				red = -300;
				vred = abs(vred);
			}
			if (ROUGE)
			{
				if (red >= 0)
					j->PutRed(red);
			}
			red += vred;
		}
		//poster?? (affiche)
		if (!retard)
			SDL_Flip(j->ecran);
		if (retard)
			retard--;
	}while(!j->quitanim);
	printf("End of DoBalls !\n");
	return 0;
}

//thread that updates the view  (thread chargé de mettre à jour le point de vue (reappellé à chaque fois))
int DoVue (void *data)
{
	Jeu *j = (Jeu *)data;
	j->dovuerun = true;
	SDL_Rect nvue; //new to area
	bool update = true;
	do
	{
		if (j->paused)
		{
			j->DoPause();
		}
		SDL_Delay(20);
		if (j->updatevue)
		{
			j->updatevue = false;
			nvue = j->vue;
			//to determine where to go
			int dx = 0, dy = 0; //sense of displacement was carried out?? (sens du deplacement a effectuer)
			if (j->vposx*CX < j->vue.x + 2*CX)
				dx = -1;
			if (j->vposx*CX >= j->vue.x + j->vue.w - 2*CX)
				dx = 1;
			if (j->vposy*CY < j->vue.y + 2*CY)
				dy = -1;
			if (j->vposy*CY >= j->vue.y + j->vue.h - 2*CY)
				dy = 1;
			nvue.x += dx*3*CX;
			nvue.y += dy*3*CY;
			if (nvue.x < 0)
				nvue.x = 0;
			if (nvue.y < 0)
				nvue.y = 0;
			if (nvue.x + nvue.w >= CX*DIMX)
				nvue.x = CX*DIMX - nvue.w;
			if (nvue.y + nvue.h >= CY*DIMY)
				nvue.y = CY*DIMY - nvue.h;
			if (nvue.x == j->vue.x && nvue.y == j->vue.y)
			{
				//j->dovuerun = false;
				//return 0;
				continue;
			}
			//performs the movement
			j->MoveVueTo2(nvue);
		}
	}while(!j->quit && !j->quitanim);
	j->dovuerun = false;
	printf("End of DoVue...\n");
}
/*
int DoVue (void *data)
{
	int dx,dy; //deplacement a effectuer
	float vx, vy; //vitesse de deplacement
	float x,y; //position courante
	int fx,fy; //position finale
	Jeu *j = (Jeu *)data;
	if (j->dovuerun)
	{
		j->updatevue = true;
		return 0;
	}
	j->dovuerun = true;
	j->updatevue = true;
	
	x = j->vue.x;
	y = j->vue.y;
	
	//effectue le deplacement
	bool reupdate = false;
	Uint32 t,rt;
	int i;
	rt = SDL_GetTicks();
	Uint16 m = NVUE;
	for (i=0;i<m;i++)
	{
		if (j->updatevue)
		{
			if (reupdate)
			{
				//vx = dx*CX/(3*NMUS); ///pas genial !!!!
				//vy = dy*CY/(3*NMUS);
				i-=NMUS;
			}else
			{
				dx = 0;
				dy = 0;
				if (j->t.posx*CX < j->vue.x + 2*CX)
					dx = -3;
				if (j->t.posx*CX >= j->vue.x + j->vue.w - 2*CX)
					dx = 3;
				if (j->t.posy*CY < j->vue.y + 2*CY)
					dy = -3;
				if (j->t.posy*CY >= j->vue.y + j->vue.h - 2*CY)
					dy = 3;
				if (!dx && !dy)
					break;
				if (abs(j->t.posx-(int)((j->vue.x+j->vue.w/2)/CX))+abs(j->t.posy-(int)((j->vue.y+j->vue.h/2)/CY)) >= (NX+NY)/2) //regarde si l'on doit se déplacer de beaucoup ou pas
				{
					Pos p = j->GoodVue ();
					fx = p.x/CX; //on se deplace de beaucoup
					fy = p.y/CY;
					m = 3*NVUE;
				}else
				{
					fx = (int)((x+dx*CX)/CX); //on se deplace normalement
					fy = (int)((y+dy*CY)/CY);
					m = NVUE;
				}
				vx = (fx*CX-x)/m;
				vy = (fy*CY-y)/m;
				i = 0;
			}
			//printf("x=%d, y=%d, fx=%d, fy=%d, vx=%d, vy=%d\n",(int)x,(int)y,(int)fx,(int)fy,vx,vy);
			j->updatevue = false;
			reupdate = true;
		}
		t = SDL_GetTicks();
		if ((int)t-rt < TVUE)
			SDL_Delay(TVUE-(t-rt));
		//else
		//	printf("DoVue à la bourre...\n");
		rt = SDL_GetTicks();//t;//+TVUE;
		x += vx;
		y += vy;
		j->vue.x = (int)x;
		j->vue.y = (int)y;
		if (x < 0)
		{
			//printf("bord !\n");
			j->vue.x = 0;
			dx = 0;
		}
		if (y < 0)
		{
			//printf("bord !\n");
			j->vue.y = 0;
			dy = 0;
		}
		if (x >= CX*DIMX - j->vue.w)
		{
			//printf("bord !\n");
			j->vue.x = CX*DIMX - j->vue.w;
			dx = 0;
		}
		if (y >= CY*DIMY - j->vue.h)
		{
			//printf("bord !\n");
			j->vue.y = CY*DIMY - j->vue.h;
			dy = 0;
		}
		if (j->quitanim || (!dx && !dy))
			break;
	}
	j->dovuerun = false;
	//printf("Fin de DoVue !\n");
	return 0;
}
*/

void Jeu::UpdateVue (Uint16 x, Uint16 y) //updates the field of view
{
	//launches the thread that updates the field of view
	vposx = x;
	vposy = y;
	updatevue = true;
	if (!dovuerun)
	{
		dovuerun = true;
		printf("call DoVue...\n");
		SDL_CreateThread(DoVue, this);
	}
}

//thread responsible for animations
int DoAnim (void *data)
{
	int x,y,dx,dy;
	Uint16 n;
	Uint32 t,rt;
	Jeu *j = (Jeu *)data;
	rt = SDL_GetTicks();
	bool ok;
	Pos p;
	do
	{
		if (j->paused)
		{
			j->DoPause();
		}
		t = SDL_GetTicks();
		if ((int)t-rt < TANIM)
			SDL_Delay(TANIM-(t-rt));
		//else
		//	printf("DoAnimation à la bourre...\n");
		rt = SDL_GetTicks();//t;//+TANIM;
		for (y=0;y<DIMY;y++)
		{
			for (x=0;x<DIMX;x++)
			{
				if (x == j->t.posx && y == j->t.posy)
				{
					if (j->r[j->t.t[x][y]].is(mechant))
					{
						j->Perdu();
					}
				}
				n = j->t.a[x][y].n;
				if (!n)
				{
					//conveyor belt?? (tapis roulant)
					if (j->r[j->t.t[x][y]].is(tapis_roulant))
					{
						j->t.a[x][y].t++;
						if (j->t.a[x][y].t >= TTAPIS_ROULANT)
						{
							j->t.a[x][y].t = 0;
							j->t.t[x][y] += 10;
							if (j->t.t[x][y] >= 249)
							{
								j->t.t[x][y] = 209;
							}
							j->Draw(x,y);
							if (j->t.posx == x && j->t.posy == y)
								j->DrawUs();
						}
					}
					//enemy
					if (j->t.t[x][y] == 240) //enemy horizontal left
					{
						ok = false;
						if (x > 0)
						{
							if (j->t.t[x-1][y] == 0 && j->t.a[x-1][y].n == 0)
							{
								j->Anime(x-1, y, 36);
								j->Anime(x, y, 37);
								ok = true;
							}
						}
						if (!ok)
						{
							if (x < DIMX-1)
							{
								if (j->t.t[x+1][y] == 0 && j->t.a[x+1][y].n == 0)
								{
									j->Anime(x, y, 34);
									j->Anime(x+1, y, 35);
								}
							}
						}
					}
					if (j->t.t[x][y] == 328) //enemy horizontal right
					{
						ok = false;
						if (x < DIMX-1)
						{
							if (j->t.t[x+1][y] == 0 && j->t.a[x+1][y].n == 0)
							{
								j->Anime(x, y, 34);
								j->Anime(x+1, y, 35);
								ok = true;
							}
						}
						if (!ok)
						{
							if (x > 0)
							{
								if (j->t.t[x-1][y] == 0 && j->t.a[x-1][y].n == 0)
								{
									j->Anime(x-1, y, 36);
									j->Anime(x, y, 37);
								}
							}
						}
					}
					if (j->t.t[x][y] == 322) //enemy vertical up
					{
						ok = false;
						if (y > 0)
						{
							if (j->t.t[x][y-1] == 0 && j->t.a[x][y-1].n == 0)
							{
								j->Anime(x, y-1, 38);
								j->Anime(x, y, 39);
								ok = true;
							}
						}
						if (!ok)
						{
							if (y < DIMY-1)
							{
								if (j->t.t[x][y+1] == 0 && j->t.a[x][y+1].n == 0)
								{
									j->Anime(x, y, 40);
									j->Anime(x, y+1, 41);
								}
							}
						}
					}
					if (j->t.t[x][y] == 340) //enemy vertical down
					{
						ok = false;
						if (y < DIMY-1)
						{
							if (j->t.t[x][y+1] == 0 && j->t.a[x][y+1].n == 0)
							{
								j->Anime(x, y, 40);
								j->Anime(x, y+1, 41);
								ok = true;
							}
						}
						if (!ok)
						{
							if (y > 0)
							{
								if (j->t.t[x][y-1] == 0 && j->t.a[x][y-1].n == 0)
								{
									j->Anime(x, y-1, 38);
									j->Anime(x, y, 39);
								}
							}
						}
					}
					//onda-mania
					if (j->r[j->t.t[x][y]].is(onda_mania))
					{
						dx = dy = 0;
						if (j->t.t[x][y] == 282) //try to go right
							dx = 1;
						if (j->t.t[x][y] == 273) //try to go left
							dx = -1;
						if (j->t.t[x][y] == 247) //try to go down
							dy = 1;
						if (j->t.t[x][y] == 274) //try to go up
							dy = -1;
						if ((Uint16)(x+dx) <= DIMX && (Uint16)(y+dy) <= DIMY)
						{
							if (x+dx == j->t.posx && y+dy == j->t.posy)
							{
								j->Perdu();
							}else
							{
								if (!j->t.t[x+dx][y+dy] && !j->t.a[x+dx][y+dy].n)
								{//l'onda-mania peut avancer
									int na,na2;
									switch (j->t.t[x][y])
									{
										case 282: //go right
											na = 23;
											na2 = 24;
											break;
										case 273: //go left
											na = 20;
											na2 = 19;
											break;
										case 274: //go up
											na = 22;
											na2 = 21;
											break;
										case 247: //go down
											na = 25;
											na2 = 26;
											break;
									}
									j->Anime(x, y, na);
									j->Anime(x+dx, y+dy, na2);
									n = j->t.a[x][y].n;
									dx = dy = 0;
								}
							}
						}
						if (dx != 0 || dy != 0)
						{
							//printf("%d : essaie de nouvelle direction... ",j->t.t[x][y]);
							//the (ondo-mania?) will try a new direction
							switch (j->t.t[x][y])
							{
								case 282: //trying to go right
									j->t.t[x][y] = 247; //go up
									break;
								case 273: //trying to go left
									j->t.t[x][y] = 274; //go down
									break;
								case 247: //trying to go down
									j->t.t[x][y] = 273; //go left
									break;
								case 274: //trying to go up
									j->t.t[x][y] = 282; //go right
									break;
							}
							//printf(" -> %d !\n",j->t.t[x][y]);
						}
					}
				}
				if (n)
				{
					if ((n == 32 || n == 42) && j->t.a[x][y].t == j->a[n].t-1) //rail that goes off //infinite stack
					{
						//printf("rail ou pile...\n");
						/*
						//printf("rail (%d,%d) s'allume...\n",x, y);
						j->ActiveRail(x, y);
						j->Draw(x, y);
						if (x == j->t.posx && y == j->t.posy)
							j->DrawUs();
						if (x == j->t.posx && y == j->t.posy && !j->moveus)
						{
							j->moveus = true;
							j->t.sens = bas;
							j->s.Play(21); //pulse
						}
						 */
						if (n == 32) //rail gets deactivated
						{
							j->DesactiveRail(x, y);
							j->Draw(x, y);
							if (x == j->t.posx && y == j->t.posy)
								j->DrawUs();
							p = GetRailSens (j->t.t[x][y]);
						}else
						{
							p.x = 0;
							p.y = 1;
						}
						if ((Uint16)(x+p.x) < DIMX && (Uint16)(y+p.y) < DIMY)
						{
							//printf("Rail vert qui s'allume (n=%d, x=%d, y=%d)...\n",j->t.a[x][y].n, x+p.x, y+p.y);
							if (j->r[j->t.t[x+p.x][y+p.y]].is(rail))
							{
								//Pos p2 = GetRailSens2 (j->t.t[x+p.x][y+p.y], GetSens(p.x, p.y));
								//if (p2.x || p2.y) //verifies that the track goes in the same direction
								{
									j->ActiveRail(x+p.x, y+p.y);
									j->Draw(x+p.x, y+p.y);
									if (x+p.x == j->t.posx && y+p.y == j->t.posy)
										j->DrawUs();
									if (x+p.x == j->t.posx && y+p.y == j->t.posy && !j->moveus)
									{
										j->moveus = true;
										j->t.sens = bas;
										j->s.Play(21); //pulse
									}
									j->Anime(x+p.x, y+p.y, 32); //rail vert qui s'eteint
								}
							}
						}
						if (n == 42)
							j->Anime(x, y, 42); //relaunch infinite stack (relance la pile infinie)
					}
					/*
					if (n == 32 && j->t.a[x][y].t >= j->a[n].t-1) //rail gets deactivated
					{
						//printf("rail (%d,%d) s'eteint...\n",x, y);
						j->DesactiveRail(x, y);
						j->t.a[x][y].t = 0; //j->t.a[x][y].t%j->a[n].t;
						j->Draw(x, y);
						if (x == j->t.posx && y == j->t.posy)
							j->DrawUs();
					}
					 */
					if (n == 11 && j->t.a[x][y].t == j->a[n].n*j->a[n].t-1) //time bomb
					{
						j->DoExplosif(x, y);
					}
					if (n == 28 && j->t.a[x][y].t == 3) //explosion
					{
						//verifies that kills us
						j->DoBall(x, y);
					}
					if (n == 33 && j->t.a[x][y].t == 3) //center of bomb explosion
					{
						//starts eventual located stuff has close?? (demarre les eventuels trucs situees a proximite)
						for (dx=-1;dx<=1;dx++)
						{
							for (dy=-1;dy<=1;dy++)
							{
								if ((Uint16)(x+dx) < DIMX && (Uint16)(y+dy) < DIMY)
								{
									j->DoBall(x+dx, y+dy);
								}
							}
						}
					}
					if (n == 16 && j->t.a[x][y].t == j->a[n].n*j->a[n].t-1) //virus
					{
						for (dx=-1;dx<=1;dx++)
						{
							for (dy=-1;dy<=1;dy++)
							{
								if (x+dx >= 0 && x+dx < DIMX && y+dy >= 0 && y+dy < DIMY)
								{
									if (!j->t.t[x+dx][y+dy])
										j->Anime(x+dx, y+dy, 16);
								}
							}
						}
					}
					if (n == 30 && j->t.a[x][y].t == j->a[n].n*j->a[n].t-1)
					{
						j->OpenPortes();
					}
					//printf("Animation %d : t = %d...\n",n,j->a[n].t);
					if (j->t.a[x][y].t % j->a[n].t == 0 && j->a[n].n) //new image
					{
						j->t.t[x][y] = j->a[n].l[j->t.a[x][y].t / j->a[n].t];
						j->Draw(x,y);
						if (j->t.posx == x && j->t.posy == y)
						{
							if (n != 27 && (n != 18 || (n==18 && (Uint16)(j->t.a[x][y].t/j->a[n].t) == j->a[n].n-1))) //death and new life
								j->DrawUs();
						}
					}
					j->t.a[x][y].t++;
					if (j->t.a[x][y].t >= j->a[n].n*j->a[n].t && j->a[n].n)
					{
						j->t.t[x][y] = j->a[n].l[j->a[n].n-1];
						j->t.a[x][y].init();
					}
				}
			}
		}
	}while(!j->quitanim);
	printf("Fin de DoAnim !!!\n");
	return 0;
}

void Jeu::Anime (Uint16 x, Uint16 y, Uint16 n) //start the animation n (x,y)
{
	//printf("Lancement de l'animation %d en (%d, %d) ...\n",n,x,y);
	if (n >= NA)
	{
		fprintf(stderr, "Erreur l'animation %d n'existe pas !\n",n);
		return;
	}
	//printf("son %d\n",a[n].s);
	//printf("temps %d\n",a[n].t);
	if (a[n].s)
	{
		//verifies that it is not too far away to hear
		if ((x-t.posx)*(x-t.posx) + (y-t.posy)*(y-t.posy) < DHEAR2)
			s.Play(a[n].s); //plays the sound
	}
	if (t.a[x][y].n != 27)
	{
		t.a[x][y].n = n;
		t.a[x][y].t = 0;
	}
}

void Jeu::Update ()
{
	
	SDL_BlitSurface(jeu, &vue, ecran, &pvue);
	SDL_Flip(ecran);
}

inline void Jeu::DrawUs ()
{
	if (quit && win == perdu)
		return;
	SDL_Rect src, pos;
	src.x = CX*(t.sens);
	src.y = CY*(t.tsens);
	src.w = CX;
	src.h = CY;
	pos.x = t.posx*CX + pr.x;
	pos.y = t.posy*CY + pr.y;
	SDL_BlitSurface(im, &src, jeu, &pos);
}

inline void Jeu::DrawSol (Uint16 x, Uint16 y)
{
	SDL_Rect src, pos;
	src.x = CX*(t.sol[x][y]%10);
	src.y = CY*(t.sol[x][y]/10);
	src.w = CX;
	src.h = CY;
	pos.x = x*CX;
	pos.y = y*CY;
	SDL_BlitSurface(im, &src, jeu, &pos);
}

inline void Jeu::Draw(Uint16 x, Uint16 y)
{
	SDL_Rect src, pos;
	if (!t.sol[x][y])
		t.sol[x][y] = 41;
	src.x = CX*(t.sol[x][y]%10);
	src.y = CY*(t.sol[x][y]/10);
	src.w = CX;
	src.h = CY;
	pos.x = x*CX;
	pos.y = y*CY;
	SDL_BlitSurface(im, &src, jeu, &pos); //ground
	if (t.t[x][y]%10 >= 4 || t.t[x][y]/10 >= 4)
	{
		src.x = CX*(t.t[x][y]%10);
		src.y = CY*(t.t[x][y]/10);
		SDL_BlitSurface(im, &src, jeu, &pos); //frame? (case)
	}
}

void Jeu::Draw()
{
	Uint16 x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			Draw(x,y);
		}
	}
	DrawUs();
}

void Jeu::initRail (Uint16 x, Uint16 y, Uint16 time, Uint8 sens) //initializes the rail (x, y) and the following (useless)
{
	if (x >= DIMX || y >= DIMY)
		return;
	if (!r[t.t[x][y]].is(rail))
		return;
	Pos p;
	/*
	switch (sens)
	{
		case gauche:
			printf("gauche !\n");
			break;
		case droite:
			printf("droite !\n");
			break;
		case bas:
			printf("bas !\n");
			break;
		case haut:
			printf("haut !\n");
			break;
	}
	 */
	p = GetRailSens(t.t[x][y], sens);
	t.a[x][y].n = 32; //green rail that lights
	t.a[x][y].t = time;
	//printf("vers (%d,%d) ...\n",p.x,p.y);
	
	DesactiveRail(x, y);
	if (!p.x && !p.y)
	{
		//printf("Wrong direction !\n");
		return;
	}
	time -= TRAIL;
	if ((Uint32)time >= a[32].t)
		time = a[32].t-1;
	initRail(x+p.x, y+p.y, time, GetSens(p.x, p.y));
}

void Jeu::DesactiveRails ()
{
	Uint16 x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (r[t.t[x][y]].is(rail))
			{
				DesactiveRail(x, y);
			}
		}
	}
}

void Jeu::initTrain () //initiates automatic rails
{
	Uint16 x,y;
	DesactiveRails();
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (t.t[x][y] == 173) //pile infinie
			{
				//printf("Infinite stack has been found in (%d,%d) ...\n",x,y);
				//initRail(x, y+1, 0, bas);
				Anime(x, y, 42);
			}
		}
	}
}

void Jeu::InitBalls ()
{
	int i;
	for (i=0;i<NBMAX;i++)
	{
		b[i].init();
	}
}

void Jeu::init ()
{
	char tamp[256];
	//initialize SDL
	//SDL_errorcode err;
#if HSON
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		fprintf(stderr,"Initialize SDL error : %s\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}
#else
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		fprintf(stderr,"Initialize SDL error : %s\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}
#endif
#if TTF
	//initialisation de ttf
	//printf("Initialise TTF...\n");
	if (TTF_Init() < 0)
	{
		fprintf(stderr,"Initialize TTF error !!!\n");
		exit(EXIT_FAILURE);
	}
	//printf("Loading the police...\n");
	MakeName("arial.ttf", tamp);
	police = TTF_OpenFont(tamp, 20);
	if (!police)
	{
		printf("Police found !\n");
		exit(EXIT_FAILURE);
	}
#endif
	//Opening the window
	ecran = SDL_SetVideoMode(SCREENX, SCREENY, 32, SDL_HWSURFACE);
    if (ecran == NULL)
    {
        fprintf(stderr, "Unable to load video mode : %s\nq", SDL_GetError());
        exit(EXIT_FAILURE);
    }
#if HSON
	//initialize sound
	s.Init();
#endif
	//Loads game images
	MakeName("images.bmp", tamp);
	im = SDL_LoadBMP(tamp);
	if (!im)
	{
		fprintf(stderr,"Error : Game images missing !\n");
	}
	SDL_SetColorKey(im, SDL_SRCCOLORKEY, SDL_MapRGB(im->format, 0, 0, 255)); //transparency
	//initialize field of view
	vue.x = 0;
	vue.y = 0;
	vue.w = NX*CX;
	vue.h = NY*CY;
	pvue = vue;
	pvue.x = (SCREENX-vue.w)/2;
	pvue.y = (SCREENY-vue.h)/2;
	dovuerun = false;
	moveus = false;
	canmove = false;
	paused = false;
	//created the image of the complete picture? (cree l'image du tableau complet)
	jeu = SDL_CreateRGBSurface(SDL_HWSURFACE, CX*DIMX, CY*DIMY, 32, 0, 0, 0, 0);
	if (!jeu)
	{
		fprintf(stderr, "Unable to create the playing surface !\n");
	}
	//cree l'image d'une case
	ic = SDL_CreateRGBSurface(SDL_HWSURFACE, CX, CY, 32, 0, 0, 0, 0);
	if (!ic)
	{
		fprintf(stderr, "Unable to create the surface of a box !\n");
	}
	//load the rules
	LoadRegles(r);
	//load animations
	//printf("Loading animations...\n");
	LoadAnims(a);
	//initialize bullets
	InitBalls();
	//initializes the point of reappearance
	t.sx = t.sy = 0;
	//initializes the control buttons
	key[tgauche] = SDLK_LEFT;
	key[tdroite] = SDLK_RIGHT;
	key[thaut] = SDLK_UP;
	key[tbas] = SDLK_DOWN;
	key[ttirer] = SDLK_RETURN;
	key[tprendre] = SDLK_SPACE;
	key[tteleporter] = SDLK_SPACE;
	key[tsuicide] = SDLK_s;
	//load preferences (if any)
	LoadPrefs();
	//initialize the data used by the author to improve? (initialise des donnes utilisees par l'editeur pour ameliorer)
	w[0].init(0);
	w[1].init(1);
	w[2].init(2);
}

void Jeu::end ()
{
	//frees the image of the box
	SDL_FreeSurface(ic);
	//frees the images of the complete picture
	SDL_FreeSurface(jeu);
	//frees images of the game
	SDL_FreeSurface(im);
	//stop sound
	s.Stop();
#if TTF
	//close ttf
	TTF_CloseFont(police);
	TTF_Quit();
#endif
	//stop SDL
	SDL_Quit();
}

char inline deplx (Uint8 sens)
{
	switch (sens)
	{
		case gauche:
			return -1;
		case haut:
		case bas:
			return 0;
		case droite:
			return 1;
	}
}

char inline deply (Uint8 sens)
{
	switch (sens)
	{
		case haut:
			return -1;
		case gauche:
		case droite:
			return 0;
		case bas:
			return 1;
	}
}

void Jeu::Update_tsens ()
{
	if (r[t.objet].is(missile))
	{
		switch (t.objet)
		{
			case 61:
				t.tsens = droite;
				break;
			case 62:
				t.tsens = gauche;
				break;
			case 63:
				t.tsens = haut;
				break;
			case 70:
				t.tsens = bas;
				break;
		}
	}else
		t.tsens = t.sens; //if no misslge
}

Uint8 GetSens (int dx, int dy)
{
	if (dx == 1)
		return droite;
	if (dx == -1)
		return gauche;
	if (dy == 1)
		return bas;
	if (dy == -1)
		return haut;
}

void Jeu::MoveUs (Uint8 sens)
{
	int dx = 0,dy = 0;
	bool train = false;
	//regarde si on est sur un rail actif
	Uint16 c = t.t[t.posx][t.posy];
	if (r[c].is(rail_actif))
	{
		//printf("Rail active !\n");
		train = true;
		Pos p = GetRailSens(c, sens);
		dx = p.x;
		dy = p.y;
		if (!dx && !dy)
			return;
		sens = GetSens(dx, dy);
		DesactiveRail(t.posx, t.posy);
		//printf("Move to (%d,%d) ...\n",dx,dy);
	}else
	{
		dx = deplx(sens);
		dy = deply(sens);
	}
	//update direction
	t.sens = sens;
	Update_tsens ();
	//check if we can move forward
	if (t.posx+dx < 0 || t.posx+dx >= DIMX || t.posy+dy < 0 || t.posy+dy >=DIMY)
		return;
	c = t.t[t.posx+dx][t.posy+dy];
	if (r[c].is(mur))
	{
		if (r[c].is(porte)) //opening one way direction? (ouverture d'un sens unic)
		{
			if ((sens == bas && c == 81) || (sens == haut && c == 82) || (sens == gauche && c == 83) || (sens == droite && c == 84))
			{
				DoPorte(t.posx+dx, t.posy+dy);
				//printf("Opening one way direction (%d,%d)\n",t.posx+dx, t.posy+dy);
			}
		}
		if (!train || !r[c].is(rail))
			return;
	}
	if (r[t.t[t.posx][t.posy]].is(porte_ouverte))
	{//close a one way direction
		t.t[t.posx][t.posy] = 80 + (t.t[t.posx][t.posy]%10); //??? (ferme le sens unic)
	}
	//closes one way direction?? (ferme les sens unic a proximite)
	int dx2,dy2;
	Uint32 x2,y2;
	for (dx2 = -1;dx2<=1;dx2++)
	{
		for (dy2 = -1; dy2 <= 1; dy2++)
		{
			if ((dx2 || dy2) && (dx2 != dx || dy2 != dy))
			{
				x2 = (Uint16)(t.posx+dx2);
				y2 = (Uint16)(t.posy+dy2);
				if (x2 < DIMX && y2 < DIMY)
				{
					if (r[t.t[x2][y2]].is(porte_))//|| (t.a[x2][y2].n >= 1 && t.a[x2][y2].n <= 4)) //open one way direction
					{
						t.t[x2][y2] = 80 + (t.t[x2][y2]%10); //??? (ferme le sens unic)
						t.a[x2][y2].n = 0;
						Draw(x2, y2);
						//printf("closing one way direction (%d, %d) !\n",x2,y2);
					}
				}
			}
		}
	}
	//update the field of view
	UpdateVue(t.posx+dx, t.posy+dy);
	s.Play(2); //son du deplacement
	Draw(t.posx, t.posy); //on clear? (on s'efface)
	//movement animation
	int i;
	SDL_Rect src, pos, rc;
	rc.x = 0;
	rc.y = 0;
	rc.w = CX;
	rc.h = CY;
	src.x = CX*(t.sens);
	src.y = CY*(t.tsens);
	src.w = CX;
	src.h = CY;
	pos.x = t.posx*CX;
	pos.y = t.posy*CY;
	pos.w = CX;
	pos.h = CY;
	Uint8 *keystate;
	SDL_Event event;
	dx2 = dy2 = 0;
	Uint32 time,rtime;
	rtime = SDL_GetTicks();
	for (i=0;i<NMUS;i++)
	{
		if (invincible)
		{
			invincible--;
		}
		
		pos.x += (CX/NMUS)*dx;
		pos.y += (CY/NMUS)*dy;
		pr.x = pos.x-t.posx*CX;
		pr.y = pos.y-t.posy*CY;
		
		if (t.a[t.posx][t.posy].n)
			Draw(t.posx, t.posy);
		if (t.a[t.posx+dx][t.posy+dy].n)
			Draw(t.posx+dx, t.posy+dy);
		
		SDL_BlitSurface(jeu, &pos, ic, &rc); //retains what is below
		SDL_BlitSurface(im, &src, jeu, &pos); //and draws us
		/*
		if (!dovuerun)
		{
			Update(); //updated
		}
		 */
		//
		time = SDL_GetTicks();
		if ((int)time-rtime < TMUS)
			SDL_Delay(TMUS-(time-rtime));
		//else
		//	printf("MoveUs à la bourre...\n");
		//printf("time = %d, rtime = %d, realtime = %d\n",time,rtime,SDL_GetTicks());
		rtime = SDL_GetTicks();//time;//+TMUS;
		//
		rc.w = CX;
		rc.h = CY;
		SDL_BlitSurface(ic, &rc, jeu, &pos); //returns
		if (train)
		{
			SDL_PollEvent(&event);
			//printf(".\n");
			keystate = SDL_GetKeyState (NULL);
			if (keystate[key[thaut]])
			{
				//printf("top !!!\n");
				dy2 = -1;
			}
			if (keystate[key[tbas]])
			{
				//printf("down !!!\n");
				dy2 = 1;
			}
			if (keystate[key[tdroite]])
			{
				//printf("right !!!\n");
				dx2 = 1;
			}
			if (keystate[key[tgauche]])
			{
				//printf("left !!!\n");
				dx2 = -1;
			}
		}
	}
	//we move up our final position
	t.posx += dx;
	t.posy += dy;
	pr.x = 0;
	pr.y = 0;
	//manages the accleration
	if (tNMUS)
	{
		tNMUS--;
		if (!tNMUS)
		{
			NMUS = NMUS0;
		}
	}
	//Starts objects that react when over (demarre les objets qui réagissent quand on passe dessus)
	if (r[c].is(react_on)) // || r[t.sol[t.posx][t.posy]].is(react_on))
	{
		//printf("react_on...\n");
		ReactOn(t.posx, t.posy);
	}
	//starts nearby stuff
	for (dx=-1;dx<=1;dx++)
	{
		for (dy=-1;dy<=1;dy++)
		{
			if (t.posx+dx >= 0 && t.posx+dx < DIMX && t.posy+dy >= 0 && t.posy+dy < DIMY)
			{
				if (r[t.t[t.posx+dx][t.posy+dy]].is(explosif))
					DoExplosif(t.posx+dx, t.posy+dy);
			}
		}
	}
	DrawUs();
	//train
	if (train && r[c].is(rail))
	{
		if (dx2 != 0 || dy2 != 0)
		{
			//printf("You want to get out (%d,%d) ?\n",dx2,dy2);
			if (!r[c].is(rail_mur))
			{
				//printf(" ...track permits...\n");
				if ((int)t.posx+dx2 >= 0 && t.posx+dx2 < DIMX && (int)t.posy+dy2 >= 0 && t.posy+dy2 < DIMY)
				{
					//printf("... you do not come out of the screen...\n");
					c = t.t[t.posx+dx2][t.posy+dy2];
					if (!r[c].is(mur))
					{
						//printf("...no wall...\n");
						//printf(" ...pulls out !!!\n");
						DesactiveRail (t.posx, t.posy); //to be on...
						MoveUs(GetSens(dx2, dy2));
						return;
					}
				}
			}
		}
		if (r[c].is(rail))
		{
			//printf("Rail reactive...\n");
			ActiveRail (t.posx, t.posy);
			MoveUs(sens);
			return;
		}
	}
	if (r[c].is(tapis_roulant))
	{
		if (canmove)
			sens = t.sens;
		s.Play(9);
		MoveUs(sens);
		return;
	}
}

void pause (Uint32 t)
{
	Uint32 ti,rti;
	rti = SDL_GetTicks();
	SDL_Event event;
	for (;;)
	{
		SDL_Delay(10);
		SDL_PollEvent(&event);
		ti = SDL_GetTicks();
		if (ti-rti > t)
			break;
		if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
		{
			SDL_PushEvent(&event);
			break;
		}
	}
}

void Jeu::Recadre ()
{
	Pos p = GoodVue();
	dovuerun = false;
	vue.x = p.x;
	vue.y = p.y;
}

void Jeu::ChooseTab (Uint16 *niveau)
{
	SDL_Event event;
	bool update = true;
	quit = false;
	do
	{
		if (update)
		{
			update = false;
			DispInfo(*niveau);
			t.sx = t.sy = 0;
#if MAC
			//UnsignedWide w1,w2,w3,w4,w5;
			//Microseconds(&w1);
#endif
			Load(*niveau);
#if MAC
			//Microseconds(&w2);
#endif
			pr.x = 0;
			pr.y = 0;
			StartVue(); //place le champ de vision où il faut
#if MAC
			//Microseconds(&w3);
#endif
			Draw();
#if MAC
			//Microseconds(&w4);
#endif
			Update();
#if MAC
			//Microseconds(&w5);
			//printf("temps de Load : %d\n",w2.lo-w1.lo);
			//printf("temps de StartVue : %d\n",w3.lo-w2.lo);
			//printf("temps de Draw : %d\n",w4.lo-w3.lo);
			//printf("temps de Update : %d\n",w5.lo-w4.lo);
#endif
		}
		if (SDL_WaitEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					win = kabort;
					quit = true;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							win = kabort;
							quit = true;
							break;
						case SDLK_LEFT:
							if (*niveau)
								*niveau = *niveau - 1;
							update = true;
							break;
							/*
						case SDLK_:
							*niveau = 0;
							update = true;
							break;
							*/
						case SDLK_F1:
							*niveau = 10;
							update = true;
							break;
						case SDLK_F2:
							*niveau = 20;
							update = true;
							break;
						case SDLK_F3:
							*niveau = 30;
							update = true;
							break;
						case SDLK_F4:
							*niveau = 40;
							update = true;
							break;
						case SDLK_F5:
							*niveau = 50;
							update = true;
							break;
						case SDLK_F6:
							*niveau = 60;
							update = true;
							break;
						case SDLK_F7:
							*niveau = 70;
							update = true;
							break;
						case SDLK_F8:
							*niveau = 90;
							update = true;
							break;
						case SDLK_F9:
							*niveau = 90;
							update = true;
							break;
						case SDLK_F10:
							*niveau = 100;
							update = true;
							break;	
						case SDLK_UP:
							*niveau = abs(rand())%101;
							update = true;
							break;
						case SDLK_RIGHT:
							*niveau = *niveau + 1;
							update = true;
							break;
					}
			}
		}
	}while(!quit);
}

void Jeu::DoPause ()
{
	do
	{
		SDL_Delay(10);
	}while(paused);
}

void Jeu::Pause ()
{
	SDL_Event event;
	paused = true;
	bool cont = true;
	do
	{
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				cont = false;
				quit = true;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						cont = false;
						break;
					case SDLK_p:
						cont = false;
						break;
				}
				break;
		}
	}while(cont);
	paused = false;
}

void Jeu::Play (Uint32 niveau)
{
	pr.x = 0;
	pr.y = 0;
	
	StartVue(); //place field of view where it is necessary?? (place le champ de vision où il faut)
	Draw();
	//Update();
	
	quit = false;
	quitanim = false;
	moveus = false;
	canmove = false;
	win = perdu;
	t.sens = bas;
	t.tsens = t.sens;
	invincible = 0;
	
	//start thread for animations
	SDL_Thread *tanim = SDL_CreateThread(DoAnim, this);
	//start thread for bulles
	SDL_Thread *tballs = SDL_CreateThread(DoBalls, this);
	
	SDL_PumpEvents();
	
	Anime(t.posx, t.posy, 18); //materialisation
	//DrawUs();
	
	SDL_Event event;
	//SDL_EnableKeyRepeat(10, 10);
	Uint32 time,rtime;
	Uint8 *keystate;
	rtime = SDL_GetTicks();
	
	pause(1200);
	
	do
	{
		time = SDL_GetTicks();
		if ((int)time-rtime < 50)
			SDL_Delay(50-(time-rtime));
		//else
		//	printf("Play à la bourre...\n");
		rtime = SDL_GetTicks();//time+50;
		if (moveus)
		{
			MoveUs(t.sens);
			moveus = false;
		}
		if (invincible)
			invincible--;
		if (SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					win = kabort;
					quit = true;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							win = kabort;
							quit = true;
							break;
#if !FINAL || TRICHE
						case SDLK_a:
							quitanim = true;
							break;
						case SDLK_e:
							Edit(niveau);
							break;
						case SDLK_m:
							if (!invincible)
								invincible = INVINCIBLE;
							else
								invincible = (Uint16)(-1);
							break;
						case SDLK_u:
							niveau--;
							DispInfo(niveau);
							t.sx = t.sy = 0;
							Load(niveau);
							pr.x = 0;
							pr.y = 0;
							StartVue(); //instead the field of view where it is necessary?? (place le champ de vision où il faut)
							Draw();
							break;
						case SDLK_l:
							niveau = abs(rand())%101;
							DispInfo(niveau);
							t.sx = t.sy = 0;
							Load(niveau);
							pr.x = 0;
							pr.y = 0;
							StartVue(); //instead the field of view where it is necessary? (place le champ de vision où il faut)
							Draw();
							break;
						case SDLK_o:
							niveau++;
							DispInfo(niveau);
							Load(niveau);
							pr.x = 0;
							pr.y = 0;
							StartVue(); //instead the field of view where it is necessary?? (place le champ de vision où il faut)
							Draw();
							break;
#endif
						case SDLK_p:
							Pause();
							break;
						case SDLK_t:
							Recadre();
							break;
						case SDLK_i:
							printf("t[%d][%d] = %d\n",t.posx,t.posy,t.t[t.posx][t.posy]);
							printf("sol = %d\n",t.sol[t.posx][t.posy]);
							break;
						case SDLK_y:
							int cpt = 0;
							for (int x=0;x<DIMX;x++)
							{
								for (int y=0;y<DIMY;y++)
								{
									if (t.a[x][y].n == 42)
										cpt++;
								}
							}
							printf("Infinite number of batteries : %d\n",cpt);
							break;
					}
					if (event.key.keysym.sym == key[tteleporter])
					{
						if (r[t.t[t.posx][t.posy]].is(teleporteur)) //teleporter
						{
							Teleporte(t.t[t.posx][t.posy]);
						}
					}
					if (event.key.keysym.sym == key[ttirer])
					{
						if (r[t.objet].is(missile)) //firing missle
						{
							int dx,dy;
							dx = dy = 0;
							if (t.objet == 61)
								dx = 1;
							if (t.objet == 62)
								dx = -1;
							if (t.objet == 70)
								dy = 1;
							if (t.objet == 63)
								dy = -1;
							s.Play(3);
							Balle(t.posx, t.posy, dx, dy);
							t.objet = 0;
							OpenBarriere();
							Update_tsens ();
							DrawUs();
						}
					}
					if (event.key.keysym.sym == key[tprendre]) //take an object
					{
						if (r[t.t[t.posx][t.posy]].is(objet))
						{
							if (t.objet)
							{//exchange two objects
								Uint16 tamp = t.objet;
								t.objet = t.t[t.posx][t.posy];
								t.t[t.posx][t.posy] = tamp;
							}else
							{//takes the object
								t.objet = t.t[t.posx][t.posy];
								t.t[t.posx][t.posy] = 0;
								CloseBarriere();
							}
							Update_tsens ();
							DrawUs();
							s.Play(13);
						}else
						{
							if (!t.t[t.posx][t.posy])
							{
								if (t.objet)
								{//depose l'objet
									t.t[t.posx][t.posy] = t.objet;
									t.objet = 0;
									Update_tsens ();
									DrawUs();
									s.Play(13);
									OpenBarriere();
								}
							}else
							{
								if (t.t[t.posx][t.posy] == 150 && t.objet == 140) //lock && key?? (serrure && cle)
								{
									t.objet = 0;
									Anime(t.posx, t.posy, 30);
									OpenBarriere();
								}
								if (r[t.t[t.posx][t.posy]].is(rail) && t.objet == 163) //rail && pile
								{
									s.Play(21);
									ActiveRail(t.posx, t.posy);
									t.objet = 0;
									//moveus = true;
									MoveUs(t.sens);
									OpenBarriere();
								}
							}
						}
					}
					if (event.key.keysym.sym == key[tsuicide])
					{
						invincible = 0;
						Perdu();
					}
					break;
			}
		}
		keystate = SDL_GetKeyState (NULL);
		if (keystate[key[thaut]])
		{
			MoveUs(haut);
		}
		if (keystate[key[tbas]])
		{
			MoveUs(bas);
		}
		if (keystate[key[tdroite]])
		{
			MoveUs(droite);
		}
		if (keystate[key[tgauche]])
		{
			MoveUs(gauche);
		}
	}while(!quit);
	
	printf("end of game...\n");
	
	switch(win)
	{
		case perdu:
			printf("You died !\n");
			printf("pause 2 sec...\n");
			pause(2000);
			s.Play(23);
			printf("pause 3 sec...\n");
			pause(3000);
			break;
		case gagne:
			printf("You won !\n");
			printf("pause 2 sec...\n");
			pause(2000);
			s.Play(22);
			printf("pause 3 sec...\n");
			pause(3000);
			break;
	}
	
	printf("terminate thread ...\n");
	
	//quit thread for animations
	quitanim = true;
	pause(10*max(TANIM,max(TVUE,TBALLS))); //wait a bit to allow the thread to stop
	SDL_PumpEvents();
	
	if (win != perdu)
	{
		t.sx = t.sy = 0; //resets place of occurrence
	}
	printf("fin de Play !\n");
}

int main(int argc, char *argv[])
{
	Jeu j;
	//Initalize the game
	j.init();
	//launch the main window
	j.Main();
	//quit the game
	j.end();
	return EXIT_SUCCESS;
}
