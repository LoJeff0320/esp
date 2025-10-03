#include <stdio.h>
#include "screenOLED.h"

void app_main(void)
{
	OLEDscreen_config_t cfg = {
		.i2c_port = I2C_NUM_0,
		.sda = 21,
		.scl = 22,
		.i2c_addr = 0x3c,
		.clk_speed_hz = 400000,
		.width = 128,
		.height = 64,
	};

	OLEDscreen_init(&cfg);
	OLEDscreen_clear();
	
	//	OLEDscreen_draw_number(10, 10, 12345);
	char buf[16];
	float weight = 0.28;
	snprintf(buf, sizeof(buf), "Weight: %.2f g\n", weight);
	OLEDscreen_draw_text(0, 0, buf);
	OLEDscreen_display();
}
