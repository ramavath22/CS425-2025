#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

const int INF = 9999;

void printDVRTable(int node, const vector<vector<int>>& dist, const vector<vector<int>>& nextHop) {
    cout << "Node " << node << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < dist.size(); ++i) {
        cout << i << "\t";
        if (dist[node][i] >= INF) cout << "INF\t";
        else               cout << dist[node][i] << "\t";
        if (i == node || nextHop[node][i] == -1) cout << "-\n";
        else                                     cout << nextHop[node][i] << "\n";
    }
    cout << endl;
}

void simulateDVR(const vector<vector<int>>& graph) {
    int n = graph.size();
    // dist[i][j]: current best cost from i to j
    vector<vector<int>> dist = graph;
    // nextHop[i][j]: the first hop on the best path from i to j
    vector<vector<int>> nextHop(n, vector<int>(n, -1));

    // --- Initialization ---
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && graph[i][j] < INF) {
                nextHop[i][j] = j;
            }
        }
    }

    // --- Iterative exchange until convergence ---
    bool updated;
    int iteration = 0;
    do {
        updated = false;
        cout << "*** DVR Iteration " << ++iteration << " ***\n";
        for (int i = 0; i < n; ++i) {
            printDVRTable(i, dist, nextHop);
        }

        // for each node i, consider each neighbor k
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < n; ++k) {
                if (k == i || graph[i][k] >= INF) continue;  // not a neighbor
                // cost to k is graph[i][k], and k knows dist[k][*]
                for (int j = 0; j < n; ++j) {
                    int viaK = (dist[k][j] >= INF ? INF : graph[i][k] + dist[k][j]);
                    if (viaK < dist[i][j]) {
                        dist[i][j] = viaK;
                        nextHop[i][j] = (k == j ? j : nextHop[i][k]);
                        updated = true;
                    }
                }
            }
        }
    } while (updated);

    cout << "--- DVR Final Tables ---\n";
    for (int i = 0; i < n; ++i) {
        printDVRTable(i, dist, nextHop);
    }
}


void printLSRTable(int src, const vector<int>& dist, const vector<int>& prev) {
    cout << "Node " << src << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int dst = 0; dst < dist.size(); ++dst) {
        if (dst == src) continue;
        cout << dst << "\t";
        if (dist[dst] >= INF) cout << "INF\t";
        else                  cout << dist[dst] << "\t";
        // backtrack to find first hop
        int hop = dst;
        if (prev[hop] == -1) {
            cout << "-\n";
        } else {
            while (prev[hop] != src) {
                hop = prev[hop];
            }
            cout << hop << "\n";
        }
    }
    cout << endl;
}

void simulateLSR(const vector<vector<int>>& graph) {
    int n = graph.size();
    for (int src = 0; src < n; ++src) {
        vector<int> dist(n, INF), prev(n, -1);
        vector<bool> visited(n, false);
        dist[src] = 0;

        // Standard Dijkstra's algorithm
        for (int iter = 0; iter < n; ++iter) {
            // pick the unvisited node u with smallest dist[u]
            int u = -1, best = INF+1;
            for (int i = 0; i < n; ++i) {
                if (!visited[i] && dist[i] < best) {
                    best = dist[i];
                    u = i;
                }
            }
            if (u == -1) break;  // remaining are unreachable
            visited[u] = true;

            // relax edges (u → v)
            for (int v = 0; v < n; ++v) {
                if (graph[u][v] < INF && !visited[v]) {
                    int alt = dist[u] + graph[u][v];
                    if (alt < dist[v]) {
                        dist[v] = alt;
                        prev[v] = u;
                    }
                }
            }
        }

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
    file >> n;
    vector<vector<int>> graph(n, vector<int>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> graph[i][j];

    file.close();
    return graph;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    string filename = argv[1];
    vector<vector<int>> graph = readGraphFromFile(filename);

    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);

    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);

    return 0;
}

