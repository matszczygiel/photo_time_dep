#include "utils.h"

std::chrono::duration<double> Clock::restart() {
    const auto dur = duration();
    _start         = std::chrono::system_clock::now();
    return dur;
}

std::chrono::duration<double> Clock::duration() const {
    const auto end = std::chrono::system_clock::now();
    return end - _start;
}

std::ostream& operator<<(std::ostream& os, const Clock& rhs) {
    os << rhs.duration().count() << " s";
    return os;
}

void punch_xgtopw_header(std::ofstream& ofs, const Control_data& control) {
    if (!ofs.is_open())
        throw std::runtime_error("GTOPW input file is not open.");

    ofs << "$INTS\n"
        << (control.use_cap ? '4' : '3') << "\nSTVH\nDIPOLE\nVELOCITY\n"
        << (control.use_cap ? "CAPINT\n" : "")
        << "$END\n";
    if (control.use_cap) {
        ofs << "$CAPAR\n"
            << control.cap_r0 << '\n'
            << control.cap_amp << '\n'
            << "$END\n";
    }
    ofs << "$REPRESENTATION\n";
    switch (control.representation) {
        case Representation::spherical:
            ofs << "spherical\n";
            break;
        case Representation::cartesian:
            ofs << "cartesian\n";
            break;
    }
    ofs << "$END\n";
    ofs << "$POINTS\n"
        << "1\n0.000 0.000 0.000\n"
        << "$END\n";
}