
#define NBMAX	100 //nombre de balles maximum

struct Ball
{
	Uint16 x,y; //position
	//SDL_Surface s;
	bool used; //la balle est-elle active ?
	
	void Do ();
};