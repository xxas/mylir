export module mylir : program;

import std;
import :lang;

namespace mylir
{
    struct Program
    {   // Loaded program blocks of memory: each separated at labels; or control-flow changes.
        std::unordered_map<std::uint32_t, lang::AnyBlock> blocks;

        struct GetBlockError
        {
            enum class Type: std::uint8_t
            {
                Mismatch,   // Provided type and block variant type differ; mismatched block types.
                Miss,       // Missed finding a block nearest to the address provided.
            } type;

            std::string message;
        };

        // Get the exact block from an address or the nearest.
        template<lang::Block T> auto get(const std::uint32_t& addr)
            -> std::expected<T&, GetBlockError>
        {   // Get the first exact address match for a block.
            if(auto it = this->blocks.find(addr); it != this->blocks.end())
            {
                if(std::holds_alternative<T>(it->second))
                {
                    return std::get<T>(it->second);
                }
                else
                {
                    return std::unexpected(GetBlockError
                        {
                            .type     = GetBlockError::Type::Mismatch,
                            .message  = std::format("Expected type of \"{}\"; while the block holds a different variant", typeid(T).name()),
                        });
                };
            }
            else
            {
                for(const auto& start: std::views::keys(this->blocks))
                {   // Get the end address of the block.
                    const auto end = start + lang::block_width(this->blocks[start]);

                    // Return if the address is within the range of the block.
                    if(addr > start && addr < end)
                    {
                        return this->blocks[start];
                    };
                };

                return std::unexpected(GetBlockError
                    {
                        .type     = GetBlockError::Type::Miss,
                        .message  = std::format("Missed locating a block for the address {}", addr),
                    });
            };
        };
    };
};
