#include "basis.h"

#include <map>
#include <sstream>

static const std::map<char, int> shell_charmap = {{'S', 0},
                                                  {'P', 1},
                                                  {'D', 2},
                                                  {'F', 3},
                                                  {'G', 4},
                                                  {'H', 5},
                                                  {'I', 6},
                                                  {'K', 7},
                                                  {'L', 8}};

static constexpr std::array<int, 11> shell_crt_siz = {1, 3, 6, 10, 15, 21, 28, 36, 45, 55};
static constexpr std::array<int, 11> shell_sph_siz = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
static constexpr std::array<char, 11> shell_labels = {'S', 'P', 'D', 'F', 'G', 'H', 'I', 'K', 'L'};

Shell char_to_shell(const char &c) {
    return static_cast<Shell>(shell_charmap.at(c));
}

char shell2char(const Shell &shell) {
    return shell_labels.at(shell_to_int(shell));
}

int shell_to_int(const Shell &shell) {
    return static_cast<int>(shell);
}

bool GTOPW_primitive::read(std::istream &is) {
    std::string line;
    if (!getline(is, line))
        throw std::runtime_error("Invalind gtopw contraction read - check file.");

    std::istringstream ssl(line);
    int gnum;
    ssl >> gnum;

    double re, im;
    ssl >> exp >> re >> im;
    coef = cdouble(re, im);

    ssl >> k[0] >> k[1] >> k[2];
    return true;
}

std::ostream &operator<<(std::ostream &os, const GTOPW_primitive &rhs) {
    constexpr auto spaces = "          ";

    os << std::scientific;
    os.precision(9);
    os << spaces;
    os.width(18);
    os << rhs.exp;
    os << spaces;
    os.width(18);
    os << rhs.coef.real();
    os.width(18);
    os << rhs.coef.imag();
    os << spaces;
    os << std::fixed;
    os.precision(5);
    os.width(10);
    os << rhs.k[0];
    os.width(10);
    os << rhs.k[1];
    os.width(10);
    os << rhs.k[2];
    os << "\n";
    return os;
}

bool GTOPW_contraction::read(std::istream &is) {
    std::string line;
    if (!getline(is, line))
        return false;
    if (line.empty())
        return false;

    std::istringstream ss(line);

    const auto shell = ss.get();
    if (shell == ' ')
        return false;

    shl = char_to_shell(shell);

    int size;
    ss >> size;
    gtopws.resize(size);

    for (int i = 0; i < size; ++i) {
        gtopws[i].read(is);
    }

    return true;
}

int GTOPW_contraction::functions_number_sph() const {
    return shell_sph_siz.at(shell_to_int(shl));
}

int GTOPW_contraction::functions_number_crt() const {
    return shell_crt_siz.at(shell_to_int(shl));
}

std::ostream &operator<<(std::ostream &os, const GTOPW_contraction &rhs) {
    os << shell2char(rhs.shl);
    os.width(3);
    os << rhs.gtopws.size();
    os << "\n";
    std::string spaces = "          ";

    for (int i = 0; i < static_cast<int>(rhs.gtopws.size()); ++i) {
        os.width(3);
        os << i + 1;
        os << rhs.gtopws[i];
    }
    return os;
}

bool Atom::read(std::istream &is, const std::string &end_token) {
    std::string line;
    if (!getline(is, line))
        return false;
    if (line.empty())
        return false;

    std::istringstream ss(line);
    std::string token;
    ss >> token;
    if (token == end_token)
        return false;

    label = token;
    ss >> charge;
    ss >> position[0] >> position[1] >> position[2];

    contractions.clear();
    GTOPW_contraction g;
    while (g.read(is))
        contractions.push_back(g);

    return true;
}

int Atom::functions_number_sph() const {
    int res{0};
    for (const auto &c : contractions) {
        res += c.functions_number_sph();
    }
    return res;
}
int Atom::functions_number_crt() const {
    int res{0};
    for (const auto &c : contractions) {
        res += c.functions_number_crt();
    }
    return res;
}

std::ostream &operator<<(std::ostream &os, const Atom &rhs) {
    constexpr auto spaces = "          ";
    os << rhs.label;
    os.width(7);
    os.precision(2);
    os << std::fixed;
    os << rhs.charge;
    os << spaces;
    os.precision(5);
    os.width(10);
    os << rhs.position[0];
    os.width(10);
    os << rhs.position[1];
    os.width(10);
    os << rhs.position[2];
    os << "\n";

    for (const auto &x : rhs.contractions)
        os << x;

    os << "\n";
    return os;
}

bool Basis::read(std::istream &is, const std::string &start_token, const std::string &end_token) {
    is.seekg(0, std::ios::beg);

    std::string line;
    while (true) {
        if (!getline(is, line))
            return false;
        if (line == start_token)
            break;
    }
    atoms.clear();
    Atom a;
    while (a.read(is, end_token))
        atoms.push_back(a);

    return true;
}
int Basis::functions_number_crt() const {
    int res{0};
    for (const auto &a : atoms) {
        res += a.functions_number_crt();
    }
    return res;
}

int Basis::functions_number_sph() const {
    int res{0};
    for (const auto &a : atoms) {
        res += a.functions_number_sph();
    }
    return res;
}

Shell Basis::get_max_shell() const {
    int max{0};
    Shell max_shl{Shell::S};
    for (const auto &a : atoms)
        for (const auto &c : a.contractions) {
            int shl = shell_to_int(c.shl);
            if (shl > max) {
                max     = shl;
                max_shl = c.shl;
            }
        }
    return max_shl;
}

void Basis::truncate_at(const Shell &shl) {
    int max = shell_to_int(shl);
    for (auto &a : atoms)
        for (auto it = a.contractions.begin(); it != a.contractions.end();) {
            if (shell_to_int(it->shl) > max)
                a.contractions.erase(it);
            else
                it++;
        }
}

std::ostream &operator<<(std::ostream &os, const Basis &rhs) {
    for (const auto &a : rhs.atoms) {
        os << a;
    }
    return os;
}

void punch_xgtopw_header(std::ofstream &ofs) {
    if (!ofs.is_open())
        throw std::runtime_error("GTOPW input file is not open.");

    ofs << "$INTS\n"
        << "4\nSTVH\nDIPOLE\nVELOCITY\nCAPINT\n"
        << "$END\n";
    ofs << "$CAPAR\n"
        << "40.0\n"
        << "5.0\n"
        << "$END\n";
    ofs << "$REPRESENTATION\n"
        << "cartesian\n"
        << "spherical\n"
        << "$END\n";
    ofs << "$POINTS\n"
        << "1\n0.000 0.000 0.000\n"
        << "$END\n";
}