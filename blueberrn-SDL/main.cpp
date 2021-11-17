#ifdef __APPLE__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <imgui.h>
#include <imgui_sdl.h>
#include <zip.h>
#include <toml.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <filesystem>
#include <queue>
#include <libblueberrn/libblueberrn.h>
#include "sdl2logo.h" // Splash screen bitmap as C-array
namespace fs = std::filesystem;
using namespace berrn;
using namespace std;
using namespace std::placeholders;

#include "translations.inl"

class SDL2Frontend : public BlueberrnFrontend
{
    public:
	SDL2Frontend(BlueberrnCore *cb) : core(cb)
	{

	}
				
	~SDL2Frontend()
	{
				
	}

	void init_config_file()
	{
	    ifstream file("config.toml");

	    if (!file.is_open())
	    {
		return;
	    }

	    toml::ParseResult toml_res = toml::parse(file);
	    file.close();

	    if (!toml_res.valid())
	    {
		cout << "Config file could not be parsed! toml::errorReason: " << toml_res.errorReason << endl;
		return;
	    }

	    toml_val = toml_res.value;

	    int lang_val = toml_val.get<int>("general.language");

	    UiLanguage language;

	    if (lang_val >= int(UiLanguage::Max))
	    {
		cout << "Unknown language value, defaulting to English..." << endl;
		language = English;
	    }
	    else
	    {
		language = (UiLanguage)lang_val;
	    }

	    set_current_language(language);
	}

	void save_config_file()
	{
	    toml_val.set("general.language", int(current_language));
	    ofstream file("config.toml");
	    toml_val.write(&file);
	    file.close();
	}
				
	bool init()
	{
	    init_config_file();

	    isdriverloaded = !core->nocmdarguments();
	    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	    {
		return sdl_error("SDL could not be initialized!");
	    }
						
	    window = SDL_CreateWindow("blueberrn-SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
		
	    if (window == NULL)
	    {
		return sdl_error("Window could not be created!");
	    }
						
	    render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
				
	    if (render == NULL)
	    {
		return sdl_error("Renderer could not be created!");
	    }

	    SDL_AudioSpec audiospec;
	    audiospec.format = AUDIO_S16SYS;
	    audiospec.freq = 48000;
	    audiospec.samples = 4096;
	    audiospec.channels = 2;
	    audiospec.callback = NULL;

	    if (SDL_OpenAudio(&audiospec, NULL) < 0)
	    {
		return sdl_error("Could not open audio!");
	    }

	    // SDL_PauseAudio(!isdriverloaded);

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

	    ImGuiIO &io = ImGui::GetIO();

	    io.Fonts->AddFontFromFileTTF("NotoSans-Regular.ttf", 21.0f, NULL, io.Fonts->GetGlyphRangesDefault());

	    ImFontConfig config;
	    config.MergeMode = true;

	    io.Fonts->AddFontFromFileTTF("NotoSansJP-Regular.otf", 21.0f, &config, io.Fonts->GetGlyphRangesJapanese());
	    io.Fonts->AddFontFromFileTTF("NotoSansKR-Regular.otf", 21.0f, &config, io.Fonts->GetGlyphRangesKorean());
	    io.Fonts->Build();

	    ImGuiSDL::Initialize(render, width, height);
	}
				
	void shutdown()
	{
	    cout << "Exiting blueberrn-SDL..." << endl;
	    save_config_file();
	    ImGuiSDL::Deinitialize();
	    ImGui::DestroyContext();
	    SDL_CloseAudio();
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
	    ImGui::Begin(get_translation("Select a driver..."));
	    ImGui::SetWindowSize(get_translation("Select a driver..."), ImVec2(400, 300));
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
			// SDL_PauseAudio(0);
		    }
		}
	    }
				
	    ImGui::End();
	}

	void menubar()
	{
	    if (ImGui::BeginMainMenuBar())
	    {
		ImVec2 guisize = ImGui::GetWindowSize();

		if (menuheight != guisize.y)
		{
		    menuheight = guisize.y;
		    resizewindow();
		}

		if (ImGui::BeginMenu(get_translation("File")))
		{
		    if (ImGui::MenuItem(get_translation("Load driver...")))
		    {
			if (!isdriverloaded)
			{
			    driverselbox = !driverselbox;
			}
		    }
										
		    if (ImGui::MenuItem(get_translation("Stop driver...")))
		    {
			if (isdriverloaded)
			{
			    core->stopdriver();
			    // SDL_PauseAudio(1);
			    resize(startwidth, startheight, 1);
			    initsplash();
			    isdriverloaded = false;
			}
		    }

		    if (ImGui::MenuItem(get_translation("Quit")))
		    {
			quit = true;
		    }

		    ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(get_translation("Language...")))
		{
		    if (ImGui::MenuItem("English"))
		    {
			cout << "Setting language to English..." << endl;
			set_current_language(English);
		    }

		    if (ImGui::MenuItem(u8"日本語"))
		    {
			cout << "Setting language to Japanese..." << endl;
			set_current_language(Japanese);
		    }

		    if (ImGui::MenuItem(u8"한국인"))
		    {
			cout << "Setting language to Korean..." << endl;
			set_current_language(Korean);
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
		switch (event.type)
		{
		    case SDL_QUIT: quit = true; break;
		    case SDL_MOUSEWHEEL: wheel = event.wheel.y;
		    case SDL_WINDOWEVENT:
		    {
			switch (event.window.event)
			{
			    case SDL_WINDOWEVENT_SIZE_CHANGED:
			    {
				SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
				SDL_RenderClear(render);
				SDL_RenderPresent(render);
			    }
			    break;
			}
		    }
		    break;
		    case SDL_KEYDOWN:
		    case SDL_KEYUP:
		    {
			bool is_pressed = (event.type == SDL_KEYDOWN);

			switch (event.key.keysym.sym)
			{
			    case SDLK_c: core->keychanged(BerrnInput::BerrnCoin, is_pressed); break;
			    case SDLK_RETURN: core->keychanged(BerrnInput::BerrnStartP1, is_pressed); break;
			    case SDLK_LEFT: core->keychanged(BerrnInput::BerrnLeftP1, is_pressed); break;
			    case SDLK_RIGHT: core->keychanged(BerrnInput::BerrnRightP1, is_pressed); break;
			    case SDLK_a: core->keychanged(BerrnInput::BerrnFireP1, is_pressed); break;
			}
		    }
		    break;
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
				
	bool sdl_error(string message)
	{
	    cout << message << " SDL_Error: " << SDL_GetError() << endl;
	    return false;
	}

	void resizewindow()
	{
	    SDL_SetWindowSize(window, (width * scale), ((height + menuheight) * scale));
	}
				
	void resize(int w, int h, int s)
	{
	    width = w;
	    height = h;
	    scale = s;
	    resizewindow();
	}
				
	void rendertex(BerrnTex tex)
	{
	    SDL_Texture *frametex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
	    SDL_SetTextureBlendMode(frametex, SDL_BLENDMODE_BLEND);
	    SDL_SetRenderTarget(render, frametex);
			
	    SDL_Rect rect = {tex.x, (tex.y + menuheight), (tex.width * scale), (tex.height * scale)};
					
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

	// Audio code (using placeholder code for now)
	// TODO: Properly implement audio logic

	int loadWAV(string filename)
	{
	    cout << "Loading WAV with filename of " << filename << endl;
	    int id = samplesounds.size();

	    cout << "Sound " << dec << id << " succesfully loaded." << endl;

	    samplesounds.push_back(0);
	    return id;
	}

	bool hasSounds()
	{
	    return !samplesounds.empty();
	}

	bool playSound(int id)
	{
	    if ((id < 0) || (id >= static_cast<int>(samplesounds.size())))
	    {
		return false;
	    }

	    cout << "Playing sound of " << dec << id << endl;
	    return true;
	}

	bool setSoundVol(int id, double vol)
	{
	    if ((id < 0) || (id >= static_cast<int>(samplesounds.size())))
	    {
		return false;
	    }

	    if ((vol < 0.0) || (vol > 1.0))
	    {
		return false;
	    }

	    cout << "Setting gain of sound " << dec << (int)id << " to " << vol << endl;
	    return true;
	}

	array<int16_t, 2> getMixedSamples()
	{
	    array<int16_t, 2> samples;
	    samples[0] = 0;
	    samples[1] = 0;
	    return samples;
	}

	void freeSounds()
	{
	    cout << "Clearing " << dec << (int)samplesounds.size() << " sounds..." << endl;
	    samplesounds.clear();
	}

	void audioCallback(array<int16_t, 2> samples)
	{
	    int16_t left = samples[0];
	    int16_t right = samples[1];
	    cout << "Left sample: " << dec << (int)left << endl;
	    cout << "Right sample: " << dec << (int)right << endl;
	    cout << endl;
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
	int menuheight = 0;
		
	bool driverselbox = false;

	bool is_dir_check = false;

	struct zip_t *zip = NULL;
				
	struct SDL2Tex
	{
	    SDL_Texture *tex = NULL;
	    SDL_Rect rect;
	};
		
	queue<SDL2Tex> maintex;

	vector<int> samplesounds;

	string basePath = "";
	string pathSeparator = "";

	toml::Value toml_val;
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