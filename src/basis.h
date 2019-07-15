#pragma once

#include <array>
#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using Vec3d   = std::array<double, 3>;
using cdouble = std::complex<double>;

enum class Shell { S = 0,
                   P = 1,
                   D = 2,
                   F = 3,
                   G = 4,
                   H = 5,
                   I = 6,
                   K = 7,
                   L = 8
};

Shell char_to_shell(const char &c);
char shell_to_char(const Shell &shell);
int shell_to_int(const Shell &shell);

struct GTOPW_primitive {
    double exp{0};
    cdouble coef{0};
    Vec3d k{0, 0, 0};

    bool read(std::istream &is);
};

std::ostream &operator<<(std::ostream &os, const GTOPW_primitive &rhs);

struct GTOPW_contraction {
    Shell shl{Shell::S};
    std::vector<GTOPW_primitive> gtopws{};

    bool read(std::istream &is);
    int functions_number_sph() const;
    int functions_number_crt() const;
};

std::ostream &operator<<(std::ostream &os, const GTOPW_contraction &rhs);

struct Atom {
    std::string label{};
    double charge{0.0};
    Vec3d position{0, 0, 0};
    std::vector<GTOPW_contraction> contractions{};

    bool read(std::istream &is, const std::string &end_token = "$END");
    int functions_number_sph() const;
    int functions_number_crt() const;
};

std::ostream &operator<<(std::ostream &os, const Atom &rhs);

struct Basis {
    std::vector<Atom> atoms{};

    bool read(std::istream &is, const std::string &start_token = "$BASIS", const std::string &end_token = "$END");
    int functions_number_sph() const;
    int functions_number_crt() const;
    Shell get_max_shell() const;
    void truncate_at(const Shell &shl);
};

std::ostream &operator<<(std::ostream &os, const Basis &rhs);

void punch_xgtopw_header(std::ofstream &ofs);