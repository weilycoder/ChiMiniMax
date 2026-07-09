# ChiMiniMax/pst_editor.py - Source code for the ChiMiniMax project.
#
# A PST (Piece-Square Table) editor for the ChiMiniMax project.
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

import tkinter as tk

from tkinter import messagebox, filedialog, ttk

from typing import Generator

SIZE = 256
TABLE = 5
TABLE_SIZE = 256 * 5

ROW_MIN, ROW_MAX = 0x3, 0xC
COL_MIN, COL_MAX = 0x3, 0xB
ROWS = ROW_MAX - ROW_MIN + 1
COLS = COL_MAX - COL_MIN + 1


def pos_to_rc(pos: int) -> tuple[int, int]:
    return ((pos >> 4) & 0x0F) - ROW_MIN, (pos & 0x0F) - COL_MIN


def rc_to_pos(row: int, col: int) -> int:
    return ((row + ROW_MIN) << 4) | (col + COL_MIN)


def pos_range() -> Generator[int, None, None]:
    for row in range(ROWS):
        for col in range(COLS):
            yield rc_to_pos(row, col)


class TableEditor(ttk.Frame):
    def __init__(self, parent: tk.Widget, data: bytearray, tid: int):
        if len(data) != TABLE_SIZE:
            raise ValueError(f"table must be of size {TABLE_SIZE}, got {len(data)}")

        if not (0 <= tid < TABLE):
            raise ValueError(f"table_id must be in range [0, {TABLE}), got {tid}")

        super().__init__(parent)
        self.data = data
        self.tid = tid

        self.initUI()

    def rc_to_pos(self, row: int, col: int) -> int:
        return self.tid * SIZE + rc_to_pos(row, col)

    def initUI(self):
        self.datas = [
            [tk.StringVar(value=str(self.data[self.rc_to_pos(row, col)])) for col in range(COLS)]
            for row in range(ROWS)
        ]
        self.entries = [
            [
                ttk.Entry(
                    self,
                    width=3,
                    justify=tk.CENTER,
                    font=("system", 10),
                    textvariable=self.datas[row][col],
                )
                for col in range(COLS)
            ]
            for row in range(ROWS)
        ]

        for row in range(ROWS):
            for col in range(COLS):
                entry = self.entries[row][col]
                entry.grid(row=row, column=col, padx=2, pady=2)
                entry.bind("<FocusIn>", lambda _, r=row, c=col: self.entry_focus_in(r, c))
                entry.bind("<FocusOut>", lambda _, r=row, c=col: self.entry_focus_out(r, c))
                entry.bind("<Return>", lambda _, r=row, c=col: self.entry_focus_get(r + 1, c))

    def entry_focus_in(self, row: int, col: int):
        self.entries[row][col].icursor(tk.END)
        self.entries[row][col].select_range(0, tk.END)

    def entry_focus_get(self, row: int, col: int):
        self.entries[row % ROWS][col % COLS].focus_set()

    def entry_focus_out(self, row: int, col: int):
        var = self.datas[row][col]

        try:
            self.data[self.rc_to_pos(row, col)] = min(max(int(var.get()), 0), 255)
        except ValueError:
            self.data[self.rc_to_pos(row, col)] = 0

        var.set(str(self.data[self.rc_to_pos(row, col)]))

    def update_table(self):
        for row in range(ROWS):
            for col in range(COLS):
                self.datas[row][col].set(str(self.data[self.rc_to_pos(row, col)]))


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.table = bytearray(TABLE_SIZE)
        self.initUI()

    def initUI(self):
        self.title("PST Editor")
        self.geometry("350x325")
        self.resizable(False, False)

        self.menubar = tk.Menu(self)
        self.config(menu=self.menubar)

        self.menubar.add_command(label="Load", command=self.load_table_dialog)
        self.menubar.add_command(label="Save As", command=self.save_table_as_dialog)
        self.menubar.add_command(label="Exit", command=self.quit)

        self.notebook = ttk.Notebook(self, takefocus=0)
        self.notebook.place(relx=0.05, rely=0.05, relwidth=0.9, relheight=0.9)

        self.notebook.add(TableEditor(self.notebook, self.table, 0), text="King/Pawn")
        self.notebook.add(TableEditor(self.notebook, self.table, 1), text="Advisor/Elephant")
        self.notebook.add(TableEditor(self.notebook, self.table, 2), text="Horse")
        self.notebook.add(TableEditor(self.notebook, self.table, 3), text="Rook")
        self.notebook.add(TableEditor(self.notebook, self.table, 4), text="Cannon")

    def load_table(self, filename: str):
        with open(filename, "rb") as f:
            data = f.read(TABLE_SIZE)
            if len(data) != TABLE_SIZE:
                raise ValueError(f"File size is not {TABLE_SIZE} bytes")
            if f.read(1):
                raise ValueError("File has extra data beyond expected size")
            self.table[:] = data

        for i in self.notebook.tabs():
            self.notebook.nametowidget(i).update_table()

    def save_table(self, filename: str):
        with open(filename, "wb") as f:
            f.write(self.table)

    def load_table_dialog(self):
        filename = filedialog.askopenfilename(
            title="Load PST Table",
            filetypes=[("PST Files", "*.dat")],
        )

        if filename:
            try:
                self.load_table(filename)
            except Exception as e:
                messagebox.showerror("Error", f"Failed to load table: {e}")

    def save_table_as_dialog(self):
        filename = filedialog.asksaveasfilename(
            title="Save PST Table As",
            defaultextension=".dat",
            filetypes=[("PST Files", "*.dat")],
        )

        if filename:
            try:
                self.save_table(filename)
            except Exception as e:
                messagebox.showerror("Error", f"Failed to save table: {e}")


if __name__ == "__main__":
    app = App()
    app.mainloop()
