#include "control_data.h"

#include <regex>

Control_data Control_data::parse_input_file(const std::string &input_file,
                                            const std::string &start_token,
                                            const std::string &end_token) {
    std::ifstream file(input_file);
    if (!file.is_open())
        throw std::runtime_error("Input file is not open!");

    const auto keys = read_keys(file, start_token, end_token);
    Control_data cd;

    auto set_unique_string = [&](const std::string &key, std::string &val) {
        const auto search = keys.find(key);
        if (search != keys.end())
            val = search->second.at(0);
    };

    auto set_unique_bool = [&](const std::string &key, bool &val) {
        const auto search = keys.find(key);
        if (search != keys.end()) {
            std::string token = search->second.at(0);
            std::transform(token.begin(), token.end(), token.begin(), ::tolower);
            if (token == "y" || token == "yes")
                val = true;
            else if (token == "n" || token == "no")
                val = false;
        }
    };

    auto set_unique_double = [&](const std::string &key, double &val) {
        const auto search = keys.find(key);
        if (search != keys.end())
            val = std::stod(search->second.at(0));
    };

    set_unique_string("JOB_NAME", cd.job_name);
    set_unique_string("RESOURCES_PATH", cd.resources_path);
    set_unique_string("FILE_1E", cd.file1E);
    set_unique_string("OUT_FILE", cd.out_file);
    set_unique_string("OUT_PATH", cd.out_path);

    set_unique_bool("WRITE", cd.write);
    set_unique_bool("USE_CAP", cd.use_cap);

    set_unique_double("OPT_INTENSITY", cd.opt_intensity);
    set_unique_double("OPT_OMEGA_EV", cd.opt_omega_eV);
    set_unique_double("OPT_CARRIER_ENVELOPE", cd.opt_carrier_envelope);
    set_unique_double("OPT_CYCLES", cd.opt_cycles);

    set_unique_double("DT", cd.dt);
    set_unique_double("MAX_T", cd.max_t);
    set_unique_double("REGISTER_DIPOLE_DT", cd.register_dip);

    set_unique_double("CAP_R0", cd.cap_r0);
    set_unique_double("CAP_AMPLITUDE", cd.cap_amp);

    {
        const auto search = keys.find("GAUGE");
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
        const auto search = keys.find("REPRESENTATION");
        if (search != keys.end()) {
            std::string gauge = search->second.at(0);
            std::transform(gauge.begin(), gauge.end(), gauge.begin(), ::tolower);

            if (gauge == "cartesian")
                cd.representation = Representation::cartesian;
            else if (gauge == "spherical")
                cd.representation = Representation::spherical;
        }
    }
    {
        const auto search = keys.find("OPT_FIELD_DIRECTION");
        if (search != keys.end()) {
            cd.opt_fielddir(0) = std::stod(search->second.at(0));
            cd.opt_fielddir(1) = std::stod(search->second.at(1));
            cd.opt_fielddir(2) = std::stod(search->second.at(2));
        }
    }

    cd.basis.read(file);

    file.close();
    return cd;
}

std::map<std::string, std::vector<std::string>> Control_data::read_keys(std::ifstream &file,
                                                                        const std::string &start_token,
                                                                        const std::string &end_token) {
    file.seekg(0, std::ios::beg);

    std::string line;
    std::map<std::string, std::vector<std::string>> keys;

    while (std::getline(file, line)) {
        if (line == start_token)
            break;
    }

    while (std::getline(file, line)) {
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

std::ostream &operator<<(std::ostream &os, const Control_data &rhs) {
    os << "============================================================================\n";
    os << "JOB_NAME                        " << rhs.job_name << '\n';
    os << "============================================================================\n";
    os << "GAUGE                           " << rhs.gauge << '\n';
    os << "REPRESENTATION                  " << rhs.representation << '\n';
    os << "============================================================================\n";
    os << "OPT_INTENSITY                   " << rhs.opt_intensity << '\n';
    os << "OPT_FIELD_DIRECTION             " << rhs.opt_fielddir.transpose() << '\n';
    os << "OPT_OMEGA_EV                    " << rhs.opt_omega_eV << '\n';
    os << "OPT_CARRIER_ENVELOPE            " << rhs.opt_carrier_envelope << '\n';
    os << "OPT_CYCLES                      " << rhs.opt_cycles << '\n';
    os << "============================================================================\n";
    os << "DT                              " << rhs.dt << '\n';
    os << "MAX_T                           " << rhs.max_t << '\n';
    os << "REGISTER_DIPOLE_DT              " << rhs.register_dip << '\n';
    os << "============================================================================\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Gauge &rhs) {
    switch (rhs) {
        case Gauge::length:
            os << "length";
            return os;
        case Gauge::velocity:
            os << "velocity";
            return os;
        case Gauge::acceleration:
            os << "acceleration";
            return os;
        default:
            assert(true);
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Representation &rhs) {
    switch (rhs) {
        case Representation::cartesian:
            os << "cartesian";
            return os;
        case Representation::spherical:
            os << "spherical";
            return os;
        default:
            assert(true);
    }
    return os;
}
