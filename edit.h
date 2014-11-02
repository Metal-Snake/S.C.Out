
#define EX	40	//nombre d'images en abscisse
#define EY	8	//nombre d'images en ordonnée

struct Edit
{
	Uint16 t[EX][EY];
	Uint16 sel; //ce qui est selectionne
	
	void Load (); //charge la liste des images de l'editeur
	void Draw (SDL_Surface *im, SDL_Surface *ecran); //affiche les images de l'editeur
	void Draw (SDL_Surface *im, SDL_Surface *ecran, Uint16 x, Uint16 y); //affiche les images de l'editeur
	void Select (Uint16 x, Uint16 y); //selectionne
};
typedef struct Edit Edit;

void Edit::Select (Uint16 x, Uint16 y) //selectionne
{
	if (x < EX && y < EY)
	{
		sel = t[x][y];
	}
}

void Edit::Load ()
{
	char tamp[256];
	MakeName("edit.txt", tamp);
	FILE *f = fopen(tamp,"r");
	if (f)
	{
		char tamp[256];
		int x,y;
		const char *ptr = tamp;
		for (y=0;y<EY;y++)
		{
			//printf("lecture de la ligne %d...\n",y);
			if (!readline(f, tamp, 256))
			{
				break;
			}
			ptr = tamp;
			//printf("%s\n",tamp);
			for (x=0;x<DIMX;x++)
			{
				ptr = findnum (ptr, &t[x][y]);
				if (!ptr && x!=DIMX-1)
				{
					printf("Attention : ligne %d incomplete (%d/%d) !\n",y,x,DIMX);
				}
			}
		}
		sel = 0;
		fclose(f);
	}else
	{
		fprintf(stderr,"Le fichier de l'editeur est impossible a ouvrir !!!\n");
	}
}

void Edit::Draw (SDL_Surface *im, SDL_Surface *ecran, Uint16 x, Uint16 y)
{
	SDL_Rect src, pos;
	src.x = CX*(t[x][y]%10);
	src.y = CY*(t[x][y]/10);
	src.w = CX;
	src.h = CY;
	pos.x = x*CX;
	pos.y = (y+DIMY)*CY;
	SDL_BlitSurface(im, &src, ecran, &pos);
}

void Edit::Draw (SDL_Surface *im, SDL_Surface *ecran)
{
	Uint16 x,y;
	for (y=0;y<EY;y++)
	{
		for (x=0;x<EX;x++)
		{
			if (t[x][y])
				Draw(im, ecran, x, y);
		}
	}
}
