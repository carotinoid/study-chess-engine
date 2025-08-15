$(document).ready(function() {
    var board = null;
    var game = new Chess();
    var $status = $('#status');
    var $fen = $('#fen');
    var playerColor = 'w'; // Default: player is White

    function onDragStart(source, piece, position, orientation) {
        // do not pick up pieces if the game is over
        if (game.game_over()) return false;

        // only pick up pieces for the side to move
        if (game.turn() !== playerColor) {
            return false;
        }
    }

    function onDrop(source, target) {
        // see if the move is legal
        var move = game.move({
            from: source,
            to: target,
            promotion: 'q' // NOTE: always promote to a queen for simplicity
        });

        // illegal move
        if (move === null) return 'snapback';

        updateStatus();

        // If it's now the engine's turn, get its move
        if (game.turn() !== playerColor) {
            window.setTimeout(getEngineMove, 250); // Add a small delay for UI update
        }
    }

    // update the board position after the piece snap 
    // for castling, en passant, pawn promotion
    function onSnapEnd() {
        board.position(game.fen());
    }

    function updateStatus() {
        var status = '';

        var moveColor = (game.turn() === 'w') ? 'White' : 'Black';

        // checkmate?
        if (game.in_checkmate()) {
            status = 'Game over, ' + moveColor + ' is in checkmate.';
        } else if (game.in_draw()) {
            status = 'Game over, drawn position';
        } else {
            status = moveColor + ' to move';
            if (game.in_check()) {
                status += ', ' + moveColor + ' is in check';
            }
        }

        $status.html(status);
        $fen.html(game.fen());
    }

    function getEngineMove() {
        // Do nothing if it's the player's turn
        if (game.turn() === playerColor) return;

        var depth = $('#depth').val();
        $('#engine-status').html('Thinking...');

        // make AJAX call to server for engine's move
        $.ajax({
            url: 'make_move',
            method: 'POST',
            contentType: 'application/json',
            data: JSON.stringify({ fen: game.fen(), depth: depth }),
            success: function(response) {
                if (response.best_move) {
                    // The server now returns a move object like { from: 'e2', to: 'e4' }
                    game.move(response.best_move);
                    board.position(game.fen());
                    updateStatus();
                    $('#engine-status').html('Ready');
                } else {
                    $('#engine-status').html('Error: No valid move received.');
                    alert("Error: Engine did not return a valid move.");
                    console.error("Received from server:", response);
                }
            },
            error: function(jqXHR, textStatus, errorThrown) {
                $('#engine-status').html('Error: Communication failed.');
                console.error("AJAX Error:", textStatus, errorThrown);
                alert("An error occurred while communicating with the engine.");
            }
        });
    }

    function newGame() {
        playerColor = $('input[name="playerColor"]:checked').val();
        game.reset();
        board.start();
        updateStatus();

        if (game.turn() !== playerColor) {
            window.setTimeout(getEngineMove, 250);
        }
    }

    function loadFen() {
        var fen = $('#fenInput').val();
        if (!game.load(fen)) {
            alert("Invalid FEN");
            return;
        }
        board.position(fen);
        updateStatus();
        $.ajax({
            url: 'load_fen',
            method: 'POST',
            contentType: 'application/json',
            data: JSON.stringify({ fen: fen }),
            success: function(response) {
                console.log("FEN loaded on server");
                // After loading FEN, check if it's engine's turn
                if (game.turn() !== playerColor) {
                    window.setTimeout(getEngineMove, 250);
                }
            }
        });
    }

    var config = {
        draggable: true,
        position: 'start',
        pieceTheme: 'static/img/chesspieces/wikipedia/{piece}.png',
        onDragStart: onDragStart,
        onDrop: onDrop,
        onSnapEnd: onSnapEnd
    };
    board = Chessboard('board', config);

    updateStatus();

    $('#newGameBtn').on('click', newGame);
    $('input[name="playerColor"]').on('change', newGame); // Start a new game when color changes
    $('#loadFenBtn').on('click', loadFen);
});
