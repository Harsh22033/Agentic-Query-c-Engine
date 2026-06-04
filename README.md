# NaturalSQL 

> **Ask your data anything. In plain English.**
> A high-performance SQL query engine built from scratch in C++17, powered by a Google Gemini 1.5 Pro agentic layer that translates natural language into multi-step queries — and explains the answers back to you.

```
$ python agent.py

NaturalSQL — Gemini-powered query engine
=========================================

Ask a question: Which product category had the highest revenue growth last month vs 3 months ago?

  Calling: get_schema({})
   Result: {tables: ["orders", "products"], columns: [...]}

  Calling: execute_query({"sql": "SELECT category, SUM(revenue) FROM orders WHERE month = '2026-05' GROUP BY category"})
   Result: [("Electronics", 48200), ("Clothing", 31400), ...]

  Calling: execute_query({"sql": "SELECT category, SUM(revenue) FROM orders WHERE month = '2026-02' GROUP BY category"})
   Result: [("Electronics", 29100), ("Clothing", 33800), ...]

 Answer: Electronics had the highest growth at +65.6% (₹29,100 → ₹48,200).
           Clothing actually declined by 7.1% over the same period.
```

---

## What makes this different

Most "natural language to SQL" demos wrap an LLM around an existing database. This project builds everything from scratch:

- **The parser** — a recursive-descent parser that tokenizes raw SQL and builds an AST, zero libraries
- **The storage engine** — column-store layout (not row-store) for fast analytical queries
- **The algorithms** — hash join O(N+M), external merge sort, B+ tree indexing
- **The memory layer** — custom arena allocator for AST nodes, no `new/delete` scatter
- **The agent** — a ReAct loop using Gemini 1.5 Pro's function calling, with your C++ engine as the tool
- **The bridge** — pybind11 connecting C++ engine to Python agent with zero-copy data transfer

The AI layer is only as fast as the tool it calls. So the focus was on making the C++ engine bulletproof first.

---

## Architecture

```
┌──────────────────────────────────────────────────┐
│              User (natural language)              │
└─────────────────────┬────────────────────────────┘
                      │
┌─────────────────────▼────────────────────────────┐
│         Gemini 1.5 Pro Agent (Python)             │
│  • Calls get_schema() to understand the data      │
│  • Plans multi-step query strategy                │
│  • Generates SQL, executes, observes, iterates    │
│  • Explains final answer in plain English         │
└──────────────┬───────────────────┬───────────────┘
               │ tool_call         │ tool_call
               │ execute_query()   │ get_schema()
┌──────────────▼───────────────────▼───────────────┐
│         pybind11 Bridge (zero-copy)               │
└──────────────────────┬───────────────────────────┘
                       │
┌──────────────────────▼───────────────────────────┐
│         NaturalSQL C++17 Engine                   │
│                                                   │
│  Lexer → Parser → AST → Executor                 │
│                                                   │
│  ┌─────────────┐  ┌──────────────┐               │
│  │ Column Store│  │  B+ Tree     │               │
│  │ (analytical)│  │  Index       │               │
│  └─────────────┘  └──────────────┘               │
│  ┌─────────────┐  ┌──────────────┐               │
│  │ Hash Join   │  │ Arena        │               │
│  │ O(N+M)      │  │ Allocator    │               │
│  └─────────────┘  └──────────────┘               │
└───────────────────────────────────────────────────┘
                       │
┌──────────────────────▼───────────────────────────┐
│              CSV Data Files                       │
└───────────────────────────────────────────────────┘
```

---

## Benchmarks

Tested on NYC Taxi dataset (~100K rows, yellow_tripdata_2024.csv).

### B+ Tree index vs full scan

| Query type | No index | B+ Tree index | Speedup |
|---|---|---|---|
| Point lookup (fare = 12.50) | 38.2 ms | 2.9 ms | **13.2×** |
| Range scan (fare between 10–20) | 41.7 ms | 4.8 ms | **8.7×** |
| Equality + ORDER BY | 54.1 ms | 6.3 ms | **8.6×** |

### Hash join vs nested loop join

| Table sizes | Nested loop | Hash join | Speedup |
|---|---|---|---|
| 10K × 5K rows | 2,140 ms | 18 ms | **118×** |
| 50K × 10K rows | timeout | 91 ms | — |

### Agent query resolution

| Question complexity | Queries executed | Total time |
|---|---|---|
| Single fact lookup | 2 (schema + 1 query) | 1.1s |
| Comparison (A vs B) | 3 queries | 1.8s |
| Multi-step analytical | 5–6 queries | 3.2s |

---

## Supported SQL syntax

```sql
-- Projection and filtering
SELECT name, age, salary FROM employees WHERE age > 25

-- Sorting and limiting
SELECT * FROM orders ORDER BY total_amount DESC LIMIT 10

-- Aggregation
SELECT department, AVG(salary) FROM employees GROUP BY department

-- Joins
SELECT o.order_id, p.product_name
FROM orders o JOIN products p ON o.product_id = p.id

-- Index creation
CREATE INDEX ON orders(fare_amount)
```

---

## Project structure

```
NaturalSQL/
├── src/
│   ├── parser/
│   │   ├── Lexer.h / Lexer.cpp       # Tokenizer — string → token list
│   │   ├── Parser.h / Parser.cpp     # Recursive descent → AST
│   │   └── AST.h                     # Node definitions (std::variant)
│   ├── storage/
│   │   ├── ColumnStore.h/cpp         # Column-oriented storage
│   │   ├── CSVLoader.h/cpp           # CSV → ColumnStore
│   │   └── BPlusTree.h/cpp           # B+ tree index
│   ├── engine/
│   │   ├── Executor.h/cpp            # AST walker + query execution
│   │   ├── HashJoin.h/cpp            # Hash join implementation
│   │   ├── MergeSort.h/cpp           # External merge sort
│   │   └── ArenaAllocator.h          # Custom memory allocator
│   └── main.cpp                      # CLI entry point
├── python/
│   ├── agent.py                      # Gemini ReAct agent loop
│   ├── bridge.py                     # pybind11 module loader
│   └── tools.py                      # Tool definitions for Gemini
├── tests/
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   ├── test_executor.cpp
│   └── test_btree.cpp
├── data/
│   └── yellow_tripdata_sample.csv    # Sample dataset
├── CMakeLists.txt
├── requirements.txt
├── .env.example
└── README.md
```

---

## Getting started

### Prerequisites

- C++17 compiler (GCC 9+ or Clang 10+)
- CMake 3.15+
- Python 3.9+
- A free Google AI Studio API key → [aistudio.google.com](https://aistudio.google.com)

### 1. Clone and build the C++ engine

```bash
git clone https://github.com/harsh-iitgoa/NaturalSQL
cd NaturalSQL

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

### 2. Install Python dependencies

```bash
pip install google-generativeai pybind11 python-dotenv rich
```

### 3. Set your API key

```bash
cp .env.example .env
# Edit .env and add your Gemini API key:
# GEMINI_API_KEY=your_key_here
```

### 4. Run the agent

```bash
python python/agent.py --data data/yellow_tripdata_sample.csv
```

### 5. Or use the SQL CLI directly (no AI)

```bash
./build/naturalsql --data data/yellow_tripdata_sample.csv
> SELECT passenger_count, AVG(fare_amount) FROM trips GROUP BY passenger_count ORDER BY passenger_count
```

---

## Design decisions

### Why column-store instead of row-store?

Most databases store rows together: `[id, name, age, salary, ...]`. For analytical queries like `SELECT AVG(salary) FROM employees`, you load all columns even though you only need one. Column-store keeps each column contiguous in memory — the CPU cache only warms up the data you actually need. On 100K rows, this alone gives a measurable speedup on aggregation queries.

### Why B+ tree over a hash index?

Hash indexes are O(1) for equality lookups but useless for range queries (`WHERE age BETWEEN 20 AND 30`). B+ trees give O(log n) for both point and range lookups, and all data lives in the leaf nodes which makes range scans a simple linked-list traversal. For analytical workloads with range predicates, B+ tree is almost always the right call.

### Why hash join over nested loop?

Nested loop join is O(N×M) — for a 50K × 10K join, that's 500 million comparisons. Hash join builds a hash table on the smaller relation (O(M)) then probes it for each row of the larger relation (O(N)), giving O(N+M) total. At 50K × 10K, hash join finishes in 91ms vs timeout for nested loop.

### Why Gemini 1.5 Pro specifically?

Gemini 1.5 Pro has a 1 million token context window. For multi-step analytical queries, the agent needs to hold the full conversation history, schema, intermediate results, and reasoning chain in context simultaneously. With smaller context windows, long query chains cause the agent to "forget" earlier results and repeat work. The 1M window eliminates this entirely.

### Why arena allocator for AST nodes?

During query parsing, hundreds of AST nodes are allocated and then freed all at once when the query finishes executing. Using `new/delete` for each node causes heap fragmentation and individual deallocation overhead. An arena allocator pre-allocates a single large block, bumps a pointer for each node (essentially free), then frees the entire block in one call. For a query with 200 AST nodes, this is 200× fewer heap operations.

---

## Key C++ concepts demonstrated

| Concept | Where |
|---|---|
| `std::variant` for typed AST nodes | `AST.h` |
| `std::unique_ptr` for tree ownership | `Parser.cpp` |
| `std::string_view` for zero-copy lexing | `Lexer.cpp` |
| Custom arena allocator | `ArenaAllocator.h` |
| Template classes | `ColumnStore.h`, `BPlusTree.h` |
| Move semantics | `ColumnStore.cpp` |
| Operator overloading | Iterator classes |
| RAII | All resource management |
| `std::chrono` benchmarking | `benchmark.cpp` |
| pybind11 C++/Python bridge | `python/bridge.py` |

---

## Running tests

```bash
cd build
ctest --output-on-failure

# Or individual test suites:
./tests/test_lexer
./tests/test_parser
./tests/test_executor
./tests/test_btree
```

---

## What I'd build next

- **Query planner** — reorder WHERE predicates by selectivity before execution (predicate pushdown)
- **Aggregation pushdown** — push GROUP BY into the storage layer to avoid materializing full result sets
- **Write support** — INSERT, UPDATE, DELETE with WAL (write-ahead log) for crash recovery
- **Parallel execution** — partition columns across threads for multi-core aggregation
- **Persistent storage** — serialize the column store to a binary format instead of re-loading CSV on each run

---

## Skills demonstrated

`C++17` `Recursive descent parsing` `Abstract Syntax Trees` `Column-store databases` `B+ Tree indexing` `Hash join` `External merge sort` `Arena allocators` `pybind11` `Agentic AI` `ReAct loops` `Google Gemini API` `Function calling` `Python` `CMake`

---

## Author

**Harsh** — B.Tech Mathematics and Computing, IIT Goa
[harsh.22033@iitgoa.ac.in](mailto:harsh.22033@iitgoa.ac.in) · [LinkedIn](https://www.linkedin.com/in/harsh) · [GitHub](https://github.com/harsh-iitgoa)

---

> *"The AI layer is only as good as the tool it calls."*
