#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"

constexpr int pieceSize = 80;
typedef uint64_t BitBoard;
constexpr BitBoard BitZero = 1ULL;

constexpr int WHITE = 0;
constexpr int BLACK = 1;

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

    void addMoveIfValid(const char *state, std::vector<BitMove>& moves, int fromRow, int fromCol, int toRow, int toCol, ChessPiece piece);

    std::vector<BitMove> generateAllMoves();
    void generateKnightMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t emptySquares);
    void generatePawnMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorAsInt);

    void findDropTarget(ImVec2 &pos) override;
    void clearBoardHighlights() override;

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    void CreatePieceAt(int position, const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    Grid* _grid;
    BitBoard _knightBitboards[64];
    BitBoard _kingBitboards[64];

    std::vector<BitMove> _moves;

    // test functions
    void TestStateNotation();

};