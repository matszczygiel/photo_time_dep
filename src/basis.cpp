#include "basis.h"

#include <map>
#include <sstream>

static constexpr std::map<char, int> shell_charmap = {{'S', 0},
                                                      {'P', 1},
                                                      {'D', 2},
                                                      {'F', 3},
                                                      {'G', 4},
                                                      {'H', 5},
                                                      {'I', 6},
                                                      {'K', 7},
                                                      {'L', 8}};

static constexpr std::array<int, 11> shell_crt_siz = {1, 3, 6, 10, 15, 21, 28, 36, 45, 55};
static constexpr std::array<int, 11> shell_sph_siz = {1, 3, 6, 10, 15, 21, 28, 36, 45, 55};
static constexpr std::array<char, 11> shell_labels  = {'S', 'P', 'D', 'F', 'G', 'H', 'I', 'K', 'L'};


Shell char_to_shell(const char &c) {
    return static_cast<Shell>(shell_charmap.at(c));
}

char shell2char(const Shell &shell) {
    return shell_labels.at(shell_to_int(shell));
}

int shell_to_int(const Shell &shell) {
    return static_cast<int>(shell);
}

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
    int functions_number() const;
};

std::ostream &operator<<(std::ostream &os, const GTOPW_primitive &rhs);

struct Atom {
    std::string label;
    double charge;
    Vec3d position;
    std::vector<GTOPW_contraction> contractions;
};

struct Basis {
    std::vector<Atom> basis{};

    bool read(std::istream &is);
    int functions_number() const;
    Shell get_max_shell() const;
    void truncate_at(const Shell &shl);
};

std::ostream &operator<<(std::ostream &os, const Basis &rhs);



std::ostream &operator<<(std::ostream &os, const GTOPW &rhs) {
    os << shell2char(rhs.shl);
    os.width(3);
    os << rhs.size;
    os << "\n";
    std::string spaces = "          ";

    for (int i = 0; i < rhs.size; ++i) {
        os.width(3);
        os << i + 1;
        os << std::scientific;
        os.precision(9);
        os << spaces;
        os.width(18);
        os << rhs.exps[i];
        os << spaces;
        os.width(18);
        os << rhs.coefs[i].real();
        os.width(18);
        os << rhs.coefs[i].imag();
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
    }
    return os;
}

bool GTOPW::read(std::istream &is) {
    std::string line;
    if (!getline(is, line))
        return false;
    if (line.empty())
        return false;

    std::istringstream ss(line);
    char moment = ss.get();
    shl         = char2shell(moment);
    ss >> size;
    exps.clear();
    coefs.clear();
    exps.reserve(size);
    coefs.reserve(size);

    for (int i = 0; i < size; ++i) {
        if (!getline(is, line))
            throw std::runtime_error("Invalind gtopw contraction read - check file.");
        std::istringstream ssl(line);
        double ex, re, im;
        int gnum;
        ssl >> gnum;
        if (gnum != i + 1)
            throw std::runtime_error("Invalind gtopw contraction read.");

        ssl >> ex >> re >> im;
        exps.push_back(ex);
        coefs.push_back(cdouble(re, im));

        double k0, k1, k2;
        ssl >> k0 >> k1 >> k2;
        if (i == 0)
            k = {k0, k1, k2};
        else if (k0 != k[0] || k1 != k[1] || k2 != k[2])
            throw std::runtime_error("Invalind gtopw contraction read - check k.");
    }

    return true;
}

int GTOPW::functions_number() const {
    return Shell::crt_siz.at(shell2int(shl));
}

bool Basis::read(std::istream &is) {
    Basis();

    std::string line;
    if (!getline(is, line))
        return false;
    if (line.empty())
        return false;

    std::istringstream ss(line);
    std::string token;
    ss >> token;
    if (token == "$END")
        return false;

    label = token;
    ss >> charge;
    ss >> position[0] >> position[1] >> position[2];
    GTOPW g;
    while (g.read(is))
        gtopws.push_back(g);

    return true;
}

std::ostream &operator<<(std::ostream &os, const Basis &rhs) {
    std::string spaces = "          ";
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

    for (const auto &x : rhs.gtopws)
        os << x;

    os << "\n";
    return os;
}

int Basis::functions_number() const {
    int num = 0;
    for (const auto &x : gtopws)
        num += x.functions_number();

    return num;
}

Shell Basis::get_max_shell() const {
    int max = 0;
    for (auto it = gtopws.begin(); it != gtopws.end(); it++) {
        auto shl = shell2int(it->get_shell());
        if (shl > max)
            max = shl;
    }
    return Shell(max);
}

void Basis::truncate_at(const Shell &shl) {
    for (auto it = gtopws.begin(); it != gtopws.end();) {
        if (shell2int(it->get_shell()) > shell2int(shl))
            gtopws.erase(it);
        else
            it++;
    }
}

void Basis::set_kvec(const Vec3d &kvec) {
    for (auto &x : gtopws)
        x.set_kvec(kvec);
}

void punch_xgtopw_header(std::ofstream &ofs) {
    if (!ofs.is_open())
        throw std::runtime_error("GTOPW input file is not open.");

    ofs << "$INTS\n";
    //    ofs << "4\nSTVH\nDIPOLE\nVELOCITY\nERI\n";
    ofs << "3\nSTVH\nDIPOLE\nVELOCITY\n";
    ofs << "$END\n";
    ofs << "$POINTS\n";
    ofs << "1\n0.000 0.000 0.000\n";
    ofs << "$END\n";
    ofs << "$BASIS\n";
}

void punch_gms_ion_header(std::ofstream &ofs) {
    if (!ofs.is_open())
        throw std::runtime_error("Gamess input file is not open.");

    ofs << " $CONTRL";
    ofs << " SCFTYP=UHF RUNTYP=ENERGY ICHARG=1 MAXIT=200 MULT=2\n  COORD=UNIQUE EXETYP=RUN ICUT=12 ISPHER=-1 QMTTOL=1e-8\n  UNITS=BOHR NPRINT=0";
    ofs << " $END\n";
    ofs << " $SCF";
    ofs << " FDIFF=.F. CONV=1.0D-8 NPUNCH=0 DIIS=.T.\n  SOSCF=.F. DIRSCF=.F. DAMP=.F. NOCONV=.F. EXTRAP=.F.";
    ofs << " $END\n";
    ofs << " $SYSTEM";
    ofs << " MWORDS=200 KDIAG=0";
    ofs << " $END\n";
    ofs << " $TRANS";
    ofs << " CUTTRF=1.0D-14";
    ofs << " $END\n";
    ofs << " $GUESS";
    ofs << " GUESS=HCORE";
    ofs << " $END\n";
    ofs << " $DATA\n";
    ofs << " Title\n";
    ofs << " C1\n";
}

void punch_gms_neutral_header_one_electron(std::ofstream &ofs) {
    if (!ofs.is_open())
        throw std::runtime_error("Gamess input file is not open.");

    ofs << " $CONTRL";
    ofs << " SCFTYP=UHF RUNTYP=ENERGY ICHARG=0 MAXIT=200 MULT=2\n  COORD=UNIQUE EXETYP=RUN ICUT=12 ISPHER=-1 QMTTOL=1e-8\n  UNITS=BOHR NPRINT=0";
    ofs << " $END\n";
    ofs << " $SCF";
    ofs << " FDIFF=.F. CONV=1.0D-8 NPUNCH=0 DIIS=.T.\n  SOSCF=.F. DIRSCF=.F. DAMP=.F. NOCONV=.F. EXTRAP=.F.";
    ofs << " $END\n";
    ofs << " $SYSTEM";
    ofs << " MWORDS=200 KDIAG=0";
    ofs << " $END\n";
    ofs << " $TRANS";
    ofs << " CUTTRF=1.0D-14";
    ofs << " $END\n";
    ofs << " $GUESS";
    ofs << " GUESS=HCORE";
    ofs << " $END\n";
    ofs << " $DATA\n";
    ofs << " Title\n";
    ofs << " C1\n";
}

void punch_gms_neutral_header(std::ofstream &ofs, const int &active_orbs_ci) {
    if (!ofs.is_open())
        throw std::runtime_error("Gamess input file is not open.");

    ofs << " $CONTRL";
    ofs << " SCFTYP=RHF RUNTYP=ENERGY ICHARG=0 MAXIT=200 MULT=1\n  COORD=UNIQUE EXETYP=RUN ICUT=12 ISPHER=-1 QMTTOL=1e-8\n  UNITS=BOHR NPRINT=0 CITYP=ALDET";
    ofs << " $END\n";
    ofs << " $SCF";
    ofs << " FDIFF=.F. CONV=1.0D-8 NPUNCH=0 DIIS=.T.\n  SOSCF=.F. DIRSCF=.F. DAMP=.F. NOCONV=.F. EXTRAP=.F.";
    ofs << " $END\n";
    ofs << " $SYSTEM";
    ofs << " MWORDS=200 KDIAG=0";
    ofs << " $END\n";
    ofs << " $TRANS";
    ofs << " CUTTRF=1.0D-14";
    ofs << " $END\n";
    ofs << " $GUESS";
    ofs << " GUESS=HCORE";
    ofs << " $END\n";
    ofs << " $CIDET";
    ofs << " NCORE=0 NACT=" << active_orbs_ci << " NELS=2 SZ=0 NSTATE=1 PRTTOL=0.000000000001";
    ofs << " $END\n";
    ofs << " $CIDRT";
    ofs << " NFZC=0 NDOC=1 NVAL=25 IEXCIT=2";
    ofs << " $END\n";
    ofs << " $DATA\n";
    ofs << " Title\n";
    ofs << " C1\n";
}