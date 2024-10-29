#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "app_timer.h"
#include "nrf_delay.h"
#include "nrfx_saadc.h"

#include "microbit_v2.h"

// Analog inputs
// Breakout pins 1 and 2
// These are GPIO pin numbers that can be used in ADC configurations
// AIN1 is breakout pin 1. AIN2 is breakout pin 2.
#define ANALOG_HR_IN  NRF_SAADC_INPUT_AIN1
// #define ANALOG_LIGHT_IN NRF_SAADC_INPUT_AIN2

// ADC channel configurations
// These are ADC channel numbers that can be used in ADC calls
#define ADC_HR_CHANNEL  0

// Global variables
APP_TIMER_DEF(pulse_timer);

// Function prototypes
void sample_timer_callback(void*);
// static void saadc_event_callback(nrfx_saadc_evt_t const*);
void adc_init(void);
// static uint16_t adc_sample_blocking(uint8_t);
// static int calculate_bpm();
int determine_timelimit(void);