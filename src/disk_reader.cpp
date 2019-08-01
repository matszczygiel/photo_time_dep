#include "disk_reader.h"

#include <algorithm>
#include <complex>
#include <fstream>
#include <stdexcept>
#include <vector>

Disk_reader::Disk_reader(const int &basis_length, const std::string &path)
    : _basis_l(basis_length), _path(path) {}

Eigen::MatrixXcd Disk_reader::load_matrix1E_bin(const int &position) const {
    std::ifstream file1E(_path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file1E.is_open())
        throw std::runtime_error("Unable to open 1E file.");

    const auto bl_sqrt = _basis_l * _basis_l;

    const auto size1E      = file1E.tellg();
    const int complex_size = size1E * sizeof(char) / sizeof(double) / 2;

    if (complex_size != bl_sqrt * _matrices1E_number)
        throw std::runtime_error("The size of 1E file does not match the basis size ( " + std::to_string(_basis_l) + " )." +
                                 "Have you used the correct 1E file? its size correspond to basis size " + std::to_string(std::sqrt(complex_size / _matrices1E_number)));

    std::vector<double> real(bl_sqrt);
    std::vector<double> imag(bl_sqrt);

    const int chunk_size = size1E / _matrices1E_number / 2;

    file1E.seekg(2 * position * chunk_size, std::ios::beg);
    file1E.read(reinterpret_cast<char *>(real.data()), chunk_size);
    file1E.read(reinterpret_cast<char *>(imag.data()), chunk_size);
    file1E.close();

    Eigen::MatrixXcd ints(_basis_l, _basis_l);

    assert(real.size() == imag.size());
    std::transform(real.begin(), real.end(), imag.begin(), ints.data(),
                   [](double &dr, double &di) {
                       return std::complex<double>(dr, di);
                   });
    return ints.transpose();
}

Eigen::MatrixXcd Disk_reader::load_S() const {
    return load_matrix1E_bin(0);
}

Eigen::MatrixXcd Disk_reader::load_H() const {
    return load_matrix1E_bin(3);
}

Eigen::MatrixXcd Disk_reader::load_Dipx() const {
    return load_matrix1E_bin(4);
}

Eigen::MatrixXcd Disk_reader::load_Dipy() const {
    return load_matrix1E_bin(5);
}

Eigen::MatrixXcd Disk_reader::load_Dipz() const {
    return load_matrix1E_bin(6);
}

Eigen::MatrixXcd Disk_reader::load_Gradx() const {
    return load_matrix1E_bin(13);
}

Eigen::MatrixXcd Disk_reader::load_Grady() const {
    return load_matrix1E_bin(14);
}

Eigen::MatrixXcd Disk_reader::load_Gradz() const {
    return load_matrix1E_bin(15);
}

Eigen::MatrixXcd Disk_reader::load_CAP() const {
    return -1i * load_matrix1E_bin(19);
}
