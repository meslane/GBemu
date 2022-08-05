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

	static uint16_t MSB;
	static uint16_t LSB;
	
	/* NOP [1 cycle] */
	if (opcode == 0x00) {
		switch (cycle) {
		case NEW_CYCLE:
			cycle = 0; //do nothing
			break;
		}
	}

	/* LD dest,src [1 cycle] */
	if (((nibble[1] >= 0x4 && nibble[1] <= 0x6)||(nibble[1] == 0x7 && nibble[0] >= 0x8)) && (nibble[0] % 0x8 != 0x6)) {
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
				if (nibble[0] >= 0x8) {
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

	/* LD dest,n [2 cycles] */
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

	/* LD dest, (HL) [2 cycles] */
	if (((nibble[1] >= 0x4 && nibble[1] <= 0x7) && (nibble[0] % 8 == 0x6) && (opcode != 0x76)) || (opcode == 0x2A) || (opcode == 0x3A)) {
		switch (cycle) {
		case NEW_CYCLE:
			switch (opcode) {
			case 0x2A: //increment
				this->AF.half[1] = this->memory[this->HL.full];
				this->HL.full++;
				break;
			case 0x3A: //decrement
				this->AF.half[1] = this->memory[this->HL.full];
				this->HL.full--;
				break;
			case 0x46:
				this->BC.half[1] = this->memory[this->HL.full];
				break;
			case 0x4E:
				this->BC.half[0] = this->memory[this->HL.full];
				break;
			case 0x56:
				this->DE.half[1] = this->memory[this->HL.full];
				break;
			case 0x5E:
				this->DE.half[0] = this->memory[this->HL.full];
				break;
			case 0x66:
				this->HL.half[1] = this->memory[this->HL.full];
				break;
			case 0x6E:
				this->HL.half[0] = this->memory[this->HL.full];
				break;
			case 0x7E:
				this->AF.half[1] = this->memory[this->HL.full];
				break;
			}
			cycle = 1;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD (HL), src [2 cycles] */
	if ((nibble[1] == 0x7 && nibble[0] <= 0x7 && opcode != 0x76) || (opcode == 0x22) || (opcode == 0x32)) {
		switch (cycle) {
		case NEW_CYCLE:
			switch (opcode) {
			case 0x22: //increment
				this->memory[this->HL.full] = this->AF.half[1];
				this->HL.full++;
				break;
			case 0x32: //decrement
				this->memory[this->HL.full] = this->AF.half[1];
				this->HL.full--;
				break;
			case 0x70:
				this->memory[this->HL.full] = this->BC.half[1];
				break;
			case 0x71:
				this->memory[this->HL.full] = this->BC.half[0];
				break;
			case 0x72:
				this->memory[this->HL.full] = this->DE.half[1];
				break;
			case 0x73:
				this->memory[this->HL.full] = this->DE.half[0];
				break;
			case 0x74:
				this->memory[this->HL.full] = this->HL.half[1];
				break;
			case 0x75:
				this->memory[this->HL.full] = this->HL.half[0];
				break;
			case 0x77:
				this->memory[this->HL.full] = this->AF.half[1];
				break;
			}
			cycle = 1;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD (HL), d8 [3 cycles] */
	if (opcode == 0x36) {
		switch (cycle) {
		case NEW_CYCLE:
			immediate = memory[PC];
			PC++;
			cycle = 2;
			break;
		case 1:
			this->memory[this->HL.full] = immediate;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD A, (BC) [2 cycles]*/
	if (opcode == 0x0A) {
		switch (cycle) {
		case NEW_CYCLE:
			this->AF.half[1] = this->memory[this->BC.full];
			cycle = 1;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD A, (DE) [2 cycles]*/
	if (opcode == 0x1A) {
		switch (cycle) {
		case NEW_CYCLE:
			this->AF.half[1] = this->memory[this->DE.full];
			cycle = 1;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD (BC), A [2 cycles]*/
	if (opcode == 0x02) {
		switch (cycle) {
		case NEW_CYCLE:
			this->memory[this->BC.full] = this->AF.half[1];
			cycle = 1;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD (DE), A [2 cycles]*/
	if (opcode == 0x12) {
		switch (cycle) {
		case NEW_CYCLE:
			this->memory[this->DE.full] = this->AF.half[1];
			cycle = 1;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD A, (nn) [4 cycles] */
	if (opcode == 0xFA) {
		switch (cycle) {
		case NEW_CYCLE:
			LSB = this->memory[PC];
			PC++;
			cycle = 3;
			break;
		case 2:
			MSB = this->memory[PC];
			PC++;
			break;
		case 1:
			this->AF.half[1] = this->memory[(MSB << 8) | (LSB & 0x00FF)];
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD (nn), A [4 cycles] */
	if (opcode == 0xEA) {
		switch (cycle) {
		case NEW_CYCLE:
			LSB = this->memory[PC];
			PC++;
			cycle = 3;
			break;
		case 2:
			MSB = this->memory[PC];
			PC++;
			break;
		case 1:
			this->memory[(MSB << 8) | (LSB & 0x00FF)] = this->AF.half[1];
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD A, (C) [2 cycles] */
	if (opcode == 0xF2) {
		switch (cycle) {
		case NEW_CYCLE:
			this->AF.half[1] = this->memory[0xFF00 | (static_cast<uint16_t>(this->BC.half[0]) & 0x00FF)];
			cycle = 1;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD (C), A [2 cycles] */
	if (opcode == 0xE2) {
		switch (cycle) {
		case NEW_CYCLE:
			this->memory[0xFF00 | (static_cast<uint16_t>(this->BC.half[0]) & 0x00FF)] = this->AF.half[1];
			cycle = 1;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LDH A, (n) [3 cycles] */
	if (opcode == 0xF0) {
		switch (cycle) {
		case NEW_CYCLE:
			immediate = this->memory[PC];
			PC++;
			cycle = 2;
			break;
		case 1:
			this->AF.half[1] = this->memory[0xFF00 | (static_cast<uint16_t>(immediate) & 0x00FF)];
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LDH (n), A [3 cycles] */
	if (opcode == 0xE0) {
		switch (cycle) {
		case NEW_CYCLE:
			immediate = this->memory[PC];
			PC++;
			cycle = 2;
			break;
		case 1:
			this->memory[0xFF00 | (static_cast<uint16_t>(immediate) & 0x00FF)] = this->AF.half[1];
			break;
		case 0:
			//do nothing
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