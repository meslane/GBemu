#include "pch.h"

#include "../cpu.h"

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

TEST(NOP, PCvalue) {
	uint8_t memory[4] = { 0 };
	gbcpu* gb = new gbcpu(memory);

	gb->tick();
	gb->tick();
	gb->tick();

	cpuDebugger* results = new cpuDebugger(*gb);

	EXPECT_EQ(results->PC, 3);
}

TEST(LD_B_d8, Load_255) {
	uint8_t memory[2] = { 0x06, 0xFF };
	gbcpu* gb = new gbcpu(memory);

	gb->tick();
	gb->tick();
	gb->tick();

	cpuDebugger* results = new cpuDebugger(*gb);

	EXPECT_EQ(results->BC.half[1], 0xFF);
}

TEST(LC_C_d8, Load_255) {
	uint8_t memory[2] = { 0x0E, 0xFF };
	gbcpu* gb = new gbcpu(memory);

	gb->tick();
	gb->tick();
	gb->tick();

	cpuDebugger* results = new cpuDebugger(*gb);

	EXPECT_EQ(results->BC.half[0], 0xFF);
}

TEST(LD_D_d8, Load_255) {
	uint8_t memory[2] = { 0x16, 0xFF };
	gbcpu* gb = new gbcpu(memory);

	gb->tick();
	gb->tick();
	gb->tick();

	cpuDebugger* results = new cpuDebugger(*gb);

	EXPECT_EQ(results->DE.half[1], 0xFF);
}

