
#include "display.h"

static uint8_t SCREEN_CLEAR[TOT_ROW*2];

static const nrfx_spim_t SPIM_INST = NRFX_SPIM_INSTANCE(2);

// Function to send a command to the display, used for instructing the display
//  to perform a certain operation or to set it into a specific state.
static void send_command(uint8_t comm) {
   
   // Set the Data/Command pin to low to indicate a command is being sent.
   nrf_gpio_pin_write(DC_PIN, 0);

   // Prepare a one-byte array with the command to be sent.
   uint8_t data[1] = {comm};

   // Set up the SPI transfer description
   // This specifies that we are sending data (TX) and the length of the data (1 byte)
   nrfx_spim_xfer_desc_t data_xfer = NRFX_SPIM_XFER_TX(&data, 1);

   // Perform the SPI transfer using the configured SPI instance (SPIM_INST)
   // and the transfer description (data_xfer). The last parameter (0) indicates
   // that no flags are used for this transfer
   nrfx_spim_xfer(&SPIM_INST, &data_xfer, 0);
}

// Send 8 bits of data, used for sending the actual content 
// (like pixel data) that is to be displayed. 
static void send_data(uint8_t dat) {

   // Indicate that we are sending data by setting DC high
   nrf_gpio_pin_write(DC_PIN, 1);

   // Prepare the data for transfer
   uint8_t data[1] = {dat};
   nrfx_spim_xfer_desc_t data_xfer = NRFX_SPIM_XFER_TX(&data, 1);

   // Transfer the data
   nrfx_spim_xfer(&SPIM_INST, &data_xfer, 0);
}

static void send_data_repeated(uint8_t dat1, uint8_t dat2, uint16_t times) {
   nrf_gpio_pin_write(DC_PIN, 1);

   // OLD CODE
   uint8_t data[2*times];
   for (int i = 0; i < times; i++) {
      data[2*i] = dat1;
      data[2*i+1] = dat2;
   }
   nrfx_spim_xfer_desc_t data_xfer = NRFX_SPIM_XFER_TX(&data, 2*times);
   nrfx_spim_xfer(&SPIM_INST, &data_xfer, 0);
}

// Send 16 bits of data 
static void send_data16(uint16_t dat) {
   nrf_gpio_pin_write(DC_PIN, 1);
   uint8_t data[2] = {(uint8_t)(dat >> 8), (uint8_t)((dat << 8) >> 8)};
   nrfx_spim_xfer_desc_t data_xfer = NRFX_SPIM_XFER_TX(&data, 2);
   nrfx_spim_xfer(&SPIM_INST, &data_xfer, 0);
}

// Send 24 bits of data 
static void send_data24(uint8_t dat1, uint8_t dat2, uint8_t dat3) {
   nrf_gpio_pin_write(DC_PIN, 1);
   uint16_t data[3] = {dat1, dat2, dat3};
   nrfx_spim_xfer_desc_t data_xfer = NRFX_SPIM_XFER_TX(&data, 3);
   nrfx_spim_xfer(&SPIM_INST, &data_xfer, 0);
}

// Send 32 bits of data 
static void send_data32(unsigned int dat) {
   nrf_gpio_pin_write(DC_PIN, 1);
   unsigned int data[1] = {dat};
   nrfx_spim_xfer_desc_t data_xfer = NRFX_SPIM_XFER_TX(&data, 1);
   nrfx_spim_xfer(&SPIM_INST, &data_xfer, 0);
}

// Function to set up the display for drawing pixels in a specified area
static void prepare_for_draw(uint16_t width, uint16_t height, uint16_t st_row, uint16_t st_col) {
    // Send the command to set the column address (0x2A)
    // This command is used to define the horizontal area of the frame memory.
    send_command(0x2A);

    // Send the starting row address.
    // This is the initial address of the frame memory row where drawing will begin.
    send_data16(st_row);

    // Send the ending row address.
    // It defines the final row of the frame memory area for drawing.
    send_data16(st_row + height - 1);

    // Send the command to set the page address (0x2B)
    // This command is used to define the vertical area of the frame memory.
    send_command(0x2B);

    // Send the starting column address.
    // This is the initial address of the frame memory column where drawing will begin.
    send_data16(st_col);

    // Send the ending column address.
    // It defines the final column of the frame memory area for drawing.
    send_data16(st_col + width - 1);

    // Send the command to write to memory (0x2C)
    // After this command, the data sent be written to the frame memory
    // in the specified row and column address range.
    send_command(0x2C);
}

// Function to clear the entire screen of the display
static void clear_screen() {
    // Set up the display for drawing across the entire screen.
    // The entire screen is defined as a rectangle from (0,0) to (TOT_COL, TOT_ROW).
    prepare_for_draw(TOT_COL, TOT_ROW, 0, 0);

    // Set the Data/Command pin to high, indicating the following SPI transfers will be data.
    nrf_gpio_pin_write(DC_PIN, 1);

    // Loop over each column to clear the screen.
    // This loop will send the clear color to each pixel in each column.
    for (int j = 0; j < TOT_COL; j++) {
        // Prepare the SPI transfer to send the clear data for one entire row.
        nrfx_spim_xfer_desc_t SCREEN_TX = NRFX_SPIM_XFER_TX(&SCREEN_CLEAR, TOT_ROW*2);

        // Perform the SPI transfer using the configured SPI instance (SPIM_INST)
        // and the transfer description (SCREEN_TX).
        nrfx_spim_xfer(&SPIM_INST, &SCREEN_TX, 0);
    }

    // Send a command to conclude the clearing process.
    send_command(0x00);
}

// Function to draw letter
static void draw_letter(char letter, uint16_t row_coord,  uint16_t col_coord, uint8_t c1, uint8_t c2) {
   // setting up the rows and cols
   prepare_for_draw(LET_WIDTH, LET_WIDTH, row_coord, col_coord);
   
   bool curr = 0;
   // row, col, row_rep, col_rep
   for (int row = 0; row < 5; row++) {
      uint8_t r = font[letter][row];
      for (int row_rep = 0; row_rep < LET_REPEAT; row_rep++) {
         for (int col = 0; col < 5; col++) {
            curr = ((r >> (col)) & 1);

            // Send the correct data (draw a part of the letter if curr == 1)
            if (curr) {
              send_data_repeated(c1, c2, LET_REPEAT);
            }
            else {
              send_data_repeated(0x00, 0x00, LET_REPEAT);
            }
         }
      }
   }
   
   // finishing 
   send_command(0x00);
}

// Function to draw word
static void draw_word(const char* s, uint16_t col_st, uint8_t c1, uint8_t c2) {
   int length = 0;
   while (s[length] != '\0') {
      length += 1;
   }
   // determining margins 
   uint16_t row_st = (TOT_ROW - (LET_WIDTH + 2*LET_GAP) * length) / 2 + LET_GAP;

   // drawing letter one by one
   for (int i = 0; i < length; i++) {
      draw_letter(s[i], row_st, col_st, c1, c2);
      row_st += (LET_WIDTH + 2*LET_GAP);
   }
}

// Function to draw phrase
void draw_phrase(char* phrase, uint8_t c1, uint8_t c2) {
   // Clears Screen
   clear_screen();

   int16_t col = WORD_COL;
   int total_length = 0;
   int curr_length = 0;

   // Iterate through the given phrase
   for (int i = 0; phrase[i] != '\0'; i++) {

      // If a space character is found, then draw the word so that we can draw
      // the next word below it
      if (phrase[i] == ' ') {
         char arr[curr_length+1];
         for (int j = curr_length; j > 0; j--) {
            arr[curr_length - j] = phrase[i - j];
         }
         arr[curr_length] = '\0';
         draw_word(arr, col, c1, c2);
         col += LET_WIDTH + 10;
         curr_length = 0;
      } else {
         curr_length++;
      }
      total_length++;
   }

   // Draw the final word
   char arr[curr_length+1];
   for (int j = curr_length; j > 0; j--) {
      arr[curr_length - j] = phrase[total_length - j];
   }

   arr[curr_length] = '\0';
   draw_word(arr, col, c1, c2);
}

// Function to draw timer onto the display
void draw_timer(uint8_t time) {
   char temp_str[10];
   sprintf(temp_str, "Timer: %2.2u", time);
   if (time > 5) {
      draw_word(temp_str, TIMER_COL, 0xFF, 0xFF);
   }
   else {
      draw_word(temp_str, TIMER_COL, 0x00, 0xFF);
   }
   
}

void draw_lives(uint8_t num_lives) {
   char temp_str[6];
   for (int i = 0; i < 5; i++) {
      if (i % 2 == 1) {
         temp_str[i] = ' ';
      }
      else {
         if (i >= 6 - 2*num_lives) {
            temp_str[i] = '+';
         }
         else {
            temp_str[i] = ' ';
         }  
      }
   }
   temp_str[5] = '\0';
   draw_word(temp_str, 0, 0x00, 0xFF);
}

static void read_and_print(uint8_t bytes) {
   uint8_t data[bytes];

   // Initialize 'data' array with 1's
   for (int i = 0; i < bytes; i++) {
      data[i] = 1;
   }

   // Data transfer 
   nrfx_spim_xfer_desc_t data_xfer = NRFX_SPIM_XFER_RX(&data[0], bytes);
   nrfx_spim_xfer(&SPIM_INST, &data_xfer, 0);
   for (int i = 0; i < bytes; i++) {
      printf("Parameter %i: %u\n", i, data[i]);
   }
}

void spim_init(void) {
  // Note: doesn't specify a chip-select pin
  // That should probably be manually controlled via the GPIO library
  nrfx_spim_config_t spim_config = NRFX_SPIM_DEFAULT_CONFIG;
  spim_config.sck_pin = EDGE_P13;
  spim_config.miso_pin = EDGE_P14;
  spim_config.mosi_pin = EDGE_P15;
  spim_config.irq_priority = 0;
  spim_config.frequency = NRF_SPIM_FREQ_8M;
  spim_config.mode = NRF_SPIM_MODE_0;
  spim_config.bit_order = NRF_SPIM_BIT_ORDER_MSB_FIRST;
  
  // configuring D/C and CS
  // D/C is a 1 for data transfers, a 0 for command.
  // 11 is D/C, 12 is CS
  nrf_gpio_cfg_output(DC_PIN);
  nrf_gpio_cfg_output(CS_PIN);
  nrf_gpio_pin_write(DC_PIN, 0);
  nrf_gpio_pin_write(CS_PIN, 0);

  // Note: without a callback handler, transfers are blocking
  nrfx_spim_init(&SPIM_INST, &spim_config, NULL, NULL);
}

// Initialize Display
void display_init(void) {
   for (int i = 0; i < TOT_ROW*2; i++) {
      SCREEN_CLEAR[i] = 0x00;
   }
   send_command(0x01);
   send_command(0x29); // display on
   send_command(0x11); // sleep out
   send_command(0x3A); // change color mode
   send_data(0x55);
   send_command(0x36); // Change memory access control
   send_data(0b00100000); // Reverse row and col
}
