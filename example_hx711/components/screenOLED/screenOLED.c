#include <stdio.h>
#include "screenOLED.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include <string.h>

#include "font8x8_basic.h"

#define SSD1306_CMD 0x00
#define SSD1306_DATA 0x40

/*
   static const uint8_t font_digits_5x7[10][5] = {
// 0
{ 0x3E, 0x51, 0x49, 0x45, 0x3E },
// 1
{ 0x00, 0x42, 0x7F, 0x40, 0x00 },
// 2
{ 0x42, 0x61, 0x51, 0x49, 0x46 },
// 3
{ 0x21, 0x41, 0x45, 0x4B, 0x31 },
// 4
{ 0x18, 0x14, 0x12, 0x7F, 0x10 },
// 5
{ 0x27, 0x45, 0x45, 0x45, 0x39 },
// 6
{ 0x3C, 0x4A, 0x49, 0x49, 0x30 },
// 7
{ 0x01, 0x71, 0x09, 0x05, 0x03 },
// 8
{ 0x36, 0x49, 0x49, 0x49, 0x36 },
// 9
{ 0x06, 0x49, 0x49, 0x29, 0x1E }
};
static const uint8_t font_digits_8x8[10][8] = {
// 0
{ 0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00 }, 
// 1
{ 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00 },
// 2
{ 0x3C, 0x66, 0x06, 0x1C, 0x30, 0x66, 0x7E, 0x00 },
// 3
{ 0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00 },
// 4
{ 0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x0C, 0x00 },
// 5
{ 0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00 },
// 6
{ 0x3C, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x3C, 0x00 },
// 7
{ 0x7E, 0x66, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x00 },
// 8
{ 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00 },
// 9
{ 0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00 }
};*/

static const char *TAG = "SSD1306";

static OLEDscreen_config_t ssd_cfg;
static uint8_t *framebuffer = NULL;

static esp_err_t send_cmd(uint8_t cmd){
	i2c_cmd_handle_t handle = i2c_cmd_link_create();
	i2c_master_start(handle);
	i2c_master_write_byte(handle, (ssd_cfg.i2c_addr << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(handle, SSD1306_CMD, true);
	i2c_master_write_byte(handle, cmd, true);
	i2c_master_stop(handle);
	esp_err_t ret = i2c_master_cmd_begin(ssd_cfg.i2c_port, handle, pdMS_TO_TICKS(100));
	i2c_cmd_link_delete(handle);
	return ret;
}

esp_err_t OLEDscreen_init(const OLEDscreen_config_t *config){
	memcpy(&ssd_cfg, config, sizeof(OLEDscreen_config_t));

	framebuffer = calloc((ssd_cfg.width * ssd_cfg.height) / 8, sizeof(uint8_t));
	if(!framebuffer) return ESP_ERR_NO_MEM;

	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = ssd_cfg.sda,
		.scl_io_num = ssd_cfg.scl,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = ssd_cfg.clk_speed_hz,
	};

	ESP_ERROR_CHECK(i2c_param_config(ssd_cfg.i2c_port, &conf));
	ESP_ERROR_CHECK(i2c_driver_install(ssd_cfg.i2c_port, I2C_MODE_MASTER, 0, 0, 0));

	// SSD1306 Init Sequence
	const uint8_t init_seq[] = {
		0xAE, // Display OFF
		0x20, 0x00, // Horizontal addressing mode
		0xB0, // Page Start Address
		0xC8, // COM Scan Direction
		0x00, // Low Column
		0x10, // High Column
		0x40, // Start Line
		0x81, 0x7F, // Contrast
		0xA1, // Segment Re-map
		0xA6, // Normal display
		0xA8, 0x3F, // Multiplex Ratio
		0xA4, // Display all on resume
		0xD3, 0x00, // Display offset
		0xD5, 0x80, // Osc frequency
		0xD9, 0xF1, // Pre-charge
		0xDA, 0x12, // COM pins config
		0xDB, 0x40, // VCOM detect
		0x8D, 0x14, // Charge pump enable
		0xAF // Display ON
	};

	for (int i = 0; i < sizeof(init_seq); i++) {
		ESP_ERROR_CHECK(send_cmd(init_seq[i]));
	}

	ESP_LOGI(TAG, "SSD1306 initialized");
	return ESP_OK;
}

esp_err_t OLEDscreen_clear(void){
	if(!framebuffer) return ESP_FAIL;
	memset(framebuffer, 0x00, (ssd_cfg.width * ssd_cfg.height) / 8);
	return OLEDscreen_display();
}

esp_err_t OLEDscreen_draw_pixel(uint8_t x, uint8_t y, bool color) {
	if (x >= ssd_cfg.width || y >= ssd_cfg.height) return ESP_ERR_INVALID_ARG;
	//uint16_t index = x + (y / 8) * ssd_cfg.width;
	if (color)
		framebuffer[x + (y / 8) * ssd_cfg.width] |= (1 << (y % 8));
	else
		framebuffer[x + (y /8) * ssd_cfg.width] &= ~(1 << (y % 8));
	return ESP_OK;
}

esp_err_t OLEDscreen_display(void) {
	for (uint8_t page = 0; page < (ssd_cfg.height / 8); page++) {
		ESP_ERROR_CHECK(send_cmd(0xB0 + page));
		ESP_ERROR_CHECK(send_cmd(0x00));
		ESP_ERROR_CHECK(send_cmd(0x10));

		i2c_cmd_handle_t handle = i2c_cmd_link_create();
		i2c_master_start(handle);
		i2c_master_write_byte(handle, (ssd_cfg.i2c_addr << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(handle, SSD1306_DATA, true);
		i2c_master_write(handle, framebuffer + (page * ssd_cfg.width), ssd_cfg.width, true);
		i2c_master_stop(handle);
		esp_err_t ret = i2c_master_cmd_begin(ssd_cfg.i2c_port, handle, pdMS_TO_TICKS(100));
		i2c_cmd_link_delete(handle);
		if (ret != ESP_OK) return ret;
	}
	return ESP_OK;
}

esp_err_t OLEDscreen_draw_digit(uint8_t x, uint8_t y, char c){
	const uint8_t *glyph = (const uint8_t *)font8x8_basic[(uint8_t)c];
	for (uint8_t i = 0; i < 8; i++){
		uint8_t col = glyph[i];
		for (uint8_t j = 0; j < 8; j++)
		{
			OLEDscreen_draw_pixel(x + j, y + i, (col >> j) & 1);
		}
	}

	return ESP_OK;
}

esp_err_t OLEDscreen_draw_number(uint8_t x, uint8_t y, int number){
	char buf[12]; // temp character array to store number with a maximum length of 11 characters (32 bits)
	snprintf(buf, sizeof(buf), "%d", number); // stores the integer number as character in the array buf

	for (int i = 0; buf[i] != '\0'; i++){ 
		OLEDscreen_draw_digit(x, y, buf[i]);
		x += 9;
	}
	return ESP_OK;
}

esp_err_t OLEDscreen_draw_text(uint8_t x, uint8_t y, const char* str){
	while (*str) {
		OLEDscreen_draw_digit(x, y, *str++);
		x+= 8;
		if(x+8 > ssd_cfg.width) break;
	}
	return ESP_OK;
}
