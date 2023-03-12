/*
 * MIT License
 *
 * Copyright (c) 2023 Navaneeth Bhardwaj
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include "moonrise_moonset_face.h"
#include "watch_utility.h"
#include <math.h>
#include "moonriset.h"

static MoonRise moon_info;

void moonrise_moonset_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(moonrise_moonset_state_t));
        memset(*context_ptr, 0, sizeof(moonrise_moonset_state_t));
        // Do any one-time tasks in here; the inside of this conditional happens only at boot.
    }
    // Do any pin or peripheral setup here; this will be called whenever the watch wakes from deep sleep.
}

void moonrise_moonset_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    moonrise_moonset_state_t *state = (moonrise_moonset_state_t *)context;

    // Handle any tasks related to your watch face coming on screen.
    moon_info.queryTime = 0;
    moon_info.riseTime = 0;
    moon_info.setTime = 0;
    moon_info.riseAz = 0;
    moon_info.setAz = 0;
    moon_info.hasRise = false;
    moon_info.hasSet = false;
    moon_info.isVisible = false;

    char buf[14];

    initClass(&moon_info);

    watch_date_time date_time = watch_rtc_get_date_time();
    uint32_t timestamp = watch_utility_date_time_to_unix_time(date_time, movement_timezone_offsets[settings->bit.time_zone] * 60);
    date_time = watch_utility_date_time_from_unix_time(timestamp, 0);
    double jd = astro_convert_date_to_julian_date(date_time.unit.year + WATCH_RTC_REFERENCE_YEAR, date_time.unit.month, date_time.unit.day, date_time.unit.hour, date_time.unit.minute, date_time.unit.second);

    movement_location_t movement_location = (movement_location_t) watch_get_backup_data(1);
    int16_t lat_centi = (int16_t)movement_location.bit.latitude;
    int16_t lon_centi = (int16_t)movement_location.bit.longitude;
    double lat = (double)lat_centi / 100.0;
    double lon = (double)lon_centi / 100.0;
    double latitude_radians = astro_degrees_to_radians(lat);
    double longitude_radians = astro_degrees_to_radians(lon);

    // watch_display_string("LU        ", 0);

    calculate(lat, lon, timestamp);
    printf("Moon rise/set nearest %.24s for latitude %.2f longitude %.2f:\n",
	 ctime(&moon_info.queryTime), lat, lon);
    printf("Preceding event:\n");
    if ((!moon_info.hasRise || (moon_info.hasRise && moon_info.riseTime > moon_info.queryTime)) &&
        (!moon_info.hasSet || (moon_info.hasSet && moon_info.setTime > moon_info.queryTime)))
        printf("\tNo moon rise or set during preceding %d hours\n", MR_WINDOW/2);
    if (moon_info.hasRise && moon_info.riseTime < moon_info.queryTime)
        printf("\tMoon rise at %.24s, Azimuth %.2f\n", ctime(&moon_info.riseTime), moon_info.riseAz);
    if (moon_info.hasSet && moon_info.setTime < moon_info.queryTime)
        printf("\tMoon set at  %.24s, Azimuth %.2f\n", ctime(&moon_info.setTime), moon_info.setAz);

    printf("Succeeding event:\n");
    if ((!moon_info.hasRise || (moon_info.hasRise && moon_info.riseTime < moon_info.queryTime)) &&
        (!moon_info.hasSet || (moon_info.hasSet && moon_info.setTime < moon_info.queryTime)))
        printf("\tNo moon rise or set during succeeding %d hours\n", MR_WINDOW/2);
    if (moon_info.hasRise && moon_info.riseTime > moon_info.queryTime) {
        printf("\tMoon rise at %.24s, Azimuth %.2f\n", ctime(&moon_info.riseTime), moon_info.riseAz);
        date_time = watch_utility_date_time_from_unix_time(moon_info.riseTime, movement_timezone_offsets[settings->bit.time_zone] * 60);
        sprintf(buf, "rI%2d%2d%02d  ", date_time.unit.day, date_time.unit.hour, date_time.unit.minute);
        watch_display_string(buf, 0);
    }
    if (moon_info.hasSet && moon_info.setTime > moon_info.queryTime) {
        printf("\tMoon set at  %.24s, Azimuth %.2f\n", ctime(&moon_info.setTime), moon_info.setAz);
        date_time = watch_utility_date_time_from_unix_time(moon_info.setTime, movement_timezone_offsets[settings->bit.time_zone] * 60);
        sprintf(buf, "SE%2d%2d%02d  ", date_time.unit.day, date_time.unit.hour, date_time.unit.minute);
        watch_display_string(buf, 0);
    }
}

bool moonrise_moonset_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    moonrise_moonset_state_t *state = (moonrise_moonset_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            // Show your initial UI here.

            break;
        case EVENT_TICK:
            // If needed, update your display here.
            break;
        case EVENT_LIGHT_BUTTON_UP:
            // You can use the Light button for your own purposes. Note that by default, Movement will also
            // illuminate the LED in response to EVENT_LIGHT_BUTTON_DOWN; to suppress that behavior, add an
            // empty case for EVENT_LIGHT_BUTTON_DOWN.
            break;
        case EVENT_ALARM_BUTTON_UP:
            // Just in case you have need for another button.
            break;
        case EVENT_TIMEOUT:
            // Your watch face will receive this event after a period of inactivity. If it makes sense to resign,
            // you may uncomment this line to move back to the first watch face in the list:
            // movement_move_to_face(0);
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            // If you did not resign in EVENT_TIMEOUT, you can use this event to update the display once a minute.
            // Avoid displaying fast-updating values like seconds, since the display won't update again for 60 seconds.
            // You should also consider starting the tick animation, to show the wearer that this is sleep mode:
            // watch_start_tick_animation(500);
            break;
        default:
            // Movement's default loop handler will step in for any cases you don't handle above:
            // * EVENT_LIGHT_BUTTON_DOWN lights the LED
            // * EVENT_MODE_BUTTON_UP moves to the next watch face in the list
            // * EVENT_MODE_LONG_PRESS returns to the first watch face (or skips to the secondary watch face, if configured)
            // You can override any of these behaviors by adding a case for these events to this switch statement.
            return movement_default_loop_handler(event, settings);
    }

    // return true if the watch can enter standby mode. Generally speaking, you should always return true.
    // Exceptions:
    //  * If you are displaying a color using the low-level watch_set_led_color function, you should return false.
    //  * If you are sounding the buzzer using the low-level watch_set_buzzer_on function, you should return false.
    // Note that if you are driving the LED or buzzer using Movement functions like movement_illuminate_led or
    // movement_play_alarm, you can still return true. This guidance only applies to the low-level watch_ functions.
    return true;
}

void moonrise_moonset_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}

