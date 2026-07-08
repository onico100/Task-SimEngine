#include <iostream>
#include <string>

#include "pwse/ConfigLoader.hpp"
#include "pwse/SimulationEngine.hpp"

namespace {

void printUsage() {
    std::cout <<
        "Personal Workload Simulation Engine\n\n"
        "Usage:\n"
        "  sim run --config <file.json>\n\n"
        "Options:\n"
        "  --config <file>   Path to a JSON simulation config (required)\n"
        "  -h, --help        Show this help message\n";
}

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty() || args[0] == "-h" || args[0] == "--help") {
        printUsage();
        return args.empty() ? 1 : 0;
    }

    if (args[0] != "run") {
        std::cerr << "Unknown command: '" << args[0] << "'\n\n";
        printUsage();
        return 1;
    }

    std::string configPath;
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--config" && i + 1 < args.size()) {
            configPath = args[++i];
        } else if (args[i] == "-h" || args[i] == "--help") {
            printUsage();
            return 0;
        } else {
            std::cerr << "Unknown argument: '" << args[i] << "'\n\n";
            printUsage();
            return 1;
        }
    }

    if (configPath.empty()) {
        std::cerr << "Error: 'sim run' requires --config <file.json>\n\n";
        printUsage();
        return 1;
    }

    try {
        pwse::SimulationConfig config = pwse::ConfigLoader::loadFromFile(configPath);
        pwse::SimulationEngine engine(config);
        engine.run();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
