//
//  Chip-8 Emulator based on this tutorial: http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
//

#include <SFML/Graphics.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>

#include <iostream>

#include "Chip8.h"

using namespace sf;
using namespace std;

const int canvasWidth = 64;
const int canvasHeight = 32;
const int scale = 8;

RenderWindow window(VideoMode(scale * canvasWidth, scale * canvasHeight), "Chip-8");

string ending = ".ch8";
string name = "Pong (1 player)";
string rom = name + ending;

struct Emulator
{
    Chip8 chip8;
    Image canvas;
    
    bool hasSound;
    SoundBuffer buffer;
    Sound beep;
    
    bool running;
    
    Emulator() {
        running = false;
        
        window.setFramerateLimit(60);
        
        if (!buffer.loadFromFile("beep.wav")) {
            hasSound = false;
        }
        hasSound = true;
        beep.setBuffer(buffer);
    }
    
    void run() {
        chip8.loadApplication(rom.c_str());
        
        running = true;
        
        while (running) {
            input();
            update();
            draw();
            sound();
        }
    }
    
    void input() {
        Event event;
        while(window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                running = false;
                break;
            }
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) running = false;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::Num1)) chip8.key[0x1] = 1;
        else chip8.key[0x1] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::Num2)) chip8.key[0x2] = 1;
        else chip8.key[0x2] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::Num3)) chip8.key[0x3] = 1;
        else chip8.key[0x3] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::Num4)) chip8.key[0xC] = 1;
        else chip8.key[0xC] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::Q)) chip8.key[0x4] = 1;
        else chip8.key[0x4] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::W)) chip8.key[0x5] = 1;
        else chip8.key[0x5] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::E)) chip8.key[0x6] = 1;
        else chip8.key[0x6] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::R)) chip8.key[0xD] = 1;
        else chip8.key[0xD] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::A)) chip8.key[0x7] = 1;
        else chip8.key[0x7] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::S)) chip8.key[0x8] = 1;
        else chip8.key[0x8] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::D)) chip8.key[0x9] = 1;
        else chip8.key[0x9] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::F)) chip8.key[0xE] = 1;
        else chip8.key[0xE] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::Y) || Keyboard::isKeyPressed(Keyboard::Key::Z)) chip8.key[0xA] = 1;
        else chip8.key[0xA] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::X)) chip8.key[0x0] = 1;
        else chip8.key[0x0] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::C)) chip8.key[0xB] = 1;
        else chip8.key[0xB] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::V)) chip8.key[0xF] = 1;
        else chip8.key[0xF] = 0;
        
        if (Keyboard::isKeyPressed(Keyboard::Key::P)) {
            Image img = window.capture();
            img.saveToFile("screenshot.png");
        }
    }
    
    void update() {
        chip8.emulateCycle();
    }
    
    void draw() {
        if (chip8.drawFlag) {
            window.clear();
            
            canvas.create(canvasWidth, canvasHeight);
            for (int i = 0; i < 2048; i++) {
                if (chip8.gfx[i] != 0) {
                    int x = i % 64;
                    int y = (i - x) / 64;
                    canvas.setPixel(x, y, Color::White);
                }
            }
            Texture t;
            t.loadFromImage(canvas);
            Sprite s(t);
            s.scale(scale, scale);
            window.draw(s);
            
            window.display();
            
            chip8.drawFlag = false;
        }
    }
    
    void sound() {
        if (chip8.soundFlag) {
            if (hasSound) {
                beep.play();
            }
            else {
                std::cout << "BEEP!" << std::endl;
            }
            chip8.soundFlag = false;
        }
    }
};

int main(int argc, char *argv[])
{
    Emulator emu;
    if (argc > 1) {
        rom = argv[1];
        std::cout << argv[1] << std::endl;
    }
    emu.run();
    
    return 0;
}