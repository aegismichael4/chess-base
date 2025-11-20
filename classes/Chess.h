#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"
#include "MagicBitboards.h"

constexpr int pieceSize = 80;
typedef uint64_t BitBoard;
constexpr BitBoard BitZero = 1ULL;

constexpr int WHITE = 0;
constexpr int BLACK = 1;

// rows (for bitboard stuff)
constexpr uint64_t ROW_1 = 0x00000000000000FFULL;
constexpr uint64_t ROW_2 = 0x000000000000FF00ULL;
constexpr uint64_t ROW_3 = 0x0000000000FF0000ULL;
constexpr uint64_t ROW_4 = 0x00000000FF000000ULL;
constexpr uint64_t ROW_5 = 0x000000FF00000000ULL;
constexpr uint64_t ROW_6 = 0x0000FF0000000000ULL;
constexpr uint64_t ROW_7 = 0x00FF000000000000ULL;
constexpr uint64_t ROW_8 = 0xFF00000000000000ULL;

// cols
constexpr uint64_t COL_1 = 0x0101010101010101ULL;
constexpr uint64_t COL_2 = COL_1 << 1;
constexpr uint64_t COL_3 = COL_1 << 2;
constexpr uint64_t COL_4 = COL_1 << 3;
constexpr uint64_t COL_5 = COL_1 << 4;
constexpr uint64_t COL_6 = COL_1 << 5;
constexpr uint64_t COL_7 = COL_1 << 6;
constexpr uint64_t COL_8 = COL_1 << 7;

// precompute for pawn movement
constexpr uint64_t NOT_COL_1 = ~COL_1;
constexpr uint64_t NOT_COL_8 = ~COL_8;

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
   // void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;
    void endTurn() override;

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;
    char stateNotation(const char *state, int row, int col);
    int intNotation(const char *state, int row, int col);

    Grid* getGrid() override { return _grid; }

    void findDropTarget(ImVec2 &pos) override;
    void clearBoardHighlights() override;

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    void CreatePieceAt(int row, int col, const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    Grid* _grid;

    std::vector<BitMove> _moves;

    // bitboards
    BitboardElement _pawnBitboards[64];
    BitboardElement _rookBitboards[64];
    BitboardElement _knightBitboards[64];
    BitboardElement _bishopBitboards[64];
    BitboardElement _queenBitboards[64];
    BitboardElement _kingBitboards[64];

    // bitboard generation
    void GenerateAllBitboards();
    BitboardElement generatePawnBitboards(int square);
    BitboardElement generateRookBitboards(int square);    
    BitboardElement generateKnightBitboards(int square);
    BitboardElement generateBishopBitboards(int square);
    BitboardElement generateQueenBitboards(int square);
    BitboardElement generateKingBitboards(int square);

    // move generation
    void addMoveIfValid(const char *state, std::vector<BitMove>& moves, int fromRow, int fromCol, int toRow, int toCol, ChessPiece piece);
    std::vector<BitMove> generateAllMoves();

    void addPawnMoves(std::vector<BitMove>& moves, BitboardElement pawnMoves, int shift);
    void generateWhitePawnMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t emptySquares, uint64_t blackOccupancy);
    void generateBlackPawnMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t emptySquares, uint64_t whiteOccupancy);

    void generateKnightMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t availableSquares);
    void generateRookMoves(std::vector<BitMove>& moves, BitboardElement rookBoard, uint64_t availableSquares, uint64_t blockedSquares);
    void generateBishopMoves(std::vector<BitMove>& moves, BitboardElement rookBoard, uint64_t availableSquares);
    void generateQueenMoves(std::vector<BitMove>& moves, BitboardElement rookBoard, uint64_t availableSquares);
    void generateKingMoves(std::vector<BitMove>& moves, BitboardElement kingBoard, uint64_t availableSquares);

    // test functions
    void TestStateNotation();

};