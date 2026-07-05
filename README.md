# raft-kv

A distributed, replicated key-value store written from scratch in modern C++,
using the [Raft consensus algorithm](https://raft.github.io/) for fault
tolerance. The RPC layer is built directly on POSIX sockets; there are no
heavyweight runtime dependencies.

[![CI](https://github.com/lfylow/raft-kv/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/lfylow/raft-kv/actions/workflows/ci.yml)

> **Status:** work in progress. Phase 0 (single-node KV over a framed TCP
> protocol) is complete and tested; Raft consensus is being built up phase by
> phase — see the roadmap below.

## What this is

`raft-kv` is a networked key-value store in which several nodes replicate the
same data and stay consistent even when nodes crash or the network splits. The
consensus layer is a hand-written implementation of Raft. The store is the
replicated state machine: committed log entries are applied to it in the same
order on every node.

## Architecture

The Raft core is deliberately decoupled from the network. It talks to peers
through a `Transport` interface: production uses a real TCP transport, while
tests use an in-memory simulated network that can **drop, delay, and partition**
messages deterministically. That is how correctness is verified without needing
real distributed hardware — and it is why the whole cluster can run (and be
demoed) as N processes on a single machine or GitHub Codespace.

## Building

Requires a C++17 compiler and CMake 3.16+.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Try it (Phase 0)

```bash
# terminal 1 — start a node on port 5000
./build/raftkv_server 5000

# terminal 2 — talk to it
./build/raftkv_client 127.0.0.1 5000 SET greeting "hello world"   # -> OK
./build/raftkv_client 127.0.0.1 5000 GET greeting                 # -> VALUE hello world
./build/raftkv_client 127.0.0.1 5000 DEL greeting                 # -> OK
```

## Roadmap

- [x] **Phase 0** — single-node in-memory KV store over a framed TCP protocol
- [ ] **Phase 1** — Raft leader election (terms, RequestVote, randomized timeouts)
- [ ] **Phase 2** — log replication (AppendEntries, commit on majority)
- [ ] **Phase 3** — apply committed entries to the KV state machine
- [ ] **Phase 4** — fault-injection tests: partitions, dropped/delayed messages, linearizability
- [ ] **Phase 5** — snapshotting, log compaction, on-disk persistence & crash recovery

## Layout

```
include/raftkv/   public headers
  net/            socket + framed messaging
  kv/             replicated state machine (the key-value store)
  raft/           consensus: types, transport interface, RaftNode
src/              implementation + server/client entry points
  net/  kv/       (implemented)
  raft/           consensus implementation (Phase 1+)
tests/            GoogleTest unit tests (run in CI on every push)
```

## License

MIT
