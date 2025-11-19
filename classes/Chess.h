#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"

constexpr int pieceSize = 80;
typedef uint64_t BitBoard;
constexpr BitBoard BitZero = 1ULL;

constexpr int WHITE = 0;
constexpr int BLACK = 1;

// for pawn movement
constexpr uint64_t ROW_3 = 0x0000000000FF0000ULL;

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
    void generateKingMoves(std::vector<BitMove>& moves, BitboardElement kingBoard, uint64_t availableSquares);

    // test functions
    void TestStateNotation();

};