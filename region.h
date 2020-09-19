
#pragma once
#include "graph.h"
#include "solver.h"
using namespace std;

class region{
    public:
        region();
        region(pair<double,double> minLoc, pair<double,double> maxLoc);
        void setCorners(pair<double,double> minLoc, pair<double,double> maxLoc);
        void assign(vector<gate*>& );
        void printGates();

        void buildMatrix(graph&);
        void solve();
        pair<double, double> propagate(pair<double, double>);
        void cutVertical(region& left, region& right);

    private: 
        void containX();
        void containY();
        double xmin, xmax, ymin, ymax;
        int gate_count;
        vector<gate*> gates; // could have been a reference variable initialized by consrtructor
        // since this is an array of pointers, it is used to update the values of x,y in the orginal graph vector
        // after solving A
        unordered_map <int, int> gate_map; 

        vector<int> row;
        vector<int> col;
        vector<double> dat;
        vector<double> bx; // resize(gate_count,0)
        vector<double> by;
};
region::region(){
}
region::region(pair<double,double> minLoc, pair<double,double> maxLoc){
    xmin = minLoc.first;
    ymin = minLoc.second;
    xmax = maxLoc.first;
    ymax = maxLoc.second;
}
void region::setCorners(pair<double,double> minLoc, pair<double,double> maxLoc){
    xmin = minLoc.first;
    ymin = minLoc.second;
    xmax = maxLoc.first;
    ymax = maxLoc.second;
}

void region::assign(vector<gate*>& gate_array){
    gates = gate_array;
    gate_count = gates.size();
    for (int i = 0; i< gate_count; i++){
        gate_map[gates[i]->gateId] = i;
    }
}
void region::containX(){
    for (gate* g : gates){
        //if x,y not in region: set them to mid region.
        if (g->location.first < xmin || g->location.first > xmax) g->location.first= (xmin+xmax)/2;
    }
}
void region::containY(){
    for (gate* g : gates){
        //if x,y not in region: set them to mid region.
        if (g->location.second < ymin || g->location.second > ymax)  g->location.second = (ymin+ymax)/2;
    }
}

void region::printGates(){
    cout << "following are gates of region: " << this << endl;
    for (gate* g: gates){
        cout << "gate " << g->gateId << " x, y = " << g->location.first << " ," << g->location.second << endl;
    }
}

pair<double, double> region::propagate(const pair<double, double> loc){
    pair<double,double> propLoc = loc;
    if(loc.first < xmin) propLoc.first = xmin;
    if(loc.first > xmax) propLoc.first = xmax;
    if (loc.second < ymin) propLoc.second = ymin;
    if(loc.second > ymax) propLoc.second = ymax;
    return propLoc;
}
void region::buildMatrix(graph& netlist_graph){
    bx.resize(gate_count,0);
    by.resize(gate_count,0);

    for (int g=0; g< gate_count; g++){
       double diag_value = 0;
       int gateId = gates.at(g)->gateId;
       // connected gates
       for(auto gpair : netlist_graph.connectedGates.at(gateId)){
           // if gpair.first in left_gates :: col = left_gates.index 
           int u = gpair.first; 
           int w = gpair.second;
           if (gate_map.count(u)){
               int index = gate_map[u] ;
                row.push_back(g);
                col.push_back(index); 
                dat.push_back(-1*w);
                diag_value += w;
           }else{
               auto prop_loc = propagate(netlist_graph.gates.at(u-1).location);
               bx[g] += w*prop_loc.first;
               by[g] += w*prop_loc.second;
               diag_value += w;
           }

       }
       for (auto ppair : netlist_graph.connectedPads.at(gateId)){
           int pId = ppair.first;
           double w = ppair.second;
           diag_value += w;
           auto prop_loc = propagate(netlist_graph.pads.at(pId-1).location);
           bx[g] += w*prop_loc.first;
           by[g] += w*prop_loc.second;
       }
       row.push_back(g);
       col.push_back(g);
       dat.push_back(diag_value);
   }

}
void region::solve(){
    coo_matrix A;
    A.n = gate_count;
    A.nnz = row.size();
    A.row.resize(A.nnz);
    A.col.resize(A.nnz);
    A.dat.resize(A.nnz);
    A.row = valarray<int>(row.data(), A.nnz);
    A.col = valarray<int>(col.data(), A.nnz);
    A.dat = valarray<double>(dat.data(), A.nnz);

    valarray<double> bx_array(bx.data(), A.n);
    valarray<double> by_array(by.data(), A.n);

    //cout << " b = " << endl;
    // print_valarray(bx_array);

    valarray<double> x(A.n);
    valarray<double> y(A.n);
    A.solve(bx_array, x); // Ax = b
    //cout << "Solving for x .... x = " << endl;
    //print_valarray(x);
    A.solve(by_array,y);
    //cout << "Solving for y .... y = " << endl;
    //print_valarray(y);

    int i = 0;
    for (gate* g: gates){
        g->location.first = x[i];
        g->location.second = y[i];
        i++;
    }

}

/*
        i =0;
       for ( gate ggg: left_gates){
           gates[ggg.gateId-1].location.first = x[i];
           i++;
       }
       */
void region::cutVertical(region& left, region& right){
    float xmid = (xmin+xmax)/2;
    left.setCorners({xmin,ymin}, {xmid,ymax});
    right.setCorners({xmid,ymin}, {xmax, ymax});

    sort(gates.begin(), gates.end(), compare_gate_x);
    vector<gate*> gL(gates.begin(), gates.end()-gate_count/2);
    vector<gate*> gR(gates.end()-gate_count/2, gates.end());
    left.assign(gL);
    left.containX();
    right.assign(gR);
    right.containX();

}
