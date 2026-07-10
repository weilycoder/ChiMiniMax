# ChiMiniMax/app.py - Source code for the ChiMiniMax project.
#
# Implementation of a Chinese Chess (Xiangqi) game with a graphical
# user interface using Tkinter.
#
# Copyright (C) 2026 weilycoder
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import chiminimax as chiM
import tkinter as tk

from tkinter import messagebox
from just_playback import Playback

from typing import Optional, Literal, cast

BOARD_WIDTH = 540
BOARD_HEIGHT = 600

GRID_SIZE = 60
PIECE_SIZE = 57

MOVE_SOUND_LENGTH = 0.2


INIT_BOARD: tuple[tuple[str, ...], ...] = (
    ("rookB", "horseB", "elephantB", "advisorB", "kingB", "advisorB", "elephantB", "horseB", "rookB"),
    ("", "", "", "", "", "", "", "", ""),
    ("", "cannonB", "", "", "", "", "", "cannonB", ""),
    ("pawnB", "", "pawnB", "", "pawnB", "", "pawnB", "", "pawnB"),
    ("", "", "", "", "", "", "", "", ""),
    ("", "", "", "", "", "", "", "", ""),
    ("pawnR", "", "pawnR", "", "pawnR", "", "pawnR", "", "pawnR"),
    ("", "cannonR", "", "", "", "", "", "cannonR", ""),
    ("", "", "", "", "", "", "", "", ""),
    ("rookR", "horseR", "elephantR", "advisorR", "kingR", "advisorR", "elephantR", "horseR", "rookR"),
)


class Assets:
    def __init__(self):
        self.board = self.load_image("board", BOARD_WIDTH, BOARD_HEIGHT)

        self.advisorB = self.load_image("AdvisorB", PIECE_SIZE, PIECE_SIZE)
        self.advisorR = self.load_image("AdvisorR", PIECE_SIZE, PIECE_SIZE)

        self.cannonB = self.load_image("CannonB", PIECE_SIZE, PIECE_SIZE)
        self.cannonR = self.load_image("CannonR", PIECE_SIZE, PIECE_SIZE)

        self.elephantB = self.load_image("ElephantB", PIECE_SIZE, PIECE_SIZE)
        self.elephantR = self.load_image("ElephantR", PIECE_SIZE, PIECE_SIZE)

        self.horseB = self.load_image("HorseB", PIECE_SIZE, PIECE_SIZE)
        self.horseR = self.load_image("HorseR", PIECE_SIZE, PIECE_SIZE)

        self.kingB = self.load_image("KingB", PIECE_SIZE, PIECE_SIZE)
        self.kingR = self.load_image("KingR", PIECE_SIZE, PIECE_SIZE)
        self.kingBD = self.load_image("KingBD", PIECE_SIZE, PIECE_SIZE)
        self.kingRD = self.load_image("KingRD", PIECE_SIZE, PIECE_SIZE)

        self.pawnB = self.load_image("PawnB", PIECE_SIZE, PIECE_SIZE)
        self.pawnR = self.load_image("PawnR", PIECE_SIZE, PIECE_SIZE)

        self.rookB = self.load_image("RookB", PIECE_SIZE, PIECE_SIZE)
        self.rookR = self.load_image("RookR", PIECE_SIZE, PIECE_SIZE)

        self.selected = self.load_image("selected", PIECE_SIZE, PIECE_SIZE)

        self.moveB = self.load_sound("moveB", MOVE_SOUND_LENGTH)
        self.moveR = self.load_sound("moveR", MOVE_SOUND_LENGTH)

    def load_image(self, name: str, width: int, height: int) -> tk.PhotoImage:
        image = tk.PhotoImage(file=f"assets/{name}.gif")

        if image.width() != width or image.height() != height:
            raise ValueError(f"{name}.gif must be {width}x{height} pixels.")

        return image

    def load_sound(self, name: str, length_limit) -> Playback:
        sound = Playback(f"assets/{name}.wav")

        if sound.duration > length_limit:
            raise ValueError(f"{name}.wav must be less than {length_limit} seconds.")

        return sound

    def get_piece_image(self, piece_name: str) -> tk.PhotoImage:
        if hasattr(self, piece_name):
            image = getattr(self, piece_name)
            if isinstance(image, tk.PhotoImage):
                return image
        raise ValueError(f"Image {ascii(piece_name)} not found in Assets.")


class Board(tk.Frame):
    def __init__(
        self,
        master: Optional[tk.Misc] = None,
        first_move_color: Literal[0, 8] = chiM.cRed,
        red_depth: Optional[int] = None,
        black_depth: Optional[int] = None,
    ):
        super().__init__(master)

        self.canvas = tk.Canvas(self, width=BOARD_WIDTH, height=BOARD_HEIGHT)
        self.canvas.place(x=0, y=0, width=BOARD_WIDTH, height=BOARD_HEIGHT)

        self.red_depth = red_depth
        self.black_depth = black_depth
        self.init_board(first_move_color)

    def init_board(self, first_move_color: Literal[0, 8]):
        self.assets = Assets()
        self.canvas.create_image(0, 0, anchor=tk.NW, image=self.assets.board)

        self.selected = self.create_piece(0, 0, self.assets.selected)
        self.canvas.itemconfig(self.selected, state=tk.HIDDEN)

        self.chiM = chiM.new_board()
        self.curr_color: Literal[0, 8] = first_move_color
        self.board: dict[tuple[int, int], int] = {}
        self.captured_pieces: list[int] = []
        self.moves: list[tuple[tuple[int, int], tuple[int, int], bool]] = []

        for y, row in enumerate(INIT_BOARD):
            for x, piece_name in enumerate(row):
                if piece_name:
                    image = self.assets.get_piece_image(piece_name)
                    self.board[(x, y)] = self.create_piece(x, y, image)

        self.master.bind("<Button-1>", lambda e: self.select_grid(e.x // GRID_SIZE, e.y // GRID_SIZE))

    def get_piece_at(self, x: int, y: int) -> int:
        return chiM.get_piece_at(self.chiM, x, y)

    def get_color_at(self, x: int, y: int) -> Literal[0, 8]:
        return cast(Literal[0, 8], self.get_piece_at(x, y) & chiM.cColorMask)

    def suggest_move(self):
        depth = self.red_depth if self.curr_color == chiM.cRed else self.black_depth
        if depth is not None:
            res = chiM.suggest_move(self.chiM, self.curr_color, depth)
            if res is not None:
                (ox, oy), (nx, ny) = res
                self.move_piece(ox, oy, nx, ny)

    def select_grid(self, x: int, y: int):
        if self.canvas.itemcget(self.selected, "state") == tk.HIDDEN:
            self.canvas.moveto(self.selected, *self.grid_position(x, y))
            if (x, y) in self.board and self.get_color_at(x, y) == self.curr_color:
                self.canvas.itemconfig(self.selected, state=tk.NORMAL)
        else:
            self.canvas.itemconfig(self.selected, state=tk.HIDDEN)
            old_x, old_y = map(lambda x: int(x // GRID_SIZE), self.canvas.coords(self.selected))
            if self.check_move(old_x, old_y, x, y):
                self.move_piece(old_x, old_y, x, y)
            elif (x, y) != (old_x, old_y):
                self.select_grid(x, y)

    def create_piece(self, x: int, y: int, image: tk.PhotoImage) -> int:
        return self.canvas.create_image(
            *self.grid_position(x, y),
            anchor=tk.NW,
            image=image,
        )

    def capture_piece(self, x: int, y: int) -> bool:
        if (x, y) in self.board:
            self.captured_pieces.append(self.board.pop((x, y)))
            self.canvas.itemconfig(self.captured_pieces[-1], state=tk.HIDDEN)
            return True
        return False

    def move_piece(self, old_x: int, old_y: int, new_x: int, new_y: int):
        chiM.make_move(self.chiM, old_x, old_y, new_x, new_y)
        captured = self.capture_piece(new_x, new_y)
        piece_id = self.board.pop((old_x, old_y))
        self.canvas.moveto(piece_id, *self.grid_position(new_x, new_y))
        self.board[(new_x, new_y)] = piece_id
        self.moves.append(((old_x, old_y), (new_x, new_y), captured))

        color = self.get_color_at(new_x, new_y)
        self.play_sound(self.assets.moveR if color == chiM.cRed else self.assets.moveB)
        self.curr_color = cast(Literal[0, 8], self.curr_color ^ chiM.cColorMask)

    def undo_move(self):
        chiM.undo_move(self.chiM)
        old_pos, new_pos, captured = self.moves.pop()
        assert old_pos not in self.board
        piece_id = self.board.pop(new_pos)

        self.canvas.moveto(piece_id, *self.grid_position(*old_pos))
        self.board[old_pos] = piece_id

        if captured:
            captured_piece_id = self.captured_pieces.pop()
            self.board[new_pos] = captured_piece_id
            self.canvas.itemconfig(captured_piece_id, state=tk.NORMAL)

    def grid_position(self, x: int, y: int) -> tuple[int, int]:
        return x * GRID_SIZE + 1, y * GRID_SIZE + 1

    def play_sound(self, sound: Playback):
        sound.play()
        while sound.playing:
            self.update()

    def check_move(self, old_x: int, old_y: int, new_x: int, new_y: int) -> bool:
        return (new_x, new_y) in chiM.generate_moves(self.chiM, old_x, old_y)


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("ChiMiniMax")
        self.geometry(f"{BOARD_WIDTH}x{BOARD_HEIGHT}")
        self.resizable(False, False)

    def initUI(self):
        self.board = Board(self)
        self.board.pack(fill=tk.BOTH, expand=True)


if __name__ == "__main__":
    app = App()

    try:
        app.initUI()
    except ValueError as e:
        messagebox.showerror("Error", str(e))
        app.destroy()
    else:
        app.mainloop()
