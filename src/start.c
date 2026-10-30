#include <stdint.h>
#include <stdio.h>
#include <cpu.h>
#include "uart.h"
#include "time.h"

extern void mon_traitant(void);

/* on peut s'entrainer a utiliser GDB avec ce code de base */
/* par exemple afficher les valeurs de n et res avec la commande display */
/* une fonction bien connue */



void kernel_start(){
    init_uart();
    if (init_ecran() != 0) {
        while (1) {
            hlt();          /* écran PCI introuvable */
        }
    }
    efface_ecran();
    printf("=== Noyau RISC-V demarre ===\n");
    /* on ne doit jamais sortir de kernel_start */
    init_traitant(mon_traitant);
    enable_timer();
    while (1) {
        /* cette fonction arrete le processeur */
        hlt();
    }
}
