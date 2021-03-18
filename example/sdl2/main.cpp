#include <SDL2/SDL.h>
#include "../../libblueberrn/include/libblueberrn/libblueberrn.h"
#include "ini.h"
#include <iostream>
#include <functional>
#include <string>
#include <fstream>
using namespace berrn;
using namespace mINI;
using namespace std;
using namespace std::placeholders;

SDL_Window *window;
SDL_Surface *surface;
SDL_Surface *screensurface;
SDL_Surface *optsurface;
invaders test2;
berrninitprog program;

int fpscount = 0;
Uint32 fpstime = 0;

INIFile file("blueberrn.ini");

INIStructure ini;

bool createini(string filename)
{
    fstream temp("blueberrn.ini");

    if (temp.good())
    {
	return false;
    }

    file.read(ini);

    ini["Resources"]["Logo"] = filename;

    file.write(ini);

    return true;
}

string getlogofilename()
{
    file.read(ini);

    string value = ini["Resources"]["Logo"];

    return value;
}

bool init()
{
    createini("sdl2logo.bmp");

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
	cout << "SDL could not be initialized! SDL_Error: " << SDL_GetError() << endl;
	return false;
    }

    window = SDL_CreateWindow("blueberrn-SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }

    screensurface = SDL_GetWindowSurface(window);

    if (screensurface == NULL)
    {
        cout << "Surface could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }

    surface = SDL_LoadBMP(getlogofilename().c_str());

    if (surface == NULL)
    {
        cout << "Surface could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }

    optsurface = SDL_ConvertSurface(surface, screensurface->format, 0);

    SDL_FreeSurface(surface);

    SDL_Rect stretchrect = {0, 0, 640, 480};
    SDL_BlitScaled(optsurface, NULL, screensurface, &stretchrect);
    SDL_UpdateWindowSurface(window);


    cout << "Welcome to blueberrn-SDL." << endl;

    if (program.driverloadedcmd)
    {
        SDL_Delay(5000);

        cout << "Initializing driver..." << endl;

	SDL_FillRect(screensurface, NULL, SDL_MapRGB(screensurface->format, 0, 0, 0));
	SDL_UpdateWindowSurface(window);
    }

    return true;
}

void initfront(int width, int height)
{
    SDL_SetWindowSize(window, width, height);
}

void deinitdriver()
{
    cout << "Deinitializing driver..." << endl;
}

void deinitfront()
{
    cout << "Shutting down..." << endl;
}

void drawsdlpixels(RGB* buffer, int width, int height, int scale)
{
    SDL_Rect pixel = {0, 0, scale, scale};

    for (int i = 0; i < width; i++)
    {
	pixel.x = (i * scale);
	for (int j = 0; j < height; j++)
	{
	    pixel.y = (j * scale);
	    uint8_t red = buffer[i + (j * width)].red;
	    uint8_t green = buffer[i + (j * width)].green;
	    uint8_t blue = buffer[i + (j * width)].blue;

	    if (SDL_FillRect(screensurface, &pixel, SDL_MapRGBA(screensurface->format, red, green, blue, 0xFF)))
	    {
		cout << "Could not render: SDL_Error: " << SDL_GetError() << endl;
	    }
	}
    }

    if (SDL_UpdateWindowSurface(window) < 0)
    {
	cout << "Could not render: SDL_Error: " << SDL_GetError() << endl;
    }
}

void deinit()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void handleinput(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN)
    {
	switch (event.key.keysym.sym)
	{
	    case SDLK_c: program.coin(true); break;
	    case SDLK_LEFT: program.keyleft(true); break;
	    case SDLK_RIGHT: program.keyright(true); break;
	    case SDLK_RETURN: program.keystart(true); break;
	    case SDLK_a: program.keyaction(true); break;
	    default: break;
	}
    }
    else if (event.type == SDL_KEYUP)
    {
	switch (event.key.keysym.sym)
	{
	    case SDLK_c: program.coin(false); break;
	    case SDLK_LEFT: program.keyleft(false); break;
	    case SDLK_RIGHT: program.keyright(false); break;
	    case SDLK_RETURN: program.keystart(false); break;
	    case SDLK_a: program.keyaction(false); break;
	    default: break;
	}
    }
}


void initcb()
{
    auto initf = bind(initfront, _1, _2);
    auto deinitf = bind(deinitdriver);
    auto drawf = bind(drawsdlpixels, _1, _2, _3, _4);
    program.setcallbacks(initf, deinitf, drawf);
}

int main(int argc, char* argv[])
{

    if (!program.getoptions(argc, argv))
    {
	return 1;
    }

    if (!init())
    {
	return 1;
    }


    SDL_Event event;
    bool quit = false;

    Uint32 framecurrenttime = 0;
    Uint32 framestarttime = 0;

    if (program.driverloadedcmd)
    {
	initcb();
	program.init();
    }

    while (!quit)
    {
	while (SDL_PollEvent(&event))
	{
	    handleinput(event);

	    if (event.type == SDL_QUIT)
	    {
		quit = true;
	    }
	    else if (event.type == SDL_WINDOWEVENT)
	    {
		if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
		{
		    screensurface = SDL_GetWindowSurface(window);
		    SDL_FillRect(screensurface, NULL, SDL_MapRGB(screensurface->format, 0, 0, 0));
		    SDL_UpdateWindowSurface(window);
		}
	    }
	}


	if (program.driverloadedcmd)
	{
	    program.run();
	}

	framecurrenttime = SDL_GetTicks();

	if ((framecurrenttime - framestarttime) < 16)
	{
	    SDL_Delay(16 - (framecurrenttime - framestarttime));
	}

	framestarttime = SDL_GetTicks();

	fpscount++;

	if ((SDL_GetTicks() - fpstime) >= 1000)
	{
	    fpstime = SDL_GetTicks();
	    fpscount = 0;
	}
    }

    if (program.driverloadedcmd)
    {
	program.deinit();
    }

    
    deinitfront();
    deinit();

    return 0;
}
