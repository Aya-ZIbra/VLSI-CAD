#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
using namespace std;

template <typename T>
ostream& printc(ostream& os, const vector<vector<T>>& C){
    for (auto v: C){
        for(T item : v){
            os << item << "  ";
        }
        os << endl;
    }
    return os;
}

template<>
ostream& printc(ostream& os, const vector<vector<pair<int,double>>>& C){
        for (auto v: C){
        for(auto item : v){
            os << "("<< item.first << " , " <<  item.second << ")";
        }
        os << endl;
    }
    return os;
}


class pad{
    public:
        int padId;
        pair<double, double> location; // x,y coordinates of pin
};
class gate{
    public:
        int gateId;
        pair<double, double> location; // x,y coordinates of gate

};

bool compare_gate_x(gate* g1, gate* g2){
    // g1 before g2
    if (g1->location.first < g2->location.first) {return 1;}
    // Order by y for x ties
    else if (g1->location.first == g2->location.first && g1->location.second< g2->location.second) {return 1;}
    else{
    // g1 after g2
    return 0;
    }
}

class graph{
    public:
        // number of gates (vertices), number of nets, number of pads 
        int G =0; 
        int N= 0; 
        int P = 0;

        // adj list
        vector<vector<pair<int, double>>> connectedGates; // one indexed
        vector<vector<pair<int, double>>> connectedPads;
        
        // gate vector
        vector<gate> gates;
        // pad vector
        vector<pad> pads;

        void read_graph(const string& fname);

    private:
        bool search_edge(int& index, const vector<pair<int,double>>& v, int g);


};

bool graph::search_edge(int& index, const vector<pair<int,double>>& v, int g){
    bool found = false;
    index = 0;
    for (auto gpair : v){ // search if this edge exists
        if (g == gpair.first){
            found = true;
            break;
        }
        index++;
    }
    return found;
}
void graph::read_graph(const string& fname){
    ifstream fs {fname};
    if (!fs) {
        cerr << "cannot open file " << fname << endl;
        exit(1);
    }

    string line;
    if (std::getline(fs,line)){
        istringstream iss(line);
        iss >> G >> N;
    }

    gates.resize(G);
    connectedGates.resize(G+1);
    connectedPads.resize(G+1);


    /* Before creating the graph, we need to determine the number of gates and pad connected to each net 
     to calculate the weights of the connecting edges
     Example:
        net 10 : G1 G3 G5 P1 ==> k = 4 ==> w = 1/(k-1)
     
     For the pads connected to each nets, the netlist syntax allows each pad to be connected 
     to only one net, which is practical, so we can use a hash table for looking up that 
     single pad connected to a net.
     
     Example:
        net 10 : P1 
     */
    vector<vector<int>> netGates(N+1);  // one-indexed Nets vector
    unordered_map<int,pad> netPads; // net,pad
    
    // Parse Gates
    for (int i= 0; i<G; i++){
        std::getline(fs, line);
        // create stream from line 
        istringstream iss(line);
        // parse the line
        int j =0;
        int temp, gateId, netCount;
        while (iss >> temp){
            
            if (j==0){
                gateId = temp;
                gates[i].gateId = gateId;
            }
            else if (j== 1){
                netCount = temp; // number of nets connected to this gate
            }else{
                int net = temp;
                netGates[net].push_back(gateId);
            }
            //cout << temp << "here" << j << endl;
            j++;
        } 
    }
    printc(cout, netGates);

    // Parse the pads 
    if (std::getline(fs,line)){
        istringstream iss(line);
        iss >> P;
    }
    pads.resize(P);
    
   
    cout << "Pin Count = " << P <<endl;
    for (int i=0; i<P; i++ ){
        std::getline(fs,line);
        istringstream iss(line);
        pad p;
        int net;
        while(iss >> p.padId >> net >> p.location.first >> p.location.second){  
            netPads[net] = p;
            pads[i] = p;
        }
    }
    
    // Construct the graph 
    /* 
    The vertices are the gates (and probably the pads) 
    The edges are the weights for each connection between 2 pairs of connected gates. 
    

    Adjacency list representation for this graph for two reasons:
    I) the graph is sparse 
    II) Accessing the connected gates is O(deg(V)) compared to O(V) in case of Adj matrix

    Example: Adj list:
        gateId  connected (gateId, weight) list
            v   u,4 s,5 t,3 
    
    You can have multiple edges between the same 2 gates when the two gates are connected via different nets.
    In this case, the weights of these edges are summed. So, we need a way to check if such edge already exists
    in the graph or not. If yes, the new weight should be added. If not, then we just push the new edge to the
    graph. 

    For this purpose, multiple implementations with different trade-offs exist: 
    n = #vertix count , m = #edge count
    1_ Adj matrix initialized to zeros | space O(n^2) | conversion to Adj list O(n^2) runtime

    2_ Adj list with local search at each vertex vector before pushing new edge. | space O(m) 
    | O(m* deg(v)) the vertex is visited once for each edge and its neighbors are searched each visit 
    to check before adding the edge.

    2'_ same as 2 but with Adj list implemented as vector of hash table map<end vertex, weight>

    3_ Edge list implemented as hash table | space O(m) | conversion to Adj list O(m) runtime
    Most optimized but requires hash specialization for the type pair<int,int> used to represent the edges
    
    Once you know the gate Id of a connected gate, you should be able to access its x,y location 
    Use Vector for int Ids and hash tables otherwise.
    
    Example:
        gate[u] returns an object of class gate

    */

   // construct adj list from netGates and netPads
   for (int net = 1; net < N+1; net++){
       // calculate weight
       int sz = netGates[net].size();
       int k;
       if(netPads.count(net)){
           k = sz +1;
       }else{
           k = sz;
       }
       double w = ((double)1) / (k-1);

       // add weights to gate-to-gate edges
       for (int i=0; i<sz; i++){
            int gi = netGates[net][i];
            for(int j= i+1; j< sz; j++){
                int gj = netGates[net][j];
                int index = 0;
                bool found = search_edge(index, connectedGates[gi], gj);
            
                if (found == true){
                    connectedGates[gi][index].second += w;
                    found = search_edge(index, connectedGates[gj], gi);
                    if (found == false) { cerr<< "Asymmetry in gate connectivity found" << endl; exit(2);}
                    connectedGates[gj][index].second += w;
                }
           
                if (found == false){
                    connectedGates[gi].push_back({gj, w});
                    connectedGates[gj].push_back({gi,w});
                }
                
            }   
        } 

        // add weight to pad-to-gate edges
        
        if(netPads.count(net)){
            pad p = netPads[net];
            for (auto g : netGates[net]){
                
                int index = 0;
                bool found = search_edge(index, connectedPads[g], p.padId);
                if (found == true){
                    connectedPads[g][index].second += w;
                } else{
                    connectedPads[g].push_back({p.padId, w});
                }
            }  
       }   
   }

   
   printc(cout, connectedGates);
   printc(cout, connectedPads);
}