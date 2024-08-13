#include <iostream>
#include <fstream> 
#include <cstdint>
#include <filesystem>
#include "render.h"

#define START_ADDRESS 0x200 
#define FONTSET_SIZE 80
#define FONTSET_ADDRESS 0x50

enum Operation {
    OP_CLEAR,
    OP_JUMP, 
    OP_CALL_SUB, 
    OP_RET_SUB, 
    OP_SKIP_CONDTIONALLY, // might need to change this one 
    OP_SET_VALUE,
    OP_SET_REGISTER,
    OP_ADD_VALUE,
    OP_ADD_REGISTER,
    OP_SUBTRACT_T1,
    OP_SUBTRACT_T2,
    OP_OR,
    OP_AND,
    OP_XOR,
    OP_SHIFT_LEFT,
    OP_SHIFT_RIGHT,
    OP_SET_INDEX,
    OP_JUMP_OFFSET,
    OP_RANDOM,
    OP_DISP,
    OP_SKIP_PRESSED,
    OP_SKIP_NOT_PRESSED,
    OP_GET_TIMER,
    OP_SET_TIMER,
    OP_SET_SOUND,
    OP_ADD_INDEX,
    OP_GET_KEY, 
    OP_FONT, 
    OP_DECIMAL_CONVERT, 
    OP_STORE,
    OP_LOAD, 
    OP_BAD
}; 

class Machine {
    public:
        int8_t mem[4096]{}; 
        int64_t dispBuff[WIDTH][HEIGHT]{}; 
        int16_t pc{}; 
        int16_t index{}; // points to location in mem
        int8_t registers[16]{}; 
        int8_t soundTimer{}; 
        int8_t delayTimer{}; 
        int16_t stack[16]{};
        int16_t valA{}; 
        int16_t valB{}; 

        Machine(); 
        void LoadROM(const char *file);
        int16_t fetch();
        Operation decode(int16_t machineCode);
        void execute(Operation op); 
        void countDown(); 
        void updateDelay(); 
        void updateSound(); 
};

Machine::Machine() {
    // define fontset 
    uint8_t font[FONTSET_SIZE] = {
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

    // load fontset into memory 
    for(int i = 0; i < FONTSET_SIZE; i++) {
        mem[FONTSET_ADDRESS + i] = font[i]; 
    }

}

void Machine::LoadROM(const char* fp) {
    std::ifstream file(fp, std::ios::binary); 

    if(!file) {
        std::cerr << "Could not read file\n"; 
        return; 
    }

    uint64_t size = std::filesystem::file_size(fp); 

    std::cout << "Reading file of size " << size << "\n"; 

    char byte{}; 
    for(int i = 0; i < size; i++) {
        mem[START_ADDRESS + i] = byte; 
    }

    pc = START_ADDRESS; 

    std::cout << "Successfully read file into memory \n"; 
}

int16_t Machine::fetch() {
    int16_t instruction = *((int16_t*)&mem[pc]); 
    pc = pc + 2; 
    return instruction; 
}

Operation Machine::decode(int16_t machineCode) {
    switch((machineCode & 0xF000) >> 12) {
        case 0:
            if(machineCode & 0x000F == 0) {
                return OP_CLEAR; // 00E0 
            }
            break; 
        case 0x1:
            return OP_JUMP; // 1NNN 
            break; 
        case 0x2:
            break; 
        case 0x3:
            break; 
        case 0x4:
            break; 
        case 0x5:
            break; 
        case 0x6:
            return OP_SET_VALUE; // 6XNN
            break;
        case 0x7:
            return OP_ADD_VALUE; // 7XNN 
            break; 
        case 0x8:
            break; 
        case 0x9:
            break;
        case 0xA:
            return OP_SET_INDEX; // ANNN
            break; 
        case 0xB:
            break;
        case 0xC:
            break; 
        case 0xD:
            break; 
        case 0xE:
            break;
        case 0xF:
            break; 
        default:
            return OP_BAD; 
            break; 
    }
    return OP_BAD; 
}

void Machine::execute(Operation op) {
    return; 
}

int main() {
    
    // initialize screen & machine
    Screen screen = Screen(); 
    Machine machine = Machine(); 

    int16_t instruction{}; 
    Operation op{}; 

    while(true) {
        instruction = machine.fetch();
        op = machine.decode(instruction); 
        machine.execute(op);
        screen.update((int64_t*) &machine.dispBuff); 
    }

    return EXIT_SUCCESS; 
}