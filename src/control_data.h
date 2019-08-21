#pragma once

#include <fstream>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include <eigen3/Eigen/Dense>

#include "basis.h"

enum class Gauge {
    velocity,
    velocity_with_Asqrt,
    length,
    acceleration
};

std::ostream &operator<<(std::ostream &os, const Gauge &rhs);

enum class Representation {
    cartesian,
    spherical
};

std::ostream &operator<<(std::ostream &os, const Representation &rhs);

class Control_data {
   public:
    std::string job_name{"job"};
    std::string resources_path{};
    std::string file1E{};
    bool write{true};
    std::string out_path{};
    std::string out_file{"res.out"};

    Gauge gauge{Gauge::length};
    Representation representation{Representation::cartesian};

    double opt_intensity{1.0e14};  //W/cm^2
    Eigen::Vector3d opt_fielddir{0.0, 0.0, 1.0};
    double opt_omega_eV{1.55};
    double opt_carrier_envelope{0.0};
    double opt_cycles{4.0};

    bool use_cap{false};
    double cap_r0{40.0};
    double cap_amp{5.0};

    double dt{0.01};
    double max_t{1000};
    double register_dip{1.0};

    Basis basis{};

    constexpr static double s_eigenval_threshold = std::numeric_limits<double>::epsilon();

    static Control_data parse_input_file(const std::string &input_file,
                                         const std::string &start_token = "$CONTROL",
                                         const std::string &end_token   = "$END");

   private:
    static std::map<std::string, std::vector<std::string>> read_keys(std::ifstream &input_file,
                                                                     const std::string &start_token,
                                                                     const std::string &end_token);
};

std::ostream &operator<<(std::ostream &os, const Control_data &rhs);
