#include "joystick.h"
#include <stdbool.h>
#include "nrf_delay.h"
#include <stdio.h> // Include for printf

// Pointer to an initialized I2C instance
static const nrf_twi_mngr_t* i2c_manager = NULL;

// I2C read and write helper functions
static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr);

// Initialize the joystick
void joystick_init(const nrf_twi_mngr_t* i2c) {
    i2c_manager = i2c;

    // Check if the joystick is responding by reading the firmware version
    uint8_t firmware_msb = i2c_reg_read(JOYSTICK_I2C_ADDRESS, JOYSTICK_FIRMWARE_VERSION_MSB_REG);
    uint8_t firmware_lsb = i2c_reg_read(JOYSTICK_I2C_ADDRESS, JOYSTICK_FIRMWARE_VERSION_LSB_REG);
    printf("Joystick Firmware Version: %d.%d\n", firmware_msb, firmware_lsb);

    // Additional initialization steps can be added here if required
}

// Read the joystick's horizontal position
uint16_t joystick_get_horizontal() {
    uint8_t x_msb = i2c_reg_read(JOYSTICK_I2C_ADDRESS, JOYSTICK_HORIZONTAL_MSB_REG);
    uint8_t x_lsb = i2c_reg_read(JOYSTICK_I2C_ADDRESS, JOYSTICK_HORIZONTAL_LSB_REG);
    return ((x_msb << 8) | x_lsb) >> 6;
}

// Read the joystick's vertical position
uint16_t joystick_get_vertical() {
    uint8_t y_msb = i2c_reg_read(JOYSTICK_I2C_ADDRESS, JOYSTICK_VERTICAL_MSB_REG);
    uint8_t y_lsb = i2c_reg_read(JOYSTICK_I2C_ADDRESS, JOYSTICK_VERTICAL_LSB_REG);
    return ((y_msb << 8) | y_lsb) >> 6;
}

// Read the joystick button state
uint8_t joystick_get_button() {
    return i2c_reg_read(JOYSTICK_I2C_ADDRESS, JOYSTICK_BUTTON_REG);
}

// Helper function to perform a 1-byte I2C read
static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr) {
    uint8_t rx_buf = 0;
    nrf_twi_mngr_transfer_t const read_transfer[] = {
        NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0)
    };

    ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);
    if (result != NRF_SUCCESS) {
        printf("I2C transaction failed! Error: %lX\n", result);
    }

    return rx_buf;
}
