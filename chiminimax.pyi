from typing import Literal, Optional

cEmpty: Literal[0]

cBlack: Literal[0]
cRed: Literal[8]
cColorMask: Literal[8]

cKing: Literal[1]
cAdvisor: Literal[2]
cElephant: Literal[3]
cHorse: Literal[4]
cRook: Literal[5]
cCannon: Literal[6]
cPawn: Literal[7]
cPieceMask: Literal[7]

def random() -> int:
    """Generate a random number."""

def new_board() -> int:
    """Create a new board and return its ID."""

def create_board() -> int:
    """Create a new board and return its ID."""

def test_board(board_id: int) -> bool:
    """Test if a board exists."""

def delete_board(board_id: int) -> None:
    """Delete a board. Raises ValueError if the board does not exist."""

def get_piece_at(board_id: int, x: int, y: int) -> int:
    """Get the piece at a position. Raises ValueError if the board does not exist or the position is out of bounds."""

def get_score(board_id: int) -> int:
    """Get the score of the board. Raises ValueError if the board does not exist."""

def reset_pst(board_id: int) -> None:
    """Reset the piece-square table. Raises ValueError if the board does not exist."""

def load_pst(board_id: int, filename: str) -> None:
    """Load a piece-square table from a file. Raises ValueError if the board does not exist or OSError if the file cannot be loaded."""

def generate_moves(board_id: int, x: int, y: int) -> list[tuple[int, int]]:
    """Generate moves for a piece. Raises ValueError if the board does not exist."""

def generate_all_moves(
    board_id: int,
    color: Literal[0, 8],
) -> list[tuple[tuple[int, int], tuple[int, int]]]:
    """Generate all possible moves. Raises ValueError if the board does not exist."""

def test_checkmate(board_id: int, color: Literal[0, 8]) -> bool:
    """Test if a color is in checkmate. Raises ValueError if the board does not exist."""

def make_move(board_id: int, from_x: int, from_y: int, to_x: int, to_y: int) -> None:
    """Make a move on the board. Raises ValueError if the board does not exist or the move is invalid."""

def undo_move(board_id: int) -> None:
    """Undo the last move on the board. Raises ValueError if the board does not exist or there are no moves to undo."""

def get_zobrist(board_id: int) -> int:
    """Get the Zobrist hash of the board. Raises ValueError if the board does not exist."""

def suggest_move(
    board_id: int,
    color: Literal[0, 8],
    depth: int,
) -> Optional[tuple[tuple[int, int], tuple[int, int]]]:
    """Suggest a move for a color at a given depth. Raises ValueError if the board does not exist or the color is invalid."""
