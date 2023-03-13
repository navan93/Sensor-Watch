#ifndef MoonRise_h
#define MoonRise_h

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

// Size of event search window in hours.
// Events further away from the search time than MR_WINDOW/2 will not be
// found.  At higher latitudes the moon rise/set intervals become larger, so if
// you want to find the nearest events this will need to increase.  Larger
// windows will increase interpolation error.  Useful values are probably from
// 12 - 48 but will depend upon your application.

#define MR_WINDOW   48	    // Even integer

typedef struct  {
    time_t queryTime;
    time_t riseTime;
    time_t setTime;
    float riseAz;
    float setAz;
    bool hasRise;
    bool hasSet;
    bool isVisible;
} MoonRise;

void calculate(double latitude, double longitude, time_t t);
void initClass(MoonRise *moon);

#endif
