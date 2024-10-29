#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include "nrf_twi_mngr.h" // Include your I2C manager header file here

// Joystick I2C address and register addresses
#define JOYSTICK_I2C_ADDRESS 0x20
#define JOYSTICK_FIRMWARE_VERSION_MSB_REG 0x01
#define JOYSTICK_FIRMWARE_VERSION_LSB_REG 0x02
#define JOYSTICK_HORIZONTAL_MSB_REG 0x03
#define JOYSTICK_HORIZONTAL_LSB_REG 0x04
#define JOYSTICK_VERTICAL_MSB_REG 0x05
#define JOYSTICK_VERTICAL_LSB_REG 0x06
#define JOYSTICK_BUTTON_REG 0x07

// Function Declarations
void joystick_init(const nrf_twi_mngr_t* i2c);
uint16_t joystick_get_horizontal(void);
uint16_t joystick_get_vertical(void);
uint8_t joystick_get_button(void);

#endif // JOYSTICK_H
