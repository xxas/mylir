export module mylir : program;

import std;
import :lang;

namespace mylir
{
    struct Program
    {
        struct Block
        {
            using Text  = std::vector<lang::Instruction>;
            using Data  = std::vector<std::uint8_t>;
            using Any   = std::variant<Text, Data>;

            // Underlying block variant; text, or data.
            Any any;

            // Get the size of the block.
            auto inline size() const
                -> std::size_t
            {
                return any.visit([](const auto& value)
                    {
                        return value.size();
                    });
            };

            // Apply a visitor to the block.
            template<class F> inline auto visit(F&& visitor)
            {   // Forward the visitor to std::variant::visit.
                return any.visit(std::forward<F>(visitor));
            };

            // Returns a std::optional containing a std::reference_wrapper to the underlying std::variant type `T`.
            template<class... Ts> struct Overloads: Ts... { using Ts::operator()...; };
            template<class T, class Optional = std::optional<std::reference_wrapper<T>>> inline auto unwrap()
                -> Optional
            {
                auto some = [](T& block)          -> Optional { return Optional(std::ref(block)); };
                auto none = []<class O>(O& block) -> Optional { return std::nullopt; };

                return any.visit(Overloads{ some, none });
            };
        };

        // Loaded program blocks of memory: each separated at labels; or control-flow changes.
        std::unordered_map<std::uint32_t, Block> blocks;

        struct GetBlockError
        {
            enum class Type: std::uint8_t
            {
                Mismatch,   // Provided type and block variant type differ; mismatched block types.
                Miss,       // Missed finding a block nearest to the address provided.
            } type;

            std::string message;
        };

        // Returns a std::reference_wrapper to a Program::Block or an Error if missed.
        auto at(const std::uint32_t& addr)
            -> std::expected<std::reference_wrapper<Block>, GetBlockError>
        {   // Try retrieving a block using the address as an exact key.
            if(auto it = this->blocks.find(addr); it != this->blocks.end())
            {
                return std::ref(it->second);
            };

            for(const auto& start: std::views::keys(this->blocks))
            {   // Get the end address of the block.
                auto end = start + this->blocks[start].size();

                // Compare for the address being in range of the block.
                if(addr >= start && addr <= end)
                {
                    return std::ref(this->blocks[start]);
                };
            };

            return std::unexpected(GetBlockError
                {
                    .type     = GetBlockError::Type::Miss,
                    .message  = std::format("Failed to find a block containing or at the address: {}", addr),
                });
        };

        // Returns a std::reference_wrapper to an unwrapped Program::Block variant type or an error if missed.
        template<class T> auto unwrap_at(const std::uint32_t& addr)
            -> std::expected<std::reference_wrapper<T>, GetBlockError>
        {   // Get the block for the address.
            if(auto block = this->at(addr); block.has_value())
            {
                return block->get().unwrap<T>()
                  .transform([]<class R>(R& ref)
                          -> std::expected<R, GetBlockError>
                      {
                          return ref;
                      })
                  .value_or(std::unexpected(GetBlockError
                      {
                          .type     = GetBlockError::Type::Mismatch,
                          .message  = std::format("Expected variant type of \"{}\"; while block variant is of a different type", typeid(T).name()),
                      }));
            }
            else
            {
                return block.error();
            };
        };
    };
};
