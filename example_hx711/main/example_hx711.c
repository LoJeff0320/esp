#include <stdio.h>
#include "hx711.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "screenOLED.h"

void app_main()
{
	OLEDscreen_config_t oled_cfg = {
		.i2c_port = I2C_NUM_0,
		.sda = 21,
		.scl = 22,
		.i2c_addr = 0x3c,
		.clk_speed_hz = 400000,
		.width = 128,
		.height = 64,
	};

	OLEDscreen_init(&oled_cfg);
	OLEDscreen_clear();	

	hx711_t scale;
	hx711_config_t hx711_cfg = {
		.dout = GPIO_NUM_33,
		.sck = GPIO_NUM_32,
		.gain = 128
	};

	hx711_init(&scale, &hx711_cfg);

	printf("Taring... Place empty bowl on scale\n");
	vTaskDelay(pdMS_TO_TICKS(3000));
	hx711_tare(&scale, 10);

	printf("Now place a known weight (e.g. 500g)\n");
	vTaskDelay(pdMS_TO_TICKS(5000));
	int32_t reading = 0;
	hx711_read(&scale, &reading);
	scale.scale = (reading - scale.offset) / 56.0f;

	while(1)
	{
		char buf[20];
		float weight = hx711_get_units(&scale, 5);
		printf("weight: %.2f g\n", weight);
		snprintf(buf, sizeof(buf), "Weight: %.2f g\n", weight);
		OLEDscreen_draw_text(0, 0, buf);
		OLEDscreen_display();
	}
}

