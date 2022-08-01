#include "cpu.h"

#include <iostream>

cpuDebugger::cpuDebugger(gbcpu target) {
	this->AF = target.AF;
	this->BC = target.BC;
	this->DE = target.DE;
	this->HL = target.HL;
	
	this->SP = target.SP;
	this->PC = target.PC;
}

uint64_t cpuDebugger::getAllRegisters() {
	return (static_cast<uint64_t>(this->AF.full) << 48) | (static_cast<uint64_t>(this->BC.full) << 32) | (static_cast<uint64_t>(this->DE.full) << 16) | static_cast<uint64_t>(this->HL.full);
}

uint32_t cpuDebugger::getBothPointers() {
	return (static_cast<uint32_t>(this->SP) << 16) | static_cast<uint32_t>(this->PC);
}

gbcpu::gbcpu(uint8_t* memory) {
	this->AF.full = 0;
	this->BC.full = 0;
	this->DE.full = 0;
	this->HL.full = 0;

	this->SP = 0;
	this->PC = 0;

	this->memory = memory;

	this->opcode = 0;
	this->cycle = 0;
}

void gbcpu::registerDump() {
	printf("A: %d F: %d\n", AF.half[1], AF.half[0]);
	printf("B: %d C: %d\n", BC.half[1], BC.half[0]);
	printf("D: %d E: %d\n", DE.half[1], DE.half[0]);
	printf("H: %d L: %d\n", HL.half[1], HL.half[0]);
	printf("SP: %d\n", SP);
	printf("PC: %d\n", PC);
	printf("Opcode: %d, Cycle: %d\n\n", opcode, cycle);
}

void gbcpu::tick() {
	static uint8_t nibble[2];

	static uint8_t* src;
	static uint8_t* dest;
	static uint8_t immediate;
	
	/* NOP (1 cycle) */
	if (opcode == 0x00) {
		switch (cycle) {
		case NEW_CYCLE:
			cycle = 0; //do nothing
			break;
		}
	}

	/* LD dest,src (1 cycle) */
	if (nibble[1] >= 0x4 && nibble[1] <= 0x7) {
		switch (cycle) {
		case NEW_CYCLE:
			switch (nibble[1]) {
			case 0x4:
				if (nibble[0] < 0x8) {
					dest = &this->BC.half[1]; //B
				}
				else {
					dest = &this->BC.half[0]; //C
				}
				break;
			case 0x5:
				if (nibble[0] < 0x8) {
					dest = &this->DE.half[1]; //D
				}
				else {
					dest = &this->DE.half[0]; //E
				}
				break;
			case 0x6:
				if (nibble[0] < 0x8) {
					dest = &this->HL.half[1]; //H
				}
				else {
					dest = &this->HL.half[0]; //L
				}
				break;
			case 0x7:
				if (nibble[0] > 0x8) {
					dest = &this->AF.half[1]; //A
				}
			}

			switch (nibble[0] % 8) {
			case 0x0:
				src = &this->BC.half[1]; //B
				break;
			case 0x1:
				src = &this->BC.half[0]; //C
				break;
			case 0x2:
				src = &this->DE.half[1]; //D
				break;
			case 0x3:
				src = &this->DE.half[0]; //E
				break;
			case 0x4:
				src = &this->HL.half[1]; //H
				break;
			case 0x5:
				src = &this->HL.half[0]; //L
				break;
			case 0x6:
				//do nothing
				break;
			case 0x7:
				src = &this->AF.half[1]; //A
				break;
			}

			*dest = *src;
			cycle = 0;
			break;
		}
	}

	/* LD r,n (2 cycles) */
	if ((nibble[0] % 8 == 0x6) && (nibble[1] <= 0x3) && (opcode != 0x36)) {
		switch (cycle) {
		case NEW_CYCLE:
			immediate = memory[PC];
			PC++;
			cycle = 1;
			break;
		case 0:
			switch (nibble[1]) {
			case 0:
				if (nibble[0] == 0x6) {
					dest = &this->BC.half[1];
				}
				else if (nibble[0] == 0xE) {
					dest = &this->BC.half[0];
				}
				break;
			case 1:
				if (nibble[0] == 0x6) {
					dest = &this->DE.half[1];
				}
				else if (nibble[0] == 0xE) {
					dest = &this->DE.half[0];
				}
				break;
			case 2:
				if (nibble[0] == 0x6) {
					dest = &this->HL.half[1];
				}
				else if (nibble[0] == 0xE) {
					dest = &this->HL.half[0];
				}
				break;
			case 3:
				if (nibble[0] == 0xE) {
					dest = &this->AF.half[1];
				}
				break;
			}
			
			*dest = immediate;
			break;
		}
		
	}

	/* fetch (happens same cycle as prev. instruction) */
	if (cycle == 0) {
		opcode = memory[PC];
		nibble[0] = opcode & 0x0F; //LSN
		nibble[1] = (opcode >> 4) & 0x0F; //MSN
		PC++;
		cycle = NEW_CYCLE; //denotes new cycle
	}

	if (cycle != 0 && cycle != NEW_CYCLE) { //underflow protection
		cycle--;
	}
}