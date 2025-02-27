#include "BBTree.h"
#include "CompactSolver.h"
#include "Instance.h"
#include "MCKPSolver.h"
#include "Solution.h"
#include <cstdlib>
#include <cxxopts.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    using namespace kpgf;
    using namespace cxxopts;

    auto cl = Options{"kpgf", "Knapsack Problem with Group Fairness"};

    // clang-format off
    cl.add_options()
        ("p,params", "Parameters file", value<std::string>())
        ("i,instance", "Instance file", value<std::string>())
        ("s,solver", "Solver [compact|bp|mckp]", value<std::string>())
        ("l,load", "Initial solution file (optional)", value<std::string>())
        ("r,resultsfile", "Results file (optional)", value<std::string>())
        ("v,verbose", "Print instance information", value<bool>()->default_value("false"));

    const auto opts = cl.parse(argc, argv);

    if(opts["params"].count() == 0u) {
        std::cerr << "You must provide a params file!\n";
        return EXIT_FAILURE;
    }

    if(opts["instance"].count() == 0u) {
        std::cerr << "You must provide an instance file!\n";
        return EXIT_FAILURE;
    }

    if(opts["solver"].count() == 0u) {
        std::cerr << "You must provide a solver!\n";
        return EXIT_FAILURE;
    }

    auto i = Instance{opts["instance"].as<std::string>(), opts["params"].as<std::string>()};

    if(opts["resultsfile"].count() > 0u) {
        const auto resf = std::filesystem::path(opts["resultsfile"].as<std::string>());
        i.params.output_dir = resf.parent_path();
        i.params.output_file = resf;
    }

    if(opts["verbose"].as<bool>()) {
        i.print();
    }

    const auto solver = opts["solver"].as<std::string>();

    if(solver == "compact") {
        auto s = CompactSolver{i};

        if(opts["load"].count() > 0u) {
            s.load_initial_solution(Solution::from_file(opts["load"].as<std::string>(), i));
        }

        const auto results = s.solve();
        results.to_file();
    } else if(solver == "mckp") {
        auto s = MCKPSolver{i};
        const auto results = s.solve();
        results.to_file();
    } else if(solver == "bp") {
        BBTree tree = BBTree{i};
        const auto results = tree.solve();

        if(results.status == FeasibleStatus::INFEASIBLE) {
            std::cout << "KPGF is infeasible!\n";
        } else if(results.status == FeasibleStatus::UNKNOWN) {
            std::cout << "KPGF can be both feasible or infeasible:\n";
            std::cout << "\t- Did not find any feasible solution within the time limit.\n";
            std::cout << "\t- Could not prove infeasibility within the time limit.\n";
        } else {
            std::cout << "KPGF is feasible.\n";
            std::cout << "\t- Best primal solution value: " << *results.primal_lb << "\n";
            std::cout << "\t- Best dual bound: " << *results.dual_ub << "\n";
            std::cout << "\t- Gap: " << 100.0 * (*results.dual_ub - *results.primal_lb) / *results.dual_ub << "%\n";
        }

        results.to_file();
    } else {
        std::cerr << "Solver " << solver << " not yet supported!\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}