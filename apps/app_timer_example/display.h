#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "font.h"

#include "nrf_delay.h"
#include "nrfx_spim.h"

#include "microbit_v2.h"

// D/C (red wire) is pin 11
// CS (white wire, chip select) is pin 12
#define DC_PIN EDGE_P11
#define CS_PIN EDGE_P12
// multiple of 5
#define LET_WIDTH 25
#define LET_REPEAT LET_WIDTH / 5
#define LET_GAP 3

#define WORD_COL 50
#define TIMER_COL 190

#define TOT_ROW 320
#define TOT_COL 240

// static void send_command(uint8_t);
// static void send_data(uint8_t);
// static void send_data_repeated(uint8_t, uint16_t);
// static void send_data16(uint16_t);
// static void send_data24(uint8_t, uint8_t, uint8_t);
// static void send_data32(unsigned int);
// static void prepare_for_draw(uint16_t, uint16_t, uint16_t, uint16_t);
// static void clear_screen();
// static void draw_letter(char, uint16_t,  uint16_t);
// static void draw_word(const char*, uint16_t);
void draw_phrase(char*, uint8_t, uint8_t);
void draw_timer(uint8_t);
// static void read_and_print(uint8_t);
void spim_init(void);
void display_init(void);
