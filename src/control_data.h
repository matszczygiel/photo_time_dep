#pragma once

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include <eigen3/Eigen/Dense>

enum class Gauge {
    velocity,
    length,
    acceleration
};

class Control_data {
   public:
    std::string job_name{"job"};
    std::string resources_path{};
    std::string file1E{};
    bool write{true};
    std::string out_path{};
    std::string out_file{"res.out"};

    Gauge gauge{Gauge::length};

    double opt_intensity{1.0e14};  //W/cm^2
    Eigen::Vector3d opt_fielddir{0.0, 0.0, 1.0};
    double opt_omega_eV{1.55};
    double opt_carrier_envelope{0.0};
    double opt_cycles{4.0};

    double dt{0.01};
    double max_t{1000};

    static Control_data parse_input_file(std::ifstream &input_file,
                                         const std::string &start_token = "$CONTROL", 
                                         const std::string &end_token = "$END");

   private:
    static std::map<std::string, std::vector<std::string>> read_keys(std::ifstream &input_file,
                                                                     const std::string &start_token, 
                                                                     const std::string &end_token);

};