#pragma once 

#include <string>

#include <eigen3/Eigen/Dense>

class Disk_reader {
   public:
    Disk_reader(const int &basis_length);

    Eigen::MatrixXcd load_S(const std::string &path) const;
    Eigen::MatrixXcd load_H(const std::string &path) const;
    Eigen::MatrixXcd load_Dipx(const std::string &path) const;
    Eigen::MatrixXcd load_Dipy(const std::string &path) const;
    Eigen::MatrixXcd load_Dipz(const std::string &path) const;
    Eigen::MatrixXcd load_Gradx(const std::string &path) const;
    Eigen::MatrixXcd load_Grady(const std::string &path) const;
    Eigen::MatrixXcd load_Gradz(const std::string &path) const;

    Eigen::VectorXd load_norms(const std::string &path) const;

   protected:
    Eigen::MatrixXcd load_matrix1E_bin(const std::string &path, const int &position) const;

   private:
    static constexpr int _matrices1E_number = 20;
    const int _basis_l;
};