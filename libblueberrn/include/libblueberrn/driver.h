#ifndef BERRN_DRIVER
#define BERRN_DRIVER

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <libblueberrn_api.h>
using namespace std;

namespace berrn
{
    struct berrnRGBA
    {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
    };
	
    inline berrnRGBA red()
    {
	return {255, 0, 0, 255};
    }
	
    inline berrnRGBA green()
    {
	return {0, 255, 0, 255};
    }
	
    inline berrnRGBA blue()
    {
	return {0, 0, 255, 255};
    }

    inline berrnRGBA cyan()
    {
	return {0, 255, 255, 255};
    }

    inline berrnRGBA magenta()
    {
	return {255, 0, 255, 255};
    }

    inline berrnRGBA yellow()
    {
	return {255, 255, 0, 255};
    }

    inline berrnRGBA black()
    {
	return {0, 0, 0, 255};
    }

    inline berrnRGBA white()
    {
	return {255, 255, 255, 255};
    }

    inline berrnRGBA fromColor(uint8_t color)
    {
	return {color, color, color, 255};
    }

    inline berrnRGBA fromRGB(uint8_t red, uint8_t green, uint8_t blue)
    {
	return {red, green, blue, 255};
    }

    struct BerrnTex
    {
	int x;
	int y;
	int width;
	int height;
	vector<berrnRGBA> buffer;
    };
	
    class LIBBLUEBERRN_API bernnvideo
    {
	public:
	    bernnvideo()
	    {
			
	    }
			
	    ~bernnvideo()
	    {
			
	    }
			
	    virtual void rendertex(BerrnTex tex) = 0;
	    virtual void drawpixels() = 0;
    };
	
    class LIBBLUEBERRN_API BlueberrnFrontend
    {
	public:
	    BlueberrnFrontend()
	    {
			
	    }
			
	    ~BlueberrnFrontend()
	    {
			
	    }
			
	    virtual bool init() = 0;
	    virtual void shutdown() = 0;
	    virtual void runapp() = 0;
	    virtual void resize(int width, int height, int scale) = 0;
	    virtual void rendertex(BerrnTex tex) = 0;
	    virtual void drawpixels() = 0;
	    virtual string getdirpath(const string subdir = "", const string romPath = "") = 0;
	    virtual bool isdirectory(string dirname) = 0;
	    virtual bool loadzip(string filename) = 0;
	    virtual vector<uint8_t> readfile(string filename) = 0;
	    virtual vector<uint8_t> readfilefromzip(string filename) = 0;
	    virtual void closezip() = 0;
	    virtual int loadWAV(string filename) = 0;
	    virtual bool hasSounds() = 0;
	    virtual bool playSound(int id) = 0;
	    virtual bool setSoundVol(int id, double vol) = 0;
	    virtual array<int16_t, 2> getMixedSamples() = 0;
	    virtual void freeSounds() = 0;
	    virtual void audioCallback(array<int16_t, 2> samples) = 0;
    };
	
    class LIBBLUEBERRN_API BlueberrnVideo : public bernnvideo
    {
	public:
	    BlueberrnVideo()
	    {
			
	    }
	
	    BlueberrnVideo(BlueberrnFrontend *cb) : front(cb)
	    {

	    }
			
	    ~BlueberrnVideo()
	    {
			
	    }
			
	    void resize(int width, int height, int scale)
	    {
		if (front != NULL)
		{
		    front->resize(width, height, scale);
		    screenwidth = width;
		    screenheight = height;
		    screenscale = scale;
		}
	    }

	    void fillrect(int x, int y, int width, int height, berrnRGBA color)
	    {
		vector<berrnRGBA> tempbuffer;
		tempbuffer.resize((screenwidth * screenheight), color);
		filltexrect(x, y, width, height, tempbuffer);
	    }

	    void filltexrect(int x, int y, int width, int height, vector<berrnRGBA> colorbuffer)
	    {
		BerrnTex texture = {x, y, width, height, colorbuffer};
		rendertex(texture);
	    }

	    void drawpixels()
	    {
		if (front != NULL)
		{
		    front->drawpixels();
		}
	    }

	private:	
	    void rendertex(BerrnTex tex)
	    {
		if (front != NULL)
		{
		    front->rendertex(tex);
		}
	    }
			
	    BlueberrnFrontend *front = NULL;
	    int screenwidth = 0;
	    int screenheight = 0;
	    int screenscale = 0;
    };

    enum BerrnInput
    {
	BerrnCoin,
	BerrnStartP1,
	BerrnLeftP1,
	BerrnRightP1,
	BerrnFireP1,
    };

    class LIBBLUEBERRN_API berrndriver
    {
	public:
	    berrndriver()
	    {

	    }

	    ~berrndriver()
	    {

	    }

	    bool startdriver()
	    {
		cout << "Driver started" << endl;
		return drvinit();
	    }

	    void stopdriver()
	    {
		drvshutdown();
		closedriver();
		cout << "Driver stopped" << endl;
	    }

	    void rundriver()
	    {
		drvrun();
	    }
      
	    void setfrontend(BlueberrnFrontend *cb)
	    {
		front = cb;
		video = new BlueberrnVideo(front);
	    }
      
	    void resize(int width, int height, int scale = 1)
	    {
		if (video != NULL)
		{
		    video->resize(width, height, scale);
		}
	    }

	    void fillrect(int x, int y, int width, int height, berrnRGBA color)
	    {
		if (video != NULL)
		{
		    video->fillrect(x, y, width, height, color);
		}
	    }

	    void filltexrect(int x, int y, int width, int height, vector<berrnRGBA> colorbuffer)
	    {
		if (video != NULL)
		{
		    video->filltexrect(x, y, width, height, colorbuffer);
		}
	    }

	    int loadWAV(string filename)
	    {
		return loadSoundWAV(filename);
	    }

	    bool setSoundVol(int id, double vol)
	    {
		if (front == NULL)
		{
		    return false;
		}

		return front->setSoundVol(id, vol);
	    }

	    bool playSound(int id)
	    {
		if (front == NULL)
		{
		    return false;
		}

		return front->playSound(id);
	    }

	    void outputSamples()
	    {
		if (front == NULL)
		{
		    return;
		}

		if (!front->hasSounds())
		{
		    return;
		}

		// array<int16_t, 2> samples = front->getMixedSamples();
		// front->audioCallback(samples);
	    }

	    void drawpixels()
	    {
		if (video != NULL)
		{
		    video->drawpixels();
		}
	    }

	    void loadROM(string filename, uint64_t start, uint64_t size, vector<uint8_t> &finaldata)
	    {
		if (!loadsingleROM(filename, start, size, finaldata))
		{
		    cout << "Could not load " << filename << endl;
		    isfilesloaded = false;
		}
	    }

	    void closedriver()
	    {
		if (!hasdriverROMs())
		{
		    return;
		}

		closefiles();
		is_dir_check = false;
		is_dir_exists = false;
		is_samples_dir = false;
		is_samples_dir_exists = false;
	    }

	    void keypressed(BerrnInput key)
	    {
		switch (key)
		{
		    case BerrnInput::BerrnCoin: drvcoin(true); break;
		    case BerrnInput::BerrnStartP1: drvstartp1(true); break;
		    case BerrnInput::BerrnLeftP1: drvleftp1(true); break;
		    case BerrnInput::BerrnRightP1: drvrightp1(true); break;
		    case BerrnInput::BerrnFireP1: drvfirep1(true); break;
		    default: break;
		}
	    }

	    void keyreleased(BerrnInput key)
	    {
		switch (key)
		{
		    case BerrnInput::BerrnCoin: drvcoin(false); break;
		    case BerrnInput::BerrnStartP1: drvstartp1(false); break;
		    case BerrnInput::BerrnLeftP1: drvleftp1(false); break;
		    case BerrnInput::BerrnRightP1: drvrightp1(false); break;
		    case BerrnInput::BerrnFireP1: drvfirep1(false); break;
		    default: break;
		}
	    }

	    bool isallfilesloaded()
	    {
		return isfilesloaded;
	    }

	    virtual string drivername() = 0;
	    virtual bool hasdriverROMs() = 0;
	    virtual bool drvinit() = 0;
	    virtual void drvshutdown() = 0;
	    virtual void drvrun() = 0;

	    virtual void drvcoin(bool pressed) = 0;
	    virtual void drvstartp1(bool pressed) = 0;
	    virtual void drvleftp1(bool pressed) = 0;
	    virtual void drvrightp1(bool pressed) = 0;
	    virtual void drvfirep1(bool pressed) = 0;
      
	    BlueberrnVideo *video = NULL;
	    BlueberrnFrontend *front = NULL;
	    bool isfilesloaded = true;

	private:
	    bool is_zip_loaded = false;
	    bool is_dir_check = false;
	    bool is_dir_exists = false;

	    bool is_samples_dir = false;
	    bool is_samples_dir_exists = false;

	    string romspath = "";

	    string samplespath = "";

	    int loadSoundWAV(string filename)
	    {
		if (front == NULL)
		{
		    return -1;
		}

		if (!is_samples_dir)
		{
		    samplespath = front->getdirpath("samples");

		    stringstream ss;
		    ss << samplespath << drivername();

		    is_samples_dir_exists = front->isdirectory(ss.str());
		    is_samples_dir = true;
		}

		if (!is_samples_dir_exists)
		{
		    // TODO: Implement loading samples from ZIP files?
		    return false;
		}

		stringstream samples_ss;
		    
		samples_ss << front->getdirpath(drivername(), samplespath) << filename;
		return front->loadWAV(samples_ss.str());
	    }

	    vector<uint8_t> loadfile(string filename)
	    {
		vector<uint8_t> temp;
		if (front == NULL)
		{
		    return temp;
		}

		if (!is_dir_check)
		{
		    romspath = front->getdirpath("roms");
		    stringstream ss;
		    ss << romspath << drivername();
		    is_dir_exists = front->isdirectory(ss.str());
		    is_dir_check = true;
		}

		if (!is_dir_exists)
		{
		    temp = loadfilefromzip(filename);
		}
		else
		{
		    stringstream file_ss;
		    
		    file_ss << front->getdirpath(drivername(), romspath) << filename;
		    temp = front->readfile(file_ss.str());
		}

		return temp;
	    }

	    vector<uint8_t> loadfilefromzip(string filename)
	    {
		vector<uint8_t> filetemp;

		if (front == NULL)
		{
		    return filetemp;
		}

		if (!is_zip_loaded)
		{
		    stringstream ss;
		    ss << romspath << drivername() << ".zip";
		    if (!front->loadzip(ss.str()))
		    {
			cout << "Error loading ZIP file." << endl;
			return filetemp;
		    }

		    is_zip_loaded = true;
		}

		filetemp = front->readfilefromzip(filename);
		return filetemp;
	    }

	    void closefiles()
	    {
		if (front == NULL)
		{
		    return;
		}

		if (front->hasSounds())
		{
		    front->freeSounds();
		}

		if (is_zip_loaded)
		{
		    front->closezip();
		    is_zip_loaded = false;
		}

	    }

	    bool loadsingleROM(string filename, uint64_t start, uint64_t size, vector<uint8_t> &finaldata)
	    {
		if (!hasdriverROMs())
		{
		    return true;
		}

		uint64_t prevsize = finaldata.size();
		finaldata.resize((prevsize + size), 0);

		vector<uint8_t> romdata = loadfile(filename);

		if (romdata.empty())
		{
		    cout << "Error: could not load ROM of " << filename << endl;
		    return false;
		}

		if (static_cast<uint64_t>(romdata.size()) != size)
		{
		    cout << "Error: invalid ROM size of " << hex << (int)(romdata.size()) << ", should be " << hex << (int)(size) << endl;
		    return false;
		}

		for (uint64_t index = 0; index < size; index++)
		{
		    finaldata.at((start + index)) = romdata.at(index);
		}

		cout << "ROM of " << filename << " succesfully loaded." << endl;

		return true;
	    }
    };

    inline vector<berrndriver*> drivers;

    #define DRIVER(name) \
	driver##name *drv##name = new driver##name(); \
	driveradd(drv##name);

    inline void driveradd(berrndriver* drv)
    {
	drivers.push_back(drv);
    }

    inline berrndriver* searchdriver(string name)
    {
	for (auto& drv : drivers)
	{
	    if (drv->drivername() == name)
	    {
		return drv;
	    }
	}

	return NULL;
    }
  
    inline vector<string> getdrivernames()
    {
  	vector<string> temp;
  	for (auto& driver : drivers)
  	{
	    temp.push_back(driver->drivername());
  	}
  		
	return temp;
    }
};

#endif // BERRN_DRIVER