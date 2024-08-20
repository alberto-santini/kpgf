## 0-1 Knapsack Problem with Group Fairness

This repository contains a set of instances and the source code of a solver for the 0-1 Knapsack with Group Fairness.

### Citation

You can cite the working paper as follows:

```bib
@techreport{kpgf,
    title={Exact Alorithms for the 0--1 Knapsack Problem with Group Fairness},
    author={Malaguti, Enrico and Paronuzzi, Paolo and Santini, Alberto},
    year=2024,
    url={...}
}
```

You can also cite this repository as follows:

```bib
@misc{kpgf_code,
    title={Source Code and Instances for the 0--1 Knapsack Problem with Group Fairness},
    author={Malaguti, Enrico and Paronuzzi, Paolo and Santini, Alberto},
    year=2024,
    url={https://github.com/alberto-santini/kpgf},
    doi={...}
}
```

### Instance Format

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

The name of the instance file is `{seed}_{n}_{l}_{cm}.txt`, where `n` is the number of items, `l` is the number of classes and `cm` is the capacity multiplier, i.e., the knapsack capacity is obtained as `cm` times the total weight of all items in the instance.
Parameter `seed` is used as a random seed generator and allows to generate multiple instances with the same values of `n`, `l` and `cm`.

### Building the program

We provide CMake files to build the source code.
A possible way to build the executable on Linux or Mac is the following:
1. Create a build directory and move into it: `mkdir build && cd build`.
2. Use CMake to configure the build environment: `cmake -DGUROBI_DIR=<path_to_gurobi>/<arch> -G"Unix Makefiles" ..`
3. Use GNU Make to build the programme: `make`.
4. The executable will be in `./kpgf`.

### License

The source code is distributed under the GNU General Public License v3.0 (see the `LICENSE.TXT` file).
Authors: Alberto Santini and Paolo Paronuzzi.

File `FindGUROBI.cmake` was made available by Matthias Miltenberger in the Gurobi support forum. Copyright Gurobi Optimization, LLC. All rights reserved.

The `cxxopts` library is distributed under the MIT License. See [here](https://github.com/jarro2783/cxxopts/blob/master/LICENSE).