#include "Chess.h"
#include "Logger.h"
#include <limits>
#include <cmath>
#include <cctype>

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
}

void Log(std::string info) {
    Logger::GetInstance().LogInfo(info);
}

#pragma region TESTS

void Chess::TestStateNotation() {

    std::string result;
    result.reserve(72);

    std::string state = stateString();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            result += std::to_string(intNotation(state.c_str(), i, j));
        }
        result += ' ';
    }

    Log(result);
}

#pragma endregion

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    if (piece == 0) {
        Logger::GetInstance().LogError("Chess piece not assigned");
        return nullptr;
    }

    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    TestStateNotation();
    
    _moves = generateAllMoves();

    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)

    int boardPos = 0;
    
    for (int i = 0; i < fen.length(); i++) {

        char c = fen[i];
        if (c == '/') continue; // end of row

        int ascii = int(c);
        if (ascii >= 48 && ascii <= 57) { // NUMBER

            boardPos += ascii - 48; //  char '1' should add 1 to board position, '6' should add 6, etc.
            continue;

        } else { // LETTER
            
            //Logger::GetInstance().LogInfo(isupper(c) ? "true" : "false");

            ChessPiece piece = NoPiece;
            switch (ascii % 32) {

                case 2:
                    piece = Bishop;
                    break;
                case 11:
                    piece = King;
                    break;
                case 14:
                    piece = Knight;
                    break;
                case 16:
                    piece = Pawn;
                    break;
                case 17:
                    piece = Queen;
                    break;
                case 18:
                    piece = Rook;
                    break;
            }
            int playerNum = !isupper(c);
            CreatePieceAt(boardPos, playerNum, piece);
            boardPos++;
        }
    }
}

void Chess::CreatePieceAt(int position, const int playerNumber, ChessPiece piece) {

    Bit* newBit = PieceForPlayer(playerNumber, piece);
    BitHolder* holder = _grid->getSquare(position % 8, position / 8);

    newBit->setPosition(holder->getPosition());
    newBit->setParent(holder);
    newBit->setGameTag(playerNumber ? piece + 128 : piece);

    holder->setBit(newBit);
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor != currentPlayer) return false;

    bool result = false;
    ChessSquare *square = (ChessSquare*)&src;
    if (square) {
        int index = square->getSquareIndex();
        for (auto move : _moves) {
            if (move.from == index) { // found a move this piece can do
                auto dest = _grid->getSquareByIndex(move.to);
                dest->setHighlighted(true);
                result = true; // don't return yet so we can highlight other found spaces
            }
        }
    }

    return result;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    ChessSquare* srcSquare = (ChessSquare*)&src;
    ChessSquare* dstSquare = (ChessSquare*)&dst;

    if (srcSquare && dstSquare) {
        int srcIndex = srcSquare->getSquareIndex();
        for (auto move : _moves) {
            if (move.from == srcIndex && move.to == dstSquare->getSquareIndex()) {
                return true;
            }
        }
    }
    return false;
}

void Chess::endTurn() {
    Game::endTurn();
    _moves = generateAllMoves();
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

#pragma region STATES

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}

char Chess::stateNotation(const char *state, int row, int col) {
    int index = row * 8 + col;
    return (state[index] / 32) + 47;
}

int Chess::intNotation(const char *state, int row, int col) {
    int index = row * 8 + col;
    return (state[index] / 32) - 1;
}

#pragma endregion

#pragma region MOVE GENERATION

bool moveInsideChessBoard(int row, int col, int rowDelta, int colDelta) {

    return (row + rowDelta >= 0 && row + rowDelta < 8 && col + colDelta >= 0 && col + colDelta < 8);
}

void Chess::addMoveIfValid(const char *state, std::vector<BitMove>& moves, int fromRow, int fromCol, int toRow, int toCol, ChessPiece piece) {
    if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8) {
        int from = fromRow * 8 + fromCol;
        int to = toRow * 8 + toCol;
        moves.emplace_back(from, to, piece);
    }
}

std::vector<BitMove> Chess::generateAllMoves() {

    std::vector<BitMove> moves;
    moves.reserve(32);
    std::string state = stateString();

    for (int i = 0; i < 64; i++) {
        switch (state[i]) {
            case 'K':
                generateKingMoves(state.c_str(), moves, i / 8, i % 8, WHITE);
                break;
            case 'k':
                generateKingMoves(state.c_str(), moves, i / 8, i % 8, BLACK);
                break;
            case 'N':
                generateKnightMoves(state.c_str(), moves, i / 8, i % 8, WHITE);
                break;
            case 'n':
                generateKnightMoves(state.c_str(), moves, i / 8, i % 8, BLACK);
                break;
            case 'P':
                generatePawnMoves(state.c_str(), moves, i / 8, i % 8, WHITE);
                break;
            case 'p':
                generatePawnMoves(state.c_str(), moves, i / 8, i % 8, BLACK);
                break;
        }
    }

    return moves;
}

void Chess::generatePawnMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorAsInt) {

    //Log("row: " + std::to_string(row) + ", col: " + std::to_string(col));

    const int direction = (colorAsInt == WHITE) ? -1 : 1;
    const int startRow = (colorAsInt == WHITE) ? 6 : 1;

    // one square forward
    if (stateNotation(state, row + direction, col) == '0') {
        addMoveIfValid(state, moves, row, col, row + direction, col, Pawn);

        //two squares from start row
        if (row == startRow && stateNotation( state, row + 2 * direction, col) == '0') {
            addMoveIfValid(state, moves, row, col, row + 2 * direction, col, Pawn);
        }
    }

    // captures
    for (int i = -1; i <= 1; i += 2) { // -1 for leftmoves, +1 for right
        if (col + i >= 0 && col + i < 8) {

            int pieceColor = intNotation(state, row + direction, col + i) - 1;
            if (pieceColor >= 0 && pieceColor != colorAsInt) {
                addMoveIfValid(state, moves, row, col, row + direction, col + i, Pawn);
            }
        
        } 
    }
}

void Chess::generateKnightMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorAsInt) {
    static const int knightMoves[8][2] = { {1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, -2}, {-2, -1}, {-1, 2}, {-2, 1} };

    for (int i = 0; i < 8; i++) {

        int rowDelta = knightMoves[i][0];
        int colDelta = knightMoves[i][1];

        if (moveInsideChessBoard(row, col, rowDelta, colDelta)) {

            int pieceColor = intNotation(state, row + rowDelta, col + colDelta) - 1; // -1 = empty, 0 = WHITE, 1 = BLACK
        
            if (pieceColor != colorAsInt) { // valid, empty square or occupied by opponent square
                addMoveIfValid(state, moves, row, col, row + rowDelta, col + colDelta, Knight);
            }

        }
    }
}

void Chess::generateKingMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorAsInt) {

    for (int rowDelta = -1; rowDelta <= 1; rowDelta++) {

        for (int colDelta = -1; colDelta <= 1; colDelta++) {

            if (moveInsideChessBoard(row, col, rowDelta, colDelta)) {

                int pieceColor = intNotation(state, row + rowDelta, col + colDelta) - 1; // -1 = empty, 0 = WHITE, 1 = BLACK
        
                if (pieceColor != colorAsInt) { // valid, empty square or occupied by opponent square
                    addMoveIfValid(state, moves, row, col, row + rowDelta, col + colDelta, King);
                }
            }

        }
    }
}

// same as base class, except valid positions don't un-highlight when you drag piece off of them
void Chess::findDropTarget(ImVec2 &pos)
{
	Grid* grid = getGrid();
	grid->forEachEnabledSquare([&](ChessSquare* square, int x, int y) {
		if (square == _oldHolder)
		{
			return;
		}
		if (square->isMouseOver(pos))
		{
			if (_dropTarget && square != _dropTarget)
			{
				_dropTarget->willNotDropBit(_dragBit);
				//_dropTarget->setHighlighted(false);
				_dropTarget = nullptr;
			}
			if (_oldHolder && square->canDropBitAtPoint(_dragBit, pos) && canBitMoveFromTo(*_dragBit, *_oldHolder, *square))
			{
				_dropTarget = square;
				_dropTarget->setHighlighted(true);
			}
		}
	});
}

void Chess::clearBoardHighlights() {
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->setHighlighted(false);
    });}