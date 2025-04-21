#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

const int INF = 9999;

void printDVRTable(int node, const vector<vector<int>>& table, const vector<vector<int>>& nextHop) {
    cout << "Node " << node << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < table.size(); ++i) {
        cout << i << "\t" << table[node][i] << "\t";
        if (nextHop[node][i] == -1) cout << "-";
        else cout << nextHop[node][i];
        cout << endl;
    }
    cout << endl;
}

void simulateDVR(const vector<vector<int>>& graph) {
    int n = graph.size();

    // 1) Initialize distance and next‑hop tables

    vector<vector<int>> dist = graph;
    vector<vector<int>> nextHop(n, vector<int>(n, -1));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && graph[i][j] < INF) {
                nextHop[i][j] = j;
            }
        }
    }

    // 2) Print initial tables

    cout << "--- Initial DVR Tables ---\n";
    for (int i = 0; i < n; ++i) {
        printDVRTable(i, dist, nextHop);
    }

        // 3) Bellman-Ford algorithm: Relax edges repeatedly until no improvements

        bool updated = true;
        int iteration = 0;
        while (updated) {
            updated = false;
            ++iteration;
    
        // Core DVR logic: Check all possible paths through neighbors

        for (int i = 0; i < n; ++i) { 

            for (int j = 0; j < n; ++j) {  

                if (i==j || dist[i][j] == 0) 
                    continue;            // Skip self-route

                for (int k = 0; k < n; ++k) {   

                    if (k == i || graph[i][k] == INF)
                         continue;       // Invalid neighbor

                    if (dist[k][j] == INF)
                         continue;       // Neighbor has no route to dest   

                    // Bellman-Ford relaxation: i → k → j vs current i → j

                    int newCost = graph[i][k] + dist[k][j];

                    if (newCost < dist[i][j]) {
                        dist[i][j] = newCost;

                        // Next hop becomes first hop towards neighbor k

                        nextHop[i][j] = (k == j) ? j : nextHop[i][k];
                        updated = true;
                    }
                }
            }
        }

        // Only print if something changed this round

        if (updated) {
            cout << "--- DVR Iteration " << iteration << " ---\n";
            for (int i = 0; i < n; ++i) {
                printDVRTable(i, dist, nextHop);
            }
        }
    }

    // 4) Print final converged tables

    cout << "--- Final DVR Tables ---\n";
    for (int i = 0; i < n; ++i) {
        printDVRTable(i, dist, nextHop);
    }
}


void printLSRTable(int src, const vector<int>& dist, const vector<int>& prev) {

    cout << "Node " << src << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";

    for (int i = 0; i < dist.size(); ++i) {
        
        if (i == src) continue;
        cout << i << "\t" << dist[i] << "\t";
        int hop = i;

        // Walk back from destination until we reach a neighbor of src

        while (prev[hop] != src && prev[hop] != -1) {

            hop = prev[hop];    // Backtrack to find first hop from source

        }

        // If prev[hop] == -1, destination unreachable

        cout << (prev[hop] == -1 ? -1 : hop) << endl;
    }
    cout << endl;
}

void simulateLSR(const vector<vector<int>>& graph) {
    int n = graph.size();

    // Run Dijkstra’s algorithm from each node as the source
    for (int src = 0; src < n; ++src) {

        vector<int> dist(n, INF);
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);

        // Distance from src to itself is zero
        dist[src] = 0;

        // Min-heap (priority queue) storing pairs (distance, node)
        priority_queue<
            pair<int, int>,
            vector<pair<int, int>>,
            greater<pair<int, int>>
        > pq;
        pq.push({0, src});

        while (!pq.empty()) {
            // Extract node u with the smallest tentative distance
            int u = pq.top().second;
            pq.pop();

            // If we've already visited u, skip it
            if (visited[u]) continue;
            visited[u] = true;

            // Relax all outgoing edges (u → v)
            for (int v = 0; v < n; ++v) {
                if (!visited[v] && graph[u][v] < INF) {
                    int alt = dist[u] + graph[u][v];
                    // If going through u gives a shorter path to v, update
                    if (alt < dist[v]) {
                        dist[v] = alt;
                        prev[v] = u;
                        pq.push({alt, v});
                    }
                }
            }
        }

        // After Dijkstra finishes for this src, print the routing table
        printLSRTable(src, dist, prev);
    }
}



vector<vector<int>> readGraphFromFile(const string& filename) {

    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }
    
    int n;

    if (!(file >> n) || n <= 0) {
        cerr << "Error: Invalid node count in " << filename << endl;
        exit(1);
    }

    vector<vector<int>> graph(n, vector<int>(n));

    for (int i = 0; i < n; ++i) {

        for (int j = 0; j < n; ++j) {

            int cost;

            if (!(file >> cost)) {
                cerr << "Error: Invalid or missing cost for edge "
                     << i << "->" << j << " in " << filename << endl;
                exit(1);
            }

            // Off‐diagonal zeros mean “no link”

            if (i != j && cost == 0) {
                graph[i][j] = INF;
            } else {
                graph[i][j] = cost;
            }
        }
    }
    file.close();

    // Guarantee self‐distance is zero

    for (int i = 0; i < n; ++i) {
        graph[i][i] = 0;
    }

    return graph;
}


bool hasNegativeEdges(const vector<vector<int>>& graph) {

    // Critical validation: DVR/LSR algorithms require non-negative weights

    for (int i = 0; i < graph.size(); ++i) {
      for (int j = 0; j < graph.size(); ++j) {
        if (i != j && graph[i][j] < 0 && graph[i][j] != INF)
          return true;
      }
    }
    return false;
}
  

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    string filename = argv[1];

    vector<vector<int>> graph = readGraphFromFile(filename);

    // Fail on negative weights - violates algorithm assumptions

    if (hasNegativeEdges(graph)) {
        cerr << "ERROR : negative edge cost detected; all link metrics must be non‑negative.\n";
        return 1;
    }
      
    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);

    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);

    return 0;
}
