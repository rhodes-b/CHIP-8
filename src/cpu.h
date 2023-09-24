#ifndef _CHIP_8_CPU_H
#define _CHIP_8_CPU_H

#include <stdint.h>

enum VREGS {
    V0,
    V1,
    V2,
    V3,
    V4,
    V5,
    V6,
    V7,
    V8,
    V9,
    VA,
    VB,
    VC,
    VD,
    VE,
    VF,
    VCOUNT
};


struct cpu_s {
    uint8_t memory[4096];
    uint8_t V[VCOUNT];
    uint8_t stack[512];  // this can be any size we want to store stack pointers
    uint8_t sp;
    uint8_t pc;
    uint16_t I; // memory access register
};

#endif /* _CHIP_8_CPU_H */