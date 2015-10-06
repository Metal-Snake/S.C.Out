bool SON = true;

#define NSONSM	26 //nombre de sons max charges en memoire
#define NSONS 32 //nombre de sons pouvant etre joues simultanement

struct Son
{
	Uint8 *data;
	Uint32 size;
};
typedef struct Son Son;

Son LoadSon (const char *file)
{
	SDL_AudioSpec wav_spec;
	Uint32 wav_length;
	Uint8 *wav_buff;
	SDL_AudioCVT  wav_cvt;
	
	SDL_AudioSpec des;
	des.freq = 11050;
	des.format = AUDIO_U8;
	//des.samples = 256;
	//des.callback = NULL;
	des.channels = 1;
	//des.userdata = NULL;
	
	if(SDL_LoadWAV(file, &wav_spec, &wav_buff, &wav_length) == NULL)
	{
		fprintf(stderr, "Impossible d'ouvrir %s : %s\n", file, SDL_GetError());
		exit(EXIT_FAILURE);
	}
	
	Son s;
	
	if (wav_spec.freq == 5525)
	{
		//conversion
		Uint8 *data = (Uint8 *)malloc(sizeof(Uint8)*wav_length*2);
		if (!data)
		{
			fprintf(stderr,"Out of memory !!!\n");
		}
		Uint32 i;
		Uint8 *from = (Uint8 *)wav_buff;
		Uint8 *dest = data;
		Uint8 rf =0;
		for (i=0;i<wav_length;i++)
		{
			*dest = (*from+2*rf)/3;
			dest++;
			*dest = (*from*2+rf)/3;
			rf = *from;
			dest++;
			from++;
		}
		SDL_FreeWAV(wav_buff);
		wav_buff = (Uint8 *)data;
		wav_length *= 2;
	}
	 
	/*
	Uint8 *data = (Uint8 *)malloc(wav_length);
	memcpy(data, wav_buff, wav_length);
	// We can delete to original WAV data now
	SDL_FreeWAV(wav_buff);
	wav_buff = data;
	*/
	
	s.data = wav_buff; //wav_cvt.buf;
	
	if (!s.data)
	{
		fprintf(stderr,"Erreur : pas de son apres conversion !!!\n");
	}
	
	s.size = wav_length; //wav_cvt.len_cvt;
	//printf("Taille du son : avant %d, apres %d\n",wav_cvt.len,s.size);
	//printf("Taille du son : %d\n",wav_length);
	return s;
}

struct RefSon
{
	Uint8 *ptr; //la ou on en est (NULL si son pas lu)
	Uint32 i; //indice du son
};
typedef struct RefSon RefSon;

void mixaudio(void *unused, Uint8 *stream, int len);

struct Sons
{
	Son s[NSONSM];
	RefSon r[NSONS];
	
	//
	void Init (); //initialise tout (et charge les sons)
	void Load (); //charge tous les sons
	void Load (const char *file, Uint16 i); //charge le i-eme son (necessairement un .wav)
	void init (); //initialise la structure
	void Start (); //demarre le systeme de son
	void Stop (); //arret du systeme de son
	void Play (Uint16 i); //joue le i-eme son
};

void Sons::Init ()
{
	init(); //initialise la structure
	Start(); //demarre le systeme de son
	Load(); //charge tous es sons
}
void Sons::Load (const char *file, Uint16 i)
{
	s[i] = LoadSon(file);
}
void Sons::Load ()
{
	int i;
	char file[256];
	char tamp[256];
	MakeNameS("son00.wav", file);
	for (i=1;i<NSONSM;i++)
	{
		file[strlen(DSONS)+3] = '0'+i/10;
		file[strlen(DSONS)+4] = '0'+i%10;
		MakeName(file, tamp);
		Load(tamp, i);
	}
}
void Sons::init ()
{
	int i;
	for (i=0;i<NSONSM;i++)
	{
		s[i].data = NULL;
		s[i].size = 0;
	}
	for (i=0;i<NSONS;i++)
	{
		r[i].ptr = NULL;
		r[i].i = 0;
	}
}
void Sons::Start ()
{
	init();
	SDL_AudioSpec des,obt;
	des.freq = 11050;
	des.format = AUDIO_U8;
	des.samples = 256;
	des.callback = mixaudio;
	des.channels = 1;
	des.userdata = this;
	if (SDL_OpenAudio(&des,&obt) < 0)
	{
		printf("Impossible d'ouvrir l'audio : %s\n",SDL_GetError());
		exit(EXIT_FAILURE);
	}
	if (obt.freq != des.freq)
	{
		printf("Frequence desiree non obtenue !!!!\n");
	}
	/*
	printf("Audio obtenu:\n");
	printf("freq : %d\n",obt.freq);
	printf("samples : %d\n",obt.samples);
	printf("channels : %d\n",obt.channels);
	printf("format : %d\n",obt.format);
	
	printf("Liste des formats:\n");
	printf("AUDIO_U8 = %d\n",AUDIO_U8);
	printf("AUDIO_S8 = %d\n",AUDIO_S8);
	printf("AUDIO_U16 = %d\n",AUDIO_U16);
	printf("AUDIO_S16 = %d\n",AUDIO_S16);
	printf("AUDIO_U16MSB = %d\n",AUDIO_U16MSB);
	printf("AUDIO_S16MSB = %d\n",AUDIO_S16MSB);
	printf("AUDIO_U16SYS = %d\n",AUDIO_U16SYS);
	printf("AUDIO_S16SYS = %d\n",AUDIO_S16SYS);
	*/
	//lancement du son
	SDL_PauseAudio(0);
}
void Sons::Stop ()
{
	 SDL_CloseAudio();
}
void Sons::Play (Uint16 i) //joue le i-eme son
{
#if HSON
	if (!SON)
		return;
#else
	return;
#endif
	if (i == 0)
		return;
	if (i >= NSONSM)
	{
		fprintf(stderr, "Ce son n'existe pas !!!\n");
		return;
	}
	//cherche un emplacement libre
	///SDL_LockAudio();
	int j;
	for (j=0;j<NSONS;j++)
	{
		if (r[j].ptr == NULL)
		{ //lance le son
			//printf("Lancement du son %d en %d ...\n",i,j);
			r[j].ptr = s[i].data;
			r[j].i = i;
			break;
		}
	}
	///SDL_UnlockAudio();
}

void mixaudio(void *userdata, Uint8 *stream, int len)
{
	int i;
	Uint32 amount;
	Sons *s = (Sons *)userdata;
	for (i=0;i<NSONS;i++)
	{
		if (s->r[i].ptr != NULL)
		{
			//printf("son %d...\n",i);
			amount = s->s[s->r[i].i].size - (s->r[i].ptr - s->s[s->r[i].i].data); //quantite de son restant a jouer
			if ( amount > len )
			{
				amount = len;
			}
			SDL_MixAudio(stream, s->r[i].ptr, amount, SDL_MIX_MAXVOLUME);
			s->r[i].ptr += amount;
			if (amount < len)
			{
				s->r[i].ptr = NULL;
			}
		}
	}
}
