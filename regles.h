
//masques pour le type
enum
{
	mur = 1<<0,
	objet = 1<<1,
	mechant = 1<<2,
	explosif = 1<<3,
	react_ball = 1<<4,
	react_on = 1<<5,
	missile = 1<<6,
	stop_ball = 1<<7,
	porte = 1<<8, //sens unic
	porte_ouverte = 1<<9, //sens unic ouvert
	onda_mania = 1<<10,
	rail = 1<<11,
	rail_actif = 1<<12,
	plante = 1<<13,
	rond_colore = 1<<14,
	teleporteur = 1<<15,
	rail_mur = 1<<16,
	act_ball = 1<<17,
	tapis_roulant = 1<<18,
	fleche = 1<<19,
	truc_ordi = 1<<20,
	onda_mania_bouge = 1<<21,
	sol = 1<<22,
	porte_ = 1<<23,
	rail_tourne = 1<<24,
	wellable = 1<<25,
	vraimur = 1<<26,
	vraiplante = 1<<27,
	fleche_sol = 1<<28
};

struct Regles
{
	Uint32 type; //type d'objet
	
	void init ();
	inline bool is (Uint32 type2);
};

inline bool Regles::is (Uint32 type2)
{
	return type & type2;
}

void Regles::init ()
{
	type = 0;
}

Uint32 getdefnum (const char *word)
{
	if (!strcmp(word, "mur"))
		return mur;
	if (!strcmp(word, "objet"))
		return objet;
	if (!strcmp(word, "mechant"))
		return mechant;
	if (!strcmp(word, "explosif"))
		return explosif;
	if (!strcmp(word, "react_ball"))
		return react_ball;
	if (!strcmp(word, "react_on"))
		return react_on;
	if (!strcmp(word, "missile"))
		return missile;
	if (!strcmp(word, "stop_ball"))
		return stop_ball;
	if (!strcmp(word, "porte"))
		return porte;
	if (!strcmp(word, "porte_ouverte"))
		return porte_ouverte;
	if (!strcmp(word, "onda-mania"))
		return onda_mania;
	if (!strcmp(word, "rail"))
		return rail;
	if (!strcmp(word, "rail_actif"))
		return rail_actif;
	if (!strcmp(word, "plante"))
		return plante;
	if (!strcmp(word, "rond_colore"))
		return rond_colore;
	if (!strcmp(word, "teleporteur"))
		return teleporteur;
	if (!strcmp(word, "rail_mur"))
		return rail_mur;
	if (!strcmp(word, "act_ball"))
		return act_ball;
	if (!strcmp(word, "tapis_roulant"))
		return tapis_roulant;
	if (!strcmp(word, "fleche"))
		return fleche;
	if (!strcmp(word, "truc_ordi"))
		return truc_ordi;
	if (!strcmp(word, "onda_mania_bouge"))
		return onda_mania_bouge;
	if (!strcmp(word, "sol"))
		return sol;
	if (!strcmp(word, "porte_"))
		return porte_;
	if (!strcmp(word, "rail_tourne"))
		return rail_tourne;
	if (!strcmp(word, "wellable"))
		return wellable;
	if (!strcmp(word, "vraimur"))
		return vraimur;
	if (!strcmp(word, "vraiplante"))
		return vraiplante;
	if (!strcmp(word, "fleche_sol"))
		return fleche_sol;
	return 0;
}

void LoadRegles (Regles *r)
{
	int i;
	for (i=0;i<NI;i++)
	{
		r[i].init();
	}
	char tamp[256];
	MakeName("regles.txt", tamp);
	FILE *f = fopen(tamp,"r");
	if (!f)
	{
		fprintf(stderr, "Impossible de charger les regles !!!\n");
		return;
	}
	
	char word[256];
	const char *ptr;
	Uint32 def = 0;
	Uint16 n;
		
	for (i=0;;i++)
	{
		//printf("Lecture de la ligne %d : ",i);
		if (!readline(f, tamp, 256))
			break;
		//printf(tamp);
		//printf("\n");
		ptr = tamp;
		if (ptr = findword(tamp, word, 256)) //cherche s'il y a une nouvelle definition
		{ //debut d'une nouvelle definition
			//printf(" mot trouve : %s , reste : %s\n",word,ptr);
			if (!(def = getdefnum (word)))
			{
				fprintf(stderr, "Le type %s est inconnu !\n",word);
			}
		}else
			ptr = tamp;
		if (def)
		{
			for(;;) //cherche tous les nombres de la definition
			{
				ptr = findnum (ptr, &n);
				if (!ptr)
					break;
				//printf("nombre trouve : %d\n",n);
				//ajoute cette nouvelle caracteristique de l'objet n
				if (n < NI)
					r[n].type |= def;
				else
				{
					fprintf(stderr, "Erreur : definition portant sur l'objet %d n'existant pas !!!\n",n);
				}
			}
		}
	}
	
	fclose(f);
}
