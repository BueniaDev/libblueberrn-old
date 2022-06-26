/*
    This file is part of libblueberrn.
    Copyright (C) 2022 BueniaDev.

    libblueberrn is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libblueberrn is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libblueberrn.  If not, see <https://www.gnu.org/licenses/>.
*/

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
#include <cassert>
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

	    // SDL_setenv("SDL_AUDIODRIVER", "disk", true);

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
						
	    render = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE);
				
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

	SDL_Surface *load_logo(bool is_xmas_logo)
	{
	    const void *mem = is_xmas_logo ? sdl2logoxmas : sdl2logo;
	    int size = is_xmas_logo ? sdl2logoxmas_len : sdl2logo_len;
	    return loadbmp(mem, size);
	}
				
	void initsplash()
	{
	    SDL_Surface *image = load_logo(is_xmas_time);
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
	    ImGui::Begin(get_translation("Select a machine..."));
	    ImGui::SetWindowSize(get_translation("Select a machine..."), ImVec2(400, 300));
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
		    if (ImGui::MenuItem(get_translation("Load machine...")))
		    {
			if (!isdriverloaded)
			{
			    driverselbox = !driverselbox;
			}
		    }
										
		    if (ImGui::MenuItem(get_translation("Stop machine...")))
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

		    if (ImGui::MenuItem(u8"Español"))
		    {
			cout << "Setting language to Spanish..." << endl;
			set_current_language(Spanish);
		    }

		    if (ImGui::MenuItem(u8"Français"))
		    {
			cout << "Setting language to French..." << endl;
			set_current_language(French);
		    }

		    if (ImGui::MenuItem(u8"Deutsch"))
		    {
			cout << "Setting language to German..." << endl;
			set_current_language(German);
		    }

		    if (ImGui::MenuItem(u8"Italiano..."))
		    {
			cout << "Setting language to Italian..." << endl;
			set_current_language(Italian);
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
				int width = event.window.data1;
				int height = event.window.data2;
				iotemp.DisplaySize.x = float(width);
				iotemp.DisplaySize.y = float(height);
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
			// blueberrn-SDL key mappings:
			// 2 - Service mode
			// 5 - Insert credit
			// 1 - P1 Start
			// Left - P1 Left
			// Right - P1 Right
			// Up - P1 Up
			// Down - P1 Down
			// A - P1 Button 1
			// B = P1 Button 2
			// C = P1 Button 3
			// Ctrl+D = Dump (for debugging purposes)
			bool is_pressed = (event.type == SDL_KEYDOWN);

			switch (event.key.keysym.sym)
			{
			    case SDLK_2: core->keychanged(BerrnInput::BerrnService, is_pressed); break;
			    case SDLK_5: core->keychanged(BerrnInput::BerrnCoin, is_pressed); break;
			    case SDLK_1: core->keychanged(BerrnInput::BerrnStartP1, is_pressed); break;
			    case SDLK_LEFT: core->keychanged(BerrnInput::BerrnLeftP1, is_pressed); break;
			    case SDLK_RIGHT: core->keychanged(BerrnInput::BerrnRightP1, is_pressed); break;
			    case SDLK_UP: core->keychanged(BerrnInput::BerrnUpP1, is_pressed); break;
			    case SDLK_DOWN: core->keychanged(BerrnInput::BerrnDownP1, is_pressed); break;
			    case SDLK_a: core->keychanged(BerrnInput::BerrnButton1P1, is_pressed); break;
			    case SDLK_b: core->keychanged(BerrnInput::BerrnButton2P1, is_pressed); break;
			    case SDLK_c: core->keychanged(BerrnInput::BerrnButton3P1, is_pressed); break;
			    case SDLK_d:
			    {
				if (isctrlpressed(event))
				{
				    core->keychanged(BerrnInput::BerrnDump, is_pressed);
				}
			    }
			    break;
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
	    auto driver = core->getDriver();

	    int rot_flags = 0;

	    if (driver != NULL)
	    {
		rot_flags = (driver->get_flags() & berrn_rot_mask);
	    }

	    if (rot_flags & berrn_swapxy)
	    {
		width = h;
		height = w;
	    }
	    else
	    {
		width = w;
		height = h;
	    }

	    scale = s;

	    resizewindow();
	}

	void drawpixels()
	{
	    auto driver = core->getDriver();
	    SDL_RenderClear(render);

	    BerrnBitmap *bitmap = driver->get_screen();

	    if (bitmap == NULL)
	    {
		return;
	    }

	    int bmp_width = bitmap->width();
	    int bmp_height = bitmap->height();

	    SDL_Surface *bmp_surface = SDL_CreateRGBSurfaceWithFormat(0, bmp_width, bmp_height, 32, SDL_PIXELFORMAT_RGBA32);

	    if (bmp_surface == NULL)
	    {
		sdl_error("Rendering surface could not be created!");
		return;
	    }

	    SDL_Rect pixel = {0, 0, 1, 1};

	    if (bitmap->format() == BerrnRGB)
	    {
		BerrnBitmapRGB *bmp = reinterpret_cast<BerrnBitmapRGB*>(bitmap);

		for (int i = 0; i < bmp_width; i++)
		{
		    pixel.x = i;
		    for (int j = 0; j < bmp_height; j++)
		    {
			pixel.y = j;
			berrnRGBA color = bmp->pixel(i, j);
			SDL_FillRect(bmp_surface, &pixel, SDL_MapRGBA(bmp_surface->format, color.red, color.green, color.blue, color.alpha));
		    }
		}
	    }

	    SDL_Texture *bmp_texture = SDL_CreateTextureFromSurface(render, bmp_surface);
	    SDL_FreeSurface(bmp_surface);
	
	    if (bmp_texture == NULL)
	    {
		sdl_error("Rendering texture could not be created!");
		return;
	    }

	    assert(render && bmp_texture);
	    SDL_Rect dst_rect = {0, menuheight, (width * scale), (height * scale)};
	    SDL_RenderCopy(render, bmp_texture, NULL, &dst_rect);
	    SDL_DestroyTexture(bmp_texture);
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
	    struct zip_t *zip = zip_open(filename.c_str(), 0, 'r');

	    if (zip == NULL)
	    {
		return -1;
	    }

	    int id = zip_files.size();

	    zip_files.push_back(zip);

	    return id;
	}

	vector<uint8_t> readfile(string dirname, string subdirname, string filename)
	{
	    vector<uint8_t> temp;
	    cout << "Reading file from local directory" << endl;
	    cout << "Directory name: " << dirname << endl;
	    cout << "Subdirectory name: " << subdirname << endl;
	    cout << "Filename: " << filename << endl;
	    cout << endl;

	    fs::path dir_path = dirname;

	    vector<fs::path> target_paths = 
	    {
		(dir_path / subdirname / filename),
		(dir_path / filename),
	    };

	    for (auto &path : target_paths)
	    {
		if (fs::exists(path))
		{
		    return loadfile(path.generic_string());
		}
	    }

	    auto target_file = fs::path(filename);

	    for (const auto &dir_entry : fs::recursive_directory_iterator(dir_path))
	    {
		auto path = dir_entry.path();
		if (path.filename() == target_file.filename())
		{
		    return loadfile(path.generic_string());
		}
	    }

	    for (const auto &dir_entry : fs::directory_iterator(dir_path))
	    {
		auto path = dir_entry.path();
		if (path.extension() == target_file.extension())
		{
		    return loadfile(path.generic_string());
		}
	    }

	    return temp;
	}

	vector<uint8_t> loadfile(string filename)
	{
	    cout << "Loading file of " << filename << endl;
	    vector<uint8_t> temp;
	    ifstream file(filename, ios::in | ios::binary | ios::ate);

	    if (file.is_open())
	    {
		streampos size = file.tellg();
		temp.resize(size, 0);
		file.seekg(0, ios::beg);
		file.read((char*)temp.data(), temp.size());
		file.close();
	    }

	    return temp;
	}

	vector<uint8_t> readfilefromzip(int id, string dirname, string filename)
	{
	    vector<uint8_t> temp;

	    if (id == -1)
	    {
		return temp;
	    }

	    auto zip_file = zip_files.at(id);

	    vector<fs::path> target_paths = 
	    {
		(fs::path(dirname) / filename),
		fs::path(filename),
	    };

	    for (auto &path : target_paths)
	    {
		if (is_zip_exists(zip_file, path))
		{
		    return loadfilefromzip(zip_file, path.generic_string());
		}
	    }

	    auto target_file = fs::path(filename);
	    fs::path file_path;

	    if (find_file_in_zip(zip_file, target_file, file_path))
	    {
		return loadfilefromzip(zip_file, file_path.generic_string());
	    }

	    return temp;
	}

	bool is_zip_exists(struct zip_t *zip, fs::path path)
	{
	    if (zip == NULL)
	    {
		return false;
	    }

	    int num_entries = zip_entries_total(zip);

	    for (int i = 0; i < num_entries; i++)
	    {
		zip_entry_openbyindex(zip, i);
		fs::path file_name = fs::path(zip_entry_name(zip));
		zip_entry_close(zip);

		if (file_name == path)
		{
		    return true;
		}
	    }

	    return false;
	}

	bool find_file_in_zip(struct zip_t *zip, fs::path target_path, fs::path &file_path)
	{
	    if (zip == NULL)
	    {
		return false;
	    }

	    int num_entries = zip_entries_total(zip);

	    for (int i = 0; i < num_entries; i++)
	    {
		zip_entry_openbyindex(zip, i);
		fs::path file_name = fs::path(zip_entry_name(zip));
		zip_entry_close(zip);

		if (file_name.filename() == target_path.filename())
		{
		    file_path = file_name;
		    return true;
		}
	    }

	    for (int i = 0; i < num_entries; i++)
	    {
		zip_entry_openbyindex(zip, i);
		fs::path file_name = fs::path(zip_entry_name(zip));
		zip_entry_close(zip);

		if (!file_name.has_parent_path())
		{
		    if (file_name.extension() == target_path.extension())
		    {
			file_path = file_name;
			return true;
		    }
		}
	    }

	    return false;
	}

	vector<uint8_t> loadfilefromzip(struct zip_t *zip, string filename)
	{
	    vector<uint8_t> data;

	    if (zip == NULL)
	    {
		return data;
	    }

	    zip_entry_open(zip, filename.c_str());
	    size_t buf_size = zip_entry_size(zip);
	    data.resize(buf_size, 0);
	    zip_entry_noallocread(zip, (void*)data.data(), data.size());
	    zip_entry_close(zip);
	    return data;
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

	// Fetch external sample rate
	uint32_t getSampleRate()
	{
	    return 48000;
	}

	// External audio callback
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

	bool isctrlpressed(SDL_Event event)
	{
	    return (event.key.keysym.mod & KMOD_CTRL) ? true : false;
	}
		
	SDL_Window *window = NULL;
	SDL_Renderer *render = NULL;
	SDL_Surface *surface = NULL;
	SDL_Surface *optsurface = NULL;
	SDL_Texture *texture = NULL;
				
	BlueberrnCore *core = NULL;

	ofstream audio_file;

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

	vector<struct zip_t*> zip_files;

	int fpscount = 0;
	Uint32 fpstime = 0;

	Uint32 framecurrenttime = 0;
	Uint32 framestarttime = 0;

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
	cout << "Error: could not load driver" << endl;
	return 1;
    }
		
    core.runapp();
    core.stopdriver(front->isdriverloaded);
    core.shutdown();
    return 0;
}