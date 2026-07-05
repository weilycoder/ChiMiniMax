cEmpty: int

cBlack: int
cRed: int
cColorMask: int

cKing: int
cAdvisor: int
cElephant: int
cHorse: int
cRook: int
cCannon: int
cPawn: int
cPieceMask: int

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

def generate_moves(board_id: int, x: int, y: int) -> list[tuple[int, int]]:
    """Generate moves for a piece. Raises ValueError if the board does not exist."""

def generate_all_moves(board_id: int, color: str) -> list[tuple[tuple[int, int], tuple[int, int]]]:
    """Generate all possible moves. Raises ValueError if the board does not exist."""

def test_checkmate(board_id: int, color: str) -> bool:
    """Test if a color is in checkmate. Raises ValueError if the board does not exist."""

def make_move(board_id: int, from_x: int, from_y: int, to_x: int, to_y: int) -> None:
    """Make a move on the board. Raises ValueError if the board does not exist or the move is invalid."""

def undo_move(board_id: int) -> None:
    """Undo the last move on the board. Raises ValueError if the board does not exist or there are no moves to undo."""
