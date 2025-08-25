## Benchmark Results after Move Ordering Improvement

### Raw Benchmark Output

```
info string Opening book loaded with 20692 positions.
FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
Best move: e2e3
Time taken: 1319.38 ms
---------------------------------
FEN: rnbqkbnr/ppp1p2p/3p2p1/4Pp2/8/2P5/PP1P1PPP/RNBQKBNR w KQkq f6 0 4
Best move: d1b3
Time taken: 17893.1 ms
---------------------------------
FEN: rnbqkbnr/pp1p1pp1/2p1p2p/8/3N4/2P5/PP1PPPPP/RNBQKB1R w KQkq - 0 4
Best move: e2e3
Time taken: 15265.8 ms
---------------------------------
FEN: 8/p1ppp2p/1p1k2r1/8/4B3/6P1/1N3PKP/8 w - - 0 1
Best move: b2c4
Time taken: 1012.88 ms
---------------------------------
FEN: 3k4/2pp4/4p3/8/Q2R2n1/2P3P1/3P1PKP/8 w - - 0 1
Best move: a4d7
Time taken: 1915.34 ms
---------------------------------
FEN: r1bqkbnr/pppppppp/8/8/3nP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 3
Best move: d1d4
Time taken: 6293.7 ms
---------------------------------
FEN: r1b1kb1r/ppppnppp/2n1pq2/8/4P3/1P3N2/P1PPBPPP/RNBQ1RK1 b kq - 0 5
Best move: f6a1
Time taken: 14199.2 ms
---------------------------------
FEN: r1bqkbnr/1ppp1ppp/p1n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 4
Best move: f3f7
Time taken: 20793.8 ms
---------------------------------
FEN: rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq a3 0 1
Best move: d7d5
Time taken: 2084.73 ms
---------------------------------
```

### Performance Comparison

| FEN | Before (ms) | After (ms) | Improvement |
| :--- | :--- | :--- | :--- |
| `rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1` | 2525.2 | 1319.38 | ~1.91x |
| `rnbqkbnr/ppp1p2p/3p2p1/4Pp2/8/2P5/PP1P1PPP/RNBQKBNR w KQkq f6 0 4` | 27189.4 | 17893.1 | ~1.52x |
| `rnbqkbnr/pp1p1pp1/2p1p2p/8/3N4/2P5/PP1PPPPP/RNBQKB1R w KQkq - 0 4` | 23703.4 | 15265.8 | ~1.55x |
| `8/p1ppp2p/1p1k2r1/8/4B3/6P1/1N3PKP/8 w - - 0 1` | 2006.8 | 1012.88 | ~1.98x |
| `3k4/2pp4/4p3/8/Q2R2n1/2P3P1/3P1PKP/8 w - - 0 1` | 4386.52 | 1915.34 | ~2.29x |
| `r1bqkbnr/pppppppp/8/8/3nP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 3` | 12817.4 | 6293.7 | ~2.04x |
| `r1b1kb1r/ppppnppp/2n1pq2/8/4P3/1P3N2/P1PPBPPP/RNBQ1RK1 b kq - 0 5` | 26394.7 | 14199.2 | ~1.86x |
| `r1bqkbnr/1ppp1ppp/p1n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 4` | 40695.5 | 20793.8 | ~1.96x |
| `rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq a3 0 1` | 3899.34 | 2084.73 | ~1.87x |