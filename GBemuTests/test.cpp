#include "pch.h"

#include "../cpu.h"

cpuDebugger* runAndDebug(uint8_t* memory, uint64_t cycles) {
	gbcpu* gb = new gbcpu(memory);
	
	for (uint64_t i = 0; i < cycles;  i++) {
		gb->tick();
	}
	
	cpuDebugger* debugger = new cpuDebugger(*gb);
	delete gb;
	return debugger;
}

TEST(CPUDebugger, getAllRegisters) {
	uint8_t memory[2] = { 0 };
	gbcpu* gb = new gbcpu(memory);
	cpuDebugger* debugger = new cpuDebugger(*gb);
	debugger->AF.full = 0x1234;
	debugger->BC.full = 0x5678;
	debugger->DE.full = 0x9ABC;
	debugger->HL.full = 0xDEF0;

	EXPECT_EQ(debugger->getAllRegisters(), 0x123456789ABCDEF0);

	delete gb;
	delete debugger;
}

TEST(CPUDebugger, getBothPointers) {
	uint8_t memory[2] = { 0 };
	gbcpu* gb = new gbcpu(memory);
	cpuDebugger* debugger = new cpuDebugger(*gb);
	debugger->SP = 0x1234;
	debugger->PC = 0xABCD;

	EXPECT_EQ(debugger->getBothPointers(), 0x1234ABCD);

	delete gb;
	delete debugger;
}

/*
NOTE:

in:
uint8_t array[100] = {1, 2}

all values past array[1] will be initialized to 0 automatically
*/

TEST(NOP, PCvalue) { //0x00
	uint8_t memory[4] = { 0 };
	cpuDebugger* results = runAndDebug(memory, 3);

	EXPECT_EQ(results->PC, 3);
	delete results;
}

TEST(LD_BC_d16, Load_0xFFFF) { //0x01
	uint8_t memory[5] = { 0x01, 0xFF, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 4);

	EXPECT_EQ(results->BC.full, 0xFFFF);
	delete results;
}

TEST(LD_BC_ptr_A, Load_255) { //0x02
	uint8_t memory[1024] = { 0x06, 0x01, 0x0E, 0xFF, 0x3E, 0xFF, 0x02 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(memory[0x1FF], 0xFF);
	delete results;
}

TEST(LD_B_d8, Load_255) { //0x06
	uint8_t memory[2] = { 0x06, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 3);

	EXPECT_EQ(results->BC.half[1], 0xFF);
	delete results;
}

TEST(LD_a16_ptr_SP, Load_0xFFFF_to_0x1A0_LSB) { //0x08
	uint8_t memory[512] = { 0x31, 0xFF, 0xFF, 0x08, 0xA0, 0x01 };
	cpuDebugger* results = runAndDebug(memory, 30);

	EXPECT_EQ(memory[0x1A0], 0xFF);
	delete results;
}

TEST(LD_a16_ptr_SP, Load_0xFFFF_to_0x1A0_MSB) { //0x08
	uint8_t memory[512] = { 0x31, 0xFF, 0xFF, 0x08, 0xA0, 0x01 };
	cpuDebugger* results = runAndDebug(memory, 30);

	EXPECT_EQ(memory[0x1A1], 0xFF);
	delete results;
}

TEST(LD_A_BC_ptr, Load_255) { //0x0A
	uint8_t memory[1024] = { 0x06, 0x01, 0x0E, 0xFF, 0x0A };
	memory[0x01FF] = 0xFF;
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->AF.half[1], 0xFF);
}

TEST(LC_C_d8, Load_255) { //0x0E
	uint8_t memory[2] = { 0x0E, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 3);

	EXPECT_EQ(results->BC.half[0], 0xFF);
	delete results;
}

TEST(LD_DE_d16, Load_0xFFFF) { //0x11
	uint8_t memory[5] = { 0x11, 0xFF, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 4);

	EXPECT_EQ(results->DE.full, 0xFFFF);
	delete results;
}

TEST(LD_DE_ptr_A, Load_255) { //0x12
	uint8_t memory[1024] = { 0x16, 0x01, 0x1E, 0xFF, 0x3E, 0xFF, 0x12 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(memory[0x1FF], 0xFF);
	delete results;
}

TEST(LD_D_d8, Load_255) { //0x16
	uint8_t memory[2] = { 0x16, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 3);

	EXPECT_EQ(results->DE.half[1], 0xFF);
	delete results;
}

TEST(LD_A_DE_ptr, Load_255) { //0x1A
	uint8_t memory[1024] = { 0x16, 0x01, 0x1E, 0xFF, 0x1A };
	memory[0x01FF] = 0xFF;
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->AF.half[1], 0xFF);
}

TEST(LD_E_d8, Load_255) { //0x1E
	uint8_t memory[2] = { 0x1E, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 3);

	EXPECT_EQ(results->DE.half[0], 0xFF);
	delete results;
}

TEST(LD_HL_d16, Load_0xFFFF) { //0x21
	uint8_t memory[5] = { 0x21, 0xFF, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 4);

	EXPECT_EQ(results->HL.full, 0xFFFF);
	delete results;
}

TEST(LD_HL_inc_ptr_A, Load_to_255_check_registers) { //0x22
	uint8_t memory[256] = { 0x2E, 0xFF, 0x3E, 0xA0, 0x22 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xA000000000000100);
	delete results;
}

TEST(LD_HL_inc_ptr_A, Load_to_255_check_memory) { //0x22
	uint8_t memory[256] = { 0x2E, 0xFF, 0x3E, 0xA0, 0x22 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[255], 0xA0);
	delete results;
}

TEST(LD_H_d8, Load_255) { //0x26
	uint8_t memory[2] = { 0x26, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 3);

	EXPECT_EQ(results->HL.half[1], 0xFF);
	delete results;
}

TEST(LD_A_HL_inc_ptr, Load_from_0xFF_check_registers) { //0x2A
	uint8_t memory[300] = {0x2E, 0xFF, 0x2A};
	memory[255] = 0xA0;
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xA000000000000100);
	delete results;
}

TEST(LD_L_d8, Load_255) { //0x2E
	uint8_t memory[2] = { 0x2E, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 3);

	EXPECT_EQ(results->HL.half[0], 0xFF);
	delete results;
}

TEST(LD_SP_d16, Load_0xFFFF) { //0x31
	uint8_t memory[5] = { 0x31, 0xFF, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 4);

	EXPECT_EQ(results->SP, 0xFFFF);
	delete results;
}

TEST(LD_HL_dec_ptr_A, Load_to_255_check_registers) { //0x32
	uint8_t memory[256] = { 0x2E, 0xFF, 0x3E, 0xA0, 0x32 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xA0000000000000FE);
	delete results;
}

TEST(LD_HL_dec_ptr_A, Load_to_255_check_memory) { //0x32
	uint8_t memory[256] = { 0x2E, 0xFF, 0x3E, 0xA0, 0x32 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[255], 0xA0);
	delete results;
}

TEST(LD_HL_ptr_d8, LOAD_255) { //0x36
	uint8_t memory[1024] = { 0x26, 0x01, 0x2E, 0xA0, 0x36, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 30);

	EXPECT_EQ(memory[0x01A0], 0xFF);
	delete results;
}

TEST(LD_A_HL_dec_ptr, Load_from_0xFF_check_registers) { //0x3A
	uint8_t memory[300] = { 0x2E, 0xFF, 0x3A };
	memory[255] = 0xA0;
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xA0000000000000FE);
	delete results;
}

TEST(LD_A_d8, Load_255) { //0x3E
	uint8_t memory[2] = { 0x3E, 0xFF };
	cpuDebugger* results = runAndDebug(memory, 3);

	EXPECT_EQ(results->AF.half[1], 0xFF);
	delete results;
}

TEST(LD_x_B, LOAD_255) {
	uint8_t memory[] = { 0x06, 0xFF, 0x40, 0x50, 0x60, 0x48, 0x58, 0x68, 0x78 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xFF00FFFFFFFFFFFF);
	delete results;
}

TEST(LD_x_C, LOAD_255) {
	uint8_t memory[] = { 0x0E, 0xFF, 0x41, 0x51, 0x61, 0x49, 0x59, 0x69, 0x79 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xFF00FFFFFFFFFFFF);
	delete results;
}

TEST(LD_x_D, LOAD_255) {
	uint8_t memory[] = { 0x16, 0xFF, 0x42, 0x52, 0x62, 0x4A, 0x5A, 0x6A, 0x7A };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xFF00FFFFFFFFFFFF);
	delete results;
}

TEST(LD_x_E, LOAD_255) {
	uint8_t memory[] = { 0x1E, 0xFF, 0x43, 0x53, 0x63, 0x4B, 0x5B, 0x6B, 0x7B };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xFF00FFFFFFFFFFFF);
	delete results;
}

TEST(LD_x_H, LOAD_255) {
	uint8_t memory[] = { 0x26, 0xFF, 0x44, 0x54, 0x64, 0x4C, 0x5C, 0x6C, 0x7C };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xFF00FFFFFFFFFFFF);
	delete results;
}

TEST(LD_x_L, LOAD_255) {
	uint8_t memory[] = { 0x2E, 0xFF, 0x45, 0x55, 0x65, 0x4D, 0x5D, 0x6D, 0x7D };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xFF00FFFFFFFFFFFF);
	delete results;
}

TEST(LD_x_A, LOAD_255) {
	uint8_t memory[] = { 0x3E, 0xFF, 0x47, 0x57, 0x67, 0x4F, 0x5F, 0x6F, 0x7F };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->getAllRegisters(), 0xFF00FFFFFFFFFFFF);
	delete results;
}

TEST(LD_x_HL_ptr, LOAD_FROM_ADDR_255) {
	uint8_t memory[256] = { 0x2E, 0xFF, 0x46, 0x56, 0x4E, 0x5E, 0x7E };
	memory[255] = 0x40;
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(results->getAllRegisters(), 0x40004040404000FF);
	delete results;
}

TEST(LD_HL_ptr_B, LOAD_64) { //0x70
	uint8_t memory[256] = { 0x2E, 0xFF, 0x06, 0x40, 0x70};
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[255], 0x40);
	delete results;
}

TEST(LD_HL_ptr_C, LOAD_64) { //0x71
	uint8_t memory[256] = { 0x2E, 0xFF, 0x0E, 0x40, 0x71 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[255], 0x40);
	delete results;
}

TEST(LD_HL_ptr_D, LOAD_64) { //0x72
	uint8_t memory[256] = { 0x2E, 0xFF, 0x16, 0x40, 0x72 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[255], 0x40);
	delete results;
}

TEST(LD_HL_ptr_E, LOAD_64) { //0x73
	uint8_t memory[256] = { 0x2E, 0xFF, 0x1E, 0x40, 0x73 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[255], 0x40);
	delete results;
}

TEST(LD_HL_ptr_H, LOAD_64) { //0x74
	uint8_t memory[65536] = { 0x2E, 0xFF, 0x26, 0x40, 0x74 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[0x40FF], 0x40);
	delete results;
}

TEST(LD_HL_ptr_L, LOAD_64) { //0x75
	uint8_t memory[65536] = { 0x2E, 0xFF, 0x26, 0x40, 0x75 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[0x40FF], 0xFF);
	delete results;
}

TEST(LD_HL_ptr_A, LOAD_64) { //0x77
	uint8_t memory[256] = { 0x2E, 0xFF, 0x3E, 0x40, 0x77 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[255], 0x40);
	delete results;
}

TEST(POP_BC, check_registers_are_same) { //0xC1
	uint8_t memory[256] = { 0x31, 0x80, 0x00, 0x01, 0xB0, 0xA0, 0xC5, 0x01, 0x00, 0x00, 0xC1 };
	cpuDebugger* results = runAndDebug(memory, 30);

	EXPECT_EQ(results->getAllRegisters(), 0x0000A0B000000000);
	delete results;
}

TEST(PUSH_BC, Push_to_0x40_check_MSB) { //0xC5
	uint8_t memory[128] = { 0x31, 0x40, 0x00, 0x01, 0xC0, 0xB0, 0xC5 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(memory[0x3F], 0xB0);
	delete results;
}

TEST(PUSH_BC, Push_to_0x40_check_LSB) { //0xC5
	uint8_t memory[128] = { 0x31, 0x40, 0x00, 0x01, 0xC0, 0xB0, 0xC5 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(memory[0x3E], 0xC0);
	delete results;
}

TEST(POP_DE, check_registers_are_same) { //0xD1
	uint8_t memory[256] = { 0x31, 0x80, 0x00, 0x11, 0xB0, 0xA0, 0xD5, 0x11, 0x00, 0x00, 0xD1 };
	cpuDebugger* results = runAndDebug(memory, 30);

	EXPECT_EQ(results->getAllRegisters(), 0x00000000A0B00000);
	delete results;
}

TEST(PUSH_DE, Push_to_0x40_check_MSB) { //0xD5
	uint8_t memory[128] = { 0x31, 0x40, 0x00, 0x11, 0xC0, 0xB0, 0xD5 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(memory[0x3F], 0xB0);
	delete results;
}

TEST(PUSH_DE, Push_to_0x40_check_LSB) { //0xD5
	uint8_t memory[128] = { 0x31, 0x40, 0x00, 0x11, 0xC0, 0xB0, 0xD5 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(memory[0x3E], 0xC0);
	delete results;
}

TEST(LDH_n_ptr_A, Load_to_0xFFFF) { //0xE0
	uint8_t memory[65536] = {0x3E, 0xA0, 0xE0, 0xFF};
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[0xFFFF], 0xA0);
	delete results;
}

TEST(POP_HL, check_registers_are_same) { //0xE1
	uint8_t memory[256] = { 0x31, 0x80, 0x00, 0x21, 0xB0, 0xA0, 0xE5, 0x21, 0x00, 0x00, 0xE1 };
	cpuDebugger* results = runAndDebug(memory, 30);

	EXPECT_EQ(results->getAllRegisters(), 0x000000000000A0B0);
	delete results;
}

TEST(LD_C_ptr_A, Load_to_0xFFFF) { //0xE2
	uint8_t memory[65536] = { 0x3E, 0xA0, 0x0E, 0xFF, 0xE2};
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[0xFFFF], 0xA0);
	delete results;
}

TEST(PUSH_HL, Push_to_0x40_check_MSB) { //0xE5
	uint8_t memory[128] = { 0x31, 0x40, 0x00, 0x21, 0xC0, 0xB0, 0xE5 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(memory[0x3F], 0xB0);
	delete results;
}

TEST(PUSH_HL, Push_to_0x40_check_LSB) { //0xE5
	uint8_t memory[128] = { 0x31, 0x40, 0x00, 0x21, 0xC0, 0xB0, 0xE5 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(memory[0x3E], 0xC0);
	delete results;
}

TEST(LD_nn_ptr_A, Load_255) { //0xEA
	uint8_t memory[1024] = { 0x3E, 0xFF, 0xEA, 0xFF, 0x01 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(memory[0x1FF], 0xFF);
	delete results;
}

TEST(LDH_A_n_ptr, Load_from_0xFFFF) { //0xF0
	uint8_t memory[65536] = {0xF0, 0xFF};
	memory[65535] = 0xA0;
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->AF.half[1], 0xA0);
	delete results;
}

TEST(POP_AF, check_registers_are_same) { //0xF1
	uint8_t memory[256] = { 0x31, 0x80, 0x00, 0x3E, 0xA0, 0xF5, 0x3E, 0x00, 0xF1 };
	cpuDebugger* results = runAndDebug(memory, 30);

	EXPECT_EQ(results->getAllRegisters(), 0xA000000000000000);
	delete results;
}

TEST(LD_A_C_ptr, Load_from_0xFFFF) { //0xF2
	uint8_t memory[65536] = { 0x0E, 0xFF, 0xF2 };
	memory[65535] = 0xA0;
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->AF.half[1], 0xA0);
	delete results;
}

TEST(PUSH_AF, Push_to_0x40_check_MSB) { //0xF5
	uint8_t memory[128] = { 0x31, 0x40, 0x00, 0x3E, 0xB0, 0xF5 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(memory[0x3F], 0xB0);
	delete results;
}

TEST(PUSH_AF, Push_to_0x40_check_LSB) { //0xF5
	uint8_t memory[128] = { 0x31, 0x40, 0x00, 0x3E, 0xB0, 0xF5 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(memory[0x3E], 0x00);
	delete results;
}

TEST(LD_HL_SP_plus_r8, Test_plus_1) { //0xF8
	uint8_t memory[128] = { 0x31, 0x40, 0x00, 0xF8, 0x01 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(results->HL.full, 0x41);
	delete results;
}

TEST(LD_HL_SP_plus_r8, Test_minus_128) { //0xF8
	uint8_t memory[128] = { 0x31, 0xFF, 0x00, 0xF8, 0x80 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(results->HL.full, 0x7F);
	delete results;
}

TEST(LD_HL_SP_plus_r8, Test_half_carry) { //0xF8
	uint8_t memory[128] = { 0x31, 0x08, 0x00, 0xF8, 0x08 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(results->AF.half[0], 0b00100000);
	delete results;
}

TEST(LD_HL_SP_plus_r8, Test_full_carry) { //0xF8
	uint8_t memory[128] = { 0x31, 0x80, 0x00, 0xF8, 0x80 };
	cpuDebugger* results = runAndDebug(memory, 20);

	EXPECT_EQ(results->AF.half[0], 0b00010000);
	delete results;
}

TEST(LD_SP_HL, Load_FFA0) { //0xF9
	uint8_t memory[10] = { 0x21, 0xA0, 0xFF, 0xF9 };
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->SP, 0xFFA0);
	delete results;
}

TEST(LD_A_nn_ptr, Load_255) { //0xFA
	uint8_t memory[1024] = { 0xFA, 0xFF, 0x01};
	memory[0x1FF] = 0xFF;
	cpuDebugger* results = runAndDebug(memory, 10);

	EXPECT_EQ(results->AF.half[1], 0xFF);
	delete results;
}