
#define NA	50	//max number of animations? (nombre max. d'animations)

#define NIM	20 //max number of frames in an animation? (nombre max. d'images dans une animation)

struct Anim
{
	Uint16 l[NIM];	//liste des images
	Uint16 n;	//nombre d'images
	Uint32	t; //temps entre deux images
	Uint16	s; //son
	
	void init ();
};

void Anim::init ()
{
	n = 0;
	t = 0;
	s = 0;
}

void LoadAnims (Anim *a)
{
	//printf("\n\n\n\n");	
	int i;
	for (i=0;i<NA;i++)
	{
		a[i].init();
	}
	char tamp[256];
	MakeName("anims.txt", tamp);
	FILE *f = fopen(tamp,"r");
	if (!f)
	{
		fprintf(stderr, "Impossible de charger les animations !!!\n");
		return;
	}
	
	const char *ptr, *ptr2;
	int n = -1; //numero de l'animation
	Uint16 k; //numéro d'image
		
	for (i=0;;i++)
	{
		//printf("Lecture de la ligne %d : ",i);
		if (!readline(f, tamp, 256))
		{
			//printf("fin...\n");
			break;
		}
		//printf(tamp);
		//printf("\n");
		ptr = findstart(tamp, &n, 256); //regarde s'il y a le debut d'une nouvelle animation
		if (ptr == NULL)
			ptr = tamp;
		else
		{
			//printf("Animation %d ...\n",n);
		}
		if (n >= NA)
		{
			fprintf(stderr, "Erreur : animation d'indice trop grand (%d) !!!\n",n);
			return;
		}
		if (n != -1)
		{
			for(;;) //cherche toutes les images de l'animation
			{
				ptr2 = findnum (ptr, &k);
				if (!ptr2)
				{
					break;
				}
				ptr = ptr2;
				//printf("nombre trouve : %d\n",k);
				//ajoute l'image a la suite
				if (k < NI)
				{
					if (a[n].n < NIM)
					{
						a[n].l[a[n].n] = k;
						a[n].n++;
					}else
						fprintf(stderr, "Erreur : trop d'images pour l'animation %d !\n",n);
				}else
				{
					fprintf(stderr, "Erreur : limage %d de l'animation %d (%d) n'existe pas !!!\n",a[n].n,n,k);
				}
			}
			//passe les espaces
			ptr = pass(ptr);
			if (*ptr == ':')
			{
				ptr++;
				//lit le temps entre deux images
				Uint16 t2;
				a[n].t = 1; //temps par defaut
				ptr2 = findnum(ptr, &t2);
				if (ptr2)
				{
					ptr = ptr2;
					a[n].t = t2;
					//printf("temps entre deux images : %d\n",t2);
				}
			}
			//passe les espaces
			ptr = pass(ptr);
			if (*ptr == ':')
			{
				ptr++;
				//lit le numero du son
				Uint16 s2;
				ptr2 = findnum(ptr, &s2);
				if (ptr2)
				{
					ptr = ptr2;
					a[n].s = s2;
					//printf("son : %d\n",s2);
				}
			}
		}
	}
	
	fclose(f);
}
