# VLSI-CAD
A basic implementation of analytical gate placer

## Placer Types: Randomized vs Analytical
Randomized type uses Simulated Annealing. In this implementation, we are considering an analytical placer.
## Wire estimation models
Two popular models are: 
a) Half Perimeter Wire Length (HPWL)
b) Quadratic model 

## Iterative quadratic placer
* Based on quadratic wire length estimation
* Model multi-point net using Clique
* Cost function = L = Σ(w_ij Δ𝑥_𝑖𝑗 )^2+(𝑤_𝑖𝑗 Δ𝑦_𝑖𝑗 )^2 
* L = Lmin when derivatives = 0
* Solve system of linear equations -> Get 𝑥_𝑖, 𝑦_𝑖
* Recursive partitioning to resolve gate clustering :Assign, Contain, Solve, repeat …


<img src=https://github.com/Aya-ZIbra/VLSI-CAD/blob/master/Doc_Resources/Steps.png width= 800 />

## Code 
### Graph class 
* Parses the netlist file into graph representation (adjacent list).
* Stores the gates, pads, their connectivity and calculated weights
### Region class
* Stores physical dimensions of region.
* Stores pointer array to gates
#### Functions 
* build matrix (system of linear equations):  𝐴𝑋=𝑏<sub>𝑥 </sub>,  𝐴𝑌=𝑏<sub>𝑦</sub>
* solve and updated gate locations
* cut Vertical, cut Horizontal
* Sorting-based assignment of gates
* Containment of connected pads/gates in region.
### Solver class
Conjugate gradient iterative solver
[A] is sparse matrix (sparse matrix representation)

## Build
```
g++ -o main main.cpp solver.cpp 
```

## Flowchart
<img src=https://github.com/Aya-ZIbra/VLSI-CAD/blob/master/Doc_Resources/FlowChartAndTraversal.png width=800/>
