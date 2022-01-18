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
	sound_timer = new BerrnTimer("Sound", scheduler, [&](int64_t, int64_t) {
	    mixSample(getRawSample());
	    outputAudio();
	});
    }

    driverexample::~driverexample()
    {

    }

    string driverexample::drivername()
    {
	return "example";
    }

    bool driverexample::hasdriverROMs()
    {
	return false;
    }

    bool driverexample::drvinit()
    {
	scheduler.reset();
	scheduler.add_device(device);
	device->reset();
	sample_val = loadWAV("loop.wav");

	if (sample_val != -1)
	{
	    setSoundLoop(sample_val, true);
	    playSound(sample_val);
	}

	bitmap = new BerrnBitmapRGB(640, 480);
	bitmap->fillcolor(red());

	resize(640, 480, 1);
	sound_timer->start(time_in_hz(getSampleRate()), true);
	return true;
    }

    void driverexample::drvshutdown()
    {
	sound_timer->stop();
	scheduler.shutdown();
    }
  
    void driverexample::drvrun()
    {
	int64_t schedule_time = scheduler.get_current_time();
	int64_t frame_time = time_in_hz(60);

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}

	setScreen(bitmap);
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
	    case BerrnInput::BerrnFireP1:
	    {
		cout << "P1 fire button has been " << key_state << endl;
	    }
	    break;
	    default: break;
	}
    }
};