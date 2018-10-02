//
// Created by Taylor Whatley on 2018-09-19.
//

#ifndef NEM_OPCODES_H
#define NEM_OPCODES_H

#include "../Internal.h"

#include <functional>

class CPU;

namespace Nem {
    typedef std::function<int(CPU*)> Instruction;

    int Unimplemented(CPU* cpu);

    int NOPInstruction(CPU* cpu);
    int NOPInstruction_i(CPU* cpu);
    int NOPInstruction_d(CPU* cpu);
    int NOPInstruction_d_x(CPU* cpu);
    int NOPInstruction_a(CPU* cpu);
    int NOPInstruction_a_x(CPU* cpu);

    // Math
    int ADCInstruction_i(CPU* cpu);
    int ADCInstruction_d(CPU* cpu);
    int ADCInstruction_d_x(CPU* cpu);
    int ADCInstruction_a(CPU* cpu);
    int ADCInstruction_a_x(CPU* cpu);
    int ADCInstruction_a_y(CPU* cpu);
    int ADCInstruction_$x(CPU* cpu);
    int ADCInstruction_$y(CPU* cpu);

    int SBCInstruction_i(CPU* cpu);
    int SBCInstruction_d(CPU* cpu);
    int SBCInstruction_d_x(CPU* cpu);
    int SBCInstruction_a(CPU* cpu);
    int SBCInstruction_a_x(CPU* cpu);
    int SBCInstruction_a_y(CPU* cpu);
    int SBCInstruction_$x(CPU* cpu);
    int SBCInstruction_$y(CPU* cpu);

    int INCInstruction_d(CPU* cpu);
    int INCInstruction_d_x(CPU* cpu);
    int INCInstruction_a(CPU* cpu);
    int INCInstruction_a_x(CPU* cpu);

    int DECInstruction_d(CPU* cpu);
    int DECInstruction_d_x(CPU* cpu);
    int DECInstruction_a(CPU* cpu);
    int DECInstruction_a_x(CPU* cpu);

    int ISCInstruction_d(CPU* cpu);
    int ISCInstruction_d_x(CPU* cpu);
    int ISCInstruction_a(CPU* cpu);
    int ISCInstruction_a_x(CPU* cpu);
    int ISCInstruction_a_y(CPU* cpu);
    int ISCInstruction_$x(CPU* cpu);
    int ISCInstruction_$y(CPU* cpu);

    int DCPInstruction_d(CPU* cpu);
    int DCPInstruction_d_x(CPU* cpu);
    int DCPInstruction_a(CPU* cpu);
    int DCPInstruction_a_x(CPU* cpu);
    int DCPInstruction_a_y(CPU* cpu);
    int DCPInstruction_$x(CPU* cpu);
    int DCPInstruction_$y(CPU* cpu);

    int INXInstruction(CPU* cpu);
    int DEXInstruction(CPU* cpu);
    int INYInstruction(CPU* cpu);
    int DEYInstruction(CPU* cpu);

    // Bitwise
    int ANDInstruction_i(CPU* cpu);
    int ANDInstruction_a(CPU* cpu);
    int ANDInstruction_a_x(CPU* cpu);
    int ANDInstruction_a_y(CPU* cpu);
    int ANDInstruction_d(CPU* cpu);
    int ANDInstruction_d_x(CPU* cpu);
    int ANDInstruction_$x(CPU* cpu);
    int ANDInstruction_$y(CPU* cpu);

    int ORAInstruction_i(CPU* cpu);
    int ORAInstruction_d(CPU* cpu);
    int ORAInstruction_d_x(CPU* cpu);
    int ORAInstruction_a(CPU* cpu);
    int ORAInstruction_a_x(CPU* cpu);
    int ORAInstruction_a_y(CPU* cpu);
    int ORAInstruction_$x(CPU* cpu);
    int ORAInstruction_$y(CPU* cpu);

    int EORInstruction_i(CPU* cpu);
    int EORInstruction_d(CPU* cpu);
    int EORInstruction_d_x(CPU* cpu);
    int EORInstruction_a(CPU* cpu);
    int EORInstruction_a_x(CPU* cpu);
    int EORInstruction_a_y(CPU* cpu);
    int EORInstruction_$x(CPU* cpu);
    int EORInstruction_$y(CPU* cpu);

    int ASLInstruction(CPU* cpu);
    int ASLInstruction_d(CPU* cpu);
    int ASLInstruction_d_x(CPU* cpu);
    int ASLInstruction_a(CPU* cpu);
    int ASLInstruction_a_x(CPU* cpu);

    int LSRInstruction(CPU* cpu);
    int LSRInstruction_d(CPU* cpu);
    int LSRInstruction_d_x(CPU* cpu);
    int LSRInstruction_a(CPU* cpu);
    int LSRInstruction_a_x(CPU* cpu);

    int ROLInstruction(CPU* cpu);
    int ROLInstruction_d(CPU* cpu);
    int ROLInstruction_d_x(CPU* cpu);
    int ROLInstruction_a(CPU* cpu);
    int ROLInstruction_a_x(CPU* cpu);

    int RORInstruction(CPU* cpu);
    int RORInstruction_d(CPU* cpu);
    int RORInstruction_d_x(CPU* cpu);
    int RORInstruction_a(CPU* cpu);
    int RORInstruction_a_x(CPU* cpu);

    int SLOInstruction_d(CPU* cpu);
    int SLOInstruction_d_x(CPU* cpu);
    int SLOInstruction_a(CPU* cpu);
    int SLOInstruction_a_x(CPU* cpu);
    int SLOInstruction_a_y(CPU* cpu);
    int SLOInstruction_$x(CPU* cpu);
    int SLOInstruction_$y(CPU* cpu);

    int RLAInstruction_d(CPU* cpu);
    int RLAInstruction_d_x(CPU* cpu);
    int RLAInstruction_a(CPU* cpu);
    int RLAInstruction_a_x(CPU* cpu);
    int RLAInstruction_a_y(CPU* cpu);
    int RLAInstruction_$x(CPU* cpu);
    int RLAInstruction_$y(CPU* cpu);

    int RRAInstruction_d(CPU* cpu);
    int RRAInstruction_d_x(CPU* cpu);
    int RRAInstruction_a(CPU* cpu);
    int RRAInstruction_a_x(CPU* cpu);
    int RRAInstruction_a_y(CPU* cpu);
    int RRAInstruction_$x(CPU* cpu);
    int RRAInstruction_$y(CPU* cpu);

    int SREInstruction_d(CPU* cpu);
    int SREInstruction_d_x(CPU* cpu);
    int SREInstruction_a(CPU* cpu);
    int SREInstruction_a_x(CPU* cpu);
    int SREInstruction_a_y(CPU* cpu);
    int SREInstruction_$x(CPU* cpu);
    int SREInstruction_$y(CPU* cpu);

    int BITInstruction_d(CPU* cpu);
    int BITInstruction_a(CPU* cpu);

    // Compare
    int CMPInstruction_i(CPU* cpu);
    int CMPInstruction_d(CPU* cpu);
    int CMPInstruction_d_x(CPU* cpu);
    int CMPInstruction_a(CPU* cpu);
    int CMPInstruction_a_x(CPU* cpu);
    int CMPInstruction_a_y(CPU* cpu);
    int CMPInstruction_$x(CPU* cpu);
    int CMPInstruction_$y(CPU* cpu);

    int CPXInstruction_i(CPU* cpu);
    int CPXInstruction_d(CPU* cpu);
    int CPXInstruction_a(CPU* cpu);

    int CPYInstruction_i(CPU* cpu);
    int CPYInstruction_d(CPU* cpu);
    int CPYInstruction_a(CPU* cpu);

    // Flow
    int JMPInstruction_a(CPU* cpu);
    int JMPInstruction_$(CPU* cpu);

    int BPLInstruction_pd(CPU* cpu);
    int BMIInstruction_pd(CPU* cpu);
    int BVCInstruction_pd(CPU* cpu);
    int BVSInstruction_pd(CPU* cpu);
    int BCCInstruction_pd(CPU* cpu);
    int BCSInstruction_pd(CPU* cpu);
    int BNEInstruction_pd(CPU* cpu);
    int BEQInstruction_pd(CPU* cpu);

    int BRKInstruction(CPU* cpu);
    int RTIInstruction(CPU* cpu);

    int JSRInstruction_a(CPU *cpu);
    int RTSInstruction(CPU* cpu);

    // Flags
    int CLCInstruction(CPU* cpu);
    int SECInstruction(CPU* cpu);
    int CLIInstruction(CPU* cpu);
    int SEIInstruction(CPU* cpu);
    int CLVInstruction(CPU* cpu);
    int CLDInstruction(CPU* cpu);
    int SEDInstruction(CPU* cpu);

    // Transfer / Loads
    int TXAInstruction(CPU* cpu);
    int TAXInstruction(CPU* cpu);
    int TYAInstruction(CPU* cpu);
    int TAYInstruction(CPU* cpu);

    int LDAInstruction_i(CPU* cpu);
    int LDAInstruction_d(CPU* cpu);
    int LDAInstruction_d_x(CPU* cpu);
    int LDAInstruction_a(CPU* cpu);
    int LDAInstruction_a_x(CPU* cpu);
    int LDAInstruction_a_y(CPU* cpu);
    int LDAInstruction_$x(CPU* cpu);
    int LDAInstruction_$y(CPU* cpu);

    int LDXInstruction_i(CPU* cpu);
    int LDXInstruction_d(CPU* cpu);
    int LDXInstruction_d_y(CPU* cpu);
    int LDXInstruction_a(CPU* cpu);
    int LDXInstruction_a_y(CPU* cpu);

    int LDYInstruction_i(CPU* cpu);
    int LDYInstruction_d(CPU* cpu);
    int LDYInstruction_d_x(CPU* cpu);
    int LDYInstruction_a(CPU* cpu);
    int LDYInstruction_a_x(CPU* cpu);

    int LAXInstruction_i(CPU* cpu);
    int LAXInstruction_d(CPU* cpu);
    int LAXInstruction_d_y(CPU* cpu);
    int LAXInstruction_a(CPU* cpu);
    int LAXInstruction_a_y(CPU* cpu);
    int LAXInstruction_$x(CPU* cpu);
    int LAXInstruction_$y(CPU* cpu);

    int STAInstruction_d(CPU* cpu);
    int STAInstruction_d_x(CPU* cpu);
    int STAInstruction_a(CPU* cpu);
    int STAInstruction_a_x(CPU* cpu);
    int STAInstruction_a_y(CPU* cpu);
    int STAInstruction_$x(CPU* cpu);
    int STAInstruction_$y(CPU* cpu);

    int STXInstruction_d(CPU* cpu);
    int STXInstruction_d_y(CPU* cpu);
    int STXInstruction_a(CPU* cpu);

    int STYInstruction_d(CPU* cpu);
    int STYInstruction_d_x(CPU* cpu);
    int STYInstruction_a(CPU* cpu);

    int SAXInstruction_d(CPU* cpu);
    int SAXInstruction_d_y(CPU* cpu);
    int SAXInstruction_a(CPU* cpu);
    int SAXInstruction_$x(CPU* cpu);

    int TXSInstruction(CPU* cpu);
    int TSXInstruction(CPU* cpu);
    int PHAInstruction(CPU* cpu);
    int PLAInstruction(CPU* cpu);
    int PHPInstruction(CPU* cpu);
    int PLPInstruction(CPU* cpu);

//    int STPInstruction(CPU* cpu);
//    int ANCInstruction_i(CPU* cpu);
//    int ALRInstruction_i(CPU* cpu);
//    int ARRInstruction_i(CPU* cpu);
//    int XAAInstruction_i(CPU* cpu);
//    int AHXInstruction_d_y(CPU* cpu);
//    int TASInstruction_a_y(CPU* cpu);
//    int SHYInstruction_a_x(CPU* cpu);
//    int SHXInstruction_a_y(CPU* cpu);
//    int AHXInstruction_a_y(CPU* cpu);
//    int LASInstruction_a_y(CPU* cpu);
//    int AXSInstruction_i(CPU* cpu);

    extern Instruction opFunctions[];
    extern string opNames[];
}

#endif //NEM_OPCODES_H
