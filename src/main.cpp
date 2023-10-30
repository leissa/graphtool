#include <cstring>

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "graphtool/parser.h"

using namespace std::literals;

int main(int argc, char** argv) {
    try {
        static const auto version = "graphtool 0.1\n";
        static const auto usage   = "USAGE:\n"
                                    "  graphtool [-?|-h|--help] [-v|--version] [-d|--dump] [-e|--eval] [<file>]\n"
                                    "\n"
                                    "Display usage information.\n"
                                    ""
                                    "OPTIONS, ARGUMENTS:\n"
                                    "  -?, -h, --help"
                                    "  -v, --version           Display version info and exit.\n"
                                    "  -c, --crit              Eliminate critical edges.\n"
                                    "  <file>                  Input file.\n";
        std::string input;
        bool crit = false;

        for (int i = 1; i < argc; ++i) {
            if (argv[i] == "-v"s || argv[i] == "--version"s) {
                std::cout << version;
                return EXIT_SUCCESS;
            } else if (argv[i] == "-?"s || argv[i] == "-h"s || argv[i] == "--help"s) {
                std::cerr << usage;
                return EXIT_SUCCESS;
            } else if (argv[i] == "-c"s || argv[i] == "--crit"s) {
                crit = true;
            } else {
                if (!input.empty()) throw std::invalid_argument("more than one input file given");
                input = argv[i];
            }
        }

        if (input.empty()) throw std::invalid_argument("no input given");

        auto driver = graphtool::Driver();
        auto path   = std::filesystem::path(input);
        auto ifs    = std::ifstream(path);
        if (!ifs) throw std::runtime_error(std::format("cannot read file \"{}\"", input));
        auto parser = graphtool::Parser(driver, ifs, &path);
        auto graph  = parser.parse_graph();

        if (auto num = driver.num_errors()) {
            std::cerr << num << " error(s) encountered" << std::endl;
            return EXIT_FAILURE;
        }

        if (crit) graph.critical_edge_elimination();
        graph.number();

        std::ofstream ofs("out.dot");
        ofs << graph;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "error: unknown exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
