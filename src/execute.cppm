export module mylir: execute;

import std;
import :lang;
import :program;
import :cpu;

namespace mylir
{
    struct Executor
    {
        using Value = Operand::Immediate;

        // Instruction operand extraction and execution context helper.
        struct Context
        {
            std::reference_wrapper<Cpu> cpu;
            std::reference_wrapper<Program> program;
        };

        struct Operands
        {   // Underlying instruction operands.
            Instruction::Operands underlying;

            // Execution context reference.
            Context context;

            struct OperandError
            {
                enum class Type: std::uint8_t
                {
                    Mismatch,     // Miswmatch of operand types without castable outcome.
                } type;

                std::string message;
            };

            // Extract an operands underlying value with the help of Executor::Context.
            template <class... Ts> struct Overloads : Ts... { using Ts::operator()...; };
            auto extract(std::size_t n)
                -> Value
            {
                auto from_imm = [](const Value& imm) -> Value { return imm; };
                auto from_register = [this](const Register& reg)
                    -> Value
                {   // Extract from the cpu registers.
                    return context.cpu.get().registers[reg.type];
                };

                auto from_addr = [this](const Operand::Address& addr)
                    -> Value
                {
                    auto& [base, offset] = addr;
                    auto& program        = this->context.program.get();

                    // Extract the data from the address + offset.
                    if(auto data = program.unwrap_at<Program::Section::Data>(base + offset); data.has_value())
                    {
                        return data
                            .transform([](auto& data_ref)
                            {   // TODO: Replace the placeholder below.
                                return *std::bit_cast<Value*>(&data_ref.get().at(0));
                            })
                            .value_or(Value{0u});
                    }
                    else
                    {   // TODO: Propagate error information from data.error().
                        return Value{0u};
                    };
                };

                // Visit the underlying operand and extract the value
                // via one of the overloaded visitor functions.
                return underlying.at(n)
                    .visit(Overloads
                    {
                        from_imm, from_register, from_addr
                    });
            };

            template<class F> auto visit(std::size_t first, std::size_t second, F&& func)
                -> std::expected<Value, int>
            {   // Directly invocable or convertible for invocation.
                auto direct   = [&func]<class T>(T&& a, T&& b)          { return std::invoke(func, a, b); };
                auto convert  = [&func]<class T, class O>(T&& a, O&& b) { return std::invoke(func, a, static_cast<T>(b)); };

                // Incompatible operands; propagate an error.
                auto error  = [](auto&, auto&)
                    {
                        return std::unexpected(OperandError
                            {
                                .type     = OperandError::Type::Mismatch,
                                .message  = "Operands are of incompatible types"
                            });
                    };

                // Extract the variants from their respective locations.
                auto v0 = this->extract(first);
                auto v1 = this->extract(second);

                // Apply the overloaded visitor to v0 and v1.
                return std::visit(Overloads
                    {
                        direct, convert, error
                    }, v0, v1);
            };
        };

        // Loaded program.
        Program program;

        // Cpu structure.
        Cpu cpu;

        struct ExecutionError
        {
            enum class Type: std::uint8_t
            {
                FetchMiss,        // Failed to fetch the next instruction via the ip register.
                Malformed,        // Malformed instruction or data.
            } type;

            std::string message;
        };
    };
};
