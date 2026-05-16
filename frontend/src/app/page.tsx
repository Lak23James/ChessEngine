'use client';

import { useState, useEffect, useRef } from 'react';
import { Chessboard } from 'react-chessboard';
import { Chess } from 'chess.js';

export default function PlayVsEngine() {
  const [game, setGame] = useState(new Chess());
  const [engineReady, setEngineReady] = useState(false);
  const workerRef = useRef<Worker | null>(null);

  useEffect(() => {
    // 1. Boot up the Web Worker
    workerRef.current = new Worker('/engine.worker.js');

    workerRef.current.onmessage = (e) => {
      if (e.data.type === 'ready') {
        setEngineReady(true);
      } 
      // 2. Listen for the Engine's response
      else if (e.data.type === 'move') {
        const moveString = e.data.move; // e.g. "e2e4"
        
        if (moveString && moveString !== "none") {
          const newGame = new Chess(game.fen());
          try {
            // Apply engine's move to our frontend board
            newGame.move({
              from: moveString.substring(0, 2),
              to: moveString.substring(2, 4),
              promotion: moveString.length > 4 ? moveString[4] : undefined
            });
            setGame(newGame);
          } catch (err) {
            console.error("Engine generated an illegal move!", moveString);
          }
        } else {
            console.log("Game Over or Engine error.");
        }
      }
    };

    return () => {
      if (workerRef.current) workerRef.current.terminate();
    };
  }, [game]);

  // 3. Handle Human Moves (White)
  function onDrop(sourceSquare: string, targetSquare: string, piece: string) {
    if (!engineReady || game.turn() !== 'w') return false; // Wait for engine

    const newGame = new Chess(game.fen());
    
    // Auto-queen promotions for simplicity
    const isPromotion = piece[1] === 'P' && targetSquare[1] === '8';

    try {
      const move = newGame.move({
        from: sourceSquare,
        to: targetSquare,
        promotion: isPromotion ? 'q' : undefined 
      });

      if (move === null) return false;
      
      setGame(newGame);

      // 4. Send the new Board State (FEN) to the C++ Engine Worker
      if (!newGame.isGameOver()) {
        workerRef.current?.postMessage({ 
            type: 'calculate', 
            fen: newGame.fen() 
        });
      }

      return true;
    } catch (e) {
      return false; // Invalid move
    }
  }

  return (
    <div className="flex flex-col items-center justify-center min-h-screen bg-gray-900 text-white font-sans">
      <h1 className="text-3xl font-bold mb-4">Play vs C++ Engine</h1>
      
      {!engineReady && (
        <p className="text-yellow-400 mb-4 animate-pulse">
            Booting WebAssembly Engine...
        </p>
      )}

      <div className="w-[500px] shadow-2xl rounded-sm overflow-hidden border-4 border-gray-700">
        <Chessboard 
          position={game.fen()} 
          onPieceDrop={onDrop} 
          boardOrientation="white"
          customDarkSquareStyle={{ backgroundColor: '#779556' }}
          customLightSquareStyle={{ backgroundColor: '#ebecd0' }}
        />
      </div>
    </div>
  );
}
