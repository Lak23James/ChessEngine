'use client';

import { useEffect, useRef, useState } from 'react';
import { Chessboard } from 'react-chessboard';
import { Chess } from 'chess.js';

export default function Home() {
  const game = useRef(new Chess());
  const worker = useRef<Worker | null>(null);
  const from = useRef<string | null>(null);

  const [fen, setFen] = useState('start');
  const [ready, setReady] = useState(false);
  const [msg, setMsg] = useState('Booting engine...');

  useEffect(() => {
    const w = new Worker('/engine.worker.js');
    worker.current = w;

    w.onmessage = (e) => {
      if (e.data.type === 'ready') {
        setReady(true);
        setMsg('Your move (White)');
      }
      if (e.data.type === 'move') {
        const mv: string = e.data.move;
        if (mv && mv !== 'none') {
          game.current.move({ from: mv.slice(0, 2), to: mv.slice(2, 4), promotion: 'q' });
          setFen(game.current.fen());
          setMsg('Your move (White)');
        }
      }
    };

    return () => w.terminate();
  }, []);

  function makeMove(src: string, dst: string): boolean {
    try {
      const result = game.current.move({ from: src, to: dst, promotion: 'q' });
      if (!result) return false;
      setFen(game.current.fen());
      setMsg('Engine thinking...');
      worker.current?.postMessage({ type: 'calculate', fen: game.current.fen() });
      return true;
    } catch {
      return false;
    }
  }

  function onSquareClick(square: string) {
    if (!ready || game.current.turn() !== 'w') return;

    if (!from.current) {
      // select piece
      const piece = game.current.get(square as any);
      if (piece && piece.color === 'w') {
        from.current = square;
        setMsg(`Selected ${square} — now click destination`);
      }
    } else {
      // attempt move
      const success = makeMove(from.current, square);
      from.current = null;
      if (!success) setMsg('Invalid move — pick a piece');
    }
  }

  function onDrop(src: string, tgt: string): boolean {
    if (!ready) return false;
    return makeMove(src, tgt);
  }

  return (
    <div style={{
      minHeight: '100vh',
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center',
      justifyContent: 'center',
      background: '#1a1a2e',
      color: 'white',
      fontFamily: 'sans-serif',
      gap: '12px',
      padding: '16px',
      boxSizing: 'border-box',
    }}>
      <h2 style={{ margin: 0 }}>Play vs C++ Engine</h2>

      <p style={{ margin: 0, color: ready ? '#4ade80' : '#facc15', fontSize: 14 }}>
        {ready ? '✅ Engine ready' : '⏳ Loading...'}
      </p>

      <div style={{ width: 400, flexShrink: 0 }}>
        <Chessboard
          id="board"
          position={fen}
          boardWidth={400}
          onPieceDrop={onDrop}
          onSquareClick={onSquareClick as any}
        />
      </div>

      <p style={{ margin: 0, color: '#aaa', fontSize: 13 }}>{msg}</p>
    </div>
  );
}
