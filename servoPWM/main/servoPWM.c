#include <stdio.h>
#include "driver/ledc.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define PWM parameters
#define SERVO_PIN 18           // GPIO pin connected to the servo
#define PWM_FREQ 50            // PWM frequency (50Hz for servo motors)
#define SERVO_MIN_PULSEWIDTH 500
#define SERVO_MAX_PULSEWIDTH 2500
#define SERVO_MAX_DEGREE 180

void servo_set_angle(int angle){
	int duty = (SERVO_MIN_PULSEWIDTH + ((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * angle) / SERVO_MAX_DEGREE);
	duty = (duty * 8192) / 20000;
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void app_main(void)
{
	ledc_timer_config_t ledc_timer = {
		.speed_mode = LEDC_LOW_SPEED_MODE,
		.timer_num = LEDC_TIMER_0,
		.duty_resolution = LEDC_TIMER_13_BIT,
		.freq_hz = PWM_FREQ,
		.clk_cfg = LEDC_AUTO_CLK,
	};
	ledc_timer_config(&ledc_timer);

	// Configure the LEDC PWM channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,   // High speed mode
        .channel = LEDC_CHANNEL_0,            // Use channel 0
        .timer_sel = LEDC_TIMER_0,            // Select the timer
        .gpio_num = SERVO_PIN,                // Set the GPIO pin for the servo
        .duty = 0,                            // Initial duty cycle (0%)
        .hpoint = 0,
    };
    ledc_channel_config(&ledc_channel);

    
// Example of moving the servo to different angles
    while (1) {
    	servo_set_angle(0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        servo_set_angle(90);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
