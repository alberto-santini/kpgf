#ifndef KPGF_MCKNAP
#define KPGF_MCKNAP

#include "Instance.h"
#include "MCKPSolver.h"
#include <string>

/* ======================================================================
				 type declarations
   ====================================================================== */

typedef int boolean;         /* logical variable */
typedef int ntype;           /* number of stages */
typedef long itype;          /* item profits and weights */
typedef long stype;          /* sum of pofit or weight */
typedef unsigned long vtype; /* solution vector */

typedef struct {      /* all problem information */
    stype capacity;   /* capacity of knapsack */
    stype dantzig;    /* the dantzig upper bound */
    stype zstar;      /* optimal solution */
    stype summul;     /* sum of multiplications */
    stype antmul;     /* number of multiplications */
    stype maxmul;     /* max multiplied set */
    stype redusets;   /* sum of reduced sets */
    stype reduitems;  /* sum of items which are tested for reduce */
    stype redukill;   /* sum of tested items which were reduced */
    stype gap;        /* current gap */
    stype partitions; /* number of partitions */
    stype domikill;   /* number of dominated-kills */
    stype lpkill;     /* number of lp-kills */
    long timepar;     /* time used for partitioning */
    long timesort;    /* time used for sorting of gradients */
    long time;        /* time used for all solution */
    long welldef;     /* is the found solution correct */
    long checked;     /* optimal solution checked */
    long iterates;    /* number of iterations to find optimal sol */
} allinfo;

std::pair<allinfo, std::vector<std::pair<int, int>>> minmcknap(
    std::vector<std::vector<kpgf::MaximalPacking>>& max_packings, const kpgf::Instance& instance
);

#endif//KPGF_MCKNAP