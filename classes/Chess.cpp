#include "Chess.h"
#include "Logger.h"
#include <limits>
#include <cmath>
#include <cctype>
#include <bitset>
#include <iostream>

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

void LogUint64(uint64_t bits) {
    Logger::GetInstance().LogInfo(std::bitset<64>(bits).to_string());
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

void Chess::GenerateAllBitboards() {

    for (int i = 0; i < 64; i++) {
        _kingBitboards[i] = generateKingBitboards(i);
        _knightBitboards[i] = generateKnightBitboards(i);
    }
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    GenerateAllBitboards();
    
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

    int currPlayer = getCurrentPlayer()->playerNumber();

    // white pieces
    uint64_t whitePawns = 0LL;
    uint64_t whiteRooks = 0LL;
    uint64_t whiteKnights = 0LL;
    uint64_t whiteBishops = 0LL;
    uint64_t whiteKing = 0LL;
    uint64_t whiteQueens = 0LL;

    // black pieces
    uint64_t blackPawns = 0LL;
    uint64_t blackRooks = 0LL;
    uint64_t blackKnights = 0LL;
    uint64_t blackBishops = 0LL;
    uint64_t blackKing = 0LL;
    uint64_t blackQueens = 0LL;

    // occupancy masks
    uint64_t whiteOccupancy = 0LL;
    uint64_t blackOccupancy = 0LL;

    for (int i = 0; i < 64; i++) {

        char piece = state[i];
        bool isBlack = piece >= 'a'; // lowercase

        if (piece != '0') {
            if (isBlack) { // lowercase
                blackOccupancy |= 1ULL << i;
            } else {
                whiteOccupancy |= 1ULL << i;
            }
        }

        switch(int(piece) % 32) {

            case 2: // bishop
                if (isBlack) blackBishops |= 1ULL << i;
                else whiteBishops |= 1ULL << i;
                break;
            case 11: // king
                if (isBlack) blackKing |= 1ULL << i;
                else whiteKing |= 1ULL << i;
                break;
            case 14: // knight
                if (isBlack) blackKnights |= 1ULL << i;
                else whiteKnights |= 1ULL << i;
                break;
            case 16: // pawn
                if (isBlack) blackPawns |= 1ULL << i;
                else whitePawns |= 1ULL << i;
                break;
            case 17: // queen
                if (isBlack) blackQueens |= 1ULL << i;
                else whiteQueens |= 1ULL << i;
                break;
            case 18: // rook
                if (isBlack) blackRooks |= 1ULL << i;
                else whiteRooks |= 1ULL << i;
                break;
        }
    }

    uint64_t allOccupancy = whiteOccupancy | blackOccupancy;

    if (currPlayer == BLACK) {
        generateKnightMoves(moves, blackKnights, ~blackOccupancy);
        generateKingMoves(moves, blackKing, ~blackOccupancy);
    } else {
        generateKnightMoves(moves, whiteKnights, ~whiteOccupancy);
        generateKingMoves(moves, whiteKing, ~whiteOccupancy);
    }

    Log("available moves: " + std::to_string(moves.size()));

    return moves;
}

void Chess::generateKnightMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t availableSquares) {
    knightBoard.forEachBit([&] (int fromSquare) {
        BitboardElement moveBitboard = BitboardElement(_knightBitboards[fromSquare].getData() & availableSquares);
        moveBitboard.forEachBit( [&] (int toSquare) {
            moves.emplace_back(fromSquare, toSquare, Knight);
        });
    });
}

void Chess::generateKingMoves(std::vector<BitMove>& moves, BitboardElement kingBoard, uint64_t availableSquares) {
    kingBoard.forEachBit([&] (int fromSquare) {
        BitboardElement moveBitboard = BitboardElement(_kingBitboards[fromSquare].getData() & availableSquares);
        moveBitboard.forEachBit([&] (int toSquare) {
            moves.emplace_back(fromSquare, toSquare, King);
        });
    });
}

#pragma region Highlight Nonsense

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

#pragma endregion

#pragma region GenerateBitboards

BitboardElement Chess::generateKnightBitboards(int square) {
    BitboardElement result = 0LL;
    int rank = square / 8;
    int file = square % 8;

    std::pair<int, int> knightOffsets[] = {
                {-1, 2},    {1, 2},
        {-2, 1},                    {2, 1},

        {-2, -1},                   {2, -1},
                {-1, -2},   {1, -2}
    };

    constexpr uint64_t oneBit = 1;
    for (auto [dr, df] : knightOffsets) {
        int r = rank + dr;
        int f = file + df;
        if (r >= 0 && r < 8 && f >= 0 && f < 8) {
            result |= oneBit << (r * 8 + f);
        }
    }

    return result;
}

BitboardElement Chess::generateKingBitboards(int square) {
    BitboardElement result = 0LL;
    int rank = square / 8;
    int file = square % 8;

    std::pair<int, int> kingOffsets[] = {
        {-1, 1}, {0, 1}, {1, 1},
        {-1, 0},         {1, 0},       
        {-1, -1}, {0, -1}, {1, -1},
    };

    constexpr uint64_t oneBit = 1;
    for (auto [dr, df] : kingOffsets) {
        int r = rank + dr;
        int f = file + df;
        if (r >= 0 && r < 8 && f >= 0 && f < 8) {
            result |= oneBit << (r * 8 + f);
        }
    }

    return result;
}

#pragma endregion