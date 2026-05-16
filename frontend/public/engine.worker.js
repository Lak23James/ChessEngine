// public/engine.worker.js
importScripts('/engine.js'); // Load the Emscripten glue code

let engineInstance = null;

console.log("Worker: Starting engine initialization...");

// Initialize the WASM module
createEngine({
  locateFile: function(path) {
    if (path.endsWith('.wasm')) {
      return '/' + path;
    }
    return path;
  }
}).then((instance) => {
    console.log("Worker: Engine initialized successfully!");
    engineInstance = instance;
    postMessage({ type: 'ready' }); // Tell React the engine is booted
}).catch(err => {
    console.error("Worker: Engine initialization failed", err);
    postMessage({ type: 'error', message: err ? err.toString() : "Unknown error" });
});

self.onmessage = function(e) {
    if (e.data.type === 'calculate' && engineInstance) {
        try {
            const fen = e.data.fen;
            const move = engineInstance.ccall(
                'get_random_legal_move',
                'string',
                ['string'],
                [fen]
            );
            postMessage({ type: 'move', move: move });
        } catch (err) {
            console.error("Worker: Calculation failed", err);
            postMessage({ type: 'error', message: err.toString() });
        }
    }
};
