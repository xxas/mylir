export module mylir:cpu;

import std;
import :lang;
import :program;

namespace mylir
{
    struct Cpu
    {   // Cpu registers; regid --> value.
        std::unordered_map<Register::Type, Operand::Immediate> registers;
    };
};
