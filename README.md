# fvsk

Kernelisation, heuristic and exact algorithms for Feedback Vertex Set (FVS) and Loop Cutset (LC). Kernelisation explained in the following paper:

Hans L. Bodlaender and Thomas C. van Dijk. 2010.
*A Cubic Kernel for Feedback Vertex Set and Loop Cutset.*
Theor. Comp. Sys. 46, 3 (April 2010), 566–597.

## Contents

Implements the following algorithms:
* Bodlaender cubic kernel for FVS (`HansbKernel.h`)
* Bodlaender & Van Dijk cubic kernel for LC (`TcdijkKernel.h`)
* Suermondt-Cooper heuristic for FVS (`SuermondtCooper.h`)
* Becker-Geiger 2-approximation for weighted FVS (`BeckerGeiger.h`)
* Becker-Bar Yeruda-Geiger probabilistic algorithm for FVS (`BeckerBarYerudaGeiger.h`)
* a branch-and-bound branching algorithm for FVS (`Bruteforcer.h`)

## Dependencies

Graph data structure and algorithms using [LEDA 5](https://www.algorithmic-solutions.com/index.php/products/leda-for-c).