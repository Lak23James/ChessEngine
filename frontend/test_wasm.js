const factory = require('./public/engine.js');
factory().then(instance => {
  console.log("Engine loaded successfully");
  const fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  const move = instance.ccall(
    'get_random_legal_move',
    'string',
    ['string'],
    [fen]
  );
  console.log("Generated move:", move);
}).catch(err => {
  console.error("Engine failed to load:", err);
});
