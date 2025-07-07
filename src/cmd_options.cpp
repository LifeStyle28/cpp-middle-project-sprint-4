#include "cmd_options.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <print>
#include <string>

#include <boost/program_options.hpp>

namespace analyser::cmd {

namespace po = boost::program_options;

template <typename T>
static void validateInputFiles(std::span<const T> files) {
    for (const auto &file : files) {
        if (!std::filesystem::exists(file)) {
            throw std::runtime_error("File does not exist: " + file);
        }
        if (!std::filesystem::is_regular_file(file)) {
            throw std::runtime_error("Path is not a regular file: " + file);
        }
    }
}

ProgramOptions::ProgramOptions() : desc_("Allowed options") {
    desc_.add_options()("help,h", "Display help message")(
        "file,f", po::value<std::vector<std::string>>(&files_)->required()->multitoken(),
        "List of files to process (required)");
}

bool ProgramOptions::Parse(int argc, char *argv[]) {
    try {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc_).run(), vm);

        if (vm.count("help")) {
            desc_.print(std::cout);
            return false;
        }

        po::notify(vm);

        if (files_.empty()) {
            std::cerr << "Error: At least one file must be specified\n";
            desc_.print(std::cout);
            return false;
        }

        validateInputFiles(std::span<const std::string>(files_));

        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error parsing command line: " << e.what() << "\n";
        desc_.print(std::cout);
        return false;
    }
}

}  // namespace analyser::cmd
