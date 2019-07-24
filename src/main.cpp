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
#include "utils.h"

using namespace std;
using namespace Eigen;

#define PHOTO_DEBUG

int main(int argc, char* argv[]) {
    const auto start = chrono::system_clock::now();

    if (!(argc == 3 || argc == 2)) {
        cout << " Proper usage: ./photo <input name> <settings>\n";
        return EXIT_SUCCESS;
    }

    const string input = argv[1];

    string setting = "n";
    if (argc == 3)
        setting = argv[2];

    ifstream file(input);
    if (!file.is_open())
        throw runtime_error("Invalid input basis file.");

    const auto control = Control_data::parse_input_file(file);
    Basis b;
    b.read(file);
    file.close();

    if (setting == "-prep") {
        const string xgtopw_input_path = control.job_name + ".inp";
        std::ofstream outfile(xgtopw_input_path);
        if (!outfile.is_open())
            throw runtime_error("Cannot open out file!");

        punch_xgtopw_header(outfile);
        outfile << "$BASIS\n"
                << b << "$END\n";
        outfile.close();

        return EXIT_SUCCESS;
    }

    const int basis_length = get_basis_functions_count(b, control.representation);

    Disk_reader reader(basis_length, control.resources_path + "/" + control.file1E);

    auto S  = reader.load_S();
    auto H  = reader.load_H();
    auto Dx = reader.load_Dipx();
    auto Dy = reader.load_Dipy();
    auto Dz = reader.load_Dipz();

#ifdef PHOTO_DEBUG
    cout << "S  \n"
         << S << "\n\n";
    cout << "H  \n"
         << H << "\n\n";
    cout << "Dx \n"
         << Dx << "\n\n";
    cout << "Dy \n"
         << Dy << "\n\n";
    cout << "Dz \n"
         << Dz << "\n\n";
#endif

    MatrixXcd Gx, Gy, Gz;  // gauge integrals
    std::function<Vector3cd(const double&)> compute_filed;

    switch (control.gauge) {
        case Gauge::length:
            Gx            = reader.load_Dipx();
            Gy            = reader.load_Dipy();
            Gz            = reader.load_Dipz();
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
            Gx            = reader.load_Gradx();
            Gy            = reader.load_Grady();
            Gz            = reader.load_Gradz();
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

#ifdef PHOTO_DEBUG
    cout << "Gx \n"
         << Gx << "\n\n";
    cout << "Gy \n"
         << Gy << "\n\n";
    cout << "Gz \n"
         << Gz << "\n\n";
#endif

    cout << " Number of threads being used: " << Eigen::nbThreads() << "\n\n";

    cout << " Computing S matrix eigenvalues.\n";
    SelfAdjointEigenSolver<MatrixXcd> es2;
    es2.compute(S);
    cout << " EigenSolver info: ";
    check_and_report_eigen_info(cout, es2.info());
    cout << " Egenvalues of S matrix:\n"
         << es2.eigenvalues() << "\n\n";

    int vecs_to_cut = 0;
    while (es2.eigenvalues()(vecs_to_cut) < Control_data::s_eigenval_threshold) {
        ++vecs_to_cut;
    }

    cout << " Cutting " + to_string(vecs_to_cut) + " linear dependent vectors.\n";
    MatrixXcd U = es2.eigenvectors().rightCols(basis_length - vecs_to_cut);

#ifdef PHOTO_DEBUG
    cout << " Transformation matrix:\n"
         << U << "\n\n";
#endif

    H  = (U.adjoint() * H * U).eval();
    S  = es2.eigenvalues().tail(basis_length - vecs_to_cut).asDiagonal();
    Dx = (U.adjoint() * Dx * U).eval();
    Dy = (U.adjoint() * Dy * U).eval();
    Dz = (U.adjoint() * Dz * U).eval();
    Gx = (U.adjoint() * Gx * U).eval();
    Gy = (U.adjoint() * Gy * U).eval();
    Gz = (U.adjoint() * Gz * U).eval();

#ifdef PHOTO_DEBUG
    cout << " Matrices after transformation\n";
    cout << "S  \n"
         << S << "\n\n";
    cout << "H  \n"
         << H << "\n\n";
    cout << "Dx \n"
         << Dx << "\n\n";
    cout << "Dy \n"
         << Dy << "\n\n";
    cout << "Dz \n"
         << Dz << "\n\n";
    cout << "Gx \n"
         << Gx << "\n\n";
    cout << "Gy \n"
         << Gy << "\n\n";
    cout << "Gz \n"
         << Gz << "\n\n";
#endif

    GeneralizedSelfAdjointEigenSolver<MatrixXcd> es(H, S);
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
        dip(0) = (state.dot(Dx * state)).real();
        dip(1) = (state.dot(Dy * state)).real();
        dip(2) = (state.dot(Dz * state)).real();
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
        const VectorXcd field = compute_filed(current_time);
        const MatrixXcd H_t   = H + field(0) * Gx + field(1) * Gy + field(2) * Gz;
        const MatrixXcd A     = S + 1i * control.dt / 2.0 * H_t;

        //const MatrixXcd B     = (S - 1i * control.dt / 2.0 * H_t) * LCAO;//use for full computations
        const VectorXcd B = (S - 1i * control.dt / 2.0 * H_t) * state;  //only the ground state

        // LCAO           = A.partialPivLu().solve(B);//use for full computations
        state = A.partialPivLu().solve(B);  //only the ground state

        const auto dip = compute_dipole_moment();
        if (i % register_interval == 0) {
            res.emplace_back(make_pair(current_time, compute_dipole_moment()));
            cout << " Iteration: " << i << " , time: " << current_time << '\n'
                 << " dipole moment: " << dip.transpose() << "\n\n";
        }
    }

    cout << " ============= END OF TIME PROPAGATION ==============\n";

    if (control.write) {
        const string res_path = control.out_path + "/" + control.out_file;

        ofstream outfile(res_path);
        outfile << scientific;
        outfile << control;
        outfile << "           time                 dipx           dipy           dipz\n";
        for (const auto& x : res) {
            outfile << setprecision(5) << setw(15) << x.first << "      ";
            outfile << setw(15) << x.second(0) << setw(15) << x.second(1) << setw(15) << x.second(2) << '\n';
        }

        outfile.close();
    }

    const auto end                                 = chrono::system_clock::now();
    const chrono::duration<double> elapsed_seconds = end - start;
    cout << " Wall time: " << setprecision(5) << fixed << elapsed_seconds.count() << " s\n\n";
    return EXIT_SUCCESS;
}
