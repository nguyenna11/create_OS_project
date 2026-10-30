#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include <string.h>

static void interface_seconde(uint32_t seconds);
void char_upright(const char *s);
void traitant();
void trap_handler(uint64_t mcause, uint64_t mie, uint64_t mip);
void init_traitant(void (*traitant)(void));
void enable_timer();
 uint32_t nbr_secondes();

#endif