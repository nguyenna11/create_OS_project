#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <string.h>

void traite_car_uart(char c);
void init_uart(void);
uint64_t init_ecran(void);
void pixel(uint32_t x, uint32_t y, uint32_t couleur);
void place_curseur(uint32_t lig, uint32_t col);
void ecrit_car(uint32_t lig, uint32_t col, char c);
void efface_ecran(void);
void defilement(void);
void traite_car(char c);
void console_putbytes(const char *s, int len);

#endif