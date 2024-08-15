#include <iostream>
#include <fstream> 
#include <cstdint>
#include <filesystem>
#include <thread>
#include <chrono>
#include <csignal> 
#include <ctime>
#include <cstdlib> 
#include "render.h"

#define START_ADDRESS 0x200 
#define FONTSET_SIZE 80
#define FONTSET_ADDRESS 0x50
#define RAM_SIZE 4096

enum Operation {
    OP_CLEAR,
    OP_JUMP, 
    OP_CALL_SUB, 
    OP_RET_SUB, 
    OP_SKIP_EQ_VAL,
    OP_SKIP_NE_VAL,
    OP_SKIP_EQ_REG,
    OP_SKIP_NE_REG,
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
        uint8_t mem[RAM_SIZE]{}; 
        int32_t dispBuff[HEIGHT][WIDTH]{}; 
        int16_t pc{}; 
        int8_t sp{}; 
        int16_t index{}; // points to location in mem
        uint8_t registers[16]{}; 
        int8_t soundTimer{}; 
        int8_t delayTimer{}; 
        int16_t stack[16]{};
        int16_t valA{}; 
        int16_t valB{}; 
        int16_t valC{}; 
        int16_t programSize{}; 
        uint8_t keypad[16]{0}; 

        Machine(); 
        void LoadROM(const char *file);
        uint16_t fetch();
        Operation decode(uint16_t machineCode);
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

    std::cout << "Reading from file " << fp << "\n"; 

    std::ifstream file(fp, std::ios::binary); 

    if(!file) {
        std::cerr << "Could not read file\n"; 
        return; 
    }


    file.seekg(0, std::ios::end); 
    int64_t size = file.tellg(); 
    file.seekg(0, std::ios::beg); 

    std::cout << "Reading file of size " << size << "\n"; 

    char byte{}; 
    for(int i = 0; i < size; i++) {
        file.read(&byte, 1); 
        mem[START_ADDRESS + i] = byte; 
    }

    pc = START_ADDRESS; 
    programSize = size; 

    std::cout << "Successfully read file into memory \n"; 
}

uint16_t Machine::fetch() {
    uint16_t instruction = mem[pc] << 8;
    instruction |= mem[pc + 1];  
    std::cout << pc << "\n"; 
    printf("0x%x\n", instruction); 
    pc = pc + 2; 
    return instruction; 
}

Operation Machine::decode(uint16_t machineCode) {
    switch((machineCode & 0xF000) >> 12) {
        case 0:
            if((machineCode & 0x000F) == 0) {
                return OP_CLEAR; // 00E0 
            } else {
                return OP_RET_SUB; // 00EE
            }
            break; 
        case 0x1:
            valA = machineCode & 0x0FFF; 
            return OP_JUMP; // 1NNN 
            break; 
        case 0x2:
            valA = machineCode & 0x0FFF; 
            return OP_CALL_SUB; // 2NNN
            break; 
        case 0x3:
            valA = (machineCode & 0x0F00) >> 8;
            valB = (machineCode & 0x00FF);
            return OP_SKIP_EQ_VAL; // 3XNN
            break; 
        case 0x4:
            valA = (machineCode & 0x0F00) >> 8;
            valB = (machineCode & 0x00FF);
            return OP_SKIP_NE_VAL; 
            break; 
        case 0x5:
            valA = (machineCode & 0x0F00) >> 8; 
            valB = (machineCode & 0x00F0) >> 4; 
            return OP_SKIP_EQ_REG; 
            break; 
        case 0x6:
            valA = (machineCode & 0x0F00) >> 8;
            valB = (machineCode & 0x00FF);
            return OP_SET_VALUE; // 6XNN
            break;
        case 0x7:
            valA = (machineCode & 0x0F00) >> 8;
            valB = (machineCode & 0x00FF);
            return OP_ADD_VALUE; // 7XNN 
            break; 
        case 0x8:
            valA = (machineCode & 0x0F00) >> 8; 
            valB = (machineCode & 0x00F0) >> 4; 
            switch(machineCode & 0x000F) {
                case 0x0:
                    return OP_SET_REGISTER;
                    break;
                case 0x1:
                    return OP_OR; 
                    break;
                case 0x2:
                    return OP_AND;
                    break;
                case 0x3:
                    return OP_XOR;
                    break;
                case 0x4:
                    return OP_ADD_REGISTER;
                    break;
                case 0x5:
                    return OP_SUBTRACT_T1;
                    break;
                case 0x6:
                    return OP_SHIFT_RIGHT; 
                case 0x7:
                    return OP_SUBTRACT_T2; 
                    break; 
                case 0xE:
                    return OP_SHIFT_LEFT; 
                default:
                    return OP_BAD; 
                    break; 
            }
            break; 
        case 0x9:
            valA = (machineCode & 0x0F00) >> 8; 
            valB = (machineCode & 0x00F0) >> 4; 
            return OP_SKIP_NE_REG; 
            break;
        case 0xA:
            valA = (machineCode & 0x0FFF); 
            return OP_SET_INDEX; // ANNN
            break; 
        case 0xB:
            valA = (machineCode & 0x0FFF); 
            return OP_JUMP_OFFSET; // BNNN
            break;
        case 0xC:
            valA = (machineCode & 0x0F00) >> 8; 
            valB = (machineCode & 0x00FF); 
            return OP_RANDOM; // CXNN
            break; 
        case 0xD:
            valA = (machineCode & 0x0F00) >> 8;
            valB = (machineCode & 0x00F0) >> 4;
            valC = (machineCode & 0x000F); 
            return OP_DISP; // 0xDXYN
            break; 
        case 0xE:
            valA = (machineCode & 0x0F00) >> 8; 
            if(machineCode & 0x000F == 0xE) {
                return OP_SKIP_PRESSED; 
                break;
            } else {
                return OP_SKIP_NOT_PRESSED; 
                break;
            }
            break;
        case 0xF:
            valA = (machineCode & 0x0F00) >> 8;
            switch(machineCode & 0x00FF) {
                case 0x07:
                    return OP_GET_TIMER;
                    break;
                case 0x15:
                    return OP_SET_TIMER; 
                    break;
                case 0x18:
                    return OP_SET_SOUND; 
                    break;
                case 0x1E:
                    return OP_ADD_INDEX; 
                    break;
                case 0x0A:
                    return OP_GET_KEY; 
                    break;
                case 0x29:
                    return OP_FONT; 
                    break;
                case 0x33:
                    return OP_DECIMAL_CONVERT; 
                    break; 
                case 0x55:
                    return OP_STORE; 
                    break;
                case 0x65:
                    return OP_LOAD; 
                    break;
                default:
                    return OP_BAD; 
                    break; 
            }
            break; 
        default:
            return OP_BAD; 
            break; 
    }
    return OP_BAD; 
}

void Machine::execute(Operation op) {

    uint16_t startX{}; 
    uint16_t startY{}; 
    uint16_t n{}; 

    switch(op) {
        case OP_CLEAR:
            memset(&dispBuff, 0, WIDTH * HEIGHT); 
            std::cout << "Clear\n";
            break; 
        case OP_JUMP:
            pc = valA;
            std::cout << "Jump\n";
            break; 
        case OP_SET_VALUE:
            registers[valA] = valB;
            std::cout << "Set Val\n";
            break; 
        case OP_ADD_VALUE:
            registers[valA] = registers[valA] + valB; 
            std::cout << "Add Value\n";
            break;
        case OP_SET_INDEX:
            index = valA; 
            std::cout << "Set Index\n";
            break; 
        case OP_DISP:
            startX = registers[valA] & 63; 
            startY = registers[valB] & 31; 
            n = valC; 
            for(int i = 0; i < n; i++) {

                uint8_t byte = mem[index + i]; 

                for(int j = 8; j > 0; j--) {
                    
                    // grab the current bit and plop it onto the screen
                    uint32_t x = startX + (8 - j); 
                    uint32_t y = startY + i;

                    if(x > WIDTH) {
                        j = 0; 
                    } else if(y > HEIGHT) {
                        j = 0; 
                        i = n; 
                    } else {
                        int32_t val{}; 
                        if((byte >> (j - 1)) & 0x1 == 1) {
                            val = 0xFFFFFFFF; 
                        }

                        if(dispBuff[y][x] != 0) {
                            registers[0xF] = 1; 
                        }

                        dispBuff[y][x] ^= val; 
                    }
                }
            }
            std::cout << "Draw\n";
            break;
        case OP_CALL_SUB:
            stack[sp] = pc;
            sp += 1; 
            pc = valA; 
            std::cout << "Call\n";
            break;
        case OP_RET_SUB:
            sp -= 1;
            pc = stack[sp]; 
            std::cout << "Ret\n";
            break; 
        case OP_SKIP_EQ_VAL:
            if(registers[valA] == valB) {
                pc += 2; 
            }
            std::cout << "Skip Eq Val\n";
            break;
        case OP_SKIP_NE_VAL:
            if(registers[valA] != valB) {
                pc += 2; 
            }
            std::cout << "Skip NE Val\n";
            break; 
        case OP_SKIP_EQ_REG:
            if(registers[valA] == registers[valB]) {
                pc += 2; 
            }
            std::cout << "Skip EQ Reg\n";
            break;
        case OP_SKIP_NE_REG:
            if(registers[valA] != registers[valB]) {
                pc += 2; 
            }
            std::cout << "Skip NE Reg\n";
            break; 
        case OP_SET_REGISTER:
            registers[valA] = registers[valB];
            std::cout << "Set Register\n";
            break; 
        case OP_OR:
            registers[valA] |= registers[valB];
            std::cout << "OR\n";
            break;
        case OP_AND:
            registers[valA] &= registers[valB];
            std::cout << "AND\n";
            break; 
        case OP_XOR:
            registers[valA] ^= registers[valB]; 
            std::cout << "XOR\n";
            break;
        case OP_ADD_REGISTER:
            registers[valA] += registers[valB]; 
            if(registers[valA] + registers[valB] > 255) {
                registers[0xF] = 1; 
            } else {
                registers[0xF] = 0; 
            }
            std::cout << "Add Register\n";
            break; 
        case OP_SUBTRACT_T1:
            registers[valA] -= registers[valB]; 
            if(registers[valA] > registers[valB]) {
                registers[0xF] = 1; 
            } else {
                registers[0xF] = 0; 
            }
            std::cout << "Subtract T1\n";
            break; 
        case OP_SUBTRACT_T2:
            registers[valA] = registers[valB] - registers[valA]; 
            if(registers[valA] < registers[valB]) {
                registers[0xF] = 1; 
            } else {
                registers[0xF] = 0; 
            }
            std::cout << "Subtract T2\n";
            break; 
        case OP_SHIFT_LEFT:
            if(registers[valA] >> 7 == 1) {
                registers[0xF] = 1; 
            } else {
                registers[0xF] = 0; 
            }
            registers[valA] = registers[valA] << 1; 
            std::cout << "Shift Left\n";
            break;
        case OP_SHIFT_RIGHT:
            if(registers[valA] & 0x01 == 1) {
                registers[0xF] = 1; 
            } else {
                registers[0xF] = 0; 
            }
            registers[valA] = registers[valA] >> 1; 
            std::cout << "Shift Right\n";
            break; 
        case OP_JUMP_OFFSET:
            pc = registers[0] + valA;
            std::cout << "Jump Offset\n";
            break; 
        case OP_RANDOM:
            std::srand(std::time(nullptr));
            registers[valA] &= (std::rand() % 256);
            std::cout << "Random\n";
            break; 
        case OP_SKIP_PRESSED:
            if(keypad[registers[valA]] == 1) {
                pc += 2; 
            }
            break; 
        case OP_SKIP_NOT_PRESSED:
            if(keypad[registers[valA]] != 1) {
                pc += 2; 
            }
            break; 
        case OP_GET_TIMER:
            registers[valA] = delayTimer; 
            break; 
        case OP_SET_TIMER:
            delayTimer = registers[valA]; 
            break;
        case OP_SET_SOUND:
            soundTimer = registers[valA];
            break; 
        case OP_ADD_INDEX:
            index += registers[valA]; 
            if(index >= 0x1000) {
                registers[0xF] = 1; 
                index %= 0x1000;
            }
            break; 
        case OP_GET_KEY:
            if(keypad[valA] != 1) {
                pc -= 2; 
            }
            break; 
        case OP_FONT:
            switch(registers[valA]) {
                case 0x00:
                    index = 0x50;
                    break; 
                case 0x01:
                    index = 0x55;
                    break;
                case 0x02:
                    index = 0x60;
                    break; 
                case 0x03:
                    index = 0x65;
                    break; 
                case 0x04:
                    index = 0x70;
                    break;
                case 0x05:
                    index = 0x75;
                    break; 
                case 0x06:
                    index = 0x80;
                    break; 
                case 0x07:
                    index = 0x85;
                    break; 
                case 0x08:
                    index = 0x90;
                    break; 
                case 0x09:
                    index = 0x95;
                    break; 
                case 0x0A:
                    index = 0x100;
                    break; 
                case 0x0B:
                    index = 0x105;
                    break; 
                case 0x0C:
                    index = 0x110;
                    break; 
                case 0x0D:
                    index = 0x115;
                    break; 
                case 0x0E:
                    index = 0x120;
                    break; 
                case 0x0F:
                    index = 0x125;
                    break; 
                default:
                    break; 
            }
            break;
        case OP_DECIMAL_CONVERT:
            mem[index] = (registers[valA] / 100);
            mem[index + 1] = (registers[valA] / 10) % 10; 
            mem[index + 2] = (registers[valA] % 10);
            break;
        case OP_STORE:
            for(int i = 0; i <= valA; i++) {
                mem[index + i] = registers[i]; 
            }
            break; 
        case OP_LOAD:
            for(int i = 0; i <= valA; i++) {
                registers[i] = mem[index + i]; 
            }
            break; 
        default:
            std::cout << "Could not proccess instruction\n"; 
            break; 
    }
    return;  
}

void signalHandler(int sig) {
    std::cout << "Interrupt signal: " << sig << "\n"; 
    exit(sig); 
}

int SDL_main(int argc, char* argv[]) {

    signal(SIGINT, signalHandler); 

    // initialize screen & machine
    Screen screen = Screen(); 
    Machine machine = Machine(); 
    uint16_t instruction{}; 
    Operation op{}; 
    SDL_Event event; 

    machine.LoadROM(argv[1]); 

    while(machine.pc <= START_ADDRESS + machine.programSize) {

        // proccess input
        memset(&machine.keypad, 0, 16); 
        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_1:
                        machine.keypad[0x0] = 1;
                        break; 
                    case SDLK_2:
                        machine.keypad[0x1] = 1;
                        break;
                    case SDLK_3:
                        machine.keypad[0x2] = 1;
                        break; 
                    case SDLK_4:
                        machine.keypad[0x3] = 1;
                        break; 
                    case SDLK_q:
                        machine.keypad[0x4] = 1; 
                        break; 
                    case SDLK_w:
                        machine.keypad[0x5] = 1;
                        break;
                    case SDLK_e:
                        machine.keypad[0x6] = 1;
                        break; 
                    case SDLK_r:
                        machine.keypad[0x7] = 1;
                        break; 
                    case SDLK_a:
                        machine.keypad[0x8] = 1;
                        break; 
                    case SDLK_s:
                        machine.keypad[0x9] = 1;
                        break; 
                    case SDLK_d:
                        machine.keypad[0xA] = 1;
                        break; 
                    case SDLK_f:
                        machine.keypad[0xB] = 1;
                        break; 
                    case SDLK_z:
                        machine.keypad[0xC] = 1;
                        break; 
                    case SDLK_x:
                        machine.keypad[0xD] = 1;
                        break; 
                    case SDLK_c:
                        machine.keypad[0xE] = 1;
                        break; 
                    case SDLK_v:
                        machine.keypad[0xF] = 1;
                        break; 
                    default:
                        break; 
                }
            }
        }


        std::this_thread::sleep_for(std::chrono::milliseconds(5)); 
        machine.delayTimer -= 1; 
        instruction = machine.fetch();
        op = machine.decode(instruction); 
        machine.execute(op);
        screen.update((int64_t*) &machine.dispBuff);
    }

    return EXIT_SUCCESS; 
}