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

#include "example.h"
using namespace berrn;

namespace berrn
{
    driverexample::driverexample()
    {
	device = new BerrnNull();
	audio = new samplesdevice(*this);
	bitmap = new BerrnBitmapRGB(640, 480);
	bitmap->clear();
    }

    driverexample::~driverexample()
    {

    }

    void driverexample::process_audio()
    {
	auto samples = audio->fetch_samples();
	add_stereo_sample(samples[0], samples[1]);
    }

    string driverexample::drivername()
    {
	return "example";
    }

    bool driverexample::drvinit()
    {
	bitmap->fillcolor(red());
	audio->init();

	sample_val = audio->load_sample("loop.wav");

	audio->set_sound_loop(sample_val, true);
	audio->play_sound(sample_val);

	get_scheduler().add_device(device);
	resize(640, 480);
	return true;
    }

    void driverexample::drvshutdown()
    {
	audio->stop_sound(sample_val);
	audio->shutdown();
    }
  
    void driverexample::drvrun()
    {
	run_scheduler();
	set_screen_bmp(bitmap);
    }

    void driverexample::keychanged(BerrnInput key, bool is_pressed)
    {
	string key_state = (is_pressed) ? "pressed" : "released";

	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		cout << "Coin button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnStartP1:
	    {
		cout << "P1 start button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnLeftP1:
	    {
		cout << "P1 left button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnRightP1:
	    {
		cout << "P1 right button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnUpP1:
	    {
		cout << "P1 up button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnDownP1:
	    {
		cout << "P1 down button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnButton1P1:
	    {
		cout << "P1 button 1 has been " << key_state << endl;
	    }
	    break;
	    default: break;
	}
    }
};