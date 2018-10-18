//
// Created by Taylor Whatley on 2018-10-17.
//

#include "Disassembler.h"

namespace Nem {
//    string opNames[] = {
//            "BRK", // NAME 0x00
//            "ORA $X", // NAME 0x01
//            "UNI", //STPInstruction, // NAME 0x02
//            "SLO $X", // NAME 0x03
//            "NOP D", // NAME 0x04
//            "ORA D", // NAME 0x05
//            "ASL D", // NAME 0x06
//            "SLO D", // NAME 0x07
//            "PHP", // NAME 0x08
//            "ORA I", // NAME 0x09
//            "ASL", // NAME 0x0A
//            "ANC I", // NAME 0x0B
//            "NOP A", // NAME 0x0C
//            "ORA A", // NAME 0x0D
//            "ASL A", // NAME 0x0E
//            "SLO A", //SLOInstruction_a, // NAME 0x0F
//            "BPL PD", // NAME 0x10
//            "ORA $Y", // NAME 0x11
//            "UNI", //STPInstruction, // NAME 0x12
//            "SLO $Y", // NAME 0x13
//            "NOP DX", // NAME 0x14
//            "ORA DX", // NAME 0x15
//            "ASL DX", // NAME 0x16
//            "SLO DX", // NAME 0x17
//            "CLC", // NAME 0x18
//            "ORA AY", // NAME 0x19
//            "NOP", // NAME 0x1A
//            "SLO AY", // NAME 0x1B
//            "NOP AX", // NAME 0x1C
//            "ORA AX", // NAME 0x1D
//            "ASL AX", // NAME 0x1E
//            "SLO AX", // NAME 0x1F
//            "JSR A", // NAME 0x20
//            "AND $X", // NAME 0x21
//            "UNI", //STPInstruction, // NAME 0x22
//            "RLA $X", // NAME 0x23
//            "BIT D", // NAME 0x24
//            "AND D", // NAME 0x25
//            "ROL D", // NAME 0x26
//            "RLA D", // NAME 0x27
//            "PLP", // NAME 0x28
//            "AND I", // NAME 0x29
//            "ROL", // NAME 0x2A
//            "ANC I", // NAME 0x2B
//            "BIT A", // NAME 0x2C
//            "AND A", // NAME 0x2D
//            "ROL A", // NAME 0x2E
//            "RLA A", // NAME 0x2F
//            "BMI PD", // NAME 0x30
//            "AND $Y", // NAME 0x31
//            "UNI", //STPInstruction, // NAME 0x32
//            "RLA $Y", // NAME 0x33
//            "NOP DX", // NAME 0x34
//            "AND DX", // NAME 0x35
//            "ROL DX", // NAME 0x36
//            "RLA DX", // NAME 0x37
//            "SEC", // NAME 0x38
//            "AND AY", // NAME 0x39
//            "NOP", // NAME 0x3A
//            "RLA AY", // NAME 0x3B
//            "NOP AX", // NAME 0x3C
//            "AND AX", // NAME 0x3D
//            "ROL AX", // NAME 0x3E
//            "RLA AX", // NAME 0x3F
//            "RTI", // NAME 0x40
//            "EOR $X", // NAME 0x41
//            "UNI", //STPInstruction, // NAME 0x42
//            "SRE $X", // NAME 0x43
//            "NOP D", // NAME 0x44
//            "EOR D", // NAME 0x45
//            "LSR D", // NAME 0x46
//            "SRE D", // NAME 0x47
//            "PHA", // NAME 0x48
//            "EOR I", // NAME 0x49
//            "LSR", // NAME 0x4A
//            "ALR I", // NAME 0x4B
//            "JMP A", // NAME 0x4C
//            "EOR A", // NAME 0x4D
//            "LSR A", // NAME 0x4E
//            "SRE A", // NAME 0x4F
//            "BVC PD", // NAME 0x50
//            "EOR $Y", // NAME 0x51
//            "UNI", //STPInstruction, // NAME 0x52
//            "SRE $Y", // NAME 0x53
//            "NOP DX", // NAME 0x54
//            "EOR DX", // NAME 0x55
//            "LSR DX", // NAME 0x56
//            "SRE DX", // NAME 0x57
//            "CLI", // NAME 0x58
//            "EOR AY", // NAME 0x59
//            "NOP", // NAME 0x5A
//            "SRE AY", // NAME 0x5B
//            "NOP AX", // NAME 0x5C
//            "EOR AX", // NAME 0x5D
//            "LSR AX", // NAME 0x5E
//            "SRE AX", // NAME 0x5F
//            "RTS", // NAME 0x60
//            "ADC $X", // NAME 0x61
//            "UNI", //STPInstruction, // NAME 0x62
//            "RRA $X", // NAME 0x63
//            "NOP D", // NAME 0x64
//            "ADC D", // NAME 0x65
//            "ROR D", // NAME 0x66
//            "RRA D", // NAME 0x67
//            "PLA", // NAME 0x68
//            "ADC I", // NAME 0x69
//            "ROR", // NAME 0x6A
//            "ARR I", // NAME 0x6B
//            "JMP $", // NAME 0x6C
//            "ADC A", // NAME 0x6D
//            "ROR A", // NAME 0x6E
//            "RRA A", // NAME 0x6F
//            "BVS PD", // NAME 0x70
//            "ADC $Y", // NAME 0x71
//            "UNI", //STPInstruction, // NAME 0x72
//            "RRA $Y", // NAME 0x73
//            "NOP DX", // NAME 0x74
//            "ADC DX", // NAME 0x75
//            "ROR DX", // NAME 0x76
//            "RRA DX", // NAME 0x77
//            "SEI", // NAME 0x78
//            "ADC AY", // NAME 0x79
//            "NOP", // NAME 0x7A
//            "RRA AY", // NAME 0x7B
//            "NOP AX", // NAME 0x7C
//            "ADC AX", // NAME 0x7D
//            "ROR AX", // NAME 0x7E
//            "RRA AX", // NAME 0x7F
//            "NOP I", // NAME 0x80
//            "STA $X", // NAME 0x81
//            "NOP I", // NAME 0x82
//            "SAX $X", // NAME 0x83
//            "STY D", // NAME 0x84
//            "STA D", // NAME 0x85
//            "STX D", // NAME 0x86
//            "SAX D", // NAME 0x87
//            "DEY", // NAME 0x88
//            "NOP I", // NAME 0x89
//            "TXA", // NAME 0x8A
//            "UNI", //XAAInstruction_i, // NAME 0x8B
//            "STY A", // NAME 0x8C
//            "STA A", // NAME 0x8D
//            "STX A", // NAME 0x8E
//            "SAX A", // NAME 0x8F
//            "BCC PD", // NAME 0x90
//            "STA $Y", // NAME 0x91
//            "UNI", //STPInstruction, // NAME 0x92
//            "UNI", //AHXInstruction_d_y, // NAME 0x93
//            "STY DX", // NAME 0x94
//            "STA DX", // NAME 0x95
//            "STX DY", // NAME 0x96
//            "SAX DY", // NAME 0x97
//            "TYA", // NAME 0x98
//            "STA AY", // NAME 0x99
//            "TXS", // NAME 0x9A
//            "UNI", //TASInstruction_a_y, // NAME 0x9B
//            "UNI", //SHYInstruction_a_x, // NAME 0x9C
//            "STA AX", // NAME 0x9D
//            "UNI", //SHXInstruction_a_y, // NAME 0x9E
//            "UNI", //AHXInstruction_a_y, // NAME 0x9F
//            "LDY I", // NAME 0xA0
//            "LDA $X", // NAME 0xA1
//            "LDX I", // NAME 0xA2
//            "LAX $X", // NAME 0xA3
//            "LDY D", // NAME 0xA4
//            "LDA D", // NAME 0xA5
//            "LDX D", // NAME 0xA6
//            "LAX D", // NAME 0xA7
//            "TAY", // NAME 0xA8
//            "LDA I", // NAME 0xA9
//            "TAX", // NAME 0xAA
//            "LAX I", // NAME 0xAB
//            "LDY A", // NAME 0xAC
//            "LDA A", // NAME 0xAD
//            "LDX A", // NAME 0xAE
//            "LAX A", // NAME 0xAF
//            "BCS PD", // NAME 0xB0
//            "LDA $Y", // NAME 0xB1
//            "UNI", //STPInstruction, // NAME 0xB2
//            "LAX $Y", // NAME 0xB3
//            "LDY DX", // NAME 0xB4
//            "LDA DX", // NAME 0xB5
//            "LDX DY", // NAME 0xB6
//            "LAX DY", // NAME 0xB7
//            "CLV", // NAME 0xB8
//            "LDA AY", // NAME 0xB9
//            "TSX", // NAME 0xBA
//            "UNI", //LASInstruction_a_y, // NAME 0xBB
//            "LDY AX", // NAME 0xBC
//            "LDA AX", // NAME 0xBD
//            "LDX AY", // NAME 0xBE
//            "LAX AY", // NAME 0xBF
//            "CPY I", // NAME 0xC0
//            "CMP $X", // NAME 0xC1
//            "NOP I", // NAME 0xC2
//            "DCP $X", // NAME 0xC3
//            "CPY D", // NAME 0xC4
//            "CMP D", // NAME 0xC5
//            "DEC D", // NAME 0xC6
//            "DCP D", // NAME 0xC7
//            "INY", // NAME 0xC8
//            "CMP I", // NAME 0xC9
//            "DEX", // NAME 0xCA
//            "AXS I", // NAME 0xCB
//            "CPY A", // NAME 0xCC
//            "CMP A", // NAME 0xCD
//            "DEC A", // NAME 0xCE
//            "DCP A", // NAME 0xCF
//            "BNE PD", // NAME 0xD0
//            "CMP $Y", // NAME 0xD1
//            "UNI", //STPInstruction, // NAME 0xD2
//            "DCP $Y", // NAME 0xD3
//            "NOP DX", // NAME 0xD4
//            "CMP DX", // NAME 0xD5
//            "DEC DX", // NAME 0xD6
//            "DCP DX", // NAME 0xD7
//            "CLD", // NAME 0xD8
//            "CMP AY", // NAME 0xD9
//            "NOP", // NAME 0xDA
//            "DCP AY", // NAME 0xDB
//            "NOP AX", // NAME 0xDC
//            "CMP AX", // NAME 0xDD
//            "DEC AX", // NAME 0xDE
//            "DCP AX", // NAME 0xDF
//            "CPX I", // NAME 0xE0
//            "SBC $X", // NAME 0xE1
//            "NOP I", // NAME 0xE2
//            "ISB $X", // NAME 0xE3
//            "CPX D", // NAME 0xE4
//            "SBC D", // NAME 0xE5
//            "INC D", // NAME 0xE6
//            "ISB D", // NAME 0xE7
//            "INX", // NAME 0xE8
//            "SBC I", // NAME 0xE9
//            "NOP", // NAME 0xEA
//            "SBC I", // NAME 0xEB
//            "CPX A", // NAME 0xEC
//            "SBC A", // NAME 0xED
//            "INC A", // NAME 0xEE
//            "ISB A", // NAME 0xEF
//            "BEQ PD", // NAME 0xF0
//            "SBC $Y", // NAME 0xF1
//            "UNI", //STPInstruction, // NAME 0xF2
//            "ISB $y", // NAME 0xF3
//            "NOP DX", // NAME 0xF4
//            "SBC DX", // NAME 0xF5
//            "INC DX", // NAME 0xF6
//            "ISB DX", // NAME 0xF7
//            "SED", // NAME 0xF8
//            "SBC AY", // NAME 0xF9
//            "NOP", // NAME 0xFA
//            "ISB AY", // NAME 0xFB
//            "NOP AX", // NAME 0xFC
//            "SBC AX", // NAME 0xFD
//            "INC AX", // NAME 0xFE
//            "ISB AX", // NAME 0xFF
//    };
}