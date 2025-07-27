# 0-1 Knapsack Problem with Group Fairness

This repository contains a set of 1787 feasible instances for the 0-1 Knapsack Problem with Group Fairness.
The instances are in folder `instances` and the generator used to create them is in folder `generator`.
Additional, larger, instances are in folder `instances_jooken`.

The repository also contains the source code of various solvers for this problem (solver `src`):

* A solver using Gurobi to solve the compact formulation of the problem.
* A solver for the extended formulation of the problem. This solver uses Dynamic Programming to enumerate a pseudopolynomial subset of columns guaranteed to solve the formulation to optimality. It then calls [David Pisinger's Multiple-Choice Knapsack solver](https://web.archive.org/web/20250219094749/http://hjemmesider.diku.dk/~pisinger/codes.html) on the resulting formulation.
* A branch-and-price solver, which is not used nor described in the paper.

Folder `results` contains the results of the algorithms for the compact and exponential formulations.
The scripts used to generate the paper figures are in folder `analysis`.
Analogously, folders `results_jooken` and `analysis_jooken` contain results and an analysis of the additional instances.

## Citation

You can cite the working paper as follows:

```bib
@techreport{kpgf,
    title={Algorithms and complexity results for the 0-1 Knapsack Problem with Group Fairness},
    author={Malaguti, Enrico and Paronuzzi, Paolo and Santini, Alberto},
    year=2025,
    url={...}
}
```

You can also cite this repository as follows:

```bib
@misc{kpgf_code,
    title={Source Code and Instances for the 0--1 Knapsack Problem with Group Fairness},
    author={Malaguti, Enrico and Paronuzzi, Paolo and Santini, Alberto},
    year=2025,
    url={https://github.com/alberto-santini/kpgf},
    doi={10.5281/zenodo.14936404}
}
```

## Instance Format

The first line contains three numbers:
1. $n$, the number of items.
2. $\ell$, the number of classes.
3. $c$, the knapsack capacity.

The next $\ell$ lines contain three numbers each. The $k$-th such line refers to class $k$ and contains:
1. The size of the class, i.e., the number of items contained in the class.
2. The resource consumption lower bound.
3. The resource consumption upper bound.

The next $n$ lines contain three numbers each. The $j$-th such line refers to item $j$ and contains:
1. The profit $p_j$.
2. The weight $w_j$.
3. The resource consumption $h_j$.

The name of the instance file is `{kpgen}-{rbdist}-{rdist}-{n}-{l}-{repetition}.txt`, where:

* `kpgen` is the generator used to create the base Knapsack instance. We use the thirteen classical generators presented in the [Where are the hard knapsack problems?](https://www.sciencedirect.com/science/article/pii/S030505480400036X) paper. If `kpgen` ends with a number (1000 or 10,000), this is the data range (parameter $R$ in the paper). Otherwise, the generator is `uncorrelated_similar`, which has a hard-coded data range of 100,000.
* `rbdist` and `rdist` are the methods to generate the resource consumption bounds for each class and the resource consumption values for each item. There is only one method for each, `rnduniform`.
* `n` is the number of items $n$.
* `l` is the number of classes $\ell$.
* `repetition` is a number between 0 and 9 used to distinguish multiple repetitions of instances generated with the same parameters and only differing in the random seed used.

## Building the program

We provide CMake files to build the source code.
A possible way to build the executable on Linux or Mac is the following:
1. Create a build directory and move into it: `mkdir build && cd build`.
2. Use CMake to configure the build environment: `cmake -DGUROBI_DIR=<path_to_gurobi>/<arch> -G"Unix Makefiles" ..`
3. Use GNU Make to build the programme: `make`.
4. The executable will be in `./kpgf`.

## License

The source code is distributed under the GNU General Public License v3.0 (see the `LICENSE.TXT` file).
Authors: Alberto Santini and Paolo Paronuzzi.

File `FindGUROBI.cmake` was made available by Matthias Miltenberger in the Gurobi support forum. Copyright Gurobi Optimization, LLC. All rights reserved.

The `cxxopts` library is distributed under the MIT License. See [here](https://github.com/jarro2783/cxxopts/blob/master/LICENSE).

The `combo` solver is made available by David Pisinger without any license. He states: "The codes may be used free of charge for academical purposes."