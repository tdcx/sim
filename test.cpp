#include "pch.h"

class M6502Test : public testing::Test{
public:
	Mem mem;
	CPU cpu;

	virtual void  SetUp() {
		cpu.Reset(mem);
	}
	virtual void  TearDown() {

	}
};

static void VerifyUnmodifiedFlagsFromLDA(const CPU& cpu, const CPU& CPUCopy) {
	EXPECT_EQ(cpu.C, CPUCopy.C);
	EXPECT_EQ(cpu.I, CPUCopy.I);
	EXPECT_EQ(cpu.D, CPUCopy.D);
	EXPECT_EQ(cpu.B, CPUCopy.B);
	EXPECT_EQ(cpu.O, CPUCopy.O);
}

TEST_F(M6502Test, CPU_DoesNothingWhenNoCycles) {
	// given:
	constexpr s32 EXPECTED_CYCLES = 0;
	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);
}

TEST_F(M6502Test, CPU_CanExecuteMoreCyclesThanRequestedIfRequiredByTheInstruction) {
	// given:
	mem[0xFFFC] = CPU::INS_LDA_IM;
	mem[0xFFFD] = 0x84;
	//when:
	CPU CPUCopy = cpu;
	constexpr s32 EXPECTED_CYCLES = 1;

	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:

	EXPECT_EQ(CyclesUsed, 2);
}

TEST_F(M6502Test, ExecutingA_BadInstructionDoesNotPutUsInAnInfiniteLoop) {
	// given:
	mem[0xFFFC] = 0x0; // unvalid instruction
	mem[0xFFFD] = 0x0;
	constexpr s32 NUM_CYCLES = 0;
	//when:
	s32 CyclesUsed = cpu.Execute(NUM_CYCLES, mem);
	CPU CPUCopy = cpu;
	//then:

	EXPECT_EQ(CyclesUsed, NUM_CYCLES);
}

TEST_F(M6502Test, LDA_IM_CanLoadValueInRegister_A) {
	// given:
	mem[0xFFFC] = CPU::INS_LDA_IM;
	mem[0xFFFD] = 0x84;
	//when:
	CPU CPUCopy = cpu;
	constexpr s32 EXPECTED_CYCLES = 2;

	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(cpu.A, 0x84);
	EXPECT_FALSE(cpu.Z);
	EXPECT_TRUE(cpu.N);

	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);

	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);
}

TEST_F(M6502Test, LDA_IM_CanLoadZeroValueInRegister_A) {
	// given:
	cpu.A = 0x44;
	mem[0xFFFC] = CPU::INS_LDA_IM;
	mem[0xFFFD] = 0x0;
	//when:
	CPU CPUCopy = cpu;
	constexpr s32 EXPECTED_CYCLES = 2;

	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(cpu.A, 0x0);
	EXPECT_TRUE(cpu.Z);
	EXPECT_FALSE(cpu.N);

	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);

	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);
}

TEST_F(M6502Test, LDA_ZP_CanLoadValueInRegister_A) {
	// given:
	mem[0xFFFC] = CPU::INS_LDA_ZP;
	mem[0xFFFD] = 0x42;
	mem[0x0042] = 0x37;
	//when:
	CPU CPUCopy = cpu;
	constexpr s32 EXPECTED_CYCLES = 3;

	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);

	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);

	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);

}

TEST_F(M6502Test, LDA_ZPX_CanLoadValueInRegister_A) {
	// given:
	cpu.X = 5;

	mem[0xFFFC] = CPU::INS_LDA_ZPX;
	mem[0XFFFD] = 0x42;
	mem[0x0047] = 0x37;
	//when:
	CPU CPUCopy = cpu;
	constexpr s32 EXPECTED_CYCLES = 4;

	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);

	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);

	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);

}

TEST_F(M6502Test, LDA_ZPX_CanLoadValueInRegister_A_WhenItWraps) {
	// given:
	cpu.X = 0xFF;

	mem[0xFFFC] = CPU::INS_LDA_ZPX;
	mem[0XFFFD] = 0x80;
	mem[0x007F] = 0x37;
	//when:
	CPU CPUCopy = cpu;
	constexpr s32 EXPECTED_CYCLES = 4;

	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);

	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);

	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);

}

TEST_F(M6502Test, LDA_ABS_CanLoadValueInRegister_A) {
	// given:
	cpu.X = 0xFF;

	mem[0xFFFC] = CPU::INS_LDA_ABS;
	mem[0XFFFD] = 0x80;
	mem[0xFFFE] = 0x44; //0x4480
	mem[0x4480] = 0x37;

	constexpr s32 EXPECTED_CYCLES = 4;
	CPU CPUCopy = cpu;
	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test, LDA_ABS_X_CanLoadValueInRegister_A) {
	// given:
	cpu.X = 0xFF;

	mem[0xFFFC] = CPU::INS_LDA_ABSY;
	mem[0XFFFD] = 0x80;
	mem[0xFFFE] = 0x44; //0x4480
	mem[0x4480] = 0x37;

	constexpr s32 EXPECTED_CYCLES = 4;
	CPU CPUCopy = cpu;
	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test, LDA_ABS_X_CanLoadValueInRegister_A_WhenItCrossesPage) {
	// given:
	cpu.X = 0xFF;

	mem[0xFFFC] = CPU::INS_LDA_ABSX;
	mem[0XFFFD] = 0x00;
	mem[0xFFFE] = 0x20; //0x3000
	mem[0x3092] = 0x37; //0x3092 + 0xFF crosses page boundaries

	constexpr s32 EXPECTED_CYCLES = 5;
	CPU CPUCopy = cpu;
	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}
TEST_F(M6502Test, LDA_ABS_Y_CanLoadValueInRegister_A) {
	// given:
	cpu.Y = 0xFF;

	mem[0xFFFC] = CPU::INS_LDA_ABSY;
	mem[0XFFFD] = 0x80;
	mem[0xFFFE] = 0x44; //0x4480
	mem[0x4480] = 0x37;

	constexpr s32 EXPECTED_CYCLES = 4;
	CPU CPUCopy = cpu;
	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

TEST_F(M6502Test, LDA_ABS_Y_CanLoadValueInRegister_A_WhenItCrossesPage) {
	// given:
	cpu.Y = 0xFF;

	mem[0xFFFC] = CPU::INS_LDA_ABSY;
	mem[0XFFFD] = 0x00;
	mem[0xFFFE] = 0x20; //0x3000
	mem[0x3092] = 0x37; //0x3092 + 0xFF crosses page boundaries

	constexpr s32 EXPECTED_CYCLES = 5;
	CPU CPUCopy = cpu;
	//when:
	s32 CyclesUsed = cpu.Execute(EXPECTED_CYCLES, mem);
	//then:
	EXPECT_EQ(cpu.A, 0x37);
	EXPECT_EQ(EXPECTED_CYCLES, CyclesUsed);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}
