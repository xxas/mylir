export module mylir: lang;

import std;

namespace mylir
{
    namespace lang
    {
        struct Register
        {
            enum class Type: std::uint8_t
            {   // 32 total count of 32-bit integer registers.
                i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11,
                i12, i13, i14, i15, i16, i17, i18, i19, i20, i21,
                i22, i23, i24, i25, i26, i27, i28, i29, i30, i31,

                // 32 total count of 32-bit floating point registers.
                f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11,
                f12, f13, f14, f15, f16, f17, f18, f19, f20, f21,
                f22, f23, f24, f25, f26, f27, f28, f29, f30, f31,

                // 32-bit instruction pointer.
                ip,
            } type;

            struct FromError
            {
                enum class Type
                {
                    Unmatched,
                } type;

                std::string message;
            };

            constexpr static auto from(const std::string& str)
                -> std::expected<Register::Type, FromError>
            {
                static std::unordered_map<std::string, Register::Type> map
                {   // Integer registers.
                    { "i0",  Type::i0  },  { "i1",  Type::i1  },  { "i2",  Type::i2  },  { "i3",  Type::i3  },
                    { "i4",  Type::i4  },  { "i5",  Type::i5  },  { "i6",  Type::i6  },  { "i7",  Type::i7  },
                    { "i8",  Type::i8  },  { "i9",  Type::i9  },  { "i10", Type::i10 }, { "i11", Type::i11 },
                    { "i12", Type::i12 }, { "i13", Type::i13 }, { "i14", Type::i14 }, { "i15", Type::i15 },
                    { "i16", Type::i16 }, { "i17", Type::i17 }, { "i18", Type::i18 }, { "i19", Type::i19 },
                    { "i20", Type::i20 }, { "i21", Type::i21 }, { "i22", Type::i22 }, { "i23", Type::i23 },
                    { "i24", Type::i24 }, { "i25", Type::i25 }, { "i26", Type::i26 }, { "i27", Type::i27 },
                    { "i28", Type::i28 }, { "i29", Type::i29 }, { "i30", Type::i30 }, { "i31", Type::i31 },

                    // Floating-point registers.
                    { "f0",  Type::f0  },  { "f1",  Type::f1  },  { "f2",  Type::f2  },  { "f3",  Type::f3  },
                    { "f4",  Type::f4  },  { "f5",  Type::f5  },  { "f6",  Type::f6  },  { "f7",  Type::f7  },
                    { "f8",  Type::f8  },  { "f9",  Type::f9  },  { "f10", Type::f10 }, { "f11", Type::f11 },
                    { "f12", Type::f12 }, { "f13", Type::f13 }, { "f14", Type::f14 }, { "f15", Type::f15 },
                    { "f16", Type::f16 }, { "f17", Type::f17 }, { "f18", Type::f18 }, { "f19", Type::f19 },
                    { "f20", Type::f20 }, { "f21", Type::f21 }, { "f22", Type::f22 }, { "f23", Type::f23 },
                    { "f24", Type::f24 }, { "f25", Type::f25 }, { "f26", Type::f26 }, { "f27", Type::f27 },
                    { "f28", Type::f28 }, { "f29", Type::f29 }, { "f30", Type::f30 }, { "f31", Type::f31 },

                    // Instruction pointer.
                    { "ip",  Type::ip  },
                };

                if(auto it = map.find(str); it != map.end())
                {
                    return it->second;
                }
                else
                {
                    return std::unexpected(Register::FromError
                        {
                            .type     = FromError::Type::Unmatched,
                            .message  = "Unable to find a matching register by mnemonic",
                        });
                };
            };
        };

        struct Operand
        {
            enum class Type: std::uint8_t
            {
                None      = 0b00,
                Register  = 0b01,
                Immediate = 0b10,
                Address   = 0b11,
                Len,
            };

            enum class Direction: std::uint8_t
            {
                Dest = 0b0,
                Src  = 0b1,
                Len,
            };

            union Flow
            {
                struct
                {
                    Operand::Direction direction: 1;
                    Operand::Type           type: 2;
                };

                std::uint8_t raw;
            };

            // Immediate 32-bit value type (variant: uint32_t; or float).
            using Immediate = std::variant<std::uint32_t, float>;

            // 32-bit address; 32-bit relative offset.
            using Address   = std::pair<std::uint32_t, std::int32_t>;

            // Underlying operand variant type (variant: Immediate; Address; or Register).
            using Variant   = std::variant<Register, Immediate, Address>;

            Variant value;

            constexpr auto type() const
                -> Operand::Type
            {   // Extract std::variant::index, then increment to follow Operand::Type pattern.
                return static_cast<Operand::Type>(value.index() + 1);
            };
        };

        // Common data-flow operand types.
        struct CommonTypes: Operand
        {
            constexpr static auto Src_any = Flow
                {
                    .direction  = Direction::Src,
                    .type       = Type::None,
                };

            constexpr static auto Dest_register = Flow
                {
                    .direction  = Direction::Dest,
                    .type       = Type::Register,
                };
        };

        struct Mnemonic
        {
            enum class Type: std::uint8_t
            {   // Memory
                Mov = 0, Push, Pop,

                // Arithmetic
                Add, Sub,

                // System
                Prnt,

                Len,
            } type;

            // Mnemonic operand data-flow pattern.
            using Df_Pattern = std::vector<Operand::Flow>;

            struct FindPatternError
            {
                enum class Type: std::uint8_t
                {
                    Unmatched,
                } type;

                std::string message;
            };

            constexpr auto pattern()
                -> std::expected<Df_Pattern, FindPatternError>
            {
                static const std::unordered_map<Mnemonic::Type, Df_Pattern> map
                {     // Memory related.
                    { Mnemonic::Type::Mov, { CommonTypes::Dest_register, CommonTypes::Src_any }},
                    { Mnemonic::Type::Pop,  { CommonTypes::Src_any }},
                    { Mnemonic::Type::Push, { CommonTypes::Src_any }},

                      // Arithmetic related.
                    { Mnemonic::Type::Add, { CommonTypes::Dest_register, CommonTypes::Src_any, CommonTypes::Src_any }},
                    { Mnemonic::Type::Sub, { CommonTypes::Dest_register, CommonTypes::Src_any, CommonTypes::Src_any }},

                      // System related.
                      { Mnemonic::Type::Prnt, { CommonTypes::Src_any }},
                };

                if(auto it = map.find(this->type); it != map.end())
                {
                    return it->second;
                }
                else
                {
                    return std::unexpected(Mnemonic::FindPatternError
                        {
                            .type     = FindPatternError::Type::Unmatched,
                            .message  = "Unable to find a matching pattern by mnemonic",
                        });
                };
            };
        };


        // Declaration of an opcodes restrictions, and data flow.
        struct Instruction
        {
            using Operands = std::vector<Operand>;

            Mnemonic mnemonic;
            Operands operands;

            union Pattern
            {   // Operation bitness.
                constexpr static std::size_t Bitness = sizeof(std::uint16_t) * 8;

                // Required bitness to represent all opcodes.
                constexpr static std::size_t Opcode_bitness   = std::bit_width(std::to_underlying(Mnemonic::Type::Len) - 1u);

                // Remaining bits after opcode, and bitness of operands.
                constexpr static std::size_t Payload_len     = Pattern::Bitness - Pattern::Opcode_bitness;
                constexpr static std::size_t Operand_bitness = std::bit_width(std::to_underlying(Operand::Type::Len) - 1u);

                // Total operands that can be represented.
                constexpr static std::size_t Operands_total  = Pattern::Payload_len / Pattern::Operand_bitness;

                struct
                {
                    std::uint8_t   opcode: Pattern::Opcode_bitness;
                    std::uint16_t payload: Pattern::Payload_len;
                };

                std::uint16_t raw;
            };

           struct ToPatternError
            {
                enum class Type: std::uint8_t
                {
                    OperandLen,   // Too many operands; unable to fit into a 16-bit instruction pattern.
                    Mismatch,     // Operand missmatch: too little/too many operands provided for data-flow pattern.
                    Unmatched,
                } type;

                std::string message;
            };

            constexpr static auto pattern(const Mnemonic& mnemonic, const Mnemonic::Df_Pattern& df_pattern)
                -> std::expected<Instruction::Pattern, Instruction::ToPatternError>
            {   // Too many operations; they cannot all be fit into the operands field.
                if(df_pattern.size() > Pattern::Operands_total)
                {
                    return std::unexpected(ToPatternError
                        {
                            .type     = ToPatternError::Type::OperandLen,
                            .message  = std::format("Expected no more than {} operands; was provided {} operands",
                                                    Pattern::Operands_total, df_pattern.size()),
                        });
                };

                std::uint16_t operands = 0;
                for(std::size_t i = 0; i < std::min(df_pattern.size(), Pattern::Operands_total); i++)
                {   // Calculate bit position, pack from right to left; making earlier operands lower in bits.
                    const std::size_t shift_amount = i * Pattern::Operand_bitness;

                    std::uint16_t encoded_operand = (std::to_underlying(df_pattern[i].direction) << 2) | std::to_underlying(df_pattern[i].type);

                    // Shift to position; merge with existing operands.
                    operands |=  (encoded_operand << shift_amount);
                };

                return Instruction::Pattern
                    {   // Convert the mnemonic to its underlying opcode value.
                        .opcode  = std::to_underlying(mnemonic.type),
                        .payload = operands,
                    };
            }; 

            constexpr auto to_pattern()
                -> std::expected<Instruction::Pattern, Instruction::ToPatternError>
            {
                if(this->operands.size() > Pattern::Operands_total)
                {
                    return std::unexpected(ToPatternError
                        {
                            .type     = ToPatternError::Type::OperandLen,
                            .message  = std::format("Expected no more than {} operands for an Instruction::Pattern; was provided {} operands",
                                                    Pattern::Operands_total, this->operands.size()),
                        });
                };

                // Expected data-flow pattern.
                auto df_pattern = mnemonic.pattern();

                if(!df_pattern)
                {
                    const auto& err = df_pattern.error();

                    return std::unexpected(ToPatternError
                        {
                            .type     = ToPatternError::Type::Unmatched,
                            .message  = err.message,
                        });
                };

                if(df_pattern->size() != this->operands.size())
                {
                    return std::unexpected(ToPatternError
                        {
                            .type     = ToPatternError::Type::Mismatch,
                            .message  = std::format("Expected {} operands for data-flow pattern; was provided {} operands",
                                                    df_pattern->size(), this->operands.size()),
                        });
                };

                return Instruction::pattern(this->mnemonic, *df_pattern);
            };
        };

        using TextBlock = std::vector<Instruction>;
        using DataBlock = std::vector<std::uint8_t>;
        using AnyBlock  = std::variant<TextBlock, DataBlock>;

        // Block constraint.
        template<class T> concept Block = std::same_as<TextBlock, T> || std::same_as<DataBlock, T>;
    
        constexpr auto block_width(const AnyBlock& block)
            -> std::size_t
        {
            if(std::holds_alternative<TextBlock>(block))
            {
                return std::get<TextBlock>(block).size();
            }
            else
            {
                return std::get<DataBlock>(block).size();
            };
        };
    };
};
