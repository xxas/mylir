export module mylir: lexer;

namespace mylir
{
    struct Lexer
    {
        struct Token
        {
            enum class Type
            {
                Instruction,    // "mov", "add"
                Directive,      // ".data", ".text"
                Label,          // "main:", "start:"

                // Instruction Operands
                Register,       // "i0", "f0"
                Immediate,      // "42", "0x2a"
                Memory,         // "[i0]", "[0x400000]"
                Symbol,         // "@a_symbol"

                StringLiteral,  // "\"Hello, World!\""
                Invalid,        // Unknown; or invalid tokens.
            } type;
        };


    };
};

