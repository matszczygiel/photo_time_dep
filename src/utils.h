#pragma once

#include <iostream>

#include <eigen3/Eigen/Core>

#include "control_data.h"
#include "basis.h"

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

inline int get_basis_functions_count(const Basis& b, const Representation& rep) {
    switch (rep) {
        case Representation::cartesian:
            return b.functions_number_crt();

        case Representation::spherical:
            return b.functions_number_sph();
        default:
            throw std::runtime_error("Unknown representation.");
            return 0;
    }
}