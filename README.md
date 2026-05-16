# ChessEngine

## Using the engine in Arena

Use the compiled executable:

- `engine.exe`

In Arena, configure the engine command to point to:

- `C:\Users\HP\OneDrive\Desktop\ChessEngine\engine.exe`

Do not point Arena to `.cpp` or `.h` files.

## Build instructions

From the workspace root, compile with:

```bash
cd "C:\Users\HP\OneDrive\Desktop\ChessEngine"
g++ -O3 bitboard.cpp movesgen.cpp ChessEngineMain.cpp uci.cpp zobrist.cpp -o engine.exe
```

## What is implemented for this phase

- UCI command loop in `uci.cpp`
- `position startpos moves ...` parsing
- `go` handling that returns a random legal move
- `bestmove [move]` output for Arena

## Next steps

The engine is connected to UCI, but still needs:

- a real search algorithm
- `stop` handling
- proper `go` time control handling
- UCI `option` support
