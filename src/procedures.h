#pragma once

#include <vector>

#include <eigen3/Eigen/Dense>

#include "basis.h"
#include "control_data.h"

inline int get_basis_functions_count(const Control_data& data) {
    switch (data.representation) {
        case Representation::cartesian:
            return data.basis.functions_number_crt();

        case Representation::spherical:
            return data.basis.functions_number_sph();
        default:
            throw std::runtime_error("Unknown representation.");
            return 0;
    }
}

void write_result(const Control_data& control, const std::vector<std::pair<double, Eigen::Vector3d>>& res);

void run_preparation(const Control_data& control);

struct Integrals {
    Eigen::MatrixXcd S{};
    Eigen::MatrixXcd H{};
    Eigen::MatrixXcd Dx{}, Dy{}, Dz{};
    Eigen::MatrixXcd Gx{}, Gy{}, Gz{};
    Eigen::MatrixXcd CAP{};

    void read_from_disk(const Control_data& control);
    void cut_linear_dependencies();
};
