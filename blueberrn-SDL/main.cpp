#ifdef __APPLE__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <imgui.h>
#include <imgui_sdl.h>
#include <cmixer.h>
#include <zip.h>
#include <toml.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <ctime>
#include <filesystem>
#include <queue>
#include <libblueberrn/libblueberrn.h>
#include "sdl2logo.h" // Splash screen bitmap as C-array
#include "sdl2logoxmas.h" // Splash screen bitmap as C-array (holiday version)
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
	    is_xmas_time = is_xmas();
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
						
	    render = SDL_CreateRenderer(window, -1, 0);
				
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

	    cm_init(audiospec.freq);
	    cm_set_master_gain(0.5);

	    SDL_PauseAudio(!isdriverloaded);

	    surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);

	    if (is_xmas_time)
	    {
		cout << "Happy holidays from blueberrn-SDL!" << endl;
	    }
	    else
	    {
		cout << "Welcome to blueberrn-SDL." << endl;
	    }

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
	    SDL_Surface *image = NULL;

	    if (is_xmas_time)
	    {
		image = loadbmp(sdl2logoxmas, sdl2logoxmas_len);
	    }
	    else
	    {
		image = loadbmp(sdl2logo, sdl2logo_len);
	    }	
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

	    stringstream base_font_en;
	    base_font_en << basePath << pathSeparator << "fonts" << pathSeparator << "NotoSans-Regular.ttf";

	    stringstream base_font_jp;
	    base_font_jp << basePath << pathSeparator << "fonts" << pathSeparator << "NotoSansJP-Regular.otf";

	    stringstream base_font_kr;
	    base_font_kr << basePath << pathSeparator << "fonts" << pathSeparator << "NotoSansKR-Regular.otf";

	    io.Fonts->AddFontFromFileTTF(base_font_en.str().c_str(), 21.0f, NULL, io.Fonts->GetGlyphRangesDefault());

	    ImFontConfig config;
	    config.MergeMode = true;

	    io.Fonts->AddFontFromFileTTF(base_font_jp.str().c_str(), 21.0f, &config, io.Fonts->GetGlyphRangesJapanese());
	    io.Fonts->AddFontFromFileTTF(base_font_kr.str().c_str(), 21.0f, &config, io.Fonts->GetGlyphRangesKorean());
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
		limitframerate();
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
			driverselbox = false;
			core->initdriver(name);
			if (core->startdriver())
			{
			    isdriverloaded = true;
			    SDL_PauseAudio(0);
			}
			else
			{
			    cout << "Error: could not start driver " << name << endl;
			    SDL_PauseAudio(1);
			    resize(startwidth, startheight, 1);
			    initsplash();
			    isdriverloaded = false;
			}
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
			    SDL_ClearQueuedAudio(1);
			    SDL_PauseAudio(1);
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
			// blueberrn-SDL key mappings (similar to MAME):
			// 5 - Insert credit
			// 1 - P1 Start
			// Left - P1 Left
			// Right - P1 Right
			// Up - P1 Up
			// Down - P1 Down
			// Left Ctrl - P1 Fire
			bool is_pressed = (event.type == SDL_KEYDOWN);

			switch (event.key.keysym.sym)
			{
			    case SDLK_5: core->keychanged(BerrnInput::BerrnCoin, is_pressed); break;
			    case SDLK_1: core->keychanged(BerrnInput::BerrnStartP1, is_pressed); break;
			    case SDLK_LEFT: core->keychanged(BerrnInput::BerrnLeftP1, is_pressed); break;
			    case SDLK_RIGHT: core->keychanged(BerrnInput::BerrnRightP1, is_pressed); break;
			    case SDLK_UP: core->keychanged(BerrnInput::BerrnUpP1, is_pressed); break;
			    case SDLK_DOWN: core->keychanged(BerrnInput::BerrnDownP1, is_pressed); break;
			    case SDLK_LCTRL: core->keychanged(BerrnInput::BerrnFireP1, is_pressed); break;
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

	void limitframerate()
	{
	    framecurrenttime = SDL_GetTicks();

	    auto driver = core->getDriver();

	    float framerate = 60;

	    if (driver != NULL)
	    {
		framerate = driver->get_framerate();
	    }

	    int64_t frame_period = (1000 / framerate);

	    if ((framecurrenttime - framestarttime) < frame_period)
	    {
		SDL_Delay(16 - (framecurrenttime - framestarttime));
	    }

	    framestarttime = SDL_GetTicks();

	    fpscount += 1;

	    if (((SDL_GetTicks() - fpstime) >= 1000))
	    {
		fpstime = SDL_GetTicks();
		stringstream title;
		title << "blueberrn-SDL-" << fpscount << " FPS";
		SDL_SetWindowTitle(window, title.str().c_str());
		fpscount = 0;
	    }
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
	    SDL_SetWindowSize(window, (width * scale), ((height * scale) + menuheight));
	}
				
	void resize(int w, int h, int s)
	{
	    width = w;
	    height = h;
	    scale = s;
	    resizewindow();
	}

	void drawpixels()
	{
	    SDL_RenderClear(render);
	    BerrnBitmap *bitmap = core->getDriver()->getScreen();

	    if (bitmap == NULL)
	    {
		return;
	    }

	    int berrn_w = bitmap->width();
	    int berrn_h = bitmap->height();

	    SDL_Texture *bitmapTex = NULL;

	    if (bitmap->format() == BerrnRGB)
	    {
		BerrnBitmapRGB *rgbBitmap = reinterpret_cast<BerrnBitmapRGB*>(bitmap);

		bitmapTex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, berrn_w, berrn_h);

		assert(bitmapTex);
		SDL_UpdateTexture(bitmapTex, NULL, rgbBitmap->data().data(), rgbBitmap->pitch());
	    }

	    SDL_Rect dst_rect = {0, menuheight, (width * scale), (height * scale)};

	    assert(render && bitmapTex);
	    SDL_RenderCopy(render, bitmapTex, NULL, &dst_rect);
	    SDL_DestroyTexture(bitmapTex);
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

	int loadzip(string filename)
	{
	    cout << "Loading ZIP file of " << filename << endl;
	    struct zip_t *zip = zip_open(filename.c_str(), 0, 'r');

	    if (zip == NULL)
	    {
		cout << "Error: could not load ZIP file" << endl;
		return -1;
	    }

	    int id = zip_files.size();

	    zip_files.push_back(zip);

	    return id;
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

	vector<uint8_t> readfilefromzip(int id, string filename)
	{
	    vector<uint8_t> temp;

	    if (id == -1)
	    {
		return temp;
	    }

	    struct zip_t *zip = zip_files.at(id);

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
	    for (auto &zip : zip_files)
	    {
		if (zip != NULL)
		{
		    zip_close(zip);
		    zip = NULL;
		}
	    }

	    zip_files.clear();
	}

	// Audio logic (samples logic powered by cmixer)
	// TODO: Make overall audio API more robust
	uint32_t getSampleRate()
	{
	    return 48000;
	}

	int loadWAV(string filename)
	{
	    int id = samplesounds.size();

	    cm_Source *src = cm_new_source_from_file(filename.c_str());

	    if (src == NULL)
	    {
		cout << "Could not load " << filename << "! cm_error: " << cm_get_error() << endl;
		return -1;
	    }

	    samplesounds.push_back(src);
	    return id;
	}

	bool hasSounds()
	{
	    return !samplesounds.empty();
	}

	bool setSoundLoop(int id, bool is_loop)
	{
	    if ((id < 0) || (id >= int(samplesounds.size())))
	    {
		return false;
	    }

	    int loop_val = (is_loop) ? 1 : 0;
	    cm_set_loop(samplesounds[id], loop_val);
	    return true;
	}

	bool playSound(int id)
	{
	    if ((id < 0) || (id >= int(samplesounds.size())))
	    {
		return false;
	    }

	    cm_play(samplesounds[id]);
	    return true;
	}

	bool stopSound(int id)
	{
	    if ((id < 0) || (id >= int(samplesounds.size())))
	    {
		return false;
	    }

	    cm_stop(samplesounds[id]);
	    return true;
	}

	bool setSoundVol(int id, double vol)
	{
	    if ((id < 0) || (id >= int(samplesounds.size())))
	    {
		return false;
	    }

	    if ((vol < 0.0) || (vol > 1.0))
	    {
		return false;
	    }

	    cm_set_gain(samplesounds[id], vol);
	    return true;
	}

	array<int16_t, 2> getMixedSamples()
	{
	    array<int16_t, 2> samples;
	    cm_Int16 dst[2];
	    cm_process(dst, 2);
	    samples[0] = dst[0];
	    samples[1] = dst[1];
	    return samples;
	}

	void freeSounds()
	{
	    for (int id = 0; id < samplesounds.size(); id++)
	    {
		if (samplesounds[id] != NULL)
		{
		    cm_destroy_source(samplesounds[id]);
		    samplesounds[id] = NULL;
		}
	    }

	    samplesounds.clear();
	}

	void audioCallback(array<int16_t, 2> samples)
	{
	    int16_t left = samples[0];
	    int16_t right = samples[1];

	    audiobuffer.push_back(left);
	    audiobuffer.push_back(right);

	    if (audiobuffer.size() >= 4096)
	    {
		audiobuffer.clear();

		while (SDL_GetQueuedAudioSize(1) > (4096 * sizeof(int16_t)))
		{
		    SDL_Delay(1);
		}

		SDL_QueueAudio(1, audiobuffer.data(), (4096 * sizeof(int16_t)));
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
	int menuheight = 0;
		
	bool driverselbox = false;

	bool is_dir_check = false;

	struct SDL2Tex
	{
	    SDL_Texture *tex = NULL;
	    SDL_Rect rect;
	};

	queue<SDL2Tex> maintex;

	struct zip_t *zip = NULL;

	vector<struct zip_t*> zip_files;

	int fpscount = 0;
	Uint32 fpstime = 0;

	Uint32 framecurrenttime = 0;
	Uint32 framestarttime = 0;

	vector<cm_Source*> samplesounds;

	vector<int16_t> audiobuffer;

	string basePath = "";
	string pathSeparator = "";

	toml::Value toml_val;

	bool is_xmas_time = false;

	bool is_xmas()
	{
	    time_t t = time(NULL);
	    tm* timeptr = localtime(&t);

	    return (timeptr->tm_mon == 11);
	}
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