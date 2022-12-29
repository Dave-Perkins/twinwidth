/*

To do:
- Include a check for all degrees being zero, then collapse them in any order

Suggested fixes:
- I'm initializing the degree sequence using push_back instead of reserve / emplace_back
- I'm only using an edge list, not an adjacency list
- Every time we collapse an edge, I'm remaking the degree sequence
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream> // to use stringstream() for tokenizing the input file's strings
#include <algorithm> // to access max_element()
#include <climits> // to access INT_MAX
using namespace std;

// Stores an edge by naming its incident vertices, and keeps track of if the edge is red:
struct Edge {
    int node1;
    int node2;
    bool red;
};

// Stores the two vertices that the algorithm chooses to collapse, with a found flag for 
// convenience while we are collapsing pairs of leaf nodes at the beginning of the algorithm:
struct Pair {
    int vertex1;
    int vertex2;
    bool found = false;
};

// =============== Overloads for << operator =============== //

ostream & operator<<(ostream &output, Edge edge) {
    // Adding 1 because PACE starts numbering at 1:
    output << edge.node1 + 1 << " " << edge.node2 + 1;
    return output;
}

ostream & operator<<(ostream &output, Pair pair) {
    output << pair.vertex1 + 1 << " " << pair.vertex2 + 1 << endl;
    return output;
}

ostream & operator<<(ostream &output, vector<Edge> &edges) {
    output << "\n=== Printing the edges === \n";
    for (Edge e : edges) {
        output << e.node1 + 1 << " " << e.node2 + 1;
        output << (e.red ? " red " : "") << endl;
    }
    return output;
}

ostream & operator<<(ostream &output, vector<int> &vec) {
    for (int v : vec) {
        output << " " << v + 1;
    }
    output << endl;
    return output;
}

ostream & operator<<(ostream &output, vector<Pair> &collapse_sequence) {
    // Adding 1 because PACE guys started numbering vertices at 1:
    for (Pair p : collapse_sequence) {
        output << p.vertex1 + 1 << " " << p.vertex2 + 1 << endl;
    }
    return output;
}

// ========================================================= //

// Tokenize a string (used for grab_data() only):
void tokenize(string const &str, vector<string> &tokens) {
    cout << "tokenizing.." << endl;
    tokens.clear();
    stringstream ss(str);
    string s;
    char delimiter = ' ';
    while (getline(ss, s, delimiter)) {
        tokens.push_back(s);
    }
}

// Grabs data from the input file:
void grab_data(string file_name, vector<Edge> &edges, vector<int> &deg_sequence, int &num_nodes) {
    cout << "grabbing data.." << endl;
    string line;
    vector<string> tokens;
    ifstream myfile("tiny_graphs/" + file_name + ".gr");
    if (myfile.is_open()) {
        while (getline(myfile, line)) {
            tokenize(line, tokens);
            if (tokens[0] == "p") { // then this is line 1 of input
                cout << "num_nodes and num_edges read from file: ";
                num_nodes = stoi(tokens[2]);
                int num_edges = 0;
                num_edges = stoi(tokens[3]);
                cout << num_nodes << " " << num_edges << endl;
            } else { // else this line of input is an edge
                cout << "edge read from file: ";
                for (string s : tokens) {
                    cout << s << " ";
                }
                cout << endl;
                Edge e;
                e.node1 = stoi(tokens[0]);
                e.node1--; // because the PACE guys start numbering nodes at 0
                cout << e.node1;
                e.node2 = stoi(tokens[1]);
                e.node2--; // because the PACE guys start numbering nodes at 0
                cout << " " << e.node2;
                e.red = false;
                cout << " " << e.red << endl;
                edges.push_back(e);
                // deg_sequence.at(e.node1)++;
                // deg_sequence.at(e.node2)++;
            }
        }
    cout << "closing the file.." << endl;
    myfile.close();
    }

    // No longer using this cin approach to reading the input file:
    // // Read number of edges and number of nodes:
    // string p;
    // cin >> p; // just reading "p" from input file (then discarding)
    // string tww;
    // cin >> tww; // just reading "tww" from input file (then discarding)
    // cin >> num_nodes; 
    // // cout << "num_nodes: " << num_nodes << endl;
    // int num_edges = 0;
    // cin >> num_edges;
    // // cout << "num_edges: " << num_edges << endl;

    // Initialize the degree sequence vector:
    // for (int i = 0; i < num_nodes; i++) {
    //     deg_sequence.push_back(0);
    // }

    // cout << "deg_sequence: " << deg_sequence << endl;
    
    // Read the edges from the input file:
    // for (int index = 0; index < num_edges; index++) {
    //     Edge e;
    //     cin >> e.node1;
    //     e.node1--; // because the PACE guys start numbering nodes at 0
    //     cin >> e.node2;
    //     e.node2--; // because the PACE guys start numbering nodes at 0
    //     e.red = false;
    //     edges.push_back(e);
    //     deg_sequence.at(e.node1)++;
    //     deg_sequence.at(e.node2)++;
    // }
}

int get_vertex_max_degree(vector<int> &deg_sequence) {
    auto max_elem = max_element(deg_sequence.begin(), deg_sequence.end());
    return distance(deg_sequence.begin(), max_elem);
}

// Returns true if vertex1 and vertex2 are neighbors:
bool are_neighbors(vector<Edge> &edges, int vertex1, int vertex2) {
    for (vector<Edge>::iterator it = edges.begin(); it != edges.end(); ++it) {
        if (it->node1 == vertex1 && it->node2 == vertex2
            || it->node2 == vertex1 && it->node1 == vertex2)
        {
            return true;
        }
    }
    return false;
}

// Returns true if vertex is NOT found in the given list of (a different vertex's) neighbors:
bool not_a_neighbor(int vertex, vector<int> neighbors) {
    for (int i = 0; i < neighbors.size(); i++) {
        if (vertex == neighbors[i]) {
            return false;
        }
    }
    return true;
}

// Returns the number of vertices neighboring vertex1 that do NOT neighbor vertex2:
int count_non_neighbors(vector<Edge> &edges, int vertex1, int vertex2) {
    // Get the neighbors of vertex1 and the neighbors of vertex2:
    vector<int> neighbors_v1;
    vector<int> neighbors_v2;
    neighbors_v1.push_back(vertex1);
    neighbors_v2.push_back(vertex2);
    for (vector<Edge>::iterator it = edges.begin(); it != edges.end(); ++it) {
        if (it->node1 == vertex1) {
            neighbors_v1.push_back(it->node2);
        } else if (it->node2 == vertex1) {
            neighbors_v1.push_back(it->node1);
        }
        if (it->node1 == vertex2) {
            neighbors_v2.push_back(it->node2);
        } else if (it->node2 == vertex2) {
            neighbors_v2.push_back(it->node1);
        }
    }
    int count = 0;
    // Count the neighbors of vertex1 that are NOT neighbors of vertex2:
    for (int nbr : neighbors_v1) {
        count += not_a_neighbor(nbr, neighbors_v2);
    }
    return count;
}

// Finds the correct pair of vertices to collapse:
Pair get_pair_to_collapse(vector<Edge> &edges, int vertex) {
    // Setup:
    int min_reds_created = INT_MAX;
    Pair desired_pair; // This will store the desired pair of vertices 
    
    // Get the neighbors of the given vertex:
    vector<int> neighbors;
    for (vector<Edge>::iterator it = edges.begin(); it != edges.end(); ++it) {
        if (it->node1 == vertex) {
            neighbors.push_back(it->node2);
        } else if (it->node2 == vertex) {
            neighbors.push_back(it->node1);
        }
    }
    
    int num_nbrs = neighbors.size();
    if (num_nbrs == 1) {
        return desired_pair;
    }
    // Consider each pair of neighbors of the given vertex:
    for (int i = 0; i < num_nbrs - 1; i++) {
        for (int j = i + 1; j < num_nbrs; j++) {
            int nbr1 = neighbors[i];
            int nbr2 = neighbors[j];
            int num_non_neighbors = count_non_neighbors(edges, nbr1, nbr2) +
                                    count_non_neighbors(edges, nbr2, nbr1);
            if (num_non_neighbors < min_reds_created) {
                min_reds_created = num_non_neighbors;
                desired_pair.vertex1 = nbr1;
                desired_pair.vertex2 = nbr2;
            }
        }
    }
    return desired_pair;
}

// Returns true if the given vertex is incident to the given edge:
bool vertex_in_edge(Edge &edge, int vertex) {
    return (vertex == edge.node1 || vertex == edge.node2);
}

// Erases all edges in which node1 == node2:
void erase_loops(vector<Edge> &edges) {
    vector<Edge>::iterator it = edges.begin();
    while (it != edges.end()) {
        if (it->node1 == it->node2) {
            it = edges.erase(it);
        } else {
            ++it;
        }
    }
}

// Overloads != so we can compare edges:
bool operator!=(Edge e1, Edge e2) {
    return (e1.node1 != e2.node1 || e1.node2 != e2.node2);
}

// Get the black-degree of the given node:
// int get_black_degree(vector<Edge> &edges, int node) {
//     int black_deg = 0;
//     for (Edge &e : edges) {
//         if (e.red && (e.node1 == node || e.node2 == node)) {
//             black_deg++;
//         }
//     }
//     return black_deg;
// }

// Erases any duplicate edge that is "before" the edge at it_current, returning true if one was erased:
bool erase_a_duplicate(vector<Edge> &edges, vector<Edge>::iterator it_current) {
    vector<Edge>::iterator it = edges.begin();
    while (it != it_current) {
        if ((it->node1 == it_current->node2 && it->node2 == it_current->node1) ||
            (it->node1 == it_current->node1 && it->node2 == it_current->node2)) 
        {
            // If we are erasing a duplicate that is red, set the current edge to red:
            if (it->red) {
                it_current->red = true;
            }
            it = edges.erase(it);
            return true;
        } else {
            ++it;
        }
    }
    return false;
}

/*
pseudocode for collapsing vertices X, Y:
1. loop thru edges, looking for XZ, and if YZ not also an edge, color XZ red
2. loop thru edges, replacing Ys with Xs
3. if entries now equal, remove that edge
*/

// Updates the edge list so it reflects the collapse of the given pair:
void update_edges(vector<Edge> &edges, Pair &pair_to_collapse) {
    int v1 = pair_to_collapse.vertex1;
    int v2 = pair_to_collapse.vertex2;
    // Loop thru edges, looking for v1-z edges, and if v2-z not also an edge, color v1-z red:
    for (vector<Edge>::iterator it = edges.begin(); it != edges.end(); ++it) {
        if (it->node1 == v1 && !are_neighbors(edges, it->node2, v2)) {
            it->red = true;
        } else if (it->node2 == v1 && !are_neighbors(edges, it->node1, v2)) {
            it->red = true;
        }
    }
    // Loop through edges, replacing all v2 with v1:
    for (vector<Edge>::iterator it = edges.begin(); it != edges.end(); ++it) {
        bool found = false;
        if (it->node1 == v2) {
            it->node1 = v1;
            found = true;
        } else if (it->node2 == v2) {
            it->node2 = v1;
            found = true;
        }
        // If this creates a duplicate edge, erase it (which shifts the iterator back a spot):
        bool erased = erase_a_duplicate(edges, it);
        if (erased) {
            --it;
        } else if (found) {
            it->red = true;
        }
    }
    // If an edge has two equal entries, erase it:
    erase_loops(edges);
}

bool all_edges_are_red(vector<Edge> &edges) {
    for (Edge &e : edges) {
        if (!e.red) {
            return false;
        }
    }
    return true;
}

// Updates the degree sequence (counting only black edges):
void update_deg_sequence(vector<Edge> &edges, vector<int> &deg_sequence, int num_nodes) {
    // Initialize the degree sequence vector
    for (int i = 0; i < num_nodes; i++) {
        deg_sequence.at(i) = 0;
    }

    bool all_edges_red = true;
    
    for (Edge e : edges) {
        if (!e.red) {
            all_edges_red = false;
            deg_sequence.at(e.node1)++;
            deg_sequence.at(e.node2)++;
        }
    }
}

// Updates the degree sequence (using both red and black edges):
void update_deg_sequence_all(vector<Edge> &edges, vector<int> &deg_sequence, int num_nodes) {
    // Initialize the degree sequence vector
    for (int i = 0; i < num_nodes; i++) {
        deg_sequence.at(i) = 0;
    }

    for (Edge e : edges) {
        deg_sequence.at(e.node1)++;
        deg_sequence.at(e.node2)++;
    }
}

// Finds the first edge containing the given vertex, and returns its neighbor:
int get_neighbor(vector <Edge> &edges, int vertex) {
    for (Edge e : edges) {
        // cout << "checking edge " << e << " and vertex " << vertex + 1 << endl;
        if (e.node1 == vertex) {
            // cout << "returning " << e.node2 + 1 << endl;
            return e.node2;
        } else if (e.node2 == vertex) {
            // cout << "returning " << e.node1 + 1 << endl;
            return e.node1;
        }
    }
    cout << "error!" << endl;
    return -1;  // error
}

/* pseudocode for collapse:
collapse all pairs of leaf nodes that share a neighbor
loop until G has collapsed to a single vertex:
  if any two leafs have the same neighbor in common:
    collapse those two leafs
  else
    find any vertex w of highest degree
    for each pair (u, v) among w's neighbors:
      let m(u, v) be the number of red edges introduced by collapsing (u, v)
    collapse the pair (u, v) that has the smallest m(u, v)
*/

// Return true if the given leaf nodes share a common neighbor:
bool have_common_neighbor(vector<Edge> &edges, int leaf1, int leaf2) {
    int nbr1 = 0;
    int nbr2 = 0;
    nbr1 = get_neighbor(edges, leaf1);
    nbr2 = get_neighbor(edges, leaf2);
    return (nbr1 == nbr2);
}

// Returns true if the graph contains any leaf nodes:
int num_leaf_nodes(vector<int> &deg_sequence) {
    int count = 0;
    for (int i = 0; i < deg_sequence.size(); i++) {
        if (deg_sequence[i] == 1) {
            count++;
        }
    }
    return count;
}

// Returns a pair of leaf nodes that share a common neighbor, if one exists:
Pair get_pair_of_leafs(vector<Edge> &edges, vector<int> &deg_sequence) {
    Pair leaf_pair{-1, -1, false};
    // First, get all of the leaf nodes:
    vector<int> leaf_nodes;
    for (int i = 0; i < deg_sequence.size(); i++) {
        if (deg_sequence[i] == 1) {
            leaf_nodes.push_back(i);
        }
    }
    // cout << "leaf_nodes: " << leaf_nodes << endl;
    // Now search for a pair of leaf nodes that share a common neighbor:
    for (int i = 0; i < leaf_nodes.size() - 1; i++) {
        for (int j = i + 1; j < leaf_nodes.size(); j++) {
            if (have_common_neighbor(edges, leaf_nodes.at(i), leaf_nodes.at(j))) {
                leaf_pair.vertex1 = leaf_nodes.at(i);
                leaf_pair.vertex2 = leaf_nodes.at(j);
                leaf_pair.found = true;
                return leaf_pair;
            }
        }
    }
    return leaf_pair;
}

void update_twinwidth(vector<Edge> edges, int &twinwidth, int num_nodes) {
    vector<int> twinwidths(num_nodes, 0);
    for (Edge e : edges) {
        if (e.red) {
            twinwidths.at(e.node1)++;
            twinwidths.at(e.node2)++;
        }
    }
    int max_twinwidth = *max_element(twinwidths.begin(), twinwidths.end());
    if (max_twinwidth > twinwidth) {
        twinwidth = max_twinwidth;
    }
}

// An initial phase collapses all pairs of leaf nodes that share a common neighbor:
void leaf_collapse_phase(vector<Edge> &edges, vector<int> &deg_sequence, vector<Pair> &collapse_sequence, int &num_nodes, int &twinwidth) {
    cout << ">>> leaf collapse phase!" << endl;
    while (true) {
        // First, check if there are any leaf nodes:
        update_deg_sequence(edges, deg_sequence, num_nodes);
        // cout << "deg_sequence: " << deg_sequence << endl;
        if (num_leaf_nodes(deg_sequence) < 2) {
            return;
        }
        Pair pair_to_collapse;
        pair_to_collapse = get_pair_of_leafs(edges, deg_sequence);
        if (pair_to_collapse.found) {
            // cout << "collapsing pair " << pair_to_collapse << endl;
            collapse_sequence.push_back(pair_to_collapse);
            update_edges(edges, pair_to_collapse);
            // cout << edges << endl;
            update_twinwidth(edges, twinwidth, num_nodes);
        } else {
            return;
        }
    }
}

void collapse(vector<Edge> &edges, vector<int> &deg_sequence, vector<Pair> &collapse_sequence, int num_nodes) {
    
    // Find a vertex with maximum black-degree:
    update_deg_sequence(edges, deg_sequence, num_nodes);
    // cout << "black-deg sequence: ";
    // for (int i : deg_sequence) {
    //     cout << i << " ";
    // }
    // cout << endl;
    int vertex_max_degree = get_vertex_max_degree(deg_sequence);

    // If that vertex is a leaf, then get a vertex with maximum degree:
    if (deg_sequence.at(vertex_max_degree) == 1 || all_edges_are_red(edges)) {
        update_deg_sequence_all(edges, deg_sequence, num_nodes);
        // cout << "all-deg sequence: ";
        // for (int i : deg_sequence) {
        //     cout << i << " ";
        // }
        // cout << endl;
        vertex_max_degree = get_vertex_max_degree(deg_sequence);
    }

    // Auto-find the pair to collapse:
    Pair pair_to_collapse = get_pair_to_collapse(edges, vertex_max_degree);
    collapse_sequence.push_back(pair_to_collapse);
    
    // Manually find the pair to collapse (for debugging):
    // Pair pair_to_collapse;
    // pair_to_collapse.vertex1 = 2;
    // pair_to_collapse.vertex2 = 5;
        
    update_edges(edges, pair_to_collapse);
}

int main(int argc,  char **argv) {
    // Read the filename from the command line:
    string file_name(argv[1]);

    // Grab the graph data from the input file:
    vector<Edge> edges;
    vector<int> deg_sequence;
    vector<Pair> collapse_sequence;
    int num_nodes = 0;
    grab_data(file_name, edges, deg_sequence, num_nodes);

    // cout << "num_nodes is: " << num_nodes << endl;
    // cout << "setting up the degree sequence.." << endl;
    // Initialize and update the degree sequence vector:
    for (int i = 0; i < num_nodes; i++) {
        deg_sequence.push_back(0);
    }
    // cout << "original deg_sequence: ";
    // for (int i : deg_sequence) {
    //     cout << i << " " << endl;
    // }
    update_deg_sequence(edges, deg_sequence, num_nodes);

    int twinwidth = 0;

    // First we collapse together all pairs of leaf nodes that share a common neighbor:
    leaf_collapse_phase(edges, deg_sequence, collapse_sequence, num_nodes, twinwidth);
    
    
    // Collapse!
    // cout << ">>> collapse phase!" << endl;
    while(edges.size() > 1) {
        // update_deg_sequence(edges, deg_sequence, num_nodes); 
        // cout << "deg_sequence: " << deg_sequence << endl;
        collapse(edges, deg_sequence, collapse_sequence, num_nodes);
        update_twinwidth(edges, twinwidth, num_nodes);
        // cout << edges << endl;
    }

    Pair final_edge;
    final_edge.vertex1 = edges[0].node1;
    final_edge.vertex2 = edges[0].node2;
    collapse_sequence.push_back(final_edge);
    
    cout << collapse_sequence << endl;
    cout << "twinwidth: " << twinwidth << endl;

    cout << "collapse_seqs/" + file_name + ".txt" << endl;
    ofstream myfile("collapse_seqs/" + file_name + ".txt");
    for (Pair p : collapse_sequence) {
        myfile << p.vertex1 + 1 << " " << p.vertex2 + 1 << endl;
    }
    myfile.close();
    
    return 0;
}