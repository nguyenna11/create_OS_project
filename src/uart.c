#include "platform.h"
#include "font.h"
#include <stdint.h>
#include <string.h>

#define COULEUR_TEXTE 0x00FFFFFF  // Blanc
#define COULEUR_FOND  0x00000000 //Fond Bleu

static uint32_t (*const display_base)[DISPLAY_HEIGHT][DISPLAY_WIDTH] = (uint32_t (*const)[DISPLAY_HEIGHT][DISPLAY_WIDTH]) BOCHS_DISPLAY_BASE_ADDRESS;

static uint32_t curseur_lig = 1;
static uint32_t curseur_col = 0;

void traite_car_uart(char c) {
    if (' ' <= c && c <= '~') {
        volatile uint8_t *uart = (volatile uint8_t *) UART_BASE;
        uart[UART_THR] = (uint8_t) c;
    }
}
uint64_t init_uart(){
    volatile  uint8_t *uart= (volatile uint8_t *) UART_BASE;
    uart[UART_LCR]=UART_LCR_DLAB;
    uint16_t divisor = UART_CLOCK_FREQ / (16 * UART_BAUD_RATE);
    uart[UART_DLH]= (divisor>>8) &0xFF;
    uart[UART_DLL]=divisor & 0xFF;
    uart[UART_FCR]=0x01;
    uart[UART_LCR]=UART_LCR_8BIT| UART_LCR_PODD;
}


uint64_t init_ecran(){
    for (uint32_t device = 0; device<32; device ++){
        uint64_t adress = PCI_ECAM_BASE_ADDRESS + (device<<15); 
        if (*(uint32_t *)adress == DISPLAY_PCI_ID){
            *(uint32_t*)(adress +0x04)= 0b111;
            *(uint32_t*)(adress +0x10)=BOCHS_DISPLAY_BASE_ADDRESS;
            *(uint32_t*)(adress +0x18)=BOCHS_CONFIG_BASE_ADDRESS;
            uint64_t dispi = BOCHS_CONFIG_BASE_ADDRESS +(0x500);
            uint16_t type_id = *(uint16_t*)(dispi+0x00);
            if ((type_id & 0xFFF0)!=0xB0C0){
                return 1;
            } 
            *(uint16_t*) (dispi + 0x08) = VBE_DISPI_DISABLED;
            *(uint16_t*) (dispi + 0x002) = DISPLAY_WIDTH;
            *(uint16_t*) (dispi + 0x004) = DISPLAY_HEIGHT;
            *(uint16_t*) (dispi +0x006) = DISPLAY_BPP;
            *(uint16_t*) (dispi + 0x00a) = 0;
            *(uint16_t*) (dispi + 0x010) = 0;
            *(uint16_t*) (dispi +0x012) =0;
            *(uint16_t*) (dispi + 0x08) = VBE_DISPI_LFB_ENABLED | VBE_DISPI_ENABLED;
            return 0;
        }
    }
    return 1;
}

void pixel(uint32_t x, uint32_t y, uint32_t couleur){
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
        return;
    (*display_base)[y][x] = couleur;
}

void ecrit_car(uint32_t lig, uint32_t col, char c){
    uint32_t x = 8*col ;
    uint32_t y = 8*lig;
    for (uint32_t i = 0 ; i<8 ; i++ ){
        uint8_t octet = font8x8_basic[(unsigned char)c][i];
        for (uint32_t j = 0; j < 8; j++) {
            if ((octet >> j) & 1) {
                pixel(x + j, y + i, COULEUR_TEXTE);
            }
            else {
                pixel(x + j, y + i, COULEUR_FOND);
            }
        }
    }   
}

void place_curseur(uint32_t lig, uint32_t col){
    if (lig >= (DISPLAY_HEIGHT / 8) || col >= (DISPLAY_WIDTH / 8)) {
        return;
    }
    curseur_lig = lig;
    curseur_col = col;
    uint32_t old_y = curseur_lig * 8 + 7;
    uint32_t old_x = curseur_col * 8;    
    for (uint32_t i = 0; i < 8; i++) {
        pixel(old_x + i, old_y, COULEUR_FOND);
    }    
    uint32_t new_y = lig * 8 + 7;
    uint32_t new_x = col * 8;    
    for (uint32_t i = 0; i < 8; i++) {
        pixel(new_x + i, new_y, COULEUR_TEXTE);
    }
}

void efface_ecran(){
    for (uint32_t y = 0; y < DISPLAY_HEIGHT; y++){
        for (uint32_t x = 0; x < DISPLAY_WIDTH; x++){
            pixel(x, y, 0);
        }
    }
    place_curseur(1,0);
}

void defilement() {
    uint32_t *fb = (uint32_t *) BOCHS_DISPLAY_BASE_ADDRESS;
    uint32_t *dest = fb + (1 * 8 * DISPLAY_WIDTH);
    uint32_t *src = fb + (2 * 8 * DISPLAY_WIDTH);
    size_t taille = (size_t)(DISPLAY_HEIGHT / 8 - 2) * 8 * DISPLAY_WIDTH * sizeof(uint32_t);
    memmove(dest, src, taille);
    uint32_t y_start = DISPLAY_HEIGHT - 8;
    for (uint32_t y = y_start; y < DISPLAY_HEIGHT; y++) {
        for (uint32_t x = 0; x < DISPLAY_WIDTH; x++) {
            pixel(x, y, COULEUR_FOND);
        }
    }
}

    void traite_car(char c) {
    uint32_t max_cols = DISPLAY_WIDTH / 8;  
    uint32_t max_ligs = DISPLAY_HEIGHT / 8;  
    if (c >= 32 && c <= 126) {
        ecrit_car(curseur_lig, curseur_col, c);
        uint32_t nxt_col = curseur_col + 1;
        uint32_t nxt_lig = curseur_lig;
        if (nxt_col >= max_cols) {
            nxt_col = 0;
            nxt_lig++;
        }
        if (nxt_lig >= max_ligs) {
            defilement();
            nxt_lig = max_ligs - 1;
        }

        place_curseur(nxt_lig, nxt_col);
    } 
    else {
        switch (c) {
            case '\b': 
                if (curseur_col > 0) {
                    place_curseur(curseur_lig, curseur_col - 1);
                }
                break;

            case '\t': 
                {
                    uint32_t nxt_col = (curseur_col + 8) & ~7; // Prochaine colonne multiple de 8
                    uint32_t nxt_lig = curseur_lig;

                    if (nxt_col >= max_cols) {
                        nxt_col = 0;
                        nxt_lig++;
                    }

                    if (nxt_lig >= max_ligs) {
                        defilement();
                        nxt_lig = max_ligs - 1;
                    }

                    place_curseur(nxt_lig, nxt_col);
                }
                break;

            case '\n': 
                {
                    uint32_t nxt_lig = curseur_lig + 1;
                    if (nxt_lig >= max_ligs) {
                        defilement();
                        nxt_lig = max_ligs - 1;
                    }
                    place_curseur(nxt_lig, 0);
                }
                break;
            case '\f': 
                efface_ecran(); 
                break;

            case '\r': 
                place_curseur(curseur_lig, 0);
                break;

            default:
                break;
        }
    }
}


void console_putbytes(const char *s, int len){
    if (s == NULL || len <= 0) {
        return;
    }
    for (int i = 0; i < len; i++) {
        traite_car_uart(s[i]);
        traite_car(s[i]);
    }
}