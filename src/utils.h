#pragma once

#include <iostream>

#include <eigen3/Eigen/Core>

#include "basis.h"
#include "control_data.h"

inline bool check_and_report_eigen_info(std::ostream& os, const Eigen::ComputationInfo& info) {
    switch (info) {
        case Eigen::ComputationInfo::NumericalIssue:
            os << " The provided data did not satisfy the prerequisites.\n";
            return true;
        case Eigen::ComputationInfo::NoConvergence:
            os << " Iterative procedure did not converge.\n";
            return true;
        case Eigen::ComputationInfo::InvalidInput:
            os << " The inputs are invalid, or the algorithm has been improperly called."
               << "When assertions are enabled, such errors trigger an assert.\n";
            return true;
        case Eigen::ComputationInfo::Success:
            os << " Success\n";
            return false;
    }
    return false;
}

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

inline void write_result(const Control_data& control, const std::vector<std::pair<double, Eigen::Vector3d>>& res) {
    if (control.write) {
        const std::string res_path = control.out_path + "/" + control.out_file;

        std::ofstream outfile(res_path);
        outfile << std::scientific;
        outfile << control;
        outfile << "           time                 dipx           dipy           dipz\n";
        for (const auto& x : res) {
            outfile << std::setprecision(5) << std::setw(15) << x.first << "      ";
            outfile << std::setw(15) << x.second(0) << std::setw(15) << x.second(1) << std::setw(15) << x.second(2) << '\n';
        }

        outfile.close();
    }
}