#include"../../../include/libblueberrn/berrn/berrn.h"
#include"../../../include/libblueberrn/berrn/drivers/drivers.h"
using namespace std;

namespace berrn
{

    berrndriver::berrndriver()
    {

    }

    berrndriver::~berrndriver()
    {

    }

    bool berrnutil::loadROM(string filename, vector<uint8_t> &mem, int start)
    {
	ifstream file(filename.c_str(), ios::binary | ios::ate);

	if (!file.is_open())
	{
	    cout << "Error: ROM could not be opened." << endl;
	    return false;
	}

	streampos size = file.tellg();

	file.seekg(0, ios::beg);
	file.read((char*)&mem[start], size);

	cout << "ROM succesfully loaded." << endl;
	file.close();
	return true;
    }
};
