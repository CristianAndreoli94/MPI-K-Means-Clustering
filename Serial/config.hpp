// Utility/config.hpp
#pragma once

#include <string>
#include <iostream>
#include <cstdlib>

struct Config {
    std::string dataset_path = "DataSet10000x10.txt"; // default
    int max_iter = 5;      // MAXITERATION
};

inline void print_usage(const char* progname) {
    std::cerr
        << "Usage: " << progname << " [options]\n\n"
        << "Options:\n"
        << "  -d, --dataset   PATH   Path to dataset file\n"
        << "  -m, --max-iter  N      Maximum number of iterations\n";
}

inline Config parse_args(int argc, char** argv) {
    Config cfg; 

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if ((arg == "-d" || arg == "--dataset") && i + 1 < argc) {
            cfg.dataset_path = argv[++i];
        } else if ((arg == "-m" || arg == "--max-iter") && i + 1 < argc) {
            cfg.max_iter = std::stoi(argv[++i]);
        } else {
            std::cerr << "Unknown or incomplete option: " << arg << "\n\n";
            print_usage(argv[0]);
            std::exit(1);
        }
    }

    return cfg;
}
