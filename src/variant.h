/*
  Fairy-Stockfish, a UCI chess variant playing engine derived from Stockfish
  Copyright (C) 2018-2022 Fabian Fichter

  Fairy-Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Fairy-Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VARIANT_H_INCLUDED
#define VARIANT_H_INCLUDED

#include <set>
#include <map>
#include <vector>
#include <string>
#include <functional>
#include <sstream>
#include <iostream>

#include "types.h"
#include "bitboard.h"

namespace Stockfish {

/// Variant struct stores information needed to determine the rules of a variant.

struct Variant {
  #define S(mg, eg) make_score(mg, eg)
  // Threshold for lazy and space evaluation
  Value LazyThreshold1    =  Value(1565);
  Value LazyThreshold2    =  Value(1102);
  Value SpaceThreshold    =  Value(11551);

  // KingAttackWeights[PieceType] contains king attack weights by piece type
  // NO_PIECE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN
  int KingAttackWeights[PIECE_TYPE_NB] = { 0, 0, 81, 52, 44, 10, 40 };

  // SafeCheck[PieceType][single/multiple] contains safe check bonus by piece type,
  // higher if multiple safe checks are possible for that piece type.
  int SafeCheck[PIECE_TYPE_NB][2] = {
      {}, {600, 600}, {803, 1292}, {639, 974}, {1087, 1878}, {759, 1132}, {600, 900}
  };

  // MobilityBonus[PieceType-2][attacked] contains bonuses for middle and end game,
  // indexed by piece type and number of attacked squares in the mobility area.
  Score MobilityBonus[PIECE_TYPE_NB - 2][4 * RANK_NB] = {
    { S(-62,-79), S(-53,-57), S(-12,-31), S( -3,-17), S(  3,  7), S( 12, 13), // Knight
      S( 21, 16), S( 28, 21), S( 37, 26) },
    { S(-47,-59), S(-20,-25), S( 14, -8), S( 29, 12), S( 39, 21), S( 53, 40), // Bishop
      S( 53, 56), S( 60, 58), S( 62, 65), S( 69, 72), S( 78, 78), S( 83, 87),
      S( 91, 88), S( 96, 98) },
    { S(-60,-82), S(-24,-15), S(  0, 17) ,S(  3, 43), S(  4, 72), S( 14,100), // Rook
      S( 20,102), S( 30,122), S( 41,133), S(41 ,139), S( 41,153), S( 45,160),
      S( 57,165), S( 58,170), S( 67,175) },
    { S(-29,-49), S(-16,-29), S( -8, -8), S( -8, 17), S( 18, 39), S( 25, 54), // Queen
      S( 23, 59), S( 37, 73), S( 41, 76), S( 54, 95), S( 65, 95) ,S( 68,101),
      S( 69,124), S( 70,128), S( 70,132), S( 70,133) ,S( 71,136), S( 72,140),
      S( 74,147), S( 76,149), S( 90,153), S(104,169), S(105,171), S(106,171),
      S(112,178), S(114,185), S(114,187), S(119,221) }
  };
  Score MaxMobility  = S(150, 200);
  Score DropMobility = S(10, 10);

  // BishopPawns[distance from edge] contains a file-dependent penalty for pawns on
  // squares of the same color as our bishop.
  Score BishopPawns[int(FILE_NB) / 2] = {
    S(3, 8), S(3, 9), S(2, 8), S(3, 8)
  };

  // KingProtector[knight/bishop] contains penalty for each distance unit to own king
  Score KingProtector[2] = { S(8, 9), S(6, 9) };

  // Outpost[knight/bishop] contains bonuses for each knight or bishop occupying a
  // pawn protected square on rank 4 to 6 which is also safe from a pawn attack.
  Score Outpost[2] = { S(57, 38), S(31, 24) };

  // PassedRank[Rank] contains a bonus according to the rank of a passed pawn
  Score PassedRank[RANK_NB] = {
    S(0, 0), S(7, 27), S(16, 32), S(17, 40), S(64, 71), S(170, 174), S(278, 262)
  };

  Score RookOnClosedFile = S(10, 5);
  Score RookOnOpenFile[2] = { S(19, 6), S(47, 26) };

  // ThreatByMinor/ByRook[attacked PieceType] contains bonuses according to
  // which piece type attacks which one. Attacks on lesser pieces which are
  // pawn-defended are not considered.
  Score ThreatByMinor[PIECE_TYPE_NB] = {
    S(0, 0), S(5, 32), S(55, 41), S(77, 56), S(89, 119), S(79, 162)
  };

  Score ThreatByRook[PIECE_TYPE_NB] = {
    S(0, 0), S(3, 44), S(37, 68), S(42, 60), S(0, 39), S(58, 43)
  };

  Value CorneredBishop = Value(50);

  // Assorted bonuses and penalties
  Score UncontestedOutpost  = S(  1, 10);
  Score BishopOnKingRing    = S( 24,  0);
  Score BishopXRayPawns     = S(  4,  5);
  Score FlankAttacks        = S(  8,  0);
  Score Hanging             = S( 69, 36);
  Score KnightOnQueen       = S( 16, 11);
  Score LongDiagonalBishop  = S( 45,  0);
  Score MinorBehindPawn     = S( 18,  3);
  Score PassedFile          = S( 11,  8);
  Score PawnlessFlank       = S( 17, 95);
  Score ReachableOutpost    = S( 31, 22);
  Score RestrictedPiece     = S(  7,  7);
  Score RookOnKingRing      = S( 16,  0);
  Score SliderOnQueen       = S( 60, 18);
  Score ThreatByKing        = S( 24, 89);
  Score ThreatByPawnPush    = S( 48, 39);
  Score ThreatBySafePawn    = S(173, 94);
  Score TrappedRook         = S( 55, 13);
  Score WeakQueenProtection = S( 14,  0);
  Score WeakQueen           = S( 56, 15);


  // Variant and fairy piece bonuses
  Score KingProximity        = S(2, 6);
  Score EndgameKingProximity = S(0, 10);
  Score ConnectedSoldier     = S(20, 20);

  int VirtualCheck = 600;
  // Added bonuses
  int pieceSquareBonus[PHASE_NB][PIECE_TYPE_NB][SQUARE_NB];
  Score promotionBonus = S(1, 1);
  Score psqValue = S(100, 100); // in centi

  #undef S

  // evaluation vars
  int pieceValue[PHASE_NB][PIECE_TYPE_NB];
  int scoreValue[PHASE_NB][TERM_NB]; // in centi
  // MATERIAL, IMBALANCE, MOBILITY, THREAT, PASSED, SPACE, VARIANT, WINNABLE, TOTAL
  std::string termToChar = "|material|imbalance|mobility|threat|passed|space|variant|winnable|total ";
  std::string scoreToChar = "";

  std::string variantTemplate = "fairy";
  std::string pieceToCharTable = "-";
  int pocketSize = 0;
  Rank maxRank = RANK_8;
  File maxFile = FILE_H;
  bool chess960 = false;
  bool twoBoards = false;
  std::string customPiece[CUSTOM_PIECES_NB] = {};
  std::set<PieceType> pieceTypes = { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
  std::string pieceTypeToChar = " pnbrq" + std::string(KING - QUEEN - 1, ' ') + "k" + std::string(PIECE_TYPE_NB - KING - 1, ' ');
  std::string pieceToChar;
  std::string pieceToCharSynonyms = std::string(PIECE_NB, ' ');
  std::string startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  Bitboard mobilityRegion[COLOR_NB][PIECE_TYPE_NB] = {};
  Rank promotionRank = RANK_8;
  std::set<PieceType, std::greater<PieceType> > promotionPieceTypes = { QUEEN, ROOK, BISHOP, KNIGHT };
  bool sittuyinPromotion = false;
  int promotionLimit[PIECE_TYPE_NB] = {}; // 0 means unlimited
  PieceType promotedPieceType[PIECE_TYPE_NB] = {};
  bool piecePromotionOnCapture = false;
  bool mandatoryPawnPromotion = true;
  bool mandatoryPiecePromotion = false;
  bool pieceDemotion = false;
  bool blastOnCapture = false;
  bool doubleStep = true;
  Rank doubleStepRank = RANK_2;
  Rank doubleStepRankMin = RANK_2;
  Bitboard enPassantRegion = AllSquares;
  bool castling = true;
  bool castlingDroppedPiece = false;
  File castlingKingsideFile = FILE_G;
  File castlingQueensideFile = FILE_C;
  Rank castlingRank = RANK_1;
  File castlingKingFile = FILE_E;
  PieceType castlingKingPiece = KING;
  PieceType castlingRookPiece = ROOK;
  PieceType kingType = KING;
  bool checking = true;
  bool dropChecks = true;
  bool mustCapture = false;
  bool mustDrop = false;
  PieceType mustDropType = ALL_PIECES;
  bool pieceDrops = false;
  bool dropLoop = false;
  bool capturesToHand = false;
  bool firstRankPawnDrops = false;
  bool promotionZonePawnDrops = false;
  bool dropOnTop = false;
  EnclosingRule enclosingDrop = NO_ENCLOSING;
  Bitboard enclosingDropStart = 0;
  Bitboard whiteDropRegion = AllSquares;
  Bitboard blackDropRegion = AllSquares;
  bool sittuyinRookDrop = false;
  bool dropOppositeColoredBishop = false;
  bool dropPromoted = false;
  PieceType dropNoDoubled = NO_PIECE_TYPE;
  int dropNoDoubledCount = 1;
  bool immobilityIllegal = false;
  bool gating = false;
  bool arrowGating = false;
  bool seirawanGating = false;
  bool cambodianMoves = false;
  Bitboard diagonalLines = 0;
  bool pass = false;
  bool passOnStalemate = false;
  bool makpongRule = false;
  bool flyingGeneral = false;
  Rank soldierPromotionRank = RANK_1;
  EnclosingRule flipEnclosedPieces = NO_ENCLOSING;
  bool freeDrops = false;

  // game end
  int nMoveRule = 50;
  int nFoldRule = 3;
  Value nFoldValue = VALUE_DRAW;
  bool nFoldValueAbsolute = false;
  bool perpetualCheckIllegal = false;
  bool moveRepetitionIllegal = false;
  ChasingRule chasingRule = NO_CHASING;
  Value stalemateValue = VALUE_DRAW;
  bool stalematePieceCount = false; // multiply stalemate value by sign(count(~stm) - count(stm))
  Value checkmateValue = -VALUE_MATE;
  bool shogiPawnDropMateIllegal = false;
  bool shatarMateRule = false;
  bool bikjangRule = false;
  Value extinctionValue = VALUE_NONE;
  bool extinctionClaim = false;
  bool extinctionPseudoRoyal = false;
  std::set<PieceType> extinctionPieceTypes = {};
  int extinctionPieceCount = 0;
  int extinctionOpponentPieceCount = 0;
  PieceType flagPiece = NO_PIECE_TYPE;
  Bitboard whiteFlag = 0;
  Bitboard blackFlag = 0;
  bool flagMove = false;
  bool checkCounting = false;
  int connectN = 0;
  MaterialCounting materialCounting = NO_MATERIAL_COUNTING;
  CountingRule countingRule = NO_COUNTING;

  // Derived properties
  bool fastAttacks = true;
  bool fastAttacks2 = true;
  std::string nnueAlias = "";
  PieceType nnueKing = KING;
  int nnueDimensions;
  bool nnueUsePockets;
  int pieceSquareIndex[COLOR_NB][PIECE_NB];
  int pieceHandIndex[COLOR_NB][PIECE_NB];
  int kingSquareIndex[SQUARE_NB];
  int nnueMaxPieces;
  bool endgameEval = false;
  bool shogiStylePromotions = false;

  void add_piece(PieceType pt, char c, std::string betza = "", char c2 = ' ') {
      pieceTypeToChar[make_piece(WHITE, pt)] = tolower(c);
      scoreToChar[make_piece(WHITE, pt) * 2 + 1] = tolower(c);

      pieceToChar[make_piece(WHITE, pt)] = toupper(c);
      pieceToChar[make_piece(BLACK, pt)] = tolower(c);
      pieceToCharSynonyms[make_piece(WHITE, pt)] = toupper(c2);
      pieceToCharSynonyms[make_piece(BLACK, pt)] = tolower(c2);
      pieceTypes.insert(pt);
      // Add betza notation for custom piece
      if (is_custom(pt))
          customPiece[pt - CUSTOM_PIECES] = betza;
  }

  void add_piece(PieceType pt, char c, char c2) {
      add_piece(pt, c, "", c2);
  }

  void remove_piece(PieceType pt) {
      pieceTypeToChar[make_piece(WHITE, pt)] = ' ';
      scoreToChar[make_piece(WHITE, pt) * 2 + 1] = ' ';

      pieceToChar[make_piece(WHITE, pt)] = ' ';
      pieceToChar[make_piece(BLACK, pt)] = ' ';
      pieceToCharSynonyms[make_piece(WHITE, pt)] = ' ';
      pieceToCharSynonyms[make_piece(BLACK, pt)] = ' ';
      pieceTypes.erase(pt);
  }

  void reset_pieces() {
      pieceTypeToChar = std::string(PIECE_NB / 2, ' ');

      for (std::string::size_type i = 0; i < pieceTypeToChar.size(); i++) {
        scoreToChar[i * 2 + 1] = ' ';
      }

      pieceToChar = std::string(PIECE_NB, ' ');
      pieceToCharSynonyms = std::string(PIECE_NB, ' ');
      pieceTypes.clear();
  }

  // Reset values that always need to be redefined
  Variant* init() {
      nnueAlias = "";
      return this;
  }

  // Pre-calculate derived properties
  Variant* conclude() {
      fastAttacks = std::all_of(pieceTypes.begin(), pieceTypes.end(), [this](PieceType pt) {
                                    return (   pt < FAIRY_PIECES
                                            || pt == COMMONER || pt == IMMOBILE_PIECE
                                            || pt == ARCHBISHOP || pt == CHANCELLOR
                                            || (pt == KING && kingType == KING))
                                          && !(mobilityRegion[WHITE][pt] || mobilityRegion[BLACK][pt]);
                                })
                    && !cambodianMoves
                    && !diagonalLines;
      fastAttacks2 = std::all_of(pieceTypes.begin(), pieceTypes.end(), [this](PieceType pt) {
                                    return (   pt < FAIRY_PIECES
                                            || pt == COMMONER || pt == FERS || pt == WAZIR || pt == BREAKTHROUGH_PIECE
                                            || pt == SHOGI_PAWN || pt == GOLD || pt == SILVER || pt == SHOGI_KNIGHT
                                            || pt == DRAGON || pt == DRAGON_HORSE || pt == LANCE
                                            || (pt == KING && kingType == KING))
                                          && !(mobilityRegion[WHITE][pt] || mobilityRegion[BLACK][pt]);
                                })
                    && !cambodianMoves
                    && !diagonalLines;

      // Initialize calculated NNUE properties
      nnueKing =  pieceTypes.find(KING) != pieceTypes.end() ? KING
                : extinctionPieceCount == 0 && extinctionPieceTypes.find(COMMONER) != extinctionPieceTypes.end() ? COMMONER
                : NO_PIECE_TYPE;
      if (nnueKing != NO_PIECE_TYPE)
      {
          std::string fenBoard = startFen.substr(0, startFen.find(' '));
          // Switch NNUE from KA to A if there is no unique piece
          if (   std::count(fenBoard.begin(), fenBoard.end(), pieceToChar[make_piece(WHITE, nnueKing)]) != 1
              || std::count(fenBoard.begin(), fenBoard.end(), pieceToChar[make_piece(BLACK, nnueKing)]) != 1)
              nnueKing = NO_PIECE_TYPE;
      }
      int nnueSquares = (maxRank + 1) * (maxFile + 1);
      nnueUsePockets = (pieceDrops && (capturesToHand || (!mustDrop && !arrowGating && pieceTypes.size() != 1))) || seirawanGating;
      int nnuePockets = nnueUsePockets ? 2 * int(maxFile + 1) : 0;
      int nnueNonDropPieceIndices = (2 * pieceTypes.size() - (nnueKing != NO_PIECE_TYPE)) * nnueSquares;
      int nnuePieceIndices = nnueNonDropPieceIndices + 2 * (pieceTypes.size() - (nnueKing != NO_PIECE_TYPE)) * nnuePockets;
      int i = 0;
      for (PieceType pt : pieceTypes)
      {
          for (Color c : { WHITE, BLACK})
          {
              pieceSquareIndex[c][make_piece(c, pt)] = 2 * i * nnueSquares;
              pieceSquareIndex[c][make_piece(~c, pt)] = (2 * i + (pt != nnueKing)) * nnueSquares;
              pieceHandIndex[c][make_piece(c, pt)] = 2 * i * nnuePockets + nnueNonDropPieceIndices;
              pieceHandIndex[c][make_piece(~c, pt)] = (2 * i + 1) * nnuePockets + nnueNonDropPieceIndices;
          }
          i++;
      }

      // Map king squares to enumeration of actually available squares.
      // E.g., for xiangqi map from 0-89 to 0-8.
      // Variants might be initialized before bitboards, so do not rely on precomputed bitboards (like SquareBB).
      // Furthermore conclude() might be called on invalid configuration during validation,
      // therefore skip proper initialization in case of invalid board size.
      int nnueKingSquare = 0;
      if (nnueKing && nnueSquares <= SQUARE_NB)
          for (Square s = SQ_A1; s < nnueSquares; ++s)
          {
              Square bitboardSquare = Square(s + s / (maxFile + 1) * (FILE_MAX - maxFile));
              if (   !mobilityRegion[WHITE][nnueKing] || !mobilityRegion[BLACK][nnueKing]
                  || (mobilityRegion[WHITE][nnueKing] & make_bitboard(bitboardSquare))
                  || (mobilityRegion[BLACK][nnueKing] & make_bitboard(relative_square(BLACK, bitboardSquare, maxRank))))
              {
                  kingSquareIndex[s] = nnueKingSquare++ * nnuePieceIndices;
              }
          }
      else
          kingSquareIndex[SQ_A1] = nnueKingSquare++ * nnuePieceIndices;
      nnueDimensions = nnueKingSquare * nnuePieceIndices;

      // Determine maximum piece count
      std::istringstream ss(startFen);
      ss >> std::noskipws;
      unsigned char token;
      nnueMaxPieces = 0;
      while ((ss >> token) && !isspace(token))
      {
          if (pieceToChar.find(token) != std::string::npos || pieceToCharSynonyms.find(token) != std::string::npos)
              nnueMaxPieces++;
      }
      if (twoBoards)
          nnueMaxPieces *= 2;

      // For endgame evaluation to be applicable, no special win rules must apply.
      // Furthermore, rules significantly changing game mechanics also invalidate it.
      endgameEval = std::none_of(pieceTypes.begin(), pieceTypes.end(), [this](PieceType pt) {
                                    return mobilityRegion[WHITE][pt] || mobilityRegion[BLACK][pt];
                                })
                    && extinctionValue == VALUE_NONE
                    && checkmateValue == -VALUE_MATE
                    && stalemateValue == VALUE_DRAW
                    && !materialCounting
                    && !flagPiece
                    && !mustCapture
                    && !checkCounting
                    && !makpongRule
                    && !connectN
                    && !blastOnCapture
                    && !capturesToHand
                    && !twoBoards
                    && kingType == KING;
    
      shogiStylePromotions = false;
      for (PieceType current: promotedPieceType)
          if (current != NO_PIECE_TYPE)
          {
              shogiStylePromotions = true;
              break;
          }

      return this;
  }
};

class VariantMap : public std::map<std::string, const Variant*> {
public:
  void init();
  template <bool DoCheck> void parse(std::string path);
  template <bool DoCheck> void parse_istream(std::istream& file);
  void clear_all();
  std::vector<std::string> get_keys();

private:
  void add(std::string s, Variant* v);
};

extern VariantMap variants;

} // namespace Stockfish

#endif // #ifndef VARIANT_H_INCLUDED
