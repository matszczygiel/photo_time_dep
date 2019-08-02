#pragma once

#include <chrono>
#include <iostream>
#include <fstream>

#include <eigen3/Eigen/Core>

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

class Clock {
   public:
    Clock() = default;
    std::chrono::duration<double> restart();
    std::chrono::duration<double> duration() const;

   private:
    std::chrono::time_point<std::chrono::system_clock> _start{std::chrono::system_clock::now()};
};

std::ostream& operator<<(std::ostream& os, const Clock& rhs);


void punch_xgtopw_header(std::ofstream &ofs, const Control_data& control);
