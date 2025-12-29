# Counter machine

An implementation of a minimal [counter machine](https://en.wikipedia.org/wiki/Counter_machine).

Inspired by the [video](https://www.youtube.com/watch?v=PXN7jTNGQIw) from Computerphile.

[Demo](https://ov7a.github.io/counter-machine/)

## How to use

This machine has two instructions:
1. `INC a`, which increments counter number `a`. Written as `+2`.
2. `ZDEC a i`, which decrements counter number `a` if it's nonzero, and jumps to instruction `i` otherwise. Written as `-2?5`.

Note that counters are numbered from 0, while instructions start at 1.

## Building

To build a regular binary:

```bash
gcc main.c -Wall --std=c2x -o main
```

To build a WASM:
```bash
zig cc -target wasm32-wasi-musl main.c -Wall --std=c2x -o main.wasm
```
