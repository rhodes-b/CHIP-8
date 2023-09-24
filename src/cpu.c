#include "cpu.h"
#include "stdio.h"

static struct cpu_s vm = {0}; 

const uint8_t sprites[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


void load_sprites() {

    for(size_t i=0; i < sizeof(sprites); i++) {
        vm.memory[i] = sprites[i];
    }
}

void load_rom(const char** rom) {

    for(size_t i=0; sizeof(rom); i++) {
        vm.memory[512 + i] = 0; // TODO: load rom
    }
}

void initalize_vm() {
    load_sprites();
    // load_rom();
    vm.pc = 512;
}

uint16_t fetch_opcode() {
    return vm.memory[vm.pc] << 8 | vm.memory[vm.pc]; 
}

void execute_opcode(uint16_t opcode) { 
    // x,y are 4 bit register locations for V
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    // raw number values were an n represents 4bits of data, so nn is 8, nnn is 16
    uint16_t nnn = (opcode & 0x0FFF);
    uint8_t  nn  = (opcode & 0x00FF);
    uint8_t  n   = (opcode & 0x000F); 

    switch(opcode & 0x0F00) {
        case 0x0000:
            switch(opcode & 0xFF) {
                case 0xE0: // clear display
                    break;
                case 0xEE: // return (set pc to value popped from sp)
                    vm.pc = vm.stack[vm.sp];
                    vm.sp--;
                    break;
            }
            break;
        case 0x1000: //  0x1NNN jmp
            vm.pc = nnn;
            break;
        case 0x2000: // 0x2NNN call
            vm.stack[vm.sp+1] = nnn;
            vm.sp++;
            break; 
        case 0x3000: // 0x3XNN cond VX eq NN (skip next instruction if true)
            if(vm.V[x] == nn) { vm.pc += 2; }
            break;
        case 0x4000: // 0x4XNN cond VX nq NN (skip next instruction if true)
            if(vm.V[x] != nn) { vm.pc += 2; }
            break;
        case 0x5000: // 0x5XY0 cond VX eq VY (skip next instruction if true)
            if(vm.V[x] == vm.V[y]) { vm.pc += 2; }
            break;
        case 0x6000: // 0x6XNN set VX to NN
            vm.V[x] = nn;
            break;
        case 0x7000: // 0x7XNN add NN to VX (dont update carry flag)
            vm.V[x] += nn;
            break; 
        case 0x8000: // 0x8XYN
            switch(opcode & 0xF) {
                case 0: // assign VX = VY
                    vm.V[x] = vm.V[y];
                    break;
                case 0x1: // bitor  VX = VX | VY
                    vm.V[x] |= vm.V[y];
                    break;
                case 0x2: // bitand VX = VX & VY
                    vm.V[x] &= vm.V[y];
                    break;
                case 0x3: // bitxor VX = VX ^ VY
                    vm.V[x] ^= vm.V[y];
                    break;
                // make sure borrow, and carry mean over/underflow (it depends)
                case 0x4: // add VX = VX + VY, if carry set VF to 1
                    if((int16_t)(vm.V[x] + vm.V[y]) > 0xFF) { vm.V[0xF] = 1; }
                    vm.V[x] += vm.V[y];
                    break;
                case 0x5: // sub VX = VX - VY, if borrow VF is 0 else 1
                    vm.V[0xF] = (int16_t)(vm.V[x] - vm.V[y]) < 0 ? 0 : 1;
                    vm.V[x] -= vm.V[y];
                    break;
                case 0x6: // bitop VF = VX & 0x1 (lsb), VX = VX >> 1 
                    vm.V[0xF] = vm.V[x] & 0x1;
                    vm.V[x] >>= 1;
                    break;
                case 0x7: // sub VX = VY - VX, if borrow VF is 0 else 1
                    vm.V[0xF] = (int16_t)(vm.V[y] - vm.V[x]) < 0 ? 0 : 1;
                    vm.V[y] -= vm.V[x];
                    break;
                case 0xE: // bitop VF = VX & 0x80 (msb), VX = VX << 1
                    vm.V[0xF] = (vm.V[x] & 0x80) >> 7;
                    vm.V[x] <<= 1;
                    break;
            }
            break;
        case 0x9000: // 0x9XY0 cond VX nq VY
            if(vm.V[x] != vm.V[y]) { vm.pc += 2; }
            break;
        case 0xA000: // 0xANNN set I = NNN
            vm.I = nnn;
            break;
        case 0xB000: // 0xBNNN set PC = V0 + NNN
            vm.pc = vm.V[0] + nnn;
            break;
        case 0xC000: // 0xCXNN set VX = rand & NN  (rand is range 0-255)
            // TODO: (need rand)
            break;
        case 0xD000: // draw sprite draw(VX, VY, N) 
        // draw sprite at VX, VY width of 8 pixels height of N, VF is set to 1 if any pixels go 
        // from drawn to undrawn and 0 if this does not happen (pixels are XOR into memeory)
            break;
        case 0xE000:
            switch(opcode & 0xFF) { // TODO: fix
                case 0x9E: // cond key() eq VX (if key pressed is stored in VX)
                    break;
                case 0xA1: // cond key() ne VX (if key pressed is not stored in VX)
                    break;
            }
            break;
        case 0xF000: // 0xFX
            switch(opcode & 0xFF) {
                case 0x07: // set VX to delay timer VX = get_delay()
                    break;
                case 0x0A: // set VX to get_key, VX = get_key() (blocking operation)
                    break;
                case 0x15: // set delay_timer(VX)
                    break;
                case 0x18: // set sound_timer(VX)
                    break;
                case 0x1E: // add VX to I, I = I + VX (VF unaffected)
                    vm.I += vm.V[x];
                    break;
                case 0x29: // I = sprite_addr(VX)
                    vm.I = vm.memory[vm.V[x]];
                    break;
                case 0x33: // store binary coded decimal of VX, I = hundreds digit, I+1 = tenths, I+2 = ones
                    // TODO: is I inc?
                    vm.memory[vm.I] =   (vm.V[x] % 1000) / 100;
                    vm.memory[vm.I+1] = (vm.V[x] % 100) / 10;
                    vm.memory[vm.I+2] = vm.V[x]  % 10;
                    break;
                case 0x55: // reg_dump(VX, &I) stores V0 to VX in memory starting at addr I (I is not modified)
                    for(int i=0; i < x; i++) {
                        vm.memory[vm.I+i] = vm.V[x];
                    }
                    break;
                case 0x65: // reg_load(VX, &I) load V0 to VX from memory at addr I (I is not modified)
                    for(int i=0; i < x; i++) {
                        vm.V[i] = vm.memory[vm.I+i];
                    }
                    break;
            }
            break;
    }
    vm.pc += 2;
}

int main(int argc, char** argv) {

    printf("Hello World\n");

    initalize_vm();

    for(;;) {
        uint16_t opcode = fetch_opcode();
        execute_opcode(opcode);
    }
    // in fetch we need to get memory[pc] << 8 | memory[pc+1] this is because opcodes are 16bit
    return 0;
}