#ifndef __CPU_H__
#define __CPU_H__

#include <cstdint>

#define HIGH_BYTE 1
#define LOW_BYTE 0

#define NEW_CYCLE 255

union registerPair {
	uint16_t full;
	uint8_t half[2];
};

class gbcpu;

class cpuDebugger {
	public: 
		registerPair AF;
		registerPair BC;
		registerPair DE;
		registerPair HL;

		uint16_t SP;
		uint16_t PC;

	public:
		cpuDebugger(gbcpu target);
		uint64_t getAllRegisters();
		uint32_t getBothPointers();
};

class gbcpu {
	friend class cpuDebugger;

	private:
		/* registers */
		registerPair AF;
		registerPair BC;
		registerPair DE;
		registerPair HL;

		uint16_t SP;
		uint16_t PC;

		uint8_t* memory;

		/* execution variables */
		uint8_t opcode;
		uint8_t cycle;

	public:
		gbcpu(uint8_t* memory);
		void tick(); //one machine cycle
		void registerDump();
};

#endif