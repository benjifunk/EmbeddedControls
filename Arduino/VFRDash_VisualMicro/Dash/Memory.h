#ifndef MEMORY_H
#define MEMORY_H
#include "shiftlight.h"

class Memory {
public:
	void getEEPROM();
	void writeMilesEEPROM();
	void writeSettingsEEPROM();
	uint32_t load_color(int cx);
	void build_segments();
	void check_first_run();

protected:
	int c1,
		c2,
		c3,
		c4,
		c5,

		brightval,        //7-seg brightness
		sb,               //strip brightness
		pixelanim = 1,
		senseoption,
		smoothing,
		rpmscaler,

		activation_rpm,
		shift_rpm,

		seg1_start = 1,
		seg1_end = 1,
		seg2_start = 2,
		seg2_end = 2,
		seg3_start = 3,
		seg3_end = 3,

		distanceMiles,
		distanceTenths,
		distanceClicks,
		totalMiles,
		totalTenths,
		totalClicks,

		DEBUG,
		NUMPIXELS,
		cal;

	long shift_rpm1,
		shift_rpm2,
		shift_rpm3,
		shift_rpm4,

		activation_rpm1,
		activation_rpm2,
		activation_rpm3,
		activation_rpm4;

private:
	
};

#endif // !MEMORY_H
