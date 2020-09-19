#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <queue>
//#include "solver.h"
#include "graph.h"
#include "region.h"
using namespace std;

int main(){
    // open netlist file
    string fname = "toy1";
    graph netlist;
    netlist.read_graph(fname);

    for (pad p: netlist.pads){
        cout << p.padId << " " << p.location.first << " " << p.location.second <<endl;
    }
    vector<gate*> globalGates;
    for (int i=0; i<netlist.gates.size(); i++){
        globalGates.push_back(& netlist.gates[i]);
    }
      
    pair<double, double> minCorner = {0,0};
    pair<double, double> maxCorner = {100, 100};
    region globe(minCorner, maxCorner);
    globe.assign(globalGates);
    //globe.buildMatrix(netlist);
    //globe.solve();
    //globe.printGates();
    
    for(auto g : netlist.gates){
        cout << "gate " << g.gateId << " x, y = " << g.location.first << " ," << g.location.second << endl;
    }
    
    queue<region> q;
    q.push(globe);
    
    int iter_count = 0;
    while(!q.empty()){
        region x = q.front();
        q.pop();
        x.buildMatrix(netlist);
        x.solve();
        cout << "Itertion " << iter_count <<endl;
        x.printGates();
        iter_count++;
        if (iter_count > 15) break;
        // TODO: Stop partitioning when the x.gate_count = 1. 
        // TODO: Add cut Horizontal
        region left, right;
        x.cutVertical(left, right);
        q.push(left);
        q.push(right);  

          
    }

    for(auto g : netlist.gates){
        cout << "gate " << g.gateId << " x, y = " << g.location.first << " ," << g.location.second << endl;
    }

    //globe.printGates();

    return 0;
}