#include "Chip8.h"

#include <iostream>
#include <cstdlib>
#include <ctime>

unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

void Chip8::init() {
    // system expects the application to be loaded at memory location 0x200
    PC = 0x200;
    opcode = 0;
    I = 0;
    SP = 0;
    
    // Clear screen, keys, memory & stack
    for (int i = 0; i < 2048; i++)
        gfx[i] = 0;
    for (int i = 0; i < 16; i++)
        key[i] = 0;
    for (int i = 0; i < 4096; i++)
        memory[i] = 0;
    for (int i = 0; i < 16; i++)
        stack[i] = 0;
    
    // Load fontset
    for (int i = 0; i < 80; i++)
        memory[i] = chip8_fontset[i];
    
    // reset timers
    delay_timer = 0;
    sound_timer = 0;
    
    drawFlag = true;
    soundFlag = false;
    
    // Set random seed for RND instruction
    srand(time(NULL));
}

void Chip8::emulateCycle() {
    // Fetch opcode
    opcode = memory[PC] << 8 | memory[PC+1];
    
    unsigned char &Vx = V[(opcode & 0x0F00) >> 8];
    unsigned char &Vy = V[(opcode & 0x00F0) >> 4];
    
    // Decode opcode
    switch(opcode & 0xF000)
    {
        case 0x0000:
            switch (opcode & 0x000F)
            {
                case 0x0000:   // 00E0 - CLS : Clear the display
                    for (int i = 0; i < 2048; i++) {
                        gfx[i] = 0x0;
                    }
                    drawFlag = true;
                    PC += 2;
                    break;
                    
                case 0x000E:   // 00EE - RET : Return from a subroutine.
                    PC = stack[SP];
                    SP--;
                    PC += 2;
                    break;
                    
                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;
            
        case 0x1000:   // 1nnn - JP addr : Jump to location nnn.
            PC = opcode & 0x0FFF;
            break;
            
        case 0x2000:   // 2nnn - CALL addr : Call subroutine at nnn.
            SP++;
            stack[SP] = PC;
            PC = opcode & 0x0FFF;
            break;
            
        case 0x3000:   // 3xkk - SE Vx, byte : Skip next instruction if Vx == kk.
            if (Vx == (opcode & 0x00FF)) {
                PC += 4;
            }
            else {
                PC += 2;
            }
            break;
            
        case 0x4000:   // 4xkk - SNE Vx, byte : Skip next instruction if Vx != kk.
            if (Vx != (opcode & 0x00FF)) {
                PC += 4;
            }
            else {
                PC += 2;
            }
            break;
            
        case 0x5000:   // 5xy0 - SE Vx, Vy : Skip next instruction if Vx == Vy.
            if (Vx == Vy) {
                PC += 4;
            }
            else {
                PC += 2;
            }
            break;
            
        case 0x6000:   // 6xkk - LD Vx, byte : Set Vx = kk.
            Vx = opcode & 0x00FF;
            PC += 2;
            break;
            
        case 0x7000:   // 7xkk - ADD Vx, byte : Set Vx = Vx + kk.
            Vx += opcode & 0x00FF;
            PC += 2;
            break;
            
        case 0x8000:
            switch (opcode & 0x000F)
            {
                case 0x0000:   // 8xy0 - LD Vx, Vy : Set Vx = Vy.
                    Vx = Vy;
                    PC += 2;
                    break;
                    
                case 0x0001:   // 8xy1 - OR Vx, Vy : Set Vx = Vx OR Vy.
                    Vx |= Vy;
                    PC += 2;
                    break;
                    
                case 0x0002:   // 8xy2 - AND Vx, Vy : Set Vx = Vx AND Vy.
                    Vx &= Vy;
                    PC += 2;
                    break;
                    
                case 0x0003:   // 8xy3 - XOR Vx, Vy : Set Vx = Vx XOR Vy.
                    Vx ^= Vy;
                    PC += 2;
                    break;
                    
                case 0x0004:   // 8xy4 - ADD Vx, Vy : Set Vx = Vx + Vy, set VF = carry.
                    if (Vy > (0xFF - Vy)) {
                        V[0xF] = 1;
                    }
                    else {
                        V[0xF] = 0;
                    }
                    Vx += Vy;
                    PC += 2;
                    break;
                    
                case 0x0005:   // 8xy5 - SUB Vx, Vy : Set Vx = Vx - Vy, set VF = NOT borrow.
                    if (Vx > Vy) {
                        V[0xF] = 1;
                    }
                    else {
                        V[0xF] = 0;
                    }
                    Vx -= Vy;
                    PC += 2;
                    break;
                    
                case 0x0006:   // 8xy6 - SHR Vx {, Vy} : Set Vx = SHR 1.
                    V[0xF] = Vx & 0x01;
                    Vx >>= 1;
                    PC += 2;
                    break;
                    
                case 0x0007:   // 8xy7 - SUBN Vx, Vy : Set Vx = Vy - Vx, set VF = NOT borrow.
                    if (Vx < Vy) {
                        V[0xF] = 1;
                    }
                    else {
                        V[0xF] = 0;
                    }
                    Vx = Vy - Vx;
                    PC += 2;
                    break;
                    
                case 0x000E:   // 8xyE - SHL Vx {, Vy} : Set Vx = SHL 1.
                    V[0xF] = Vx >> 7;
                    Vx <<= 1;
                    PC += 2;
                    break;
                
                default:
                    printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
            }
            break;
            
        case 0x9000:   // 9xy0 - SNE Vx, Vy : Skip next instruction if Vx != Vy.
            if (Vx != Vy) {
                PC += 4;
            }
            else {
                PC += 2;
            }
            break;
            
        case 0xA000:    // Annn - LD I, addr : Set I = nnn.
            I = opcode & 0x0FFF;
            PC += 2;
            break;
            
        case 0xB000:    // Bnnn - Jp V0, addr : Jump to location nnn + V0.
            PC = (opcode & 0x0FFF) + V[0];
            break;
            
        case 0xC000:    // Cxkk - RND Vx, byte : Set Vx = random byte AND kk.
            Vx = (rand() % 0xFF) & (opcode & 0x00FF);
            PC += 2;
            break;
            
        case 0xD000:    // Cxyn - DRW Vx, Vy, nibble : Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        {
			unsigned short n = opcode & 0x000F;
			unsigned short pixel;
            
			V[0xF] = 0;
			for (int yline = 0; yline < n; yline++)
			{
				pixel = memory[I + yline];
				for(int xline = 0; xline < 8; xline++)
				{
					if((pixel & (0x80 >> xline)) != 0)
					{
						if(gfx[(Vx + xline + ((Vy + yline) * 64))] == 1)
						{
							V[0xF] = 1;
						}
                        int index = Vx + xline + ((Vy + yline) * 64);
                        if (index >= 0 && index < 2048) {
                            gfx[index] ^= 1;
                        }
						
					}
				}
			}
            
			drawFlag = true;
			PC += 2;
		}
            break;
            
        case 0xE000:
            switch (opcode & 0x00FF)
            {
                case 0x009E:    // Ex9E - SKP Vx : Skip next instruction if key with the value of Vx is pressed.
                    if (key[Vx] != 0) {
                        PC += 4;
                    }
                    else {
                        PC += 2;
                    }
                    break;
                    
                case 0x00A1:    // ExA1 - SKNP Vx : Skip next instruction if key with the value of Vx is not pressed.
                    if (key[Vx] == 0) {
                        PC += 4;
                    }
                    else {
                        PC += 2;
                    }
                    break;
                    
                default:
                    printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
            }
            break;
            
        case 0xF000:
            switch (opcode & 0x00FF)
            {
                case 0x0007:    // Fx07 - LD Vx, DT : Set Vx = delay timer value.
                    Vx = delay_timer;
                    PC += 2;
                    break;
                    
                case 0x000A:    // Fx0A - LD Vx, K : Wait for a key press, store the value of the key in Vx.
                {
                    bool keyPress = false;
                    
                    for (int i = 0; i < 16; i++) {
                        if (key[i] != 0) {
                            Vx = i;
                            keyPress = true;
                        }
                    }
                    
                    if (!keyPress) {
                        return;
                    }
                    
                    PC += 2;
                }
                    break;
                    
                case 0x0015:    // Fx15 - LD DT, Vx : Set delay timer = Vx.
                    delay_timer = Vx;
                    PC += 2;
                    break;
                    
                case 0x0018:    // Fx18 - LD ST, Vx : Set sound timer = Vx.
                    sound_timer = Vx;
                    PC += 2;
                    break;
                    
                case 0x001E:    // Fx1E - ADD I, Vx : Set I = I + Vx.
                    if (I + Vx > 0xFFF) {
                        V[0xF] = 1;
                    }
                    else {
                        V[0xF] = 0;
                    }
                    I += Vx;
                    PC += 2;
                    break;
                    
                case 0x0029:    // Fx29 - LD F, Vx : Set I = location of sprite for digit Vx.
                    I = Vx * 0x5;
                    PC += 2;
                    break;
                    
                case 0x0033:    // Fx33 - LD B, Vx : Store BCD representation of Vx in memory locations I, I+1, I+2.
                    memory[I]     = Vx / 100;
					memory[I + 1] = (Vx / 10) % 10;
					memory[I + 2] = (Vx % 100) % 10;
					PC += 2;
                    break;
                    
                case 0x0055:    // Fx55 - LD [I], Vx : Store registers V0 through Vx in memory at location I.
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        memory[I + i] = V[i];
                    }
//                    I += ((opcode & 0x0F00) >> 8) + 1;
                    PC += 2;
                    break;
                    
                case 0x0065:    // Fx65 - LD Vx, [I] : Read registers V0 through Vx from memory starting at location I.
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        V[i] = memory[I + i];
                    }
//                    I += ((opcode & 0x0F00) >> 8) + 1;
                    PC += 2;
                    break;
                    
                default:
                    printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
            }
            break;
            
        default:
            printf("Unknown opcode: 0x%X\n", opcode);
    }
    
    // Update timers
    if (delay_timer > 0) {
        delay_timer--;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            soundFlag = true;
        }
        sound_timer--;
    }
}

bool Chip8::loadApplication(const char * filename) {
    init();
    
    printf("Loading: %s\n", filename);
    
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("File error\n");
        return false;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    printf("Filesize: %d\n", (int)size);
    
    char *buffer = (char *)malloc(sizeof(char) * size);
    if (buffer == NULL) {
        printf("Memory error\n");
        return false;
    }
    
    size_t result = fread(buffer, 1, size, file);
    if (result != size) {
        printf("Reading error\n");
        return false;
    }
    
    if ((4096 - 512) > size) {
        for (int i = 0; i < size; i++) {
            memory[i + 512] = buffer[i];
        }
    }
    else {
        printf("ROM too big for memory.\n");
    }
    
    fclose(file);
    free(buffer);
    
    return true;
}
