1. clone this repo
```bash
git clone https://github.com/carotinoid/study-chess-engine
```

2. build
```bash
make
make test # ALL TESTS PASSED! expected
```

3. run engine
```bash
./chessai
```

4. enjoy this with web
```bash
python3 -m venv .venv
. ./.venv/bin/activate
pip install flask
./web/server.py
```