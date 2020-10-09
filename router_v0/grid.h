#ifndef _GRID_H_
#define _GRID_H_

#include <vector>
#include <iostream>
#include <queue>
#include <array>
#include <stack>
#include <fstream>
#include <sstream>

using namespace std;
class cell{
    public:
        int cost = 1; // empty 1, blocked -1
        int pred = 0; // encoded pred : E, W, N, S, Up , Dn = 1, 2, 3, 4, 5, 6 // 0 = don't care // -1 = Source
        int eflag = 0; // expanded (visited) = 1 // to be replaced by hash set "expanded"
                                // explored = 2
};

struct location{
    public:
    int x;
    int y;
    int l;
    bool operator==(const location &other);
    location operator+(const location &other);
    location operator-(const location &other);
};

const vector<location> shift {{1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0, 1}, {0,0,-1}};
using vertex = pair<int, location>; // pair (cost, location) used to represent cells in the queue

class grid{
    public:
        vector<vector<cell>> mat; //matrix of cells
        int size_x, size_y; // number of cells in x and y direction
        int bendPenalty, viaPenalty;
        void resize_mat();
        void read_grid(const char *fname);
        void print(ostream & os);
        void fill_BFS(const location &loc_S, const location &loc_T);
        vector<location> retrace(const location &loc_S, const location &loc_T);
        void cleanup(vector<location> &route);
    private:
        array<location,6> get_adj_loc(const vertex &v, array<location,6> &adj_loc);
        void set_flag(const vertex& v, string s);
        bool isInsideSearchSpace(const location &loc);
        bool isBlocked(const location &loc);
        bool isExpanded(const location &loc);
        bool isExplored(const location &loc);
        int evaluatePathcost(const vertex &S,location loc_v, location loc_T);
        void set_pred(location loc_v, location loc_S);
};

#endif