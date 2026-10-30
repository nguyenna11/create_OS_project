#include "platform.h"
#include "uart.h"
#include "cpu.h"
#include "time.h"
#include <string.h>

static volatile uint32_t timer_ticks;

static void interface_seconde(uint32_t seconds){
    uint32_t hours = (seconds / 3600) % 100;
    uint32_t minutes = (seconds / 60) % 60;
    uint32_t remaining_seconds = seconds % 60;
    char text[9] = {
        (char)('0' + hours / 10),
        (char)('0' + hours % 10),
        ':',
        (char)('0' + minutes / 10),
        (char)('0' + minutes % 10),
        ':',
        (char)('0' + remaining_seconds / 10),
        (char)('0' + remaining_seconds % 10),
        '\0'
    };
    char_upright(text);
}

void char_upright(const char *s){
    uint32_t max_cols = DISPLAY_WIDTH/8;
    size_t len = strlen(s);
    if (len>max_cols){
        s+=len - max_cols;
        len = max_cols;
    }
    uint32_t col = max_cols - (uint32_t)len;
    for (int i = 0 ; i < (int)len;i++){
        ecrit_car(0,col+(uint32_t)i,s[i]);
    }
}

void trap_handler(uint64_t mcause, uint64_t mie, uint64_t mip){
    (void)mie;
    (void)mip;
    uint64_t cause = mcause & 0x7fffffffffffffff;
    if (cause == IRQ_M_TMR) {
        volatile uint64_t *mtime = (volatile uint64_t *)(uintptr_t)CLINT_TIMER;
        volatile uint64_t *mtimecmp = (volatile uint64_t *)(uintptr_t)CLINT_TIMER_CMP;
        *mtimecmp = *mtime + TIMER_FREQ / IT_FREQ;

        timer_ticks++;
        if (timer_ticks % IT_FREQ == 0) {
            interface_seconde(timer_ticks / IT_FREQ);
        }
    }
}
void traitant(){

}
extern void mon_traitant(void);

 void init_traitant(void(*traitant)(void)){
        __asm__("csrw mtvec, %0":: "r"(traitant));
 }

 void enable_timer(){
    volatile uint64_t *mtime = (volatile uint64_t *)(uintptr_t)CLINT_TIMER;
    volatile uint64_t *mtimecmp = (volatile uint64_t *)(uintptr_t)CLINT_TIMER_CMP;
    *mtimecmp = *mtime + TIMER_FREQ / IT_FREQ;
    uint64_t mask = (unsigned long) 0x01 << IRQ_M_TMR;
    __asm__ volatile ("csrs mie, %0" : : "r"(mask) : "memory");
    enable_it();
 }
 uint32_t nbr_secondes(){
    return timer_ticks / IT_FREQ;
 }
