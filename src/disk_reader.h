#pragma once

#include <string>

#include <eigen3/Eigen/Dense>

class Disk_reader {
   public:
    Disk_reader(const int &basis_length, const std::string &path);

    Eigen::MatrixXcd load_S() const;
    Eigen::MatrixXcd load_H() const;
    Eigen::MatrixXcd load_Dipx() const;
    Eigen::MatrixXcd load_Dipy() const;
    Eigen::MatrixXcd load_Dipz() const;
    Eigen::MatrixXcd load_Gradx() const;
    Eigen::MatrixXcd load_Grady() const;
    Eigen::MatrixXcd load_Gradz() const;

   protected:
    Eigen::MatrixXcd load_matrix1E_bin(const int &position) const;

   private:
    static constexpr int _matrices1E_number = 20;
    const int _basis_l;
    const std::string _path;
};