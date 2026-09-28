#include <format>
#include <fstream>
#include <iostream>

#include <fe/cli.h>
#include <fe/error.h>

#include "graphtool/parser.h"

int main(int argc, char** argv) {
    // fe::CodeDiag renders a diagnostic when it is *recorded*, so decide on color up front.
    fe::term::resolve_mode();
    graphtool::Driver driver; // outlives the handler below: it writes into the Driver's Diag

    try {
        bool show_help = false, show_version = false, crit = false;
        int verbose      = 0;
        auto inc_verbose = [&](bool) { ++verbose; };
        std::string input;

        auto loc_style = [&](const std::string& t) -> std::string {
            // clang-format off
            if      (t == "full"  ) driver.diag().loc_style = fe::Loc::Style::Full;
            else if (t == "rowcol") driver.diag().loc_style = fe::Loc::Style::RowCol;
            else if (t == "row"   ) driver.diag().loc_style = fe::Loc::Style::Row;
            else if (t == "msvc"  ) driver.diag().loc_style = fe::Loc::Style::MSVC;
            else return std::format("'{}' is not a location style", t);
            // clang-format on
            return {};
        };

        // clang-format off
        auto cli = fe::Cli("graphtool", "Computes dominance-related properties of a Graphviz DOT digraph.")
            .help(show_help)
            .opt(show_version           ,          "-v", "--version"   , "Display version info and exit.")
            .opt(crit                   ,          "-c", "--crit"      , "Eliminate critical edges.")
            .opt(inc_verbose            , ""     , "-V", "--verbose"   , "Raises the log level from error to warn (unreachable nodes) and info (the chosen exit); repeatable.").cardinality(0, 2)
            .grp("Diagnostics")
            .opt(loc_style              , "style", ""  , "--loc-style" , "How a diagnostic spells out a source location: `full` (`path:row:col-row:col`), `rowcol` (`path:row:col`), `row` (`path:row`), or msvc (`path(row,col)`).")
            .opt(driver.diag().no_snippet,         ""  , "--no-snippet", "Does not render the offending source line and caret underneath a diagnostic.")
            .opt(driver.diag().gutter   , "width", ""  , "--gutter"    , "Width of a diagnostic's line-number column.")
            .opt(driver.diag().max_rows , "num"  , ""  , "--max-rows"  , "Maximum number of rows a diagnostic's snippet renders before eliding its middle; `0` elides nothing.")
            .opt(driver.diag().max_errors,"num"  , ""  , "--max-errors", "Maximum number of errors to report before dropping the rest; `0` reports all of them.")
            .opt(driver.diag().werror   ,          ""  , "--werror"    , "Treats warnings as errors.")
            .arg(input, "file", "Input file.")
            .epilog("The results are written next to `<file>` as `<file>.forward.dot`, `<file>.backward.dot`, `<file>.dom_tree.dot`, `<file>.postdom_tree.dot`, `<file>.dom_frontiers.dot`, and `<file>.postdom_frontiers.dot`.");
        // clang-format on

        if (auto err = cli.parse(argc, argv)) throw std::invalid_argument(*err);

        if (show_help) {
            std::cerr << cli;
            return EXIT_SUCCESS;
        }

        if (show_version) {
            std::cout << "graphtool " GRAPHTOOL_VERSION " (fe " FE_VERSION ")\n";
            return EXIT_SUCCESS;
        }

        driver.log().set(&std::cerr).set((fe::Log::Level)verbose);

        if (input.empty()) throw std::invalid_argument("no input given");

        auto path = std::filesystem::path(input);
        auto src  = driver.src().add(path).first;
        if (!src) throw std::runtime_error(std::format("cannot read file \"{}\"", input));
        auto parser = graphtool::Parser(driver, *src);
        auto graph  = parser.parse_graph();

        driver.error().ack(); // throws what it collected; merely reports the warnings

        if (crit) graph.critical_edge_elimination();
        auto fw = graphtool::BiGraph<0>(graph);
        auto bw = graphtool::BiGraph<1>(graph);

        auto out = [&input](const char* suffix) {
            auto name = input + suffix;
            auto ofs  = std::ofstream(name);
            if (!ofs) throw std::runtime_error(std::format("cannot write file \"{}\"", name));
            return ofs;
        };

        auto forward           = out(".forward.dot");
        auto backward          = out(".backward.dot");
        auto dom               = out(".dom_tree.dot");
        auto postdom           = out(".postdom_tree.dot");
        auto dom_frontiers     = out(".dom_frontiers.dot");
        auto postdom_frontiers = out(".postdom_frontiers.dot");

        fw.dump_cfg(forward);
        bw.dump_cfg(backward);
        fw.dump_dom_tree(dom);
        bw.dump_dom_tree(postdom);
        fw.dump_dom_frontiers(dom_frontiers);
        bw.dump_dom_frontiers(postdom_frontiers);
    } catch (const fe::Error::Bail& bail) {
        std::cerr << bail; // already rendered, so the Driver it came from may be long gone
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "error: unknown exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
