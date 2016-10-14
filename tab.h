#define DIMX	40 //dimension of a table (in boxes)
#define DIMY	24

#define TRAIL	5 //time between a displacement of a box to another green (useless)

//possible directions
enum
{
	left = 0,
	up = 1,
	right = 2,
	down = 3
};

struct InfoAnim
{
	Uint16 n; //number of animations
	Uint32 t; //time
	
	void init ();
};

void InfoAnim::init ()
{
	n = 0;
	t = 0;
}

struct Tab
{
	Uint8 sol[DIMX][DIMY];
	Uint16 t[DIMX][DIMY];
	InfoAnim a[DIMX][DIMY]; //used for animations (after time)
	Uint16	posx; //our position
	Uint16	posy;
	Uint16	sx; //respawn position
	Uint16	sy;
	Uint16	so; //objet que l'on a quand on réapparait
	Uint8	sens; //our direction
	Uint8	tsens; //our direction of shooting
	Uint16	objet;
	
	void Load (FILE *f); //load picture from file
	void Save (FILE *f); //save picture to file 
	void Load (Uint8 n); //load picture n
	void Save (Uint8 n); //save picture n
	void Load (const char *file); //load picture from file
	void Save (const char *file); //save picture to file
	void Default (); //default picture (empty)
};

struct Pos
{
	Sint16 x;
	Sint16 y;
};

void Tab::Default ()
{
	int x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			t[x][y] = 0;
			sol[x][y] = 41;
			a[x][y].init();
		}
	}
	for (x=0;x<DIMX;x++)
	{
		t[x][0] = 26;
		t[x][DIMY-1] = 26;
	}
	for (y=0;y<DIMY;y++)
	{
		t[0][y] = 17;
		t[DIMX-1][y] = 17;
	}
	t[0][0] = 19;
	t[DIMX-1][0] = 18;
	t[0][DIMY-1] = 29;
	t[DIMX-1][DIMY-1] = 14;
	t[1][1] = 283;
	posx = 1;
	posy = 1;
}

void Tab::Load (FILE *f)
{
	//initialise les sens
	sens = 1;
	tsens = 1;
	//initialise l'objet
	objet = 0;
	//initialise le tableau d'animations
	int i,j;
	for (j=0;j<DIMY;j++)
	{
		for (i=0;i<DIMX;i++)
		{
			a[i][j].init();
		}
	}
	int x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (!Read(&t[x][y],f))
			{
				fprintf(stderr,"Erreur de lecture !!!\n");
				return;
			}
			if (t[x][y] == 11)
			{
				posx = x;
				posy = y;
				t[x][y] = 283; //truc ou on reapparait
			}
			if (t[x][y] >= NI)
				t[x][y] = 0;
		}
	}
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (!fread(&sol[x][y],sizeof(Uint8),1,f))
			{
				fprintf(stderr,"Erreur pendant la lecture du sol !!!\n");
				return;
			}
		}
	}
	//on reapparait au bon endroit
	if (sx != 0 || sy != 0)
	{
		posx = sx;
		posy = sy;
		objet = so;
	}
}

void Tab::Save (FILE *f)
{
	int x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (!Write(t[x][y],f))
			{
				fprintf(stderr,"Erreur d'ecriture !!!\n");
				return;
			}
		}
	}
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			if (!fwrite(&sol[x][y],sizeof(Uint8),1,f))
			{
				fprintf(stderr,"Erreur pendant l'ecriture du sol !!!\n");
				return;
			}
		}
	}
}

void Tab::Load (Uint8 n) //charge le tableau numero n
{
	char tamp[256];
#if EDIT
	char txt[40];
	sprintf(txt,"tableau%d",n);
	Load(txt);
#else
	MakeName("LEVEL.DAT", tamp);
	FILE *f = fopen(tamp,"r");
	if (!f)
	{
		fprintf(stderr,"Impossible d'ouvrir le fichier de niveaux !\n");
		return;
	}
	Uint16 N; //nombre de tableaux
	Read(&N,f);
	if (n >= N)
	{
		printf("Le tableau %d n'existe pas (tableaux de 0 a %d) !!!\n",n,N-1);
		return;
	}
	if (fseek(f,40*36*2*n,SEEK_CUR))
	{
		printf("Erreur : le fseek ne marche pas !!!\n");
	}
	Load(f);
	fclose(f);
#endif
}

void Tab::Save (Uint8 n) //charge le tableau numero n
{
	char tamp[256];
#if EDIT
	char txt[40];
	sprintf(txt,"tableau%d",n);
	Save(txt);
#else
	MakeName("LEVEL.DAT", tamp);
	FILE *f = fopen(tamp,"r+");
	if (!f)
	{
		fprintf(stderr,"Impossible d'ouvrir le fichier de niveaux !\n");
		return;
	}
	Uint16 N; //nombre de tableaux
	Read(&N,f);
	if (n >= N)
	{
		printf("Le tableau %d n'existe pas (tableaux de 0 a %d) !!!\n",n,N-1);
		return;
	}
	if (fseek(f,40*36*2*n,SEEK_CUR))
	{
		printf("Erreur : le fseek ne marche pas !!!\n");
	}
	Save(f);
	fclose(f);
#endif
}

void Tab::Load (const char *file)
{
	Default();
	char tamp[256];
	MakeNameTAB(file, tamp);
	FILE *f = fopen(tamp,"r");
	if (!f)
	{
		fprintf(stderr,"Impossible d'ouvrir le fichier %s pour le lire !\n",tamp);
		return;
	}
	Load(f);
	fclose(f);
}

void Tab::Save (const char *file)
{
	char tamp[256];
	MakeNameTAB(file, tamp);
	FILE *f = fopen(tamp,"w");
	if (!f)
	{
		fprintf(stderr,"Impossible d'ouvrir le fichier %s pour l'ecrire !\n",tamp);
		return;
	}
	Save(f);
	fclose(f);
}

