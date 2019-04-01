#include <psp2/power.h>
#include <psp2/rtc.h>
#include <stdio.h>

#include "common.h"
#include "textures.h"

static int percent_width = 0;

static char *StatusBar_GetCurrentTime(void) {
	static char buffer[27];

	SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);
	int hours = sceRtcGetHour(&time);
	int am_or_pm = 0;

	if (hours < 12)
		am_or_pm = 1;
	if (hours == 0)
		hours = 12;
	else if (hours > 12)
		hours = hours - 12;

	if ((hours >= 1) && (hours < 10))
		snprintf(buffer, 27, "%2i:%02i %s", hours, sceRtcGetMinute(&time), am_or_pm ? "AM" : "PM");
	else
		snprintf(buffer, 27, "%2i:%02i %s", hours, sceRtcGetMinute(&time), am_or_pm ? "AM" : "PM");

	return buffer;
}

static void StatusBar_GetBatteryStatus(int x, int y) {
	int percent = 0;
	SceBool state = SCE_FALSE;
	char buf[13];

	if (R_FAILED(state = scePowerIsBatteryCharging()))
		state = SCE_FALSE;

	if (R_SUCCEEDED(percent = scePowerGetBatteryLifePercent())) {
		if (percent < 20)
			vita2d_draw_texture(battery_low, x, 4);
		else if ((percent >= 20) && (percent < 30)) {
			if (state)
				vita2d_draw_texture(battery_20_charging, x, 4);
			else
				vita2d_draw_texture(battery_20, x, 4);
		}
		else if ((percent >= 30) && (percent < 50)) {
			if (state)
				vita2d_draw_texture(battery_50_charging, x, 4);
			else
				vita2d_draw_texture(battery_50, x, 4);
		}
		else if ((percent >= 50) && (percent < 60)) {
			if (state)
				vita2d_draw_texture(battery_50_charging, x, 4);
			else
				vita2d_draw_texture(battery_50, x, 4);
		}
		else if ((percent >= 60) && (percent < 80)) {
			if (state)
				vita2d_draw_texture(battery_60_charging, x, 4);
			else
				vita2d_draw_texture(battery_60, x, 4);
		}
		else if ((percent >= 80) && (percent < 90)) {
			if (state)
				vita2d_draw_texture(battery_80_charging, x, 4);
			else
				vita2d_draw_texture(battery_80, x, 4);
		}
		else if ((percent >= 90) && (percent < 100)) {
			if (state)
				vita2d_draw_texture(battery_90_charging, x, 4);
			else
				vita2d_draw_texture(battery_90, x, 4);
		}
		else if (percent == 100) {
			if (state)
				vita2d_draw_texture(battery_full_charging, x, 4);
			else
				vita2d_draw_texture(battery_full, x, 4);
		}

		snprintf(buf, 13, "%d%%", percent);
		percent_width = vita2d_font_text_width(font, 25, buf);
		vita2d_font_draw_text(font, (x - percent_width - 5), y, RGBA8(255, 255, 255, 255), 25, buf);
	}
	else {
		snprintf(buf, 13, "%d%%", percent);
		percent_width = vita2d_font_text_width(font, 25, buf);
		vita2d_font_draw_text(font, (x - percent_width - 5), y, RGBA8(255, 255, 255, 255), 25, buf);
		vita2d_draw_texture(battery_unknown, x, 4);
	}
}

void StatusBar_Display(void) {
	int width = 0, height = 0;
	vita2d_font_text_dimensions(font, 25, StatusBar_GetCurrentTime(), &width, &height);

	StatusBar_GetBatteryStatus(((950 - width) - (32 + 10)), ((40 - height) / 2) + 25);
	vita2d_font_draw_text(font, (950 - width), ((40 - height) / 2) + 25, RGBA8(255, 255, 255, 255), 25, StatusBar_GetCurrentTime());
}
