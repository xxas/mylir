import std;
import mylir;

constexpr static auto Program = R"(
  $data
    @integer_0 .emit(zero, 4);     # Emit 4 bytes of zero'd memory.
    @integer_1 .emit(int, 0x100);  # Emit an integer initialized to 0x100.

  $text
  @main
    mov  i0, @integer_1        # Moves the data from the integer_0 label into the i0 register.
    add  @integer_0, i0, i1    # Adds i1 to i0 and stores within integer_0 label.
    prnt i2                    # Prints the i0 value.
    prnt @integer_0            # Prints the @integer_0 symbol. 


    )";

int main()
{
    std::print("hello world");

    return 0;
}
