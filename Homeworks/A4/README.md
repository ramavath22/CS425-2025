# **CS425 Assignment 4: Routing Protocols**

## Team Members

| Team members | Roll no. |
| ------------ | :------: |
| L Sree Sahithi    |  220577  |
| Ramavath Dinesh Naik        |  220866  |
| Gowthan Chand   |  220313  |

#  Routing Protocol Simulator

##  Overview

This program simulates two fundamental network routing algorithms:

- **Distance Vector Routing (DVR)**
- **Link State Routing (LSR)**

Given an adjacency matrix as input (representing node-to-node costs), the program computes and prints routing tables for each node using both algorithms.



##  How to Compile 

 **Important**: Ensure `routing_sim.cpp` is not already running or open in another process to avoid "Permission Denied" errors.

### Compiling


```bash
make
```

## How to Run the Code
Once compiled, run the executable with your input file:

```bash
./routing_sim input.txt
```
### **Code Flow**
```plaintext
1. Start program execution via main().
2. Read input graph from input file.
3. Validate input to ensure all weights are non-negative.
4. Run Distance Vector Routing (DVR):
   a. Initialize distance and next-hop tables.
   b. Iteratively update using Bellman-Ford style relaxation.
   c. Print routing tables after each update.
5. Run Link State Routing (LSR):
   a. For each node, run Dijkstra’s algorithm using min-heap.
   b. Record distances and predecessors.
   c. Reconstruct next hops using backtracking.
   d. Print final routing tables.
```
```
   main()
    │
    ├── readGraphFromFile(filename)     # Parse adjacency matrix
    │   └── Replace 0 (off-diagonal) with INF, validate format
    │
    ├── hasNegativeEdges(graph)         # Safety check for input
    │   └── Return error if negative edge weight found
    │
    ├── simulateDVR(graph)              # Run Distance Vector Routing
    │   ├── Initialize dist[][] and nextHop[][]
    │   ├── Print initial tables
    │   └── Repeat until convergence:
    │       ├── For each i, j, k → check i → k → j
    │       ├── If better path found, update dist & nextHop
    │       └── Print table if changes occurred
    │
    └── simulateLSR(graph)              # Run Link State Routing
        └── For each source node src:
            ├── Dijkstra using priority_queue
            │   ├── Track dist[], prev[], visited[]
            │   ├── Relax edges from current node
            │   └── Update queue with better distances
            └── printLSRTable(src, dist, prev)  # Reconstruct paths

```
##  Assignment Objective

The goal of this assignment is to implement and simulate two fundamental network routing algorithms — **Distance Vector Routing (DVR)** and **Link State Routing (LSR)** — in C++.

The simulation should:
- Accept an adjacency matrix as input (representing network topology).
- Compute routing tables for all nodes using both DVR and LSR algorithms.
- Print routing tables:
  - For **DVR**: after each iteration and at convergence.
  - For **LSR**: after running Dijkstra’s algorithm from each node.

## Implementation Details

The project is implemented in C++ in a modular fashion, with separate functions for handling Distance Vector Routing (DVR), Link State Routing (LSR), and file parsing. Below is a breakdown of each major function:

---

###  `simulateDVR(const vector<vector<int>>& graph)`

This function implements the **Distance Vector Routing (DVR)** algorithm using a Bellman-Ford–style approach.

- Initializes each node’s routing table with direct neighbor costs.
- Maintains a `dist[][]` matrix for current shortest path estimates and a `nextHop[][]` matrix for routing paths.
- Iteratively updates the tables:
  - For each node `i`, it checks all possible intermediate neighbors `k` to reach destination `j`.
  - If `cost(i→k) + cost(k→j)` is better than the current `cost(i→j)`, it updates the distance and sets `nextHop[i][j]` to the appropriate neighbor.
- Tracks whether updates occurred in each iteration using a `bool updated` flag.
- Stops when all routing tables converge (no updates needed).
- **Output**:
  - Initial routing tables.
  - Tables after each iteration (only if updates occurred).
  - Final converged routing tables.

---

###  `printDVRTable(int node, const vector<vector<int>>& table, const vector<vector<int>>& nextHop)`

- Helper function to print the routing table of a specific node.
- Displays: `Destination`, `Cost`, and `Next Hop` for each reachable node.

---

###  `simulateLSR(const vector<vector<int>>& graph)`

This function implements the **Link State Routing (LSR)** algorithm using an optimized **Dijkstra’s algorithm with a min-heap (priority queue)**.

- For each node `src`:
  - Initializes:
    - `dist[]`: Holds shortest known distances from `src` to all nodes.
    - `prev[]`: Holds the previous node in the shortest path for reconstruction.
    - `visited[]`: Marks nodes whose shortest distance is finalized.
  - A **min-heap priority queue** is used to efficiently select the unvisited node with the smallest tentative distance.
  - For each extracted node `u`, all its neighbors `v` are relaxed:
    - If the new path `src → u → v` is better, `dist[v]` and `prev[v]` are updated.
    - The neighbor is pushed into the priority queue with the new distance.
- Once Dijkstra’s completes, the `prev[]` array is used to reconstruct the **next hop** for each destination.
- **Output**:
  - A routing table for each node showing:
    - Destination
    - Shortest path cost
    - Next hop

---

###  `printLSRTable(int src, const vector<int>& dist, const vector<int>& prev)`

- Helper function to print the LSR routing table for a given source node.
- Backtracks from each destination using the `prev[]` array to determine the correct **next hop** on the shortest path from the source.

---

###  `readGraphFromFile(const string& filename)`

- Reads the adjacency matrix from an input file.
- Replaces off-diagonal `0` values with `INF` (9999) to indicate no direct connection.
- Ensures that the self-cost (diagonal entries) is always `0`.
- Returns a 2D matrix representing the network graph.

---

### `hasNegativeEdges(const vector<vector<int>>& graph)`

- Validates the input graph for negative edge weights.
- DVR and LSR both assume non-negative costs; this function ensures those constraints are upheld.

###  `main(int argc, char *argv[])`

- Entry point of the program.
- Expects the path to the input file as a command-line argument.
- Calls `readGraphFromFile()` to load the adjacency matrix.
- Runs both simulations in sequence:
  1. Distance Vector Routing via `simulateDVR()`
  2. Link State Routing via `simulateLSR()`
-  INF = 9999 is a large constant used to represent "no link" between two nodes.
---

## Function Call Flow Diagram

The following diagram shows the function call flow in the updated program:

```
             ┌───────────────┐
             │    main()     │
             └──────┬────────┘
                    │
        ┌───────────▼────────────┐
        │  readGraphFromFile()   │
        └───────────┬────────────┘
                    │
        ┌───────────▼────────────┐
        │ hasNegativeEdges()     │
        └──────┬─────────────────┘
               │
   ┌───────────▼──────────────┐
   │                          │
   ▼                          ▼
simulateDVR(graph)     simulateLSR(graph)
   │                          │
   ▼                          ▼
printDVRTable()         Dijkstra’s Algorithm
                              │
                              ▼
                      printLSRTable()



```



## Assumptions
- It is assumed that all edge weights are non-negative.
- The value `INF = 9999` is used in the program to represent infinity, indicating the absence of a direct connection between two nodes.



##  Testing

We have tested the functionality of `routing_sim.cpp` using the sample inputs provided in the assignment PDF. The output generated by our implementation was compared with the expected output format and results, and it matched exactly, confirming the correctness of our simulation.


**Input Format (input.txt):**

- The first line contains a single integer n — the number of nodes.

- The next n lines each contain n space-separated integers representing the adjacency matrix of the network.
- Input Rules
  - The diagonal entries (i.e., distance to self) must be 0.

  - If there is no direct link between nodes i and j, the cost should be 0 in the file, and the program will internally treat it as INF = 9999.

  - Costs must be symmetric (if i→j has a cost x, then j→i must also be x) for undirected graphs.




**Example Input-1:**
  ```
 4
0 10 100 30
10 0 20 40
100 20 0 10
30 40 10 0

  ```



- ## Our Output:

```
--- Distance Vector Routing Simulation ---
--- Initial DVR Tables ---
Node 0 Routing Table:
Dest    Cost    Next Hop
0       0       -
1       10      1
2       100     2
3       30      3

Node 1 Routing Table:
Dest    Cost    Next Hop
0       10      0
1       0       -
2       20      2
3       40      3

Node 2 Routing Table:
Dest    Cost    Next Hop
0       100     0
1       20      1
2       0       -
3       10      3

Node 3 Routing Table:
Dest    Cost    Next Hop
0       30      0
1       40      1
2       10      2
3       0       -

--- DVR Iteration 1 ---
Node 0 Routing Table:
Dest    Cost    Next Hop
0       0       -
1       10      1
2       30      1
3       30      3

Node 1 Routing Table:
Dest    Cost    Next Hop
0       10      0
1       0       -
2       20      2
3       30      2

Node 2 Routing Table:
Dest    Cost    Next Hop
0       30      1
1       20      1
2       0       -
3       10      3

Node 3 Routing Table:
Dest    Cost    Next Hop
0       30      0
1       30      2
2       10      2
3       0       -

--- Final DVR Tables ---
Node 0 Routing Table:
Dest    Cost    Next Hop
0       0       -
1       10      1
2       30      1
3       30      3

Node 1 Routing Table:
Dest    Cost    Next Hop
0       10      0
1       0       -
2       20      2
3       30      2

Node 2 Routing Table:
Dest    Cost    Next Hop
0       30      1
1       20      1
2       0       -
3       10      3

Node 3 Routing Table:
Dest    Cost    Next Hop
0       30      0
1       30      2
2       10      2
3       0       -


--- Link State Routing Simulation ---
Node 0 Routing Table:
Dest    Cost    Next Hop
1       10      1
2       30      1
3       30      3

Node 1 Routing Table:
Dest    Cost    Next Hop
0       10      0
2       20      2
3       30      2

Node 2 Routing Table:
Dest    Cost    Next Hop
0       30      1
1       20      1
3       10      3

Node 3 Routing Table:
Dest    Cost    Next Hop
0       30      0
1       30      2
2       10      2

```
---

 **Example Input-2:**
  ```
4
0 10 -100 30
10 0 20 -40
100 20 0 10
30 40 -10 0

  ```
- ## Output:
```
   ERROR : negative edge cost detected; all link metrics must be non‑negative.
```

## Challenges 

- Started with a simple O(n²) Dijkstra (linear scan) then optimized to O(m + n log n) using a priority queue.  
- The heap version introduced stale‑entry and cost‑comparison bugs that broke both shortest‑path values and next‑hop reconstruction.  
- Fixed these by enforcing strict visited‑node checks, INF guards, and propagating the correct first hop on each relaxation.  


---
## Contribution of Team Members

| Team Member              | Contribution (%) | Work Done                                                    |
| :----------------------- | :--------------: | :----------------------------------------------------------- |
| Sree Sahithi <br/> (220577) |      33.33%         | Implemented the `simulateDVR()` function,Debugging, README
| Ramavath Dinesh Naik <br/> (220866)     |      33.34%         | Implemented the `simulateLSR()` function using Dijkstra’s algorithm, Debugging, Inline Comments       |
| Gowtham Chand <br/> (220313) |      33.33%         | Verified the correctness of both DVR and LSR implementations, README           |

## Sources Used

- [Dijkstra's algorithm using priority queue](https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-using-priority_queue-stl/)  
- [Bellman-Ford algorithm](https://www.w3schools.com/dsa/dsa_algo_graphs_bellmanford.php/)  
- [Regarding ambiguity of zero cost for 'no cost or no link'](https://piazza.com/class/m5h01uph1h12eb/post/178)





## Declaration

We, (**Sree Sahithi, Dinesh Naik and Gowtham chand**) declare that this assignment was completed independently without plagiarism. Any external sources used are appropriately referenced.
