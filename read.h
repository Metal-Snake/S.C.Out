void MakeName (const char *txt, char *res)
{
	Uint16 i;
	for (i=0;i<strlen(RESOURCES);i++)
	{
		res[i] = RESOURCES[i];
	}
	for (i=strlen(RESOURCES);i<=strlen(RESOURCES)+strlen(txt);i++)
	{
		res[i] = txt[i-strlen(RESOURCES)];
	}
	//printf("Nom obtenu : %s + %s = %s\n",RESOURCES, txt, res);
}

void MakeNameS (const char *txt, char *res)
{
	Uint16 i;
	for (i=0;i<strlen(DSONS);i++)
	{
		res[i] = DSONS[i];
	}
	for (i=strlen(DSONS);i<=strlen(DSONS)+strlen(txt);i++)
	{
		res[i] = txt[i-strlen(DSONS)];
	}
	//printf("Nom obtenu : %s + %s = %s\n",RESOURCES, txt, res);
}

void MakeNameTAB (const char *txt, char *res)
{
	Uint16 i;
	for (i=0;i<strlen(TAB);i++)
	{
		res[i] = TAB[i];
	}
	for (i=strlen(TAB);i<=strlen(TAB)+strlen(txt);i++)
	{
		res[i] = txt[i-strlen(TAB)];
	}
	//printf("Nom obtenu : %s + %s = %s\n",RESOURCES, txt, res);
}

bool Read (Uint16 *n, FILE *f)
{
	Uint8 c;
	Uint16 N;
	if (!fread(&c,sizeof(c),1,f))
		return false;
	N = c;
	if (!fread(&c,sizeof(c),1,f))
		return false;
	N |= c<<8;
	*n = N;
	return true;
}
bool Write (Uint16 n, FILE *f)
{
	Uint8 c;
	c = n%256;
	if (!fwrite(&c,sizeof(c),1,f))
		return false;
	c = n/256;
	if (!fwrite(&c,sizeof(c),1,f))
		return false;
	return true;
}
/*
bool Read (Uint16 *n, int f)//FILE *f)
{
	Uint8 c;
	Uint16 N;
	//if (!fread(&c,sizeof(c),1,f))
	//      return false;
	read(f, &c, 1);
	//return false;
	N = c;
	//if (!fread(&c,sizeof(c),1,f))
	//      return false;
	read(f, &c, 1);
	N |= c<<8;
	*n = N;
	return true;
}
*/
int seek (int f, Uint32 pos)
{
	Uint32 i;
	unsigned char c;
	for (i=0;i<pos;i++)
	{
			read(f, &c, 1);
	}
	return 0;
}

bool readline (FILE *f, char *txt, long max)
{
	char c,rc;
	c = 0;
	int i;
	for (i=0;;i++)
	{
		rc = c;
		if (!fread(&c,sizeof(c),1,f) || i>=max)
		{
			if (i>=max)
				printf("Attention : ligne trop longue !!!\n");
			return false;
		}
		if (c=='/' && rc=='/')
		{
			txt--;
			*txt = 0;
			txt++;
		}
		if (c == '\n' || c == 13 || c == 0)
		{
			*txt = 0;
			break;
		}
		*txt = c;
		txt++;
	}
	return true;
}

const char *findword (const char *tamp, char *word, int max) //cherche un nom de variable
{
	int i;
	bool started = false;
	for (i=0;i<max;i++)
	{
		if ((*tamp >= 'a' && *tamp <= 'z') || (*tamp >= 'A' && *tamp <= 'Z'))
			started = true;
		if (started)
		{
			if (*tamp == ' ' || *tamp == '	' || *tamp == 0 || *tamp == '\n' || *tamp == 13)
			{
				*word = 0;
				return tamp;
			}
			*word = *tamp;
			word++;
		}else
		{
			if (*tamp == 0 || *tamp == '\n' || *tamp == 13)
				return NULL;
		}
		tamp++;
	}
	return NULL;
}

const char *pass (const char *txt) //passe les espaces
{
	while(*txt == ' ' || *txt == '	' || *txt == '\240')
	{
		txt++;
	}
	return txt;
}

const char *readnum (const char *txt, int *n) //lit un entier positif
{
	if (*txt < '0' || *txt > '9')
		return NULL;
	*n = 0;
	while (*txt >= '0' && *txt <= '9')
	{
		*n = *n * 10;
		*n = *n + (*txt-'0');
		txt++;
	}
	return txt;
}

const char *findstart (const char *txt, int *n, int max)
{
	int i = 0;
	int n2;
	//passe les espaces
	txt = pass(txt);
	//lit un nombre entier
	txt = readnum(txt, &n2);
	if (!txt)
		return NULL;
	//passe les espaces
	txt = pass(txt);
	//lit ':'
	if (*txt == ':')
	{
		*n = n2;
		return txt+1;
	}else
		return NULL;
}

const char *findnum (const char *word, Uint16 *n)
{
	/*
	bool started = false;
	int i = 0;
	*n = 0;
	for (;;)
	{
		if (*word >= '0' && *word <= '9')
			started = true;
		if (started)
		{
			if (*word < '0' || *word > '9')
			{
				return word;
			}
			*n = *n * 10;
			*n += *word - '0';
			i++;
			if (i >= 6)
				return NULL;
		}else
		{
			if (*word != ' ' && *word != '	')
				return NULL;
		}
		word++;
	}
	*/
	//passe les espaces
	word = pass(word);
	//lit un nombre
	int n2;
	word = readnum(word,&n2);
	*n = n2;
	return word;
}
