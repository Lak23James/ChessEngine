// public/engine.worker.js
importScripts('/engine.js'); // Load the Emscripten glue code

let engineInstance = null;

// Initialize the WASM module
createEngine().then((instance) => {
    engineInstance = instance;
    postMessage({ type: 'ready' }); // Tell React the engine is booted
});

self.onmessage = function(e) {
    if (e.data.type === 'calculate' && engineInstance) {
        const fen = e.data.fen;
        
        // Call our C++ function: const char* get_random_legal_move(char* fen)
        const move = engineInstance.ccall(
            'get_random_legal_move', // C++ function name
            'string',                // Return type
            ['string'],              // Argument types
            [fen]                    // Arguments
        );
        
        // Send the move back to React (e.g. "e2e4")
        postMessage({ type: 'move', move: move });
    }
};
