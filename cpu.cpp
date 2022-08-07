#include "cpu.h"
#include "utils.h"

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

uint8_t gbcpu::getFlag(uint8_t flag) {
	return getBit(this->AF.half[0], flag);
}

void gbcpu::setFlag(uint8_t flag, uint8_t val) {
	this->AF.half[0] = setBit(this->AF.half[0], flag, val);
}

void gbcpu::ALU(uint8_t operation, uint8_t r2) {
	/* subtract flag */
	if (operation == 2 || operation == 3 || operation == 7) {
		this->setFlag(S_FLAG, 1);
	}
	else {
		this->setFlag(S_FLAG, 0);
	}

	switch (operation) {
	case 0: //ADD
		/* full carry logic */
		if ((((this->AF.half[1] & 0xff) + (r2 & 0xff)) & 0x100) == 0x100) {
			this->setFlag(C_FLAG, 1);
		}
		else {
			this->setFlag(C_FLAG, 0);
		}
		/* half carry logic */
		if ((((this->AF.half[1] & 0xf) + (r2 & 0xf)) & 0x10) == 0x10) {
			this->setFlag(H_FLAG, 1);
		}
		else {
			this->setFlag(H_FLAG, 0);
		}

		this->AF.half[1] += r2;
		break;
	case 1: //ADC
		if ((((this->AF.half[1] & 0xff) + (r2 & 0xff) + this->getFlag(C_FLAG)) & 0x100) == 0x100) {
			this->setFlag(C_FLAG, 1);
		}
		else {
			this->setFlag(C_FLAG, 0);
		}

		if ((((this->AF.half[1] & 0xf) + (r2 & 0xf) + this->getFlag(C_FLAG)) & 0x10) == 0x10) {
			this->setFlag(H_FLAG, 1);
		}
		else {
			this->setFlag(H_FLAG, 0);
		}

		this->AF.half[1] += (r2 + this->getFlag(C_FLAG));
		break;
	case 2: //SUB
		if ((((this->AF.half[1] & 0xff) - (r2 & 0xff)) & 0x100) == 0x100) {
			this->setFlag(C_FLAG, 1);
		}
		else {
			this->setFlag(C_FLAG, 0);
		}

		if ((((this->AF.half[1] & 0xf) - (r2 & 0xf)) & 0x10) == 0x10) {
			this->setFlag(H_FLAG, 1);
		}
		else {
			this->setFlag(H_FLAG, 0);
		}

		this->AF.half[1] -= r2;
		break;
	case 3: //SBC
		if ((((this->AF.half[1] & 0xff) - (r2 & 0xff) - this->getFlag(C_FLAG)) & 0x100) == 0x100) {
			this->setFlag(C_FLAG, 1);
		}
		else {
			this->setFlag(C_FLAG, 0);
		}

		if ((((this->AF.half[1] & 0xf) - (r2 & 0xf) - this->getFlag(C_FLAG)) & 0x10) == 0x10) {
			this->setFlag(H_FLAG, 1);
		}
		else {
			this->setFlag(H_FLAG, 0);
		}

		this->AF.half[1] -= r2;
		this->AF.half[1] -= this->getFlag(C_FLAG);
		break;
	case 4: //AND
		this->AF.half[1] &= r2;
		this->setFlag(H_FLAG, 1); //AND sets half carry to 1
		break;
	case 5: //XOR
		this->AF.half[1] ^= r2;
		break;
	case 6: //OR
		this->AF.half[1] |= r2;
		break;
	case 7: //CP
		if (this->AF.half[1] - r2 == 0) {
			this->setFlag(Z_FLAG, 1);
		}
		else {
			this->setFlag(Z_FLAG, 0);
		}
		break;
	}

	/* zero flag is set by all operations */
	if ((this->AF.half[1] == 0) && (operation != 7)) {
		this->setFlag(Z_FLAG, 1);
	}
	else {
		this->setFlag(Z_FLAG, 0);
	}
}

void gbcpu::tick() {
	static uint8_t nibble[2];

	static uint8_t* src;
	static uint8_t* dest;
	static uint8_t immediate;
	static registerPair immediate16;
	
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
			immediate16.half[0] = this->memory[PC]; //LSB
			PC++;
			cycle = 3;
			break;
		case 2:
			immediate16.half[1] = this->memory[PC]; //MSB
			PC++;
			break;
		case 1:
			this->AF.half[1] = this->memory[immediate16.full];
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
			immediate16.half[0] = this->memory[PC]; //LSB
			PC++;
			cycle = 3;
			break;
		case 2:
			immediate16.half[1] = this->memory[PC]; //MSB
			PC++;
			break;
		case 1:
			this->memory[immediate16.full] = this->AF.half[1];
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

	/* LD rr, nn [3 cycles ]*/
	if ((nibble[0] == 0x1) && (nibble[1] <= 0x3)) {
		switch (cycle) {
		case NEW_CYCLE:
			immediate16.half[0] = this->memory[PC]; //LSB
			PC++;
			cycle = 2;
			break;
		case 1:
			immediate16.half[1] = this->memory[PC]; //MSB
			PC++;
			break;
		case 0:
			switch (opcode) {
			case 0x01:
				this->BC.full = immediate16.full;
				break;
			case 0x11:
				this->DE.full = immediate16.full;
				break;
			case 0x21:
				this->HL.full = immediate16.full;
				break;
			case 0x31:
				this->SP = immediate16.full;
				break;
			}
			break;
		}
	}

	/* LD (a16), SP [5 cycles] */
	if (opcode == 0x08) {
		switch (cycle) {
		case NEW_CYCLE:
			immediate16.half[0] = this->memory[PC]; //LSB
			PC++;
			cycle = 4;
			break;
		case 3:
			immediate16.half[1] = this->memory[PC]; //MSB
			PC++;
			break;
		case 2:
			this->memory[immediate16.full] = static_cast<uint8_t>(this->SP & 0x00FF); //LSB
			break;
		case 1:
			this->memory[immediate16.full + 1] = static_cast<uint8_t>((this->SP >> 8) & 0x00FF); //MSB
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD SP, HL */
	if (opcode == 0xF9) {
		switch (cycle) {
		case NEW_CYCLE:
			this->SP = this->HL.full;
			cycle = 1;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* PUSH rr [4 cycles] */
	if ((nibble[1] >= 0xC) && (nibble[0] == 0x5)) {
		switch (cycle) {
		case NEW_CYCLE:
			this->SP--;
			cycle = 3;
			break;
		case 2:
			switch (opcode) {
			case 0xC5:
				memory[this->SP] = this->BC.half[1];
				break;
			case 0xD5:
				memory[this->SP] = this->DE.half[1];
				break;
			case 0xE5:
				memory[this->SP] = this->HL.half[1];
				break;
			case 0xF5:
				memory[this->SP] = this->AF.half[1];
				break;
			}
			this->SP--;
			break;
		case 1:
			switch (opcode) {
			case 0xC5:
				memory[this->SP] = this->BC.half[0];
				break;
			case 0xD5:
				memory[this->SP] = this->DE.half[0];
				break;
			case 0xE5:
				memory[this->SP] = this->HL.half[0];
				break;
			case 0xF5:
				memory[this->SP] = this->AF.half[0];
				break;
			}
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* POP rr [3 cycles] */
	if ((nibble[1] >= 0xC) && (nibble[0] == 0x1)) {
		switch (cycle) {
		case NEW_CYCLE:
			switch (opcode) {
			case 0xC1:
				this->BC.half[0] = this->memory[SP];
				break;
			case 0xD1:
				this->DE.half[0] = this->memory[SP];
				break;
			case 0xE1:
				this->HL.half[0] = this->memory[SP];
				break;
			case 0xF1:
				this->AF.half[0] = this->memory[SP];
				break;
			}
			this->SP++;
			cycle = 2;
			break;
		case 1:
			switch (opcode) {
			case 0xC1:
				this->BC.half[1] = this->memory[SP];
				break;
			case 0xD1:
				this->DE.half[1] = this->memory[SP];
				break;
			case 0xE1:
				this->HL.half[1] = this->memory[SP];
				break;
			case 0xF1:
				this->AF.half[1] = this->memory[SP];
				break;
			}
			this->SP++;
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* LD HL, SP+s8 */
	if (opcode == 0xF8) {
		switch (cycle) {
		case NEW_CYCLE:
			immediate = this->memory[PC];
			PC++;
			cycle = 2;
			break;
		case 1:
			/* flag setting */
			if ((((this->SP & 0xf) + (immediate & 0xf)) & 0x10) == 0x10) { //half carry
				this->AF.half[0] = setBit(this->AF.half[0], H_FLAG, 1);
			}
			else {
				this->AF.half[0] = setBit(this->AF.half[0], H_FLAG, 0);
			}
			if ((((this->SP & 0xff) + (immediate & 0xff)) & 0x100) == 0x100) { //full carry
				this->AF.half[0] = setBit(this->AF.half[0], C_FLAG, 1);
			}
			else {
				this->AF.half[0] = setBit(this->AF.half[0], C_FLAG, 0);
			}

			this->HL.full = this->SP + static_cast<int8_t>(immediate);
			break;
		case 0:
			//do nothing
			break;
		}
	}

	/* 8 bit ALU Operations [1 cycle] */
	if ((nibble[1] >= 0x8) && (nibble[1] <= 0xB) && (nibble[1] % 8 != 0x6)) {
		switch (cycle) {
		case NEW_CYCLE:
			switch (nibble[0] % 8) {
			case 0x0:
				immediate = this->BC.half[1];
				break;
			case 0x1:
				immediate = this->BC.half[0];
				break;
			case 0x2:
				immediate =this->DE.half[1];
				break;
			case 0x3:
				immediate = this->DE.half[0];
				break;
			case 0x4:
				immediate = this->HL.half[1];
				break;
			case 0x5:
				immediate = this->HL.half[0];
				break;
			case 0x7:
				immediate = this->AF.half[1];
				break;
			}

			this->ALU((opcode - 0x80) / 8, immediate);
			cycle = 0;
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