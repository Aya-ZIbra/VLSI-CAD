#include <iostream>
#include <queue>
#include <stack>
#include "grid.h"
using namespace std;


int main(){
    cout<<"VLSI Router starting ..." <<endl;
    grid G;
    G.size_x = 5;
    G.size_y = 5;
    G.resize_mat();
    G.print(cout);

    // Input Pins location
    location S_pos {1,1,1};
    location T_pos {3,3,1};
    
    // I. Filling
    cout << "Filling stage..." << endl;
    G.fill_BFS(S_pos, T_pos);
    // II. Retracing
    cout<< "Retrace stage ..." << endl;
    vector<location> route = G.retrace(S_pos, T_pos);
    for (location l : route){
        cout << l.x << "\t" << l.y << endl;
    }
    
    // III.clean up
    G.cleanup(route);
    G.print(cout);
    return 0;
}