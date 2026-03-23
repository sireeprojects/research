#include "cmdline_parser.h"
#include "messenger.h"


namespace vtb {

void CmdlineParser::add_argument(std::string_view short_n, std::string_view long_n, 
                                std::string_view desc, bool has_val, 
                                bool required, std::vector<std::string> allowed) {
    Option opt{std::string(short_n), std::string(long_n), std::string(desc), has_val, required, allowed};
    registry[std::string(short_n)] = opt;
    registry[std::string(long_n)] = opt;
    unique_options.push_back(opt);
}

void CmdlineParser::parse(int argc, char* argv[]) {
    int user_start_index = -1;
    
    // Find the DPDK '--' separator to skip EAL parameters
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--") {
            user_start_index = i + 1;
            break;
        }
    }

    // Only parse if the separator was found and there are tokens following it
    if (user_start_index != -1 && user_start_index < argc) {
        for (int i = user_start_index; i < argc; ++i) {
            std::string arg = argv[i];
            auto it = registry.find(arg);
            
            if (it != registry.end()) {
                const auto& opt = it->second;
                if (opt.expects_value) {
                    if (i + 1 < argc && argv[i+1][0] != '-') {
                        std::string val = argv[++i];
                        // Validate against enumeration list if provided
                        if (!opt.allowed_values.empty()) {
                            if (std::find(opt.allowed_values.begin(), opt.allowed_values.end(), val) == opt.allowed_values.end()) {
                                throw std::runtime_error("Error: Value '" + val + "' is not allowed for " + arg);
                            }
                        }
                        results[opt.long_name].push_back(val);
                    } else {
                        throw std::runtime_error("Error: " + arg + " requires an associated value.");
                    }
                } else {
                    results[opt.long_name].push_back("true");
                }
            } else {
                throw std::runtime_error("Error: Unknown application argument '" + arg + "'");
            }
        }
    }
    validate_requirements();
}

bool CmdlineParser::has(std::string_view name) const {
    return results.count(std::string(name));
}

void CmdlineParser::print_help() const {
    std::cout << "\nUsage: <DPDK EAL options> -- [Application options]\n\n";
    std::cout << std::left << std::setw(25) << "Option" << "Description\n";
    std::cout << std::string(75, '-') << "\n";
    for (const auto& opt : unique_options) {
        std::string names = opt.short_name + ", " + opt.long_name;
        std::cout << std::left << std::setw(25) << names << opt.description;
        if (!opt.allowed_values.empty()) {
            std::cout << " [";
            for(size_t i=0; i < opt.allowed_values.size(); ++i) 
                std::cout << opt.allowed_values[i] << (i == opt.allowed_values.size()-1 ? "" : "|");
            std::cout << "]";
        }
        if (opt.is_required) std::cout << " (Required)";
        std::cout << "\n";
    }
    std::cout << std::endl;
}

void CmdlineParser::validate_requirements() {
    for (const auto& opt : unique_options) {
        if (opt.is_required && !has(opt.long_name)) {
            throw std::runtime_error("Error: Required argument '" + opt.long_name + "' is missing.");
        }
    }
}

}
