#pragma once

class Chip8 {
public:
    Chip8() {}
    ~Chip8() {}

    void emulateCycle();    // emulates one cycle
    bool loadApplication(const char * filename);    // Load ROM
    
    bool drawFlag;                  // draw flag
    bool soundFlag;                 // sound flag
    unsigned char gfx[64 * 32];     // screen of 2048 = 64 * 32 pixels
    unsigned char key[16];          // HEX based keybad (0x0 - 0xF)
    
    void init();
    
private:
    unsigned short opcode;          // each opcode is 2 bytes long
    
    unsigned char memory[4096];     // 4K of memory
    
    unsigned short stack[16];       // stack with 16 levels
    
    unsigned short SP;              // stack pointer
    
    unsigned char V[16];            // 15 CPU registers (8 bit) V0-VE. VF used for 'carry flag'
    
    unsigned short I;               // index register (16 bit)
    
    unsigned short PC;              // program counter (16 bit)
    
    unsigned char delay_timer;      // delay timer
    
    unsigned char sound_timer;      // sound timer
};
