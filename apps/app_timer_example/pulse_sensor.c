#include "pulse_sensor.h"

// TODO make hysteresis based on the average / try to calibrate different graphs
// #define HYSTERESIS 100
#define PEAK_LIMIT 8192*3/2

static uint16_t pulse_array[1500];
static int arr_index = 0;
static int latest_bpm = 0;
static int rolling_avg = 0;
static int num_avg = 0;

static int calculate_bpm();
static uint16_t adc_sample_blocking(uint8_t);

void sample_timer_callback(void* _unused) {
  // Do things periodically here
  uint16_t hr_volt = adc_sample_blocking(ADC_HR_CHANNEL);
  hr_volt = hr_volt >> 2;

  // printf("%lu\n", hr_volt);

  if (hr_volt > PEAK_LIMIT) {
    hr_volt = 0;
  }
  pulse_array[arr_index] = hr_volt;
  arr_index += 1;
  if (arr_index >= 1500)
  {
  	// TODO: exclude outliers from avg calculations
  	arr_index = 0;
  	latest_bpm = calculate_bpm();
  	rolling_avg = (rolling_avg * num_avg + latest_bpm) / (num_avg + 1);
  	num_avg += 1;
  	printf("\nBPM %i\n", latest_bpm);
    printf("Rolling average %i\n", rolling_avg);
  }
}

static void saadc_event_callback(nrfx_saadc_evt_t const* _unused) {
  // don't care about saadc events
  // ignore this function
}

void adc_init(void) {
  // Initialize the SAADC
  nrfx_saadc_config_t saadc_config = {
    .resolution = NRF_SAADC_RESOLUTION_14BIT,
    .oversample = NRF_SAADC_OVERSAMPLE_DISABLED,
    .interrupt_priority = 4,
    .low_power_mode = false,
  };
  ret_code_t error_code = nrfx_saadc_init(&saadc_config, saadc_event_callback);
  APP_ERROR_CHECK(error_code);

  // Initialize pulse sensor channel
  nrf_saadc_channel_config_t hr_channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(ANALOG_HR_IN);
  error_code = nrfx_saadc_channel_init(ADC_HR_CHANNEL, &hr_channel_config);
  APP_ERROR_CHECK(error_code);

}

static uint16_t adc_sample_blocking(uint8_t channel) {
  // read ADC counts (0-4095)
  // this function blocks until the sample is ready
  uint16_t adc_counts = 0;
  ret_code_t error_code = nrfx_saadc_sample_convert(channel, &adc_counts);
  APP_ERROR_CHECK(error_code);

  // convert ADC counts to volts
  // 12-bit ADC with range from 0 to 3.6 Volts
  // TODO

  // return voltage measurement
  return adc_counts;
}

static int calculate_bpm(void)
{
   // calculate bpm based on the readings in pulse_array
   // 1500 array values = 3 seconds of data
   int count = 0;
   int sum = 0;
   for (int i = 0; i < 1500; i++)
   {
      sum += pulse_array[i];
   }
   float avg = sum / 1500.0;
   printf("Average val: %f\n", avg);
   int prev = 0;
   for (int i = 0; i < 1500; i++)
   {
      if (prev == 0 && pulse_array[i] > avg * 1.1)
      {
      	 count += 1;
      	 prev = 1;
      }
      if (prev == 1 && pulse_array[i] < avg * 0.9)
      {
      	 prev = 0;
      }
   }
   
   // convert to BPS by dividing by 3, then multiply by 60 for BPM
   return count * 5;
}

int determine_timelimit(void)
{
   // different tiers based on heart rate
   // the higher the heart rate the shorter the time
   if (latest_bpm > 1.2 * rolling_avg)
   {
   	return 7;
   }
   else if (latest_bpm > 1.1 * rolling_avg)
   {
   	return 15;
   }
   else
   {
   	return 20;
   }
}
