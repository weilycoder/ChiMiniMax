---
description: "Use when editing chiminimax.hpp, the C++ core board logic, move generation, Zobrist hashing, or piece-square table helpers. Keeps the header aligned with the Python bridge and mailbox-board conventions."
applyTo: "chiminimax.hpp"
---
# chiminimax.hpp Guidelines

- Treat this header as core engine code. Keep changes local and preserve the current 16x16 mailbox board, 9x10 public coordinate mapping, and bit-flag piece encoding.
- Do not change public board semantics unless the Python bridge in [chiminimax.cpp](../../chiminimax.cpp) and the stub file in [chiminimax.pyi](../../chiminimax.pyi) are updated in the same change.
- Keep helper constants and functions `static` or `constexpr` when they are implementation details; avoid adding unnecessary headers or dependencies.
- Preserve the existing move-generation style and legality checks; prefer minimal edits over refactors.
- Treat the Zobrist table bounds as intentional: `i = 0` stays uninitialized so empty squares do not accumulate hash contribution, and `j = 255` is excluded because it does not correspond to a real board position.
- When touching evaluation or PST-related logic, keep symmetry between red and black via the current mirrored-position convention.
- Follow [AGENTS.md](../../AGENTS.md) for repository-wide build and validation guidance.
