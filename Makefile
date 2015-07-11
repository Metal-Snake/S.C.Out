SCOUT	: main.cpp sound.h tab.h regles.h read.h anims.h edit.h
	g++ -x objective-c++ -c main.cpp -I/usr/local/include/SDL/
	g++ -o build/SCOUT main.o -lSDL -lSDL_ttf -lm `sdl-config --cflags --libs`