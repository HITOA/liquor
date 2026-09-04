# Liquor
A small engine made as a fun side-project. The initial goal was to make an engine above 2000 Elo;
It should now plays comfortably above that. Though it has not yet been tested outside lichess.

It works with classic negamax search and a 768-input NNUE eval.

### Features

Here is a list of all the features of the engine by categories. The engine supports the Universal Chess Interface (UCI)  
as the main way to communicate.

Board Representation & Move Generation:
- [Chess Library](https://github.com/Disservin/chess-library)

Search:
- Iterative Deepening
- Negamax
- PVS (Principal Variation Search)
- Quiescence Search
- Transposition Table
- Null-Move Pruning
- Reverse Futility Pruning
- Log Based LMR (Late Move Reduction)
- Check Extension
- Triangular PV Table

Move Ordering:
- TT / PV move
- MvvLva (Most Valuable Victim, Less Valuable Attacker)
- Killer Move
- History

Evaluation:
- NNUE 768 Board (768 → 512)x2 → 1
- Inference is [MantaRay](https://github.com/TheBlackPlague/MantaRay)

The NNUE was trained on a mix of Stockfish and Lichess games.  
Around ~80% of games come from Stockfish against itself with fishtest.  
Around ~20% of games come from filtered Lichess monthly game database.  
The mix of both come from the lack of bad position understanding if using
only Stockfish vs Stockfish games. Lichess games help by providing examples of 
material deficit without compensation.