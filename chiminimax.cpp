/**
 * ChiMiniMax/chiminimax.cpp - Source code for the ChiMiniMax project.
 *
 * The bridge between Python and C++ for the ChiMiniMax project.
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

#include "chiminimax.hpp"
#include <Python.h>
#include <array>
#include <string_view>
#include <unordered_map>

static std::unordered_map<std::uint64_t, cBoard> chiminimax_boards;

static PyObject *chiminimax_random(PyObject *self, PyObject *args) {
  if (!PyArg_ParseTuple(args, ""))
    return NULL;
  return PyLong_FromUnsignedLongLong(rng());
}

static PyObject *chiminimax_new_board(PyObject *self, PyObject *args) {
  if (!PyArg_ParseTuple(args, ""))
    return NULL;

  std::uint64_t board_id = rng();
  chiminimax_boards[board_id] = cBoard();
  return PyLong_FromUnsignedLongLong(board_id);
}

static PyObject *chiminimax_test_board(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  if (!PyArg_ParseTuple(args, "K", &board_id))
    return NULL;

  auto it = chiminimax_boards.find(board_id);
  if (it != chiminimax_boards.end()) {
    Py_RETURN_TRUE;
  } else {
    Py_RETURN_FALSE;
  }
}

static PyObject *chiminimax_delete_board(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  if (!PyArg_ParseTuple(args, "K", &board_id))
    return NULL;

  auto it = chiminimax_boards.find(board_id);
  if (it != chiminimax_boards.end()) {
    chiminimax_boards.erase(it);
    Py_RETURN_NONE;
  } else {
    PyErr_SetString(PyExc_ValueError, "Board ID not found.");
    return NULL;
  }
}

#define INDEX_TO_X(index) ((index & 0x0F) - 3)
#define INDEX_TO_Y(index) ((index >> 4) - 3)
#define POS_TO_INDEX(x, y) ((y + 3) * 16 + (x + 3))

#define ASSERT_BOARD_EXISTS(board_id, name)                                                                  \
  auto name = chiminimax_boards.find(board_id);                                                              \
  if (name == chiminimax_boards.end()) {                                                                     \
    PyErr_SetString(PyExc_ValueError, "Board ID not found.");                                                \
    return NULL;                                                                                             \
  }

#define ASSERT_POS(pos_x, pos_y)                                                                             \
  if (pos_x < 0 || pos_x >= 9 || pos_y < 0 || pos_y >= 10) {                                                 \
    PyErr_SetString(PyExc_ValueError, "Position out of bounds.");                                            \
    return NULL;                                                                                             \
  }

static bool check_color_id(const int color_id) {
  static constexpr auto errmsg = []() {
    std::array<char, 42> buffer{};
    static constexpr std::string_view msg = "Color must be 0x00 (Red) or 0x00 (Black).";
    std::copy(msg.begin(), msg.end(), buffer.begin());
    buffer[16] ^= (cRed >> 4), buffer[17] ^= (cRed & 0x0F);
    buffer[30] ^= (cBlack >> 4), buffer[31] ^= (cBlack & 0x0F);
    return buffer;
  }();

  if (color_id != cRed && color_id != cBlack) {
    PyErr_SetString(PyExc_ValueError, errmsg.data());
    return false;
  }

  return true;
}

static PyObject *chiminimax_get_piece_at(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  std::int32_t pos_x, pos_y;
  if (!PyArg_ParseTuple(args, "Kii", &board_id, &pos_x, &pos_y))
    return NULL;

  ASSERT_POS(pos_x, pos_y);
  ASSERT_BOARD_EXISTS(board_id, it);

  std::uint8_t pos = POS_TO_INDEX(pos_x, pos_y);
  std::uint8_t piece = it->second.getPieceAt(pos);
  return PyLong_FromUnsignedLongLong(piece);
}

static PyObject *chiminimax_get_king_pos(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  int color_id;
  if (!PyArg_ParseTuple(args, "Ki", &board_id, &color_id))
    return NULL;
  if (!check_color_id(color_id))
    return NULL;

  ASSERT_BOARD_EXISTS(board_id, it);

  std::uint8_t king_pos = it->second.getKingPos(color_id);

  if (king_pos == 0)
    Py_RETURN_NONE; // King is captured, no valid position

  return Py_BuildValue("(ii)", INDEX_TO_X(king_pos), INDEX_TO_Y(king_pos));
}

static PyObject *chiminimax_get_score(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  if (!PyArg_ParseTuple(args, "K", &board_id))
    return NULL;

  ASSERT_BOARD_EXISTS(board_id, it);

  std::int32_t score = it->second.getScore();
  return PyLong_FromLong(score);
}

static PyObject *chiminimax_reset_pst(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  if (!PyArg_ParseTuple(args, "K", &board_id))
    return NULL;

  ASSERT_BOARD_EXISTS(board_id, it);

  it->second.reset_pst();
  Py_RETURN_NONE;
}

static PyObject *chiminimax_load_pst(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  const char *filename;
  if (!PyArg_ParseTuple(args, "Ks", &board_id, &filename))
    return NULL;

  ASSERT_BOARD_EXISTS(board_id, it);

  try {
    it->second.load_pst(filename);
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_OSError, e.what());
    return NULL;
  }

  Py_RETURN_NONE;
}

static PyObject *chiminimax_generate_moves(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  std::int32_t pos_x, pos_y;
  if (!PyArg_ParseTuple(args, "Kii", &board_id, &pos_x, &pos_y))
    return NULL;
  ASSERT_POS(pos_x, pos_y);
  ASSERT_BOARD_EXISTS(board_id, it);

  PyObject *moves_list = PyList_New(0);
  if (!moves_list)
    return NULL;

  std::uint8_t pos = POS_TO_INDEX(pos_x, pos_y);
  for (const std::uint8_t move : it->second.generateMovesWithCheck(pos)) {
    PyObject *move_tuple = Py_BuildValue("(ii)", INDEX_TO_X(move), INDEX_TO_Y(move));
    if (!move_tuple) {
      Py_DECREF(moves_list);
      return NULL;
    }
    if (PyList_Append(moves_list, move_tuple)) {
      Py_DECREF(move_tuple);
      Py_DECREF(moves_list);
      return NULL;
    }
    Py_DECREF(move_tuple);
  }
  return moves_list;
}

static PyObject *chiminimax_generate_all_moves(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  int color_id;
  if (!PyArg_ParseTuple(args, "Ki", &board_id, &color_id))
    return NULL;
  if (!check_color_id(color_id))
    return NULL;

  ASSERT_BOARD_EXISTS(board_id, it);

  PyObject *all_moves_list = PyList_New(0);
  if (!all_moves_list)
    return NULL;

  for (const auto &[from, to] : it->second.generateAllMovesWithCheck(color_id)) {
    std::uint8_t from_x = INDEX_TO_X(from);
    std::uint8_t from_y = INDEX_TO_Y(from);
    std::uint8_t to_x = INDEX_TO_X(to);
    std::uint8_t to_y = INDEX_TO_Y(to);
    PyObject *move_tuple = Py_BuildValue("(ii)(ii)", from_x, from_y, to_x, to_y);
    if (!move_tuple) {
      Py_DECREF(all_moves_list);
      return NULL;
    }
    if (PyList_Append(all_moves_list, move_tuple)) {
      Py_DECREF(move_tuple);
      Py_DECREF(all_moves_list);
      return NULL;
    }
    Py_DECREF(move_tuple);
  }
  return all_moves_list;
}

static PyObject *chiminimax_test_checkmate(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  int color_id;
  if (!PyArg_ParseTuple(args, "Ki", &board_id, &color_id))
    return NULL;
  if (!check_color_id(color_id))
    return NULL;

  ASSERT_BOARD_EXISTS(board_id, it);

  bool is_checkmate = it->second.testCheckmate(color_id);
  if (is_checkmate) {
    Py_RETURN_TRUE;
  } else {
    Py_RETURN_FALSE;
  }
}

static PyObject *chiminimax_make_move(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  std::int32_t from_x, from_y, to_x, to_y;
  if (!PyArg_ParseTuple(args, "Kiiii", &board_id, &from_x, &from_y, &to_x, &to_y))
    return NULL;

  ASSERT_POS(from_x, from_y);
  ASSERT_POS(to_x, to_y);
  ASSERT_BOARD_EXISTS(board_id, it);

  std::uint8_t from = POS_TO_INDEX(from_x, from_y);
  std::uint8_t to = POS_TO_INDEX(to_x, to_y);

  if (!it->second.makeMove(from, to)) {
    PyErr_SetString(PyExc_ValueError, "Invalid move.");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject *chiminimax_undo_move(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  if (!PyArg_ParseTuple(args, "K", &board_id))
    return NULL;

  ASSERT_BOARD_EXISTS(board_id, it);

  if (!it->second.undoMove()) {
    PyErr_SetString(PyExc_ValueError, "No moves to undo.");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject *chiminimax_get_zobrist(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  if (!PyArg_ParseTuple(args, "K", &board_id))
    return NULL;

  ASSERT_BOARD_EXISTS(board_id, it);

  std::uint64_t zobrist = it->second.getZobrist();
  return PyLong_FromUnsignedLongLong(zobrist);
}

static PyObject *chiminimax_suggest_move(PyObject *self, PyObject *args) {
  std::uint64_t board_id;
  int color_id, depth;
  if (!PyArg_ParseTuple(args, "Kii", &board_id, &color_id, &depth))
    return NULL;
  if (!check_color_id(color_id))
    return NULL;

  ASSERT_BOARD_EXISTS(board_id, it);

  auto [from, to] = it->second.suggestMove(color_id, depth);
  if (from == 0 && to == 0)
    Py_RETURN_NONE; // No valid move found

  std::uint8_t from_x = INDEX_TO_X(from);
  std::uint8_t from_y = INDEX_TO_Y(from);
  std::uint8_t to_x = INDEX_TO_X(to);
  std::uint8_t to_y = INDEX_TO_Y(to);

  return Py_BuildValue("(ii)(ii)", from_x, from_y, to_x, to_y);
}

static PyMethodDef chiminimax_methods[] = {
    {"random", chiminimax_random, METH_VARARGS, "Generate a random number."},
    {"new_board", chiminimax_new_board, METH_VARARGS, "Create a new board and return its ID."},
    {"create_board", chiminimax_new_board, METH_VARARGS, "Create a new board and return its ID."},
    {"test_board", chiminimax_test_board, METH_VARARGS, "Test if a board exists."},
    {"delete_board", chiminimax_delete_board, METH_VARARGS,
     "Delete a board. Raises ValueError if the board does not exist."},
    {"get_piece_at", chiminimax_get_piece_at, METH_VARARGS,
     "Get the piece at a specific position. Raises ValueError if the board does not exist or the position is "
     "out of bounds."},
    {"get_king_pos", chiminimax_get_king_pos, METH_VARARGS,
     "Get the position of the king for a specific color. Raises ValueError if the board does not exist or "
     "the color is invalid."},
    {"generate_moves", chiminimax_generate_moves, METH_VARARGS,
     "Generate moves for a piece. Raises ValueError if the board does not exist."},
    {"generate_all_moves", chiminimax_generate_all_moves, METH_VARARGS,
     "Generate all possible moves. Raises ValueError if the board does not exist."},
    {"test_checkmate", chiminimax_test_checkmate, METH_VARARGS,
     "Test if a color is in checkmate. Raises ValueError if the board does not exist."},
    {"make_move", chiminimax_make_move, METH_VARARGS,
     "Make a move on the board. Raises ValueError if the board does not exist or the move is invalid."},
    {"undo_move", chiminimax_undo_move, METH_VARARGS,
     "Undo the last move on the board. Raises ValueError if the board does not exist or there are no moves "
     "to undo."},
    {"get_score", chiminimax_get_score, METH_VARARGS,
     "Get the score of the board. Raises ValueError if the board does not exist."},
    {"reset_pst", chiminimax_reset_pst, METH_VARARGS,
     "Reset the piece-square table. Raises ValueError if the board does not exist."},
    {"load_pst", chiminimax_load_pst, METH_VARARGS,
     "Load a piece-square table from a file. Raises ValueError if the board does not exist or OSError if the "
     "file cannot be loaded."},
    {"get_zobrist", chiminimax_get_zobrist, METH_VARARGS,
     "Get the Zobrist hash of the board. Raises ValueError if the board does not exist."},
    {"suggest_move", chiminimax_suggest_move, METH_VARARGS,
     "Suggest a move for a color at a given depth. Raises ValueError if the board does not exist or the "
     "color is invalid."},
    {NULL, NULL, 0, NULL}};

static PyModuleDef chiminimax_module = {
    PyModuleDef_HEAD_INIT,
    "chiminimax",
    NULL,               /* m_doc */
    -1,                 /* m_size */
    chiminimax_methods, /* m_methods */
    NULL,               /* m_slots */
    NULL,               /* m_traverse */
    NULL,               /* m_clear */
    NULL,               /* m_free */
};

#define ADD_CONST_INT(module, name)                                                                          \
  if (PyModule_AddIntConstant(module, #name, name) != 0) {                                                   \
    Py_DECREF(module);                                                                                       \
    return NULL;                                                                                             \
  }

PyMODINIT_FUNC PyInit_chiminimax(void) {
  PyObject *module = PyModule_Create(&chiminimax_module);
  if (!module)
    return NULL;

  ADD_CONST_INT(module, cEmpty);
  ADD_CONST_INT(module, cBlack);
  ADD_CONST_INT(module, cRed);
  ADD_CONST_INT(module, cColorMask);
  ADD_CONST_INT(module, cKing);
  ADD_CONST_INT(module, cAdvisor);
  ADD_CONST_INT(module, cElephant);
  ADD_CONST_INT(module, cHorse);
  ADD_CONST_INT(module, cRook);
  ADD_CONST_INT(module, cCannon);
  ADD_CONST_INT(module, cPawn);
  ADD_CONST_INT(module, cPieceMask);

  return module;
}

int main(int argc, char *argv[]) {
  PyStatus status;
  PyConfig config;
  PyObject *pmodule;
  PyConfig_InitPythonConfig(&config);

  if (PyImport_AppendInittab("chiminimax", PyInit_chiminimax) == -1) {
    fprintf(stderr, "Error: could not extend in-built modules table\n");
    exit(1);
  }

  status = PyConfig_SetBytesString(&config, &config.program_name, argv[0]);
  if (PyStatus_Exception(status)) {
    goto exception;
  }

  status = Py_InitializeFromConfig(&config);
  if (PyStatus_Exception(status)) {
    goto exception;
  }
  PyConfig_Clear(&config);

  pmodule = PyImport_ImportModule("chiminimax");
  if (!pmodule) {
    PyErr_Print();
    fprintf(stderr, "Error: could not import module 'chiminimax'\n");
  }

  return 0;

exception:
  PyConfig_Clear(&config);
  Py_ExitStatusException(status);
}
