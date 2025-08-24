#!/usr/bin/env python3

from flask import Flask, render_template, request, jsonify
import subprocess
import threading
import queue
import os

# Get the absolute path of the directory containing server.py
_root_dir = os.path.dirname(os.path.abspath(__file__))
_project_root = os.path.dirname(_root_dir)

app = Flask(__name__,
            static_folder=os.path.join(_root_dir, 'static'),
            template_folder=os.path.join(_root_dir, 'templates'))

# --- Chess Engine Communication ---
engine_process = None
engine_queue = queue.Queue()

def uci_reader(pipe, q):
    while True:
        line = pipe.readline().strip()
        if line:
            q.put(line)

def start_engine():
    global engine_process
    engine_path = os.path.join(_project_root, 'chessai')
    engine_process = subprocess.Popen(
        [engine_path],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
        universal_newlines=True
    )
    # Start a thread to read engine's stdout without blocking
    threading.Thread(target=uci_reader, args=(engine_process.stdout, engine_queue), daemon=True).start()
    # Initialize engine to UCI mode
    send_uci_command("uci")

def send_uci_command(command):
    if engine_process:
        print(f"Sending to engine: {command}", flush=True)
        engine_process.stdin.write(command + '\n')
        engine_process.stdin.flush()

def get_engine_response(stop_token, timeout=60):
    lines = []
    while True:
        try:
            line = engine_queue.get(timeout=timeout) # Use the provided timeout
            print(f"Received from engine: {line}", flush=True)
            lines.append(line)
            if stop_token in line:
                return lines
        except queue.Empty:
            return ["Engine timed out"]

# --- Flask Routes ---
@app.route('/')
def index():
    return render_template('index.html')

@app.route('/new_game', methods=['POST'])
def new_game():
    send_uci_command("ucinewgame")
    send_uci_command("isready")
    get_engine_response("readyok")
    return jsonify({'status': 'ok'})

@app.route('/make_move', methods=['POST'])
def make_move():
    fen = request.json['fen']
    depth = request.json.get('depth', 5) # Get depth, default to 5
    timeout = int(request.json.get('timeout', 60)) # Get timeout, default to 60

    # Send position to engine
    send_uci_command(f"position fen {fen}")
    
    # Make sure engine is ready
    send_uci_command("isready")
    get_engine_response("readyok", timeout=timeout)

    # Ask engine to find best move
    send_uci_command(f"go depth {depth}")
    response_lines = get_engine_response("bestmove", timeout=timeout)
    
    best_move_str = ""
    for line in response_lines:
        if line.startswith("bestmove"):
            best_move_str = line.split(' ')[1]
            break
            
    if best_move_str:
        # Convert the move string (e.g., "e2e4", "e7e8q") into a move object
        from_sq = best_move_str[0:2]
        to_sq = best_move_str[2:4]
        promotion = best_move_str[4] if len(best_move_str) == 5 else None
        
        move_obj = {'from': from_sq, 'to': to_sq}
        if promotion:
            move_obj['promotion'] = promotion

        return jsonify({'best_move': move_obj})
    else:
        print(f"Engine failed to return bestmove. Full response: {response_lines}")
        return jsonify({'error': 'Engine did not return a move', 'details': response_lines}), 500

@app.route('/load_fen', methods=['POST'])
def load_fen():
    fen = request.json['fen']
    send_uci_command(f"position fen {fen}")
    send_uci_command("isready")
    get_engine_response("readyok")
    return jsonify({'status': 'ok'})

if __name__ == '__main__':
    start_engine()
    app.run(debug=True, port=3000)
