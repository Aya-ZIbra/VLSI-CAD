#include "grid.h"

bool location::operator==(const location &other){
    if (this->x == other.x && this->y==other.y && this->l == other.l){
        return true;
    }
    return false;
}

location location::operator+(const location &other){
    location sum;
    sum.x = this->x + other.x;
    sum.y = this->y + other.y;
    sum.l = this->l + other.l;
    return sum;
}

location location::operator-(const location &other){
    location sum;
    sum.x = this->x - other.x;
    sum.y = this->y - other.y;
    sum.l = this->l - other.l;
    return sum;
}
void grid::resize_mat(){
    mat.resize(size_y);
    for (auto &v : mat){
        v.resize(size_x);
    }
}

void grid::print(ostream & os){
    os << "Printing Grid Matrix ..."<< endl;
    for (auto &v : mat){
        for (auto &c : v){
            os << c.cost << '\t';
        }
        os << endl;
    }
}

void grid::read_grid(const char *fname){
    ifstream fs {fname};
    if (!fs) {
        cerr << "cannot open file " << fname << endl;
        exit(1);
    }

    string line;
    if (getline(fs,line)){
        istringstream iss(line);
        iss >> size_x >> size_y >> bendPenalty>> viaPenalty;
        cout << size_x << '\t' << size_y << '\t' << bendPenalty << viaPenalty << endl;
    }
    resize_mat();
    for (int j=0; j<size_y; j++){
        getline(fs,line);
        istringstream iss(line);
        int i=0;
        while(iss >> mat[j][i].cost) {
            i++;
        }
    }
}

void grid::fill_BFS(const location &loc_S, const location &loc_T){ 
    int cost_S = mat[loc_S.y][loc_S.x].cost;
    vertex S {cost_S, loc_S}; //path cost, location

    // data structure 
    queue<vertex> q;
    q.push(S);
    set_flag(S,"Explored");
    set_pred(loc_S, loc_S);
    // Breadth First Search 
    while(!q.empty()){
        vertex v = q.front();
        q.pop();
        set_flag(v, "Expanded");
        // Stop when you reach the Target
        if (v.second == loc_T){
            break;
        }
        // explore the neighborhood of v [Adjacent cells]
        // Get the location of adjacent cells
        array<location,6> adj_loc;
        adj_loc = get_adj_loc(v, adj_loc);
        for (location loc: adj_loc){
            // check if inside search space and not blocked
            // check if not already expanded
            if (isInsideSearchSpace(loc) && !isBlocked(loc) && !isExpanded(loc)){
                if (!isExplored(loc)){
                    /**  when modeling the grid as a graph, all the edges leading to a cell X are equal cost
                     * edges = cell.cost
                     * Result: lowest pathcost to this cell is the one calculated when the cell is first explored
                     * Hence, no need for key update as in general Dijkstra's SSSP. 
                     * */
                    int pathcost = evaluatePathcost(v,loc, loc_T);
                    vertex v_adj = {pathcost, loc}; 
                    q.push(v_adj);
                    set_flag(v_adj, "Explored");
                    // this is the only point where we know how we reached this cell
                    // set pred
                    set_pred(loc, v.second);
                }
            }
        }
    }
}
/**/
vector<location> grid::retrace(const location &loc_S, const location &loc_T){
    vector<location> route;
    stack<location> routeStack;
    cell c = mat[loc_T.y][loc_T.x];
    routeStack.push(loc_T);
    location loc = loc_T;
    while(c.pred != -1){
        loc = loc + shift[c.pred];
        routeStack.push(loc);
        // update cell
        c = mat[loc.y][loc.x];
    }
    // Assert route.top == loc_S
    while(!routeStack.empty()){
        route.push_back(routeStack.top());
        routeStack.pop();
    }
    return route;
}
void grid::cleanup(vector<location> &route){
    for(int j=0; j<size_y; j++){
        for (int i=0; i< size_x; i++){
            mat[j][i].eflag = 0;
            mat[j][i].pred = 0;
        }
    }
    // Mark route as blockage in grid
    for (location loc : route){
        mat[loc.y][loc.x].cost = -1;
    }
}
void grid::set_pred(location loc_v,  location loc_S){
    if (loc_S == loc_v) mat[loc_v.y][loc_v.x].pred= -1; // this is the source
    else {
        for(int i = 0; i<6;i++){
            if(loc_S == loc_v+shift[i]) {
                mat[loc_v.y][loc_v.x].pred=i;
                break;
            }
        }
    }
}
int grid::evaluatePathcost(const vertex &S,location loc_v, location loc_T){
    int pathcost_S = S.first;
    location loc_S = S.second;
    int cost_v =  mat[loc_v.y][loc_v.x].cost;
    int pathcost;
    pathcost = pathcost_S + cost_v; 
    // To add estimate for cost from v to T... This assumes H(v,T) = 0
    // To add penality for bends and vias
    return pathcost;
}

array<location,6> grid::get_adj_loc(const vertex &v, array<location,6> & adj_loc){ // How to pass array?
    location loc_v = v.second;
    int l = loc_v.l; int x = loc_v.x; int y = loc_v.y;
    for (int i=0; i<6; i++){
        adj_loc[i] = loc_v - shift[i];
    }
    return adj_loc;
}

void grid::set_flag(const vertex& v, string s){
    int x = v.second.x;
    int y = v.second.y;
    if( s == "Expanded") {
        mat[y][x].eflag = 1;
    } else if (s == "Explored"){
        mat[y][x].eflag = 2;
    } else {
        // Raise "Unknown Flag"
        exit(1);
    }
}

bool grid::isInsideSearchSpace(const location &loc){
    int x = loc.x;
    int y = loc.y;
    
    if (x < 0 || y < 0){
        return false; // Raise exception of NEG_LOC
    }
    if (x >= size_x || y >= size_y){
        return false; // logging (debug): out of search region bounds
    } 
    return true;
}

bool grid::isBlocked(const location &loc){
    int x = loc.x;
    int y = loc.y;
    if (mat[y][x].cost == -1){
        return true; // logging (debug) : blockage
    } 
    return false;
}

bool grid::isExpanded(const location &loc){
    int x = loc.x;
    int y = loc.y;
    if (mat[y][x].eflag == 1){
        return true; // already expanded
    }
    return false;
} 

bool grid::isExplored(const location &loc){
    int x = loc.x;
    int y = loc.y;
    if(mat[y][x].eflag == 2){
        return true;
    }
    return false;
}
    