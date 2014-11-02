#define DIMX	40 //dimension d'un tableau (en cases)
#define DIMY	24

#define TRAIL	5 //temps entre un deplacement d'une case a l'autre du vert (inutile)

//sens possibles
enum
{
	gauche = 0,
	haut = 1,
	droite = 2,
	bas = 3
};

struct InfoAnim
{
	Uint16 n; //numero de l'animation
	Uint32 t; //temps
	
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
	InfoAnim a[DIMX][DIMY]; //utilise pour les animations (compte le temps)
	Uint16	posx; //notre position
	Uint16	posy;
	Uint16	sx; //position ou on reapparait
	Uint16	sy;
	Uint16	so; //objet que l'on a quand on réapparait
	Uint8	sens; //notre sens
	Uint8	tsens; //notre sens de tir
	Uint16	objet;
	
	void Load (FILE *f); //charge le tableau du fichier f
	void Save (FILE *f); //enregistre le tableau dans le fichier f
	void Load (Uint8 n); //charge le tableau numero n
	void Save (Uint8 n); //enregistre le tableau numero n
	void Load (const char *file); //charge le tableau file
	void Save (const char *file); //enregistre le tableau file
	void Default (); //tableau vide par defaut
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

/*
void Tab::Load (Uint8 n) //charge le tableau numero n
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
	char tamp[256];
	MakeName("LEVEL.DAT", tamp);
	int f = open(tamp, O_RDONLY);
	if (!f)
	{
		fprintf(stderr,"Impossible d'ouvrir le fichier de niveaux !\n");
		return;
	}
	Uint8 c;
	Uint16 N; //nombre de tableaux
	Read(&N,f);
	if (n >= N)
	{
		printf("Le tableau %d n'existe pas (tableaux de 0 a %d) !!!\n",n,N-1);
		return;
	}
	if (seek(f,40*36*2*n))//,SEEK_CUR))
	{
		printf("Erreur : le fseek ne marche pas !!!\n");
	}
	int x,y;
	for (y=0;y<DIMY;y++)
	{
		for (x=0;x<DIMX;x++)
		{
			Read(&t[x][y],f);
			/*
			if (eof(f))
			{
				fprintf(stderr,"Erreur de lecture !!!\n");
				return;
			}
			 *//*
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
			//if (!fread(&sol[x][y],sizeof(Uint8),1,f))
			read(f, &sol[x][y], 1);
			/*
			if(eof(f))
			{
				fprintf(stderr,"Erreur pendant la lecture du sol !!!\n");
				return;
			}
			 *//*
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
*/