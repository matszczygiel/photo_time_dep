#include <chrono>
#include <cmath>
#include <complex>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <eigen3/Eigen/Dense>

#include "basis.h"
#include "constants.h"
#include "control_data.h"
#include "disk_reader.h"
#include "procedures.h"
#include "utils.h"

using namespace std;
using namespace Eigen;

int main(int argc, char* argv[]) {
    const Clock clk;

    if (!(argc == 3 || argc == 2)) {
        cerr << " Proper usage: ./photo <input name> <settings>\n";
        return EXIT_SUCCESS;
    }

    const auto control = Control_data::parse_input_file(argv[1]);

    if (argc == 3 && string(argv[2]) == "-prep") {
        run_preparation(control);

        cout << " Wall time: " << setprecision(5) << fixed << clk << "\n\n";
        return EXIT_SUCCESS;
    }

    cout << " Number of threads being used: " << Eigen::nbThreads() << "\n\n";
    cout << scientific;

    Integrals ints;
    ints.read_from_disk(control);

#ifdef PHOTO_DEBUG
    cout << "S  \n" << ints.S << "\n\n";
    cout << "H  \n" << ints.H << "\n\n";
    cout << "Dx \n" << ints.Dx << "\n\n";
    cout << "Dy \n" << ints.Dy << "\n\n";
    cout << "Dz \n" << ints.Dz << "\n\n";
    cout << "Gx \n" << ints.Gx << "\n\n";
    cout << "Gy \n" << ints.Gy << "\n\n";
    cout << "Gz \n" << ints.Gz << "\n\n";
    cout << "CAP \n" << ints.CAP << "\n\n";
#endif

    const auto U = ints.cut_linear_dependencies();

#ifdef PHOTO_DEBUG
    cout << " Matrices after transformation\n";
    cout << "S  \n" << ints.S << "\n\n";
    cout << "H  \n" << ints.H << "\n\n";
    cout << "Dx \n" << ints.Dx << "\n\n";
    cout << "Dy \n" << ints.Dy << "\n\n";
    cout << "Dz \n" << ints.Dz << "\n\n";
    cout << "Gx \n" << ints.Gx << "\n\n";
    cout << "Gy \n" << ints.Gy << "\n\n";
    cout << "Gz \n" << ints.Gz << "\n\n";
    cout << "CAP \n" << ints.CAP << "\n\n";
#endif

    std::function<Vector3cd(const double&)> compute_filed;
    std::function<MatrixXcd(const double&)> compute_interaction;

    switch (control.gauge) {
        case Gauge::length:
            compute_filed = [&](const double& time) {
                Vector3cd E0 = control.opt_fielddir;
                E0 /= E0.norm();
                E0 *= sqrt(control.opt_intensity / intensity_to_au);
                const double omega = control.opt_omega_eV / au_to_ev;
                const auto& cycles = control.opt_cycles;
                const auto& pcep   = control.opt_carrier_envelope;

                if (time >= cycles * 2 * M_PI / omega)
                    return Vector3cd{0, 0, 0};

                E0 *= sin(omega * time / (2 * cycles)) * sin(omega * time / (2 * cycles)) * sin(omega * time + pcep);
                return E0;
            };

            compute_interaction = [&](const double& time) {
                const auto field = compute_filed(time);
                return field(0) * ints.Dx + field(1) * ints.Dy + field(2) * ints.Dz;
            };
            break;
        case Gauge::velocity:
        case Gauge::velocity_with_Asqrt:
            compute_filed = [&](const double& time) {
                Vector3cd E0 = control.opt_fielddir;
                E0 /= E0.norm();
                E0 *= sqrt(control.opt_intensity / intensity_to_au);
                const double omega = control.opt_omega_eV / au_to_ev;
                const auto& cycles = control.opt_cycles;
                const auto& pcep   = control.opt_carrier_envelope;

                if (time >= cycles * 2 * M_PI / omega)
                    return Vector3cd{0, 0, 0};

                E0 *= -1.0 / (omega * (2.0 - 2.0 / (cycles * cycles))) *
                      (-cos(pcep) / (cycles * cycles) +
                       (-1.0 + 1.0 / (cycles * cycles) + cos(omega * time / cycles)) * cos(omega * time + pcep) +
                       (1.0 / cycles) * sin(omega * time / cycles) * sin(omega * time + pcep));
                return E0;
            };

            switch (control.gauge) {
                case Gauge::velocity:
                    compute_interaction = [&](const double& time) {
                        const auto field = compute_filed(time);
                        return -1.0i * (field(0) * ints.Gx + field(1) * ints.Gy + field(2) * ints.Gz);
                    };
                    break;

                case Gauge::velocity_with_Asqrt:
                    compute_interaction = [&](const double& time) {
                        const auto field = compute_filed(time);
                        return -1.0i * (field(0) * ints.Gx + field(1) * ints.Gy + field(2) * ints.Gz) +
                               ints.S * field.squaredNorm() / 2.0;
                    };
                    break;

                default:
                    break;
            }
            break;

        default:
            throw runtime_error("Currently only length and velocity gauge are supported!");
    }

    cout << " Computing eigenstates of H.\n";

    GeneralizedSelfAdjointEigenSolver<MatrixXcd> es(ints.H, ints.S);
    cout << "   EigenSolver info: ";
    if (check_and_report_eigen_info(cout, es.info())) {
        cerr << "exiting...\n";
        return EXIT_FAILURE;
    }

    //    MatrixXcd LCAO = es.eigenvectors();     //use for full computations
    VectorXcd state = es.eigenvectors().col(0);  // only the ground state
    cout << "   Egenvalues of H matrix:\n"
         << es.eigenvalues().format(IOFormat(StreamPrecision, 0, " ", "\n", "     ", "", "", "")) << "\n\n"
         << std::flush;

    auto compute_dipole_moment = [&]() {
        Vector3d dip;
        //        const VectorXcd state = LCAO.col(0); //use for full computations
        dip(0) = (state.dot(ints.Dx * state)).real();
        dip(1) = (state.dot(ints.Dy * state)).real();
        dip(2) = (state.dot(ints.Dz * state)).real();
        return dip;
    };

    auto compute_norm = [&]() { return sqrt(state.dot(ints.S * state).real()); };

    auto compute_energy = [&]() { return state.dot(ints.H * state).real(); };

    cout << " ================= TIME PROPAGATION =================\n";
    const int steps             = std::round(control.max_t / control.dt);
    const int register_interval = std::round(control.register_dip / control.dt);
    double current_time         = 0.0;

    vector<tuple<double, Vector3d, double, double, double>> res;
    res.reserve(steps / register_interval + 1);
    res.emplace_back(make_tuple(current_time, compute_dipole_moment(), compute_norm(), compute_energy(), 0.0));
    if (control.dump) {
        std::string path = control.dump_path + "/dump-0.dat";
        std::ofstream dump{path};
        if (!dump.is_open())
            throw std::runtime_error("Cannot open dump file: " + path);

        dump << "# t = " << std::scientific << current_time << '\n' << std::setprecision(5) << U * state;
    }

    for (int i = 1; i <= steps; ++i) {
        current_time += control.dt;
        // Remove CAP if you want
        const MatrixXcd H_int = compute_interaction(current_time);
        const MatrixXcd H_t   = ints.H + H_int + ints.CAP;
        const MatrixXcd A     = ints.S + 1i * control.dt / 2.0 * H_t;

        // const MatrixXcd B     = (S - 1i * control.dt / 2.0 * H_t) * LCAO;//use for full
        // computations
        const VectorXcd B = (ints.S - 1i * control.dt / 2.0 * H_t) * state;  // only the ground
                                                                             // state

        // LCAO           = A.partialPivLu().solve(B);//use for full computations
        state = A.partialPivLu().solve(B);  // only the ground state

        const auto dip              = compute_dipole_moment();
        const auto norm             = compute_norm();
        const auto energy           = compute_energy() / norm / norm;
        const auto expectation_Hint = state.dot(H_int * state).real() / norm / norm;

        if (i % register_interval == 0) {
            if (control.dump) {
                std::ofstream dump{control.dump_path + "/dump-" + std::to_string(i) + ".dat"};
                dump << "# t = " << std::scientific << current_time << '\n' << std::setprecision(5) << U * state;
            }
            res.emplace_back(make_tuple(current_time, dip, norm, energy, expectation_Hint));
            cout << " Iteration: " << i << " , time: " << current_time << '\n'
                 << "   dipole moment: " << dip.transpose() << '\n'
                 << "   norm:          " << norm << '\n'
                 << "   energy (<H0>): " << energy << "\n"
                 << "   <Hint>:        " << expectation_Hint << "\n\n"
                 << std::flush;
        }
    }

    cout << " ============= END OF TIME PROPAGATION ==============\n";

    write_result(control, res);

    cout << " Wall time: " << setprecision(5) << fixed << clk << "\n\n";
    return EXIT_SUCCESS;
}
