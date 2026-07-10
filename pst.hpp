#include <array>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>

class Pst {
  static_assert(sizeof(std::uint8_t) == 1, "std::uint8_t must be 1 byte");

private:
  constexpr static std::size_t SIZE = 256, CNT = 5;
  constexpr static std::size_t table_size = SIZE * CNT;
  std::array<std::uint8_t, table_size> table{};

public:
  Pst() { loadDefault(); }

  // King = 1, Advisor = 2, Elephant = 3, Horse = 4, Rook = 5, Cannon = 6, Pawn = 7
  std::int32_t getScore(std::uint8_t piece, std::uint8_t pos) const {
    if (static_cast<std::size_t>(pos) >= SIZE)
      return 0;

    switch (piece) {
    case 1:
    case 7:
      return static_cast<std::int32_t>(table[SIZE * 0 + pos]);
    case 2:
    case 3:
      return static_cast<std::int32_t>(table[SIZE * 1 + pos]);
    case 4:
      return static_cast<std::int32_t>(table[SIZE * 2 + pos]);
    case 5:
      return static_cast<std::int32_t>(table[SIZE * 3 + pos]);
    case 6:
      return static_cast<std::int32_t>(table[SIZE * 4 + pos]);
    default:
      return 0;
    }
  }

  void loadDefault() {
    static auto rc_to_pos = [](std::uint8_t row, std::uint8_t col) -> std::uint8_t {
      return ((row + 0x3) << 4) | (col + 0x3);
    };

    static auto sub_abs = [](std::uint8_t a, std::uint8_t b) -> std::uint8_t {
      return (a > b) ? (a - b) : (b - a);
    };

    static auto sub_max0 = [](std::uint8_t a, std::uint8_t b) -> std::uint8_t {
      return (a > b) ? (a - b) : 0;
    };

    static auto pawn = [](std::uint8_t r, std::uint8_t c) -> std::uint8_t {
      if (r == 0)
        return 20 + 3 * (4 - sub_abs(c, 4));
      else if (r <= 4)
        return 40 + 2 * (4 - r) + 2 * (4 - sub_abs(c, 4));
      else
        return 10 + 2 * (7 - r) + sub_max0(2, sub_abs(c, 4));
    };

    for (std::uint8_t row = 0; row < 10; ++row) {
      for (std::uint8_t col = 0; col < 9; ++col) {
        table[SIZE * 0 + rc_to_pos(row, col)] = pawn(row, col); // Pawn
        table[SIZE * 1 + rc_to_pos(row, col)] = 30;             // Advisor / Elephant
        table[SIZE * 2 + rc_to_pos(row, col)] = 100;            // Horse
        table[SIZE * 3 + rc_to_pos(row, col)] = 200;            // Rook
        table[SIZE * 4 + rc_to_pos(row, col)] = 95;             // Cannon
      }
    }
  }

  void save(const char *filename) const {
    if (!filename)
      throw std::runtime_error("Filename is null");
    std::ofstream file(filename, std::ios::binary);
    if (!file)
      throw std::runtime_error("Failed to open file for writing");
    file.write(reinterpret_cast<const char *>(table.data()), static_cast<std::streamsize>(table_size));
    if (!file)
      throw std::runtime_error("Failed to write data to file");
  }

  void load(const char *filename) {
    if (!filename)
      throw std::runtime_error("Filename is null");
    std::ifstream file(filename, std::ios::binary);
    if (!file)
      throw std::runtime_error("Failed to open file for reading");
    static std::uint8_t buffer[table_size];
    file.read(reinterpret_cast<char *>(buffer), static_cast<std::streamsize>(table_size));
    if (!file || file.gcount() != static_cast<std::streamsize>(table_size))
      throw std::runtime_error("Failed to read data from file");
    for (std::size_t i = 0; i < table_size; ++i)
      table[i] = buffer[i];
  }
};
