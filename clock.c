/**
 * Copyright (c) 2010 William Light <wrl@illest.net>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * test.c
 * basic program to test all of the output commands to the monome.
 */

#define _XOPEN_SOURCE 600

#include <string.h>
#include <time.h>
#include <monome.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define DEFAULT_MONOME_DEVICE "/dev/tty.usbserial-m40h0924"

#define BPM 98
typedef int bool;
static const int true = 1;
static const int false = 0;

uint8_t eighths_pattern[8][9] = {
	{0,0,0,0,0,0,0,1,1},
	{0,0,0,0,0,1,1,1,1}, 
	{0,0,0,1,1,1,1,1,1}, 
	{0,1,1,0,1,0,1,0,1}, 
    {0,0,1,0,1,0,1,0,1},
    {0,0,0,1,1,1,1,1,1},
    {0,0,0,0,0,1,1,1,1},
    {0,0,0,0,0,0,0,1,1}
};

// Progress meter with range [0,8] with one-eighth unit precision.
// The patterns for each 1/8th unit is designed so each value is 
// highly distinguishable from the others, and the multiples of
// 1/4 are easily distinguishable from non-multiples.
// In "Reverse" mode, "on' rows start on the right of the grid rather
// than the left.
void progress(monome_t* monome, int num, int eighths, bool reverse)
{
    // handle full rows
    for(int c = 0; c < 8; ++c)
    {
        for(int r = 0; r < 8; ++r)
        {
            int on = reverse ? c > 7-num : c < num;
            monome_led_set(monome, c, r, on);
        }
    }
    // handle "eighths" column
    if(num >= 0 && num < 8 && eighths >= 0 && eighths <= 8)
    {
        // find column to store fractional vale
        int c = reverse ? 7-num : num;
        for(int r = 0; r < 8; ++r)
        {
            int on = eighths_pattern[r][eighths];
            monome_led_set(monome, c, r, on);
        }
    }
}

static void chill(int speed) {
	struct timespec rem, req;

	req.tv_sec  = 0;
	req.tv_nsec = ((60000 / (BPM * speed)) * 1000000);

	nanosleep(&req, &rem);
}

// Display times between 4:00 am and 8:00 pm. Start with "all off"
// in the morning, progress to "all on" at Noon, and decrease to "all off"
// in the evening.
void display_time(monome_t* monome, int hour, int minute, bool am)
{
    int eighths = (int)((minute)/7.5);
    if(am)
    {
        if(hour < 4) 
        {
            monome_led_all(monome, 0);
            return;
        }
        progress(monome, hour-4, eighths, false);
    }
    else
    {
        if(hour > 8)
        {
            monome_led_all(monome, 0);
            return;
        }
        progress(monome, 7-hour, 8-eighths, true);
    }
}
void test_time(monome_t *monome) 
{
    char in;
    for(int hour24 = 4; hour24 <= 20; ++hour24)
    {
        int hour12 = hour24;
        bool am;
        if(hour12 >= 12)
        {
            hour12 -= 12;
            am = false;
        }
        else
            am = true;
        for(float minute = 0; minute < 60; minute += 7.5)
        {
            int minute_int = (int) ceil(minute);
            printf("time: %d:%d %s\n", hour12, minute_int, am ? "am" : "pm");
            display_time(monome, hour12, minute_int, am);
            chill(3);
        }
    }
}

void fade_out(monome_t *monome) {
	unsigned int i = 0x10;

	while( i-- ) {
		monome_led_intensity(monome, i);
		chill(16);
	}
}

int main(int argc, char **argv) {
	monome_t *monome;
	int i;
	i =0;

	if( !(monome = monome_open((argc == 2 ) ? argv[1] : DEFAULT_MONOME_DEVICE, "8000")) )
		return -1;

	test_time(monome);

	chill(400);

	monome_led_all(monome, 1);
	fade_out(monome);

	monome_led_all(monome, 0);
	monome_led_intensity(monome, 0x0F);

	return 0;
}
