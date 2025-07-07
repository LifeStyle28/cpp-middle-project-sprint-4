#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <boost/program_options.hpp>

namespace analyser::cmd {

class ProgramOptions {
public:
    ProgramOptions();
    ~ProgramOptions() = default;

    bool Parse(int argc, char *argv[]);

    const std::vector<std::string> &GetFiles() const { return files_; }

private:
    std::vector<std::string> files_;
    boost::program_options::options_description desc_;
};

}  // namespace analyser::cmd
