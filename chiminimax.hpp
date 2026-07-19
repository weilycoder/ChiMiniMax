/**
 * ChiMiniMax/chiminimax.hpp - Source code for the ChiMiniMax project.
 *
 * The core logic of Chiminimax.
 *
 * Copyright (C) 2026 weilycoder
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pst.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <generator>
#include <limits>
#include <mutex>
#include <random>
#include <stack>
#include <unordered_map>
#include <utility>
#include <vector>

#define Q_MAX_REC_DEPTH 16

static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big,
              "Unsupported endianness");

static std::mt19937_64 rng{[]() -> std::uint64_t {
  const char *seed = "CMiniMax";
  if constexpr (std::endian::native == std::endian::little)
    return *(reinterpret_cast<const std::uint64_t *>(seed));
  else if constexpr (std::endian::native == std::endian::big)
    return std::byteswap(*(reinterpret_cast<const std::uint64_t *>(seed)));
}()};

static constexpr std::bitset<256> cInBoard = []() {
  std::bitset<256> bits;
  for (std::size_t y = 3; y < 13; ++y)
    for (std::size_t x = 3; x < 12; ++x)
      bits.set(y * 16 + x);
  return bits;
}();

static constexpr std::bitset<256> cInPalace = []() {
  std::bitset<256> bits;
  for (std::size_t y = 3; y < 6; ++y)
    for (std::size_t x = 6; x < 9; ++x)
      bits.set(y * 16 + x);
  for (std::size_t y = 10; y < 13; ++y)
    for (std::size_t x = 6; x < 9; ++x)
      bits.set(y * 16 + x);
  return bits;
}();

static constexpr std::uint8_t flip_x(const std::uint8_t pos) noexcept { return 0xFE - (pos ^ 0xF0); }
static constexpr std::uint8_t flip_y(const std::uint8_t pos) noexcept { return pos ^ 0xF0; }
static constexpr std::uint8_t flip_a(const std::uint8_t pos) noexcept { return 0xFE - pos; }

static constexpr std::int8_t cUp = -16;
static constexpr std::int8_t cDown = 16;
static constexpr std::int8_t cLeft = -1;
static constexpr std::int8_t cRight = 1;

static constexpr std::uint8_t cEmpty = 0;

static constexpr std::uint8_t cBlack = 0;
static constexpr std::uint8_t cRed = 8;
static constexpr std::uint8_t cColorMask = 0x08;

static constexpr std::uint8_t cKing = 1;
static constexpr std::uint8_t cAdvisor = 2;
static constexpr std::uint8_t cElephant = 3;
static constexpr std::uint8_t cHorse = 4;
static constexpr std::uint8_t cRook = 5;
static constexpr std::uint8_t cCannon = 6;
static constexpr std::uint8_t cPawn = 7;
static constexpr std::uint8_t cPieceMask = 0x07;

static constexpr bool sameColor(std::uint8_t a, std::uint8_t b) noexcept {
  return (a & cColorMask) == (b & cColorMask);
}

static constexpr std::int8_t cHorseDelta[4][2] = {{-33, -31}, {-18, 14}, {-14, 18}, {31, 33}};
static constexpr std::int8_t cKingDelta[4] = {-16, -1, 1, 16};
static constexpr std::int8_t cAdvisorDelta[4] = {-17, -15, 15, 17};
static constexpr std::int8_t cElephantDelta[4] = {-34, -30, 30, 34};
static constexpr std::int8_t cHorseKingDelta[4][2] = {{-33, -18}, {-31, -14}, {14, 31}, {18, 33}};

static constexpr auto initSquares = []() {
  std::array<std::uint8_t, 256> squares{};

  squares[0x33] = cBlack | cRook;
  squares[0x34] = cBlack | cHorse;
  squares[0x35] = cBlack | cElephant;
  squares[0x36] = cBlack | cAdvisor;
  squares[0x37] = cBlack | cKing;
  squares[0x38] = cBlack | cAdvisor;
  squares[0x39] = cBlack | cElephant;
  squares[0x3A] = cBlack | cHorse;
  squares[0x3B] = cBlack | cRook;
  squares[0x54] = cBlack | cCannon;
  squares[0x5A] = cBlack | cCannon;
  squares[0x63] = cBlack | cPawn;
  squares[0x65] = cBlack | cPawn;
  squares[0x67] = cBlack | cPawn;
  squares[0x69] = cBlack | cPawn;
  squares[0x6B] = cBlack | cPawn;

  squares[0xC3] = cRed | cRook;
  squares[0xC4] = cRed | cHorse;
  squares[0xC5] = cRed | cElephant;
  squares[0xC6] = cRed | cAdvisor;
  squares[0xC7] = cRed | cKing;
  squares[0xC8] = cRed | cAdvisor;
  squares[0xC9] = cRed | cElephant;
  squares[0xCA] = cRed | cHorse;
  squares[0xCB] = cRed | cRook;
  squares[0xA4] = cRed | cCannon;
  squares[0xAA] = cRed | cCannon;
  squares[0x93] = cRed | cPawn;
  squares[0x95] = cRed | cPawn;
  squares[0x97] = cRed | cPawn;
  squares[0x99] = cRed | cPawn;
  squares[0x9B] = cRed | cPawn;

  return squares;
}();

static const std::uint64_t initZobrist = rng();
static const auto zobristTable = []() {
  std::array<std::array<std::uint64_t, 256>, 16> zobrist{};
  for (std::size_t i = 1; i < 16; ++i)
    for (std::size_t j = 0; j < 255; ++j)
      zobrist[i][j] = rng();
  return zobrist;
}();

struct Move {
  std::uint8_t from, to;

  Move() noexcept : from(0), to(0) {}
  Move(std::uint8_t _from, std::uint8_t _to) noexcept : from(_from), to(_to) {}

  std::uint16_t toUInt16() const noexcept { return (static_cast<std::uint16_t>(from) << 8) | to; }
};

struct Step {
  Move move;
  std::uint8_t captured;
  std::uint8_t moverColor;
  bool givesCheck;
  std::uint64_t zobrist;

  Step(const std::uint8_t _from, const std::uint8_t _to, const std::uint8_t _captured,
       const std::uint8_t _moverColor, const bool _givesCheck = false,
       const std::uint64_t _zobrist = static_cast<std::uint64_t>(0)) noexcept
      : move{_from, _to}, captured(_captured), moverColor(_moverColor), givesCheck(_givesCheck),
        zobrist(_zobrist) {}
};

struct MoveHistory {
  std::vector<Step> steps;
  std::unordered_map<std::uint64_t, std::vector<std::size_t>> zobristHistory;

  MoveHistory() noexcept : steps(), zobristHistory() {
    zobristHistory.reserve(1024);
    zobristHistory[initZobrist].emplace_back(0); // Initial position
  }

  void pushStep(std::uint8_t from, std::uint8_t to, std::uint8_t captured, std::uint8_t moverColor,
                bool givesCheck, std::uint64_t zobrist) noexcept {
    steps.emplace_back(from, to, captured, moverColor, givesCheck, zobrist);
    zobristHistory[zobrist].emplace_back(steps.size());
  }

  bool popStep() noexcept {
    if (steps.empty())
      return false;

    const auto &lastStep = steps.back();
    zobristHistory.at(lastStep.zobrist).pop_back();
    if (zobristHistory.at(lastStep.zobrist).empty())
      zobristHistory.erase(lastStep.zobrist);
    steps.pop_back();
    return true;
  }

  std::uint8_t repStatus() const noexcept {
    if (moveCount() <= 1)
      return 0;

    const std::uint8_t color = lastStep().moverColor;
    const std::uint64_t eZobrist = lastStep().zobrist;
    const auto it = zobristHistory.find(eZobrist);

    if (it == zobristHistory.end() || it->second.size() < 2)
      return 0;

    bool selfAllCheck = true, oppAllCheck = true;
    std::size_t repeatStart = it->second[it->second.size() - 2];
    for (std::size_t i = repeatStart; i < steps.size(); ++i) {
      const Step &step = steps[i];
      if (step.moverColor == color)
        selfAllCheck = selfAllCheck && step.givesCheck;
      else
        oppAllCheck = oppAllCheck && step.givesCheck;
    }

    return (selfAllCheck ? 2 : 0) | (oppAllCheck ? 4 : 0) | 1;
  }

  Step &lastStep() noexcept { return steps.back(); }
  const Step &lastStep() const noexcept { return steps.back(); }

  bool empty() const noexcept { return steps.empty(); }

  std::size_t moveCount() const noexcept { return steps.size(); }
};

class cBoard {
public:
  static constexpr std::int32_t winScore = 1 << 30;
  static constexpr std::int32_t winLimit = winScore >> 1;

  class lock_failed : public std::runtime_error {
  public:
    lock_failed() : std::runtime_error("Failed to acquire lock for board") {}
  };

  template <typename mutex_type> class throwing_lock_guard {
  public:
    explicit throwing_lock_guard(mutex_type &__m) : _M_device(__m) {
      if (!__m.try_lock())
        throw lock_failed();
    }

    ~throwing_lock_guard() { _M_device.unlock(); }

    throwing_lock_guard(const throwing_lock_guard &) = delete;
    throwing_lock_guard &operator=(const throwing_lock_guard &) = delete;

  private:
    mutex_type &_M_device;
  };

  std::mutex mtx;

private:
  std::int32_t eScore = 0, drawScore = 0;
  std::array<std::uint64_t, 4> eZobrist{initZobrist, initZobrist, initZobrist, initZobrist};

  Pst table;
  MoveHistory moveHistory;

  std::array<std::uint8_t, 256> squares = initSquares;

  std::array<std::int32_t, 1 << 16> history{}; // history heuristic indexed by move.toUInt16()

  constexpr bool testMove(std::uint8_t piece, std::uint8_t to) const noexcept {
    return cInBoard.test(to) && (squares[to] == 0 || !sameColor(squares[to], piece));
  }

  constexpr bool testKingMove(std::uint8_t piece, std::uint8_t to) const noexcept {
    return cInPalace.test(to) && (squares[to] == 0 || !sameColor(squares[to], piece));
  }

  std::int16_t getScore(std::uint8_t pos) const noexcept {
    if (squares[pos] == 0)
      return 0;
    if ((squares[pos] & cColorMask) == cRed)
      return table.getScore(squares[pos] & cPieceMask, pos);
    else
      return -table.getScore(squares[pos] & cPieceMask, 254 - pos);
  }

  void subScore(std::uint8_t from, std::uint8_t to) noexcept { eScore -= getScore(from) + getScore(to); }
  void addScore(std::uint8_t from, std::uint8_t to) noexcept { eScore += getScore(from) + getScore(to); }

  std::uint64_t getZobrist(std::uint8_t pos, std::uint8_t type) const noexcept {
    switch (type) {
    case 0:
      return zobristTable[squares[pos]][pos];
    case 1:
      return zobristTable[squares[pos]][flip_x(pos)];
    case 2:
      return zobristTable[squares[pos]][flip_y(pos)];
    case 3:
      return zobristTable[squares[pos]][flip_a(pos)];
    default:
      return 0;
    }
  }

  void applyZobrist(std::uint8_t from, std::uint8_t to) noexcept {
    eZobrist[0] ^= getZobrist(from, 0) ^ getZobrist(to, 0);
    eZobrist[1] ^= getZobrist(from, 1) ^ getZobrist(to, 1);
    eZobrist[2] ^= getZobrist(from, 2) ^ getZobrist(to, 2);
    eZobrist[3] ^= getZobrist(from, 3) ^ getZobrist(to, 3);
  }

  std::generator<std::uint8_t> generateKingMoves(const std::uint8_t pos) const noexcept {
    const std::uint8_t piece = squares[pos];
    for (const auto &move : cKingDelta) {
      const std::uint8_t to = pos + move;
      if (testKingMove(piece, to))
        co_yield to;
    }
  }

  std::generator<std::uint8_t> generateAdvisorMoves(const std::uint8_t pos) const noexcept {
    const std::uint8_t piece = squares[pos];
    for (const auto &move : cAdvisorDelta) {
      const std::uint8_t to = pos + move;
      if (testKingMove(piece, to))
        co_yield to;
    }
  }

  std::generator<std::uint8_t> generateElephantMoves(const std::uint8_t pos) const noexcept {
    const std::uint8_t piece = squares[pos];
    for (std::size_t i = 0; i < 4; ++i) {
      const std::uint8_t to = pos + cElephantDelta[i];
      const std::uint8_t eye = pos + cAdvisorDelta[i];
      if (squares[eye] == 0 && ((pos < 0x80) == (to < 0x80)) && testMove(piece, to))
        co_yield to;
    }
  }

  std::generator<std::uint8_t> generateHorseMoves(const std::uint8_t pos, const std::int8_t moveDelta[4][2],
                                                  const std::int8_t legDelta[4]) const noexcept {
    const std::uint8_t piece = squares[pos];
    for (std::size_t i = 0; i < 4; ++i) {
      const std::uint8_t to1 = pos + moveDelta[i][0];
      const std::uint8_t to2 = pos + moveDelta[i][1];
      const std::uint8_t leg = pos + legDelta[i];
      if (squares[leg] == 0) {
        if (testMove(piece, to1))
          co_yield to1;
        if (testMove(piece, to2))
          co_yield to2;
      }
    }
  }

  std::generator<std::uint8_t> generateHorseMoves(const std::uint8_t pos) const noexcept {
    co_yield std::ranges::elements_of(generateHorseMoves(pos, cHorseDelta, cKingDelta));
  }

  std::generator<std::uint8_t> generateRookMoves(const std::uint8_t pos) const noexcept {
    const std::uint8_t piece = squares[pos];
    for (const auto &move : {cUp, cDown, cLeft, cRight}) {
      for (uint8_t to = pos + move; cInBoard.test(to); to += move) {
        if (squares[to] == 0) {
          co_yield to;
        } else {
          if (!sameColor(squares[to], piece))
            co_yield to;
          break;
        }
      }
    }
  }

  std::generator<std::uint8_t> generateCannonMoves(const std::uint8_t pos) const noexcept {
    const std::uint8_t piece = squares[pos];
    for (const auto &move : {cUp, cDown, cLeft, cRight}) {
      bool jumped = false;
      for (uint8_t to = pos + move; cInBoard.test(to); to += move) {
        if (squares[to] == 0) {
          if (!jumped)
            co_yield to;
        } else {
          if (!jumped) {
            jumped = true;
          } else {
            if (!sameColor(squares[to], piece))
              co_yield to;
            break;
          }
        }
      }
    }
  }

  std::generator<std::uint8_t> generatePawnMoves(const std::uint8_t pos) const noexcept {
    const std::uint8_t piece = squares[pos];
    const std::uint8_t forward = (piece & cColorMask) ? cUp : cDown;
    const std::uint8_t toForward = pos + forward;
    if (testMove(piece, toForward))
      co_yield toForward;

    const bool crossedRiver = (piece & cColorMask) ? (pos < 0x80) : (pos >= 0x80);
    if (crossedRiver || (piece & cPieceMask) != cPawn) {
      for (const auto &move : {cLeft, cRight}) {
        const uint8_t to = pos + move;
        if (testMove(piece, to))
          co_yield to;
      }
    }
  }

  bool undoMoveUnlocked() noexcept {
    if (moveHistory.empty())
      return false;
    Step step = moveHistory.lastStep();
    moveHistory.popStep();
    applyZobrist(step.move.from, step.move.to), subScore(step.move.from, step.move.to);
    squares[step.move.from] = squares[step.move.to], squares[step.move.to] = step.captured;
    applyZobrist(step.move.from, step.move.to), addScore(step.move.from, step.move.to);
    return true;
  }

  bool makeMoveUnlocked(std::uint8_t from, std::uint8_t to) noexcept {
    const std::uint8_t moverColor = squares[from] & cColorMask;

    for (const std::uint8_t move : generateMoves(from))
      if (move == to) {
        std::uint8_t captured = squares[to];
        applyZobrist(from, to), subScore(from, to);
        squares[to] = squares[from], squares[from] = cEmpty;
        applyZobrist(from, to), addScore(from, to);
        moveHistory.pushStep(from, to, captured, moverColor, testCheck(moverColor ^ cColorMask), eZobrist[0]);
        if (testCheck(moverColor) || repStatus() == 3)
          return undoMoveUnlocked(), false;
        return true;
      }
    return false;
  }

  std::int32_t quiescence(int depth, int32_t alpha, int32_t beta, uint8_t color) noexcept {
    const std::uint8_t repStatus = moveHistory.repStatus();
    if (repStatus == 1 || repStatus == 7)
      return drawScore;

    const std::int32_t inCheck = testCheck(color) ? winLimit : 0;

    if (!inCheck) {
      int32_t standPat = (color == cRed) ? eScore : -eScore;
      alpha = std::max(alpha, standPat);
      if (alpha >= beta)
        return alpha;
    } else if (depth > Q_MAX_REC_DEPTH) {
      return static_cast<std::int32_t>(moveHistory.moveCount()) - winScore;
    }

    std::vector<std::pair<Move, std::int32_t>> moves;
    for (const auto &pr : generateAllMoves(color)) {
      if (!makeMoveUnlocked(pr.from, pr.to))
        continue;
      const int32_t givesCheck = (depth <= Q_MAX_REC_DEPTH && testCheck(color ^ cColorMask)) ? winLimit : 0;
      const std::int32_t captured = moveHistory.lastStep().captured;
      const std::int32_t rank = table.getScore(captured & cPieceMask, pr.to) + givesCheck + inCheck;
      if (rank != 0)
        moves.emplace_back(pr, rank);
      undoMoveUnlocked();
    }

    std::sort(moves.begin(), moves.end(), [&](const auto &a, const auto &b) { return a.second > b.second; });

    for (const auto &pr : moves) {
      if (!makeMoveUnlocked(pr.first.from, pr.first.to))
        continue;

      const std::int32_t score = -quiescence(depth + 1, -beta, -alpha, color ^ cColorMask);
      undoMoveUnlocked();

      alpha = std::max(alpha, score);
      if (alpha >= beta)
        break; // beta cutoff
    }

    return alpha;
  }

  std::int32_t negamax(int depth, std::int32_t alpha, std::int32_t beta, std::uint8_t color,
                       Move *outBestMove = nullptr) noexcept {
    const std::uint8_t repStatus = moveHistory.repStatus();
    if (repStatus == 1 || repStatus == 7)
      return drawScore;
    if (depth == 0)
      return quiescence(0, alpha, beta, color);

    std::int32_t best = static_cast<std::int32_t>(moveHistory.moveCount()) - winScore;

    std::vector<Move> moves;
    for (const auto &pr : generateAllMoves(color))
      moves.emplace_back(pr);

    if (moves.size() > 1) {
      std::sort(moves.begin(), moves.end(),
                [&](const Move &a, const Move &b) { return history[a.toUInt16()] > history[b.toUInt16()]; });
    }

    Move localBest{0, 0};
    for (const auto &pr : moves) {
      if (!makeMoveUnlocked(pr.from, pr.to))
        continue;

      const std::int32_t val = -negamax(depth - 1, -beta, -alpha, color ^ cColorMask);
      undoMoveUnlocked();

      if (val > best)
        best = val, localBest = pr;
      alpha = std::max(alpha, val);
      if (alpha >= beta) {
        history[pr.toUInt16()] += depth * depth;
        break; // beta cutoff
      }
    }

    if (localBest.from || localBest.to)
      history[localBest.toUInt16()] += depth * depth;
    if (outBestMove && (localBest.from && localBest.to))
      *outBestMove = localBest;

    return best;
  }

public:
  cBoard() = default;

  std::uint8_t getPieceAt(std::uint8_t pos) const noexcept { return squares[pos]; }

  std::uint64_t getZobrist() const noexcept { return eZobrist[0]; }

  void reset_pst() {
    throwing_lock_guard<std::mutex> lock(mtx);
    table.loadDefault(), eScore = 0;
    for (std::uint8_t y = 3; y < 13; ++y)
      for (std::uint8_t x = 3; x < 12; ++x)
        eScore += getScore(y * 16 + x);
  }

  void load_pst(const char *filename) {
    throwing_lock_guard<std::mutex> lock(mtx);
    table.load(filename), eScore = 0;
    for (std::uint8_t y = 3; y < 13; ++y)
      for (std::uint8_t x = 3; x < 12; ++x)
        eScore += getScore(y * 16 + x);
  }

  void setDrawScore(std::int16_t score) {
    throwing_lock_guard<std::mutex> lock(mtx);
    drawScore = score;
  }

  std::int32_t getScore() const noexcept { return eScore; }

  std::generator<std::uint8_t> generateMoves(const std::uint8_t pos) const noexcept {
    const std::uint8_t piece = squares[pos];

    switch (piece & cPieceMask) {
    case cKing:
      co_yield std::ranges::elements_of(generateKingMoves(pos));
      break;
    case cAdvisor:
      co_yield std::ranges::elements_of(generateAdvisorMoves(pos));
      break;
    case cElephant:
      co_yield std::ranges::elements_of(generateElephantMoves(pos));
      break;
    case cHorse:
      co_yield std::ranges::elements_of(generateHorseMoves(pos));
      break;
    case cRook:
      co_yield std::ranges::elements_of(generateRookMoves(pos));
      break;
    case cCannon:
      co_yield std::ranges::elements_of(generateCannonMoves(pos));
      break;
    case cPawn:
      co_yield std::ranges::elements_of(generatePawnMoves(pos));
      break;
    }
  }

  std::generator<Move> generateAllMoves(std::uint8_t color) const noexcept {
    for (std::size_t y = 3; y < 13; ++y) {
      for (std::size_t x = 3; x < 12; ++x) {
        std::uint8_t pos = static_cast<std::uint8_t>(y * 16 + x);
        if (squares[pos] != cEmpty && (squares[pos] & cColorMask) == color)
          for (const std::uint8_t move : generateMoves(pos))
            co_yield Move{pos, move};
      }
    }
  }

  std::vector<std::uint8_t> generateMovesWithCheck(std::uint8_t pos) {
    throwing_lock_guard<std::mutex> lock(mtx);
    const std::uint8_t piece = squares[pos];
    std::vector<std::uint8_t> validMoves;
    for (const std::uint8_t move : generateMoves(pos)) {
      if (makeMoveUnlocked(pos, move)) {
        undoMoveUnlocked();
        validMoves.emplace_back(move);
      }
    }
    return validMoves.shrink_to_fit(), validMoves;
  }

  std::vector<Move> generateAllMovesWithCheck(std::uint8_t color) {
    throwing_lock_guard<std::mutex> lock(mtx);
    std::vector<Move> validMoves;
    for (std::size_t y = 3; y < 13; ++y) {
      for (std::size_t x = 3; x < 12; ++x) {
        std::uint8_t pos = static_cast<std::uint8_t>(y * 16 + x);
        if (squares[pos] != cEmpty && (squares[pos] & cColorMask) == color)
          for (const std::uint8_t move : generateMoves(pos)) {
            if (makeMoveUnlocked(pos, move)) {
              undoMoveUnlocked();
              validMoves.emplace_back(pos, move);
            }
          }
      }
    }
    return validMoves.shrink_to_fit(), validMoves;
  }

  std::uint8_t getKingPos(std::uint8_t color) const noexcept {
    if (color == cBlack) {
      for (std::uint8_t y = 3; y < 6; ++y)
        for (std::uint8_t x = 6; x < 9; ++x)
          if (squares[y * 16 + x] == (cBlack | cKing))
            return y * 16 + x;
    } else {
      for (std::uint8_t y = 10; y < 13; ++y)
        for (std::uint8_t x = 6; x < 9; ++x)
          if (squares[y * 16 + x] == (cRed | cKing))
            return y * 16 + x;
    }
    return 0; // Should never reach here if the board is valid
  }

  bool testCheck(std::uint8_t color) const noexcept {
    std::uint8_t kingPos = getKingPos(color);
    if (kingPos == 0)
      return true; // King is captured, checkmate
    for (const std::uint8_t move : generateRookMoves(kingPos))
      if (squares[move] == ((color ^ cColorMask) | cRook))
        return true;
      else if (squares[move] == ((color ^ cColorMask) | cKing))
        return true; // Directly facing the opponent's king
    for (const std::uint8_t move : generateCannonMoves(kingPos))
      if (squares[move] == ((color ^ cColorMask) | cCannon))
        return true;
    for (const std::uint8_t move : generateHorseMoves(kingPos, cHorseKingDelta, cAdvisorDelta))
      if (squares[move] == ((color ^ cColorMask) | cHorse))
        return true;
    for (const std::uint8_t move : generatePawnMoves(kingPos))
      if (squares[move] == ((color ^ cColorMask) | cPawn))
        return true;
    return false;
  }

  std::uint8_t repStatus() const noexcept { return moveHistory.repStatus(); }

  bool makeMove(std::uint8_t from, std::uint8_t to) {
    throwing_lock_guard<std::mutex> lock(mtx);
    return makeMoveUnlocked(from, to);
  }

  bool undoMove() {
    throwing_lock_guard<std::mutex> lock(mtx);
    return undoMoveUnlocked();
  }

  Move suggestMove(std::uint8_t color, int depth) {
    throwing_lock_guard<std::mutex> lock(mtx);
    Move bestMove{0, 0};
    if (depth <= 0)
      return bestMove;

    constexpr std::int32_t aspirationWindow = 32;
    std::int32_t alpha = -winScore, beta = winScore;
    std::int32_t lastScore = 0;

    for (int d = 1; d <= depth; ++d) {
      if (d > 1)
        alpha = lastScore - aspirationWindow, beta = lastScore + aspirationWindow;

      Move layerBest{0, 0};
      std::int32_t score = negamax(d, alpha, beta, color, &layerBest);

      if (score <= alpha) {
        score = negamax(d, -winScore, beta, color, &layerBest);
      } else if (score >= beta) {
        score = negamax(d, alpha, winScore, color, &layerBest);
      }

      lastScore = score;
      if (layerBest.from || layerBest.to)
        bestMove = layerBest;

      if (std::abs(score) >= winLimit)
        break; // Stop searching if a decisive score is found
    }
    return bestMove;
  }
};
