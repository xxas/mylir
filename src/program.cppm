export module mylir: program;

import std;
import :lang;

namespace mylir
{   // Intermediate Program representation
    struct Program
    {
        struct Section
        {
            using Text        = std::vector<Instruction>;
            using Data        = std::vector<std::uint8_t>;
            using Any         = std::variant<Text, Data>;

            // Underlying section variant; text, data, symbol table, etc.
            Any any;

            // Get the size of the block.
            auto size() const
                -> std::size_t
            {
                return any.visit([](const auto &value) { return value.size(); });
            };

            // Apply a visitor to the block.
            template <class F> auto visit(F &&visitor)
            { // Forward the visitor to std::variant::visit.
                return any.visit(std::forward<F>(visitor));
            };

            // Returns a std::optional containing a std::reference_wrapper to the
            // underlying std::variant type `T`.
            template <class... Ts> struct Overloads : Ts... { using Ts::operator()...; };
            template <class T, class Optional = std::optional<std::reference_wrapper<T>>> auto unwrap()
                -> Optional
            {
                auto some = [](T &block)          -> Optional { return Optional(std::ref(block)); };
                auto none = []<class O>(O &block) -> Optional { return std::nullopt; };

                return any.visit(Overloads
                {
                    some, none
                });
            };
        };

        // Loaded program blocks of memory: each separated at labels; or control-flow changes.
        std::unordered_map<std::uint32_t, Section> sections;

        // Key to access the entry point function.
        std::uint32_t entry_point;

        struct GetSectionError
        {
            enum class Type: std::uint8_t
            {
                Mismatch, // Provided type and section variant type differ; mismatched section types.
                Miss,     // Missed section; couldn't locate by exact address or by section range.
            } type;

            std::string message;
        };

        // Returns a std::reference_wrapper to a Program::Section or an error if missed.
        auto at(std::uint32_t n)
            -> std::expected<std::reference_wrapper<Section>, GetSectionError>
        { // Try retrieving a block using the address as an exact key.
            if(auto it = this->sections.find(n); it != this->sections.end())
            {
                return std::ref(it->second);
            };

            for (const auto& start: std::views::keys(this->sections))
            {   // Get the end address of the block.
                auto end = start + this->sections[start].size();

                // Compare for the address being in range of the block.
                if(n >= start && n <= end)
                {
                    return std::ref(this->sections[start]);
                };
            };

            return std::unexpected(GetSectionError
                {
                    .type     = GetSectionError::Type::Miss,
                    .message  = std::format("Failed to find a block containing or at the address: {}", n),
                });
        };

        // Returns a std::reference_wrapper to an unwrapped Program::Block variant type or an error if missed.
        template <class T> auto unwrap_at(const std::uint32_t &n)
            -> std::expected<std::reference_wrapper<T>, GetSectionError>
        {   // Get the block for the address.
            if(auto section = this->at(n); section.has_value())
            {
                return section->get().unwrap<T>()
                    .transform([]<class R>(R ref)
                        -> std::expected<R, GetSectionError>
                  {
                      return ref;
                  })
                    .value_or(std::unexpected(GetSectionError
                  {
                      .type     = GetSectionError::Type::Mismatch,
                      .message  = std::format("Expected section type of \"{}\";"
                                                    "while section is of a different type",
                                                    typeid(T).name()),
                  }));
            }
            else
            {
                return std::unexpected(section.error());
            };
        };
    };
};
