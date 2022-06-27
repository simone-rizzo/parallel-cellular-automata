# parallel-cellular-automata
## Read the report file pdf.

### Cellular automata
A cellular automaton consists of a regular grid of cells, where each cell may be in one of a finite number
of states. The grid to be considered is toroidal 2D mesh, where the last row (column) must be considered
adjacent to the first row (column). An initial state is determined by an assignment of a particular state for
each cell. A new generation is created according to some fixed set of rules that determine the new state
of each cell in terms of the current state of the cell and the states of the cells in its neighborhood. The rule
for updating the status of cells is the same for each cell and does not change over time and is applied to
the entire grid simultaneously. The computation of a cellular automaton consists of the computation of
the changes in grid cell status for a given number of iterations.
A programming API must be provided that supports the definition of the grid dimensions, the rules used
to compute a cell as a function of its current status and its 8-neighbor cell status, the initial state of the
grid and the number of iterations to be computed and the execution of the defined simulation.
