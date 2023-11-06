//
// pch.h
//

#pragma once

#include "gtest/gtest.h"

#include <stdio.h>
#include <stdlib.h>
#include <cassert>

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;
using s32 = signed int;

struct Mem {
	static constexpr u32 MAX_MEM = 1024 * 64;
	Byte Data[MAX_MEM];

	void Initialise() {
		for (u32 i = 0; i < MAX_MEM; i++) {
			Data[i] = 0;
		}
	}
	
	Byte operator[](u32 Address) const {
		assert(Address < MAX_MEM);
		return Data[Address];
	}

	Byte& operator[](u32 Address) {
		assert(Address < MAX_MEM);
		return Data[Address];
	}

	void WriteWord(s32& Cycles, Word Value, u32 Address) {
		Data[Address] = Value & 0xFF;
		Data[Address + 1] = (Value >> 8);
		Cycles -= 2;
	}
};

struct CPU {

	Word PC; // Program Counter
	Byte SP; // Stack Pointer

	Byte A, X, Y;  // Registros

	Byte C : 1; // Estado del procesador
	Byte Z : 1; // Estado del procesador
	Byte I : 1; // Estado del procesador
	Byte D : 1; // Estado del procesador
	Byte B : 1; // Estado del procesador
	Byte O : 1; // Estado del procesador
	Byte N : 1; // Estado del procesador

	void Reset(Mem& memory) {
		PC = 0xFFFC;
		SP = 0x0100;
		C = Z = I = D = B = O = N = 0;
		A = X = Y = 0;

		memory.Initialise();
	}

	Byte FetchByte(s32& Cycles, Mem& memory) {
		Byte Data = memory[PC];
		PC++;
		Cycles--;
		return Data;
	}

	Word FetchWord(s32& Cycles, Mem& memory) {
		Word Data = memory[PC];
		PC++;
		Data |= (memory[PC] << 8);
		PC++;
		Cycles -= 2;
		return Data;
	}

	Byte ReadByte(s32& Cycles, Byte Address, Mem& memory) {
		Byte Data = memory[Address];
		Cycles--;
		return Data;
	}

	static constexpr Byte INS_LDA_IM = 0xA9,
		INS_LDA_ZP = 0xA5,
		INS_LDA_ZPX = 0xB5,
		INS_LDA_ABS = 0xAD,
		INS_LDA_ABSX = 0xBD,
		INS_LDA_ABSY = 0xB9,
		INS_LDA_INX = 0xA1,
		INS_LDA_INY = 0xB1,
		INS_JSR = 0x20;
		

	void LDASetStatus() {
		Z = (A == 0);
		N = ((A >> 7) & 1);
	}
	

	s32 Execute(s32 Cycles, Mem& memory) {
		const s32 CyclesRequested = Cycles;
		while (Cycles > 0) {
			Byte Instruction = FetchByte(Cycles, memory);
			switch (Instruction) {
			case INS_LDA_IM: {
				Byte Value = FetchByte(Cycles, memory);
				A = Value;
				LDASetStatus();
			}break;
			case INS_LDA_ZP: {
				Byte ZeroPageAddress = FetchByte(Cycles, memory);
				A = ReadByte(Cycles, ZeroPageAddress, memory);
				LDASetStatus();
			}break;
			case INS_LDA_ZPX: {
				Byte ZeroPageAddress = FetchByte(Cycles, memory);
				ZeroPageAddress += X;
				Cycles--;
				A = ReadByte(Cycles, ZeroPageAddress, memory);
				LDASetStatus();
			}break;
			case INS_LDA_ABS: {
				Word SubAddress = FetchWord(Cycles, memory);
				memory.WriteWord(Cycles, PC - 1, SP);
				PC = SubAddress;
				SP += 2;
				Cycles--;
			}break;
			case INS_LDA_ABSX: {
				Word SubAddress = FetchWord(Cycles, memory);
				memory.WriteWord(Cycles, PC - 1, SP);
				PC = SubAddress;
				SP += 2;
				Cycles--;
			}break;
			case INS_JSR: {
				
				
			}break;
			default: {
				printf("Instruction not handled! %u\n", Instruction);
			}break;
			}
		}
		const s32 NumCyclesUsed = CyclesRequested - Cycles;
		return NumCyclesUsed;
	}
};
