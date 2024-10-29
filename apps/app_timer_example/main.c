// #include <stdbool.h>
// #include <stdint.h>
// #include <stdio.h>

#include "app_timer.h"
#include "nrf_delay.h"
#include "microbit_v2.h"
#include "display.h"
#include "pulse_sensor.h"
#include "joystick.h"
#include "nrf_twi_mngr.h"

// Timer definition
APP_TIMER_DEF(my_timer);

// Global variables
NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);

// Time structure
typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    bool is_pm;
} Time;

// Current time and alarm time
Time current_time = {12, 0, 0, false}; // 12:00:00 AM
Time alarm_time = {12, 0, 5, false};   // 12:01:00 AM

// Function to increment time
void increment_time(Time *time) {
    time->seconds++;
    if (time->seconds >= 60) {
        time->seconds = 0;
        time->minutes++;
        if (time->minutes >= 60) {
            time->minutes = 0;
            time->hours++;
            if (time->hours > 12) {
                time->hours = 1;
            } else if (time->hours == 12) {
                time->is_pm = !time->is_pm;
            }
        }
    }
}

// Function to check if two times are equal
bool is_time_equal(Time *time1, Time *time2) {
    return (time1->hours == time2->hours) && 
           (time1->minutes == time2->minutes) &&
           (time1->seconds == time2->seconds) && 
           (time1->is_pm == time2->is_pm);
}

// Function to print time
void print_time(const Time *time) {
    printf("%02d:%02d:%02d %s\n", time->hours, time->minutes, time->seconds, time->is_pm ? "PM" : "AM");
}

// Function to display time
void display_time(const Time *time) {
  // Convert current time to screen. 
  char curr_time[30];
  sprintf(curr_time, "%02d:%02d:%02d %s\n", time->hours, time->minutes, time->seconds, time->is_pm ? "PM" : "AM");
  draw_phrase(curr_time, 0xFF, 0xFF);
}

// Timer callback function
static void timer_callback(void* _unused) {
    increment_time(&current_time);

    if (is_time_equal(&current_time, &alarm_time)) {
        nrf_gpio_pin_toggle(LED_ROW1);
    }
}

int main(void) {
    printf("Alarm Clock started!\n");

    // SCREEN
    // Delay screen starting
    nrf_delay_ms(500);
    // Initialize SPI controller
    spim_init();
    display_init();

    ////////////////////////////////////////////////////////////////

    // ALARM

    // initialize LED pins
    nrf_gpio_pin_dir_set(LED_COL1, NRF_GPIO_PIN_DIR_OUTPUT);
    nrf_gpio_pin_dir_set(LED_ROW1, NRF_GPIO_PIN_DIR_OUTPUT);
    nrf_gpio_pin_clear(LED_COL1);
    nrf_gpio_pin_clear(LED_ROW1);

    // // Initialize app timer
    app_timer_init();
    app_timer_create(&my_timer, APP_TIMER_MODE_REPEATED, timer_callback);

    // Start the app timer (ticks every second)
    app_timer_start(my_timer, 32768, NULL); // Assuming 32.768 kHz clock

    ////////////////////////////////////////////////////////////////

    // PULSE
    
    // initialize ADC
    // adc_init();

    // initialize app timers
    // app_timer_init();
    // app_timer_create(&pulse_timer, APP_TIMER_MODE_REPEATED, sample_timer_callback);

    // start timer
    // change the rate to whatever you want
    // app_timer_start(pulse_timer, 262, NULL);

    ////////////////////////////////////////////////////////////////

    // JOYSTICK

    // Initialize I2C for the joystick (if not already initialized)
    //i2c_init(); // Uncomment and replace with actual I2C initialization function if needed

    // Initialize driver
    // Initialize I2C peripheral and driver
    nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
    i2c_config.scl = I2C_SCL;
    i2c_config.sda = I2C_SDA;
    i2c_config.frequency = NRF_TWIM_FREQ_100K;
    i2c_config.interrupt_priority = 0;
    nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);

    // Initialize joystick
    joystick_init(&twi_mngr_instance);

    ////////////////////////////////////////////////////////////////

    // Loop forever
    while (1) {
      // Display the time on the screen. 
      display_time(&current_time);
      //printf(joystick_get_horizontal());
      //printf(joystick_get_vertical());
      nrf_delay_ms(1000);
    }
}