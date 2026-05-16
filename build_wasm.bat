@echo off
cd C:\Users\HP\OneDrive\Desktop\ChessEngine
call .\emsdk\emsdk activate latest
call .\emsdk\emsdk_env.bat
emcc wasm_wrapper.cpp bitboard.cpp movesgen.cpp zobrist.cpp -o engine.js -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" -s EXPORTED_FUNCTIONS="['_get_random_legal_move','_malloc','_free']" -s MODULARIZE=1 -s EXPORT_NAME="createEngine" -O3 -s WASM=1
copy engine.js frontend\public\
copy engine.wasm frontend\public\
echo DONE
