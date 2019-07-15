#include "control_data.h"

#include <regex>

Control_data Control_data::parse_input_file(std::ifstream &input_file,
                                            const std::string &start_token,
                                            const std::string &end_token) {
    if (!input_file.is_open())
        throw std::runtime_error("Input file is not open!");

    const auto keys = read_keys(input_file, start_token, end_token);
    Control_data cd;

    auto set_unique_string = [&](const std::string &key, std::string &val) {
        auto search = keys.find(key);
        if (search != keys.end())
            val = search->second.at(0);
    };

    set_unique_string("JOB_NAME", cd.job_name);
    set_unique_string("RESOURCES_PATH", cd.resources_path);
    set_unique_string("FILE_1E", cd.file1E);
    set_unique_string("OUT_FILE", cd.out_file);
    set_unique_string("OUT_PATH", cd.out_path);

    {
        auto search = keys.find("GAUGE");
        if (search != keys.end()) {
            std::string gauge = search->second.at(0);
            std::transform(gauge.begin(), gauge.end(), gauge.begin(), ::tolower);

            if (gauge == "length")
                cd.gauge = Gauge::length;
            else if (gauge == "velocity")
                cd.gauge = Gauge::velocity;
            else if (gauge == "acceleration")
                cd.gauge = Gauge::acceleration;
        }
    }
    {
        auto search = keys.find("WRITE");
        if (search != keys.end()) {
            std::string gauge = search->second.at(0);
            std::transform(gauge.begin(), gauge.end(), gauge.begin(), ::tolower);

            if (gauge == "true")
                cd.write = true;
            else if (gauge == "false")
                cd.write = false;
        }
    }

    auto set_unique_double = [&](const std::string &key, double &val) {
        auto search = keys.find(key);
        if (search != keys.end())
            val = std::stod(search->second.at(0));
    };

    set_unique_double("OPT_INTENSITY", cd.opt_intensity);
    set_unique_double("OPT_OMEGA_EV", cd.opt_omega_eV);
    set_unique_double("OPT_CARRIER_ENVELOPE", cd.opt_carrier_envelope);
    set_unique_double("OPT_CYCLES", cd.opt_cycles);

    set_unique_double("DT", cd.dt);
    set_unique_double("MAX_T", cd.max_t);

    {
        auto search = keys.find("OPT_FIELD_DIRECTION");
        if (search != keys.end()) {
            cd.opt_fielddir(0) = std::stod(search->second.at(0));
            cd.opt_fielddir(1) = std::stod(search->second.at(1));
            cd.opt_fielddir(2) = std::stod(search->second.at(2));
        }
    }
    return cd;
}

std::map<std::string, std::vector<std::string>> Control_data::read_keys(std::ifstream &input_file,
                                                                        const std::string &start_token,
                                                                        const std::string &end_token) {
    input_file.seekg(0, std::ios::beg);
    
    std::string line;
    std::map<std::string, std::vector<std::string>> keys;

    while (std::getline(input_file, line)) {
        if (line == start_token)
            break;
    }

    while (std::getline(input_file, line)) {
        if (line.empty())
            continue;
        if (line == end_token)
            break;

        const std::regex reg("\\s+");

        std::sregex_token_iterator beg(line.begin(), line.end(), reg, -1);
        std::sregex_token_iterator end;

        const std::string key = *beg;
        const std::vector<std::string> vec(++beg, end);
        keys.emplace(std::make_pair(key, vec));
    }

    return keys;
}