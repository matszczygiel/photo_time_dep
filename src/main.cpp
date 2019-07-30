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

    Integrals ints;
    ints.read_from_disk(control);

#ifdef PHOTO_DEBUG
    cout << "S  \n"
         << ints.S << "\n\n";
    cout << "H  \n"
         << ints.H << "\n\n";
    cout << "Dx \n"
         << ints.Dx << "\n\n";
    cout << "Dy \n"
         << ints.Dy << "\n\n";
    cout << "Dz \n"
         << ints.Dz << "\n\n";
    cout << "Gx \n"
         << ints.Gx << "\n\n";
    cout << "Gy \n"
         << ints.Gy << "\n\n";
    cout << "Gz \n"
         << ints.Gz << "\n\n";
#endif

    std::function<Vector3cd(const double&)> compute_filed;

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
            break;
        case Gauge::velocity:
            compute_filed = [&](const double& time) {
                Vector3cd E0 = control.opt_fielddir;
                E0 /= E0.norm();
                E0 *= sqrt(control.opt_intensity / intensity_to_au);
                const double omega = control.opt_omega_eV / au_to_ev;
                const auto& cycles = control.opt_cycles;
                const auto& pcep   = control.opt_carrier_envelope;

                if (time >= cycles * 2 * M_PI / omega)
                    return Vector3cd{0, 0, 0};

                E0 *= 1i / (omega * (2.0 - 2.0 / (cycles * cycles))) *
                      (-cos(pcep) / (cycles * cycles) + (-1.0 + 1.0 / (cycles * cycles) + cos(omega * time / cycles)) * cos(omega * time + pcep) + (1.0 / cycles) * sin(omega * time / cycles) * sin(omega * time + pcep));
                return E0;
            };
            break;
        default:
            throw runtime_error("Currently only length and velocity gauge are supported!");
    }
    cout << " Number of threads being used: " << Eigen::nbThreads() << "\n\n";

    ints.cut_linear_dependencies();

#ifdef PHOTO_DEBUG
    cout << " Matrices after transformation\n";
    cout << "S  \n"
         << ints.S << "\n\n";
    cout << "H  \n"
         << ints.H << "\n\n";
    cout << "Dx \n"
         << ints.Dx << "\n\n";
    cout << "Dy \n"
         << ints.Dy << "\n\n";
    cout << "Dz \n"
         << ints.Dz << "\n\n";
    cout << "Gx \n"
         << ints.Gx << "\n\n";
    cout << "Gy \n"
         << ints.Gy << "\n\n";
    cout << "Gz \n"
         << ints.Gz << "\n\n";
#endif

    GeneralizedSelfAdjointEigenSolver<MatrixXcd> es(ints.H, ints.S);
    cout << " EigenSolver info: ";
    if (check_and_report_eigen_info(cout, es.info())) {
        cout << "exiting...\n";
        return EXIT_FAILURE;
    }

    //    MatrixXcd LCAO = es.eigenvectors();     //use for full computations
    VectorXcd state = es.eigenvectors().col(0);  //only the ground state
    cout << " Egenvalues of H matrix:\n"
         << es.eigenvalues() << "\n\n";

    auto compute_dipole_moment = [&]() {
        Vector3d dip;
        //        const VectorXcd state = LCAO.col(0); //use for full computations
        dip(0) = (state.dot(ints.Dx * state)).real();
        dip(1) = (state.dot(ints.Dy * state)).real();
        dip(2) = (state.dot(ints.Dz * state)).real();
        return dip;
    };

    cout << " ================= TIME PROPAGATION =================\n";
    const int steps             = std::round(control.max_t / control.dt);
    const int register_interval = std::round(control.register_dip / control.dt);
    double current_time         = 0.0;

    vector<pair<double, Vector3d>> res;
    res.reserve(steps / register_interval + 1);
    res.emplace_back(make_pair(current_time, compute_dipole_moment()));

    for (int i = 1; i < steps; ++i) {
        current_time += control.dt;
        const Vector3cd field = compute_filed(current_time);
        const MatrixXcd H_t   = ints.H + field(0) * ints.Gx + field(1) * ints.Gy + field(2) * ints.Gz;
        const MatrixXcd A     = ints.S + 1i * control.dt / 2.0 * H_t;

        //const MatrixXcd B     = (S - 1i * control.dt / 2.0 * H_t) * LCAO;//use for full computations
        const VectorXcd B = (ints.S - 1i * control.dt / 2.0 * H_t) * state;  //only the ground state

        // LCAO           = A.partialPivLu().solve(B);//use for full computations
        state = A.partialPivLu().solve(B);  //only the ground state

        const auto dip = compute_dipole_moment();
        if (i % register_interval == 0) {
            res.emplace_back(make_pair(current_time, dip));
            cout << " Iteration: " << i << " , time: " << current_time << '\n'
                 << " dipole moment: " << dip.transpose() << "\n\n";
        }
    }

    cout << " ============= END OF TIME PROPAGATION ==============\n";

    write_result(control, res);

    cout << " Wall time: " << setprecision(5) << fixed << clk << "\n\n";
    return EXIT_SUCCESS;
}
