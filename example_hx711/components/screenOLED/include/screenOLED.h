#pragma once 

#include "driver/i2c.h"
#include "esp_err.h"

#ifdef _cplusplus
extern "C" {
#endif

typedef struct {
	i2c_port_t i2c_port;
	gpio_num_t sda;
	gpio_num_t scl;
	uint8_t i2c_addr;
	uint32_t clk_speed_hz;
	uint8_t width;
	uint8_t height;
} OLEDscreen_config_t;

typedef struct {
	gpio_num_t sda;
	gpio_num_t scl;
} OLEDscreen_t;

esp_err_t OLEDscreen_init(const OLEDscreen_config_t *config);
esp_err_t OLEDscreen_clear(void);
esp_err_t OLEDscreen_draw_pixel(uint8_t x, uint8_t y, bool color);
esp_err_t OLEDscreen_display(void);
esp_err_t OLEDscreen_draw_number(uint8_t x, uint8_t y, int number);
esp_err_t OLEDscreen_draw_digit(uint8_t x, uint8_t y, char digit);
esp_err_t OLEDscreen_draw_text(uint8_t x, uint8_t y, const char* str);

#ifdef _cplusplus
}
#endif
