#ifdef __APPLE__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <imgui.h>
#include <imgui_sdl.h>
#include <zip.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <queue>
#include <libblueberrn/libblueberrn.h>
#include "sdl2logo.h" // Splash screen bitmap as C-array
namespace fs = std::filesystem;
using namespace berrn;
using namespace std;

class SDL2Frontend : public BlueberrnFrontend
{
    public:
	SDL2Frontend(BlueberrnCore *cb) : core(cb)
	{

	}
				
	~SDL2Frontend()
	{
				
	}
				
	bool init()
	{
	    isdriverloaded = !core->nocmdarguments();
	    if (SDL_Init(SDL_INIT_VIDEO) < 0)
	    {
		sdl_error("SDL could not be initialized!");
		return false;
	    }
						
	    window = SDL_CreateWindow("blueberrn-SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
		
	    if (window == NULL)
	    {
		sdl_error("Window could not be created!");
		return false;
	    }
						
	    render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
				
	    if (render == NULL)
	    {
		sdl_error("Renderer could not be created!");
		return false;
	    }

	    surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);				
	    cout << "Welcome to blueberrn-SDL." << endl;

	    initbasepath();
			
	    initui();						
	    return true;
	}

	SDL_Surface *loadbmp(const void *mem, int size)
	{
	    return SDL_LoadBMP_RW(SDL_RWFromConstMem(mem, size), 0);
	}
				
	void initsplash()
	{
	    SDL_Surface *image = loadbmp(sdl2logo, sdl2logo_len);		
	    optsurface = SDL_ConvertSurface(image, surface->format, 0);
	    SDL_FreeSurface(image);
						
	    SDL_Rect rect = {0, 0, width, height};
	    SDL_BlitScaled(optsurface, NULL, surface, &rect);
	    texture = SDL_CreateTextureFromSurface(render, surface);
	}
				
	void initui()
	{
	    initsplash();
	    ImGui::CreateContext();
	    ImGuiSDL::Initialize(render, width, height);
	}
				
	void shutdown()
	{
	    ImGuiSDL::Deinitialize();
	    ImGui::DestroyContext();
	    SDL_DestroyTexture(texture);
	    SDL_DestroyRenderer(render);
	    SDL_DestroyWindow(window);
	    SDL_Quit();
	}
				
	void runapp()
	{
	    while (!quit)
	    {
		ImGuiIO &io = ImGui::GetIO();
		pollevents(io);
		runmachine();
	    }
	}
				
	void driverselectionbox()
	{
	    ImGui::Begin("Select driver...");
	    ImGui::SetWindowSize("Select driver...", ImVec2(400, 300));
	    vector<string> names = core->getdrvnames();
						
	    for (string name : names)
	    {
		if (ImGui::Selectable(name.c_str()))
		{
		    if (!isdriverloaded)
		    {
			core->initdriver(name);
			core->startdriver();
			driverselbox = false;
			isdriverloaded = true;
		    }
		}
	    }
				
	    ImGui::End();
	}

	void menubar()
	{
	    if (ImGui::BeginMainMenuBar())
	    {
		if (ImGui::BeginMenu("File"))
		{
		    if (ImGui::MenuItem("Load driver..."))
		    {
			if (!isdriverloaded)
			{
			    driverselbox = !driverselbox;
			}
		    }
										
		    if (ImGui::MenuItem("Stop driver..."))
		    {
			if (isdriverloaded)
			{
			    core->stopdriver();
			    resize(startwidth, startheight, 1);
			    initsplash();
			    isdriverloaded = false;
			}
		    }

		    ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	    }
	}
				
	void guistuff()
	{
	    menubar();
	}
		
	void pollevents(ImGuiIO& iotemp)
	{
	    while (SDL_PollEvent(&event))
	    {
		if (event.type == SDL_QUIT)
		{
		    quit = true;
		}
		else if (event.type == SDL_MOUSEWHEEL)
		{
		    wheel = event.wheel.y;
		}
		else if (event.type == SDL_WINDOWEVENT)
		{
		    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
		    {
			SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
			SDL_RenderClear(render);
			SDL_RenderPresent(render);
		    }
		}
		else if (event.type == SDL_KEYDOWN)
		{
		    switch (event.key.keysym.sym)
		    {
			case SDLK_c: core->keypressed(BerrnInput::BerrnCoin); break;
			case SDLK_RETURN: core->keypressed(BerrnInput::BerrnStartP1); break;
			case SDLK_LEFT: core->keypressed(BerrnInput::BerrnLeftP1); break;
			case SDLK_RIGHT: core->keypressed(BerrnInput::BerrnRightP1); break;
			case SDLK_a: core->keypressed(BerrnInput::BerrnFireP1); break;
		    }
		}
		else if (event.type == SDL_KEYUP)
		{
		    switch (event.key.keysym.sym)
		    {
			case SDLK_c: core->keyreleased(BerrnInput::BerrnCoin); break;
			case SDLK_RETURN: core->keyreleased(BerrnInput::BerrnStartP1); break;
			case SDLK_LEFT: core->keyreleased(BerrnInput::BerrnLeftP1); break;
			case SDLK_RIGHT: core->keyreleased(BerrnInput::BerrnRightP1); break;
			case SDLK_a: core->keyreleased(BerrnInput::BerrnFireP1); break;
		    }
		}
	    }
						
	    int mousex = 0;
	    int mousey = 0;
	    const int buttons = SDL_GetMouseState(&mousex, &mousey);
						
	    iotemp.DeltaTime = (1.0f / 60.f);
	    iotemp.MousePos = ImVec2(static_cast<float>(mousex), static_cast<float>(mousey));
	    iotemp.MouseDown[0] = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT));
	    iotemp.MouseDown[1] = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT));
	    iotemp.MouseWheel = static_cast<float>(wheel);
	}
				
	void rundriver()
	{
	    if (isdriverloaded)
	    {
		core->rundriver();
	    }
	}
				
	void runmachine()
	{
	    ImGui::NewFrame();
	    guistuff();
						
	    rundriver();
					
	    if (driverselbox)
	    {
		driverselectionbox();
	    }
				
	    if (!isdriverloaded)
	    {
		SDL_RenderCopy(render, texture, NULL, NULL);
	    }
		
	    renderui();
	}
				
	void renderui()
	{
	    ImGui::Render();
	    ImGuiSDL::Render(ImGui::GetDrawData());
	    SDL_RenderPresent(render);
	}
				
	void sdl_error(string message)
	{
	    cout << message << " SDL_Error: " << SDL_GetError() << endl;
	}
				
	void resize(int w, int h, int s)
	{
	    width = w;
	    height = h;
	    scale = s;
	    SDL_SetWindowSize(window, (width * scale), (height * scale));
	}
				
	void rendertex(BerrnTex tex)
	{
	    SDL_Texture *frametex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
	    SDL_SetTextureBlendMode(frametex, SDL_BLENDMODE_BLEND);
	    SDL_SetRenderTarget(render, frametex);
			
	    SDL_Rect rect = {tex.x, tex.y, (tex.width * scale), (tex.height * scale)};
					
	    assert(render && frametex);
	    SDL_UpdateTexture(frametex, NULL, tex.buffer.data(), (width * sizeof(berrnRGBA)));
				
	    maintex.push({frametex, rect});
	}
				
	void drawpixels()
	{
	    SDL_RenderClear(render);

	    while (!maintex.empty())
	    {
		SDL2Tex tex = maintex.front();
		SDL_SetRenderTarget(render, NULL);
		SDL_RenderCopy(render, tex.tex, NULL, &tex.rect);
		maintex.pop();
		SDL_DestroyTexture(tex.tex);
	    }
	}

	void initbasepath()
	{
	    basePath = SDL_GetBasePath();
	    pathSeparator = basePath.substr(basePath.length() - 1);
	}

	string getdirpath(const string subdir, const string romPath)
	{
	    string dirprefix = (romPath.empty()) ? basePath : romPath;
	    stringstream ss;
	    ss << dirprefix;
	    if (!subdir.empty())
	    {
		ss << subdir << pathSeparator;
	    }

	    return ss.str();
	}

	bool isdirectory(string dirname)
	{
	    fs::path path = dirname;
	    return fs::is_directory(path);
	}

	bool loadzip(string filename)
	{
	    zip = zip_open(filename.c_str(), 0, 'r');

	    if (zip == NULL)
	    {
		cout << "Error: could not load ZIP file" << endl;
		return false;
	    }

	    return true;
	}

	vector<uint8_t> readfile(string filename)
	{
	    vector<uint8_t> temp;
	    ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

	    if (!file.is_open())
	    {
		cout << "Unable to load file." << endl;
		return temp;
	    }

	    streampos size = file.tellg();
	    temp.resize(size, 0);
	    file.seekg(0, ios::beg);
	    file.read((char*)temp.data(), size);
	    file.close();
	    return temp;
	}

	vector<uint8_t> readfilefromzip(string filename)
	{
	    vector<uint8_t> temp;

	    if (zip == NULL)
	    {
		cout << "Error: ZIP entry is NULL" << endl;
		return temp;
	    }

	    int zipentry = zip_entry_open(zip, filename.c_str());

	    if (zipentry < 0)
	    {
		cout << "Error: could not open entry in ZIP file" << endl;
		zip_entry_close(zip);
		return temp;
	    }

	    size_t bufsize = zip_entry_size(zip);

	    temp.resize(bufsize, 0);

	    zip_entry_noallocread(zip, temp.data(), temp.size());

	    zip_entry_close(zip);
	    return temp;
	}

	void closezip()
	{
	    if (zip != NULL)
	    {
		zip_close(zip);
		zip = NULL;
	    }
	}
		
	SDL_Window *window = NULL;
	SDL_Renderer *render = NULL;
	SDL_Surface *surface = NULL;
	SDL_Surface *optsurface = NULL;
	SDL_Texture *texture = NULL;
				
	BlueberrnCore *core = NULL;

	bool isdriverloaded = false;
		
	bool quit = false;
	SDL_Event event;
				
	int startwidth = 800;
	int startheight = 600;
	int startscale = 1;
				
	int width = startwidth;
	int height = startheight;
	int scale = startscale;
				
	int wheel = 0;
		
	bool driverselbox = false;

	bool is_dir_check = false;

	struct zip_t *zip = NULL;
				
	struct SDL2Tex
	{
	    SDL_Texture *tex = NULL;
	    SDL_Rect rect;
	};
		
	queue<SDL2Tex> maintex;

	string basePath = "";
	string pathSeparator = "";
};

BlueberrnCore core;

int main(int argc, char* argv[])
{
    SDL2Frontend *front = new SDL2Frontend(&core);
    core.setfrontend(front);

    if (!core.getoptions(argc, argv))
    {
	return 1;
    }
		
    if (!core.init())
    {
	return 1;
    }
		
    if (!core.startdriver(front->isdriverloaded))
    {
	return 1;
    }
		
    core.runapp();
    core.stopdriver(front->isdriverloaded);
    core.shutdown();
    return 0;
}