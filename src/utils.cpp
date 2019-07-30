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