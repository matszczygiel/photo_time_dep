
#include "procedures.h"

#include <iomanip>
#include <iostream>
#include <string>

#include "disk_reader.h"
#include "utils.h"

using namespace std;
using namespace Eigen;

void write_result(const Control_data& control, const vector<tuple<double, Vector3d, double, double, double>>& res) {
    if (control.write) {
        const string res_path = control.out_path + "/" + control.out_file;

        ofstream outfile(res_path);
        outfile << scientific;
        outfile << control;
        outfile << "#        time             dipx          dipy          dipz          norm        energy        <Hint>\n";

        for (const auto& x : res) {
            outfile << setprecision(5) << setw(13) << get<0>(x) << "   ";
            outfile << setw(14) << get<1>(x)(0) << setw(14) << get<1>(x)(1) << setw(14) << get<1>(x)(2);
            outfile << setw(14) << get<2>(x);
            outfile << setw(14) << get<3>(x);
            outfile << setw(14) << get<4>(x) << '\n';
        }

        outfile.close();
    }
}

void run_preparation(const Control_data& control) {
    const string xgtopw_input_path = control.job_name + ".inp";
    ofstream outfile(xgtopw_input_path);
    if (!outfile.is_open())
        throw runtime_error("Cannot open out file!");

    punch_xgtopw_header(outfile, control);
    outfile << "$BASIS\n" << control.basis << "$END\n";
    outfile.close();
}

void Integrals::read_from_disk(const Control_data& control) {
    Disk_reader reader(get_basis_functions_count(control), control.resources_path + "/" + control.file1E);

    S   = reader.load_S();
    H   = reader.load_H();
    Dx  = reader.load_Dipx();
    Dy  = reader.load_Dipy();
    Dz  = reader.load_Dipz();
    CAP = reader.load_CAP();
    Gx  = reader.load_Gradx();
    Gy  = reader.load_Grady();
    Gz  = reader.load_Gradz();
}

MatrixXcd Integrals::cut_linear_dependencies() {
    cout << " Cutting linear dependencies: \n"
         << "   Computing S matrix eigenvalues.\n";
    SelfAdjointEigenSolver<MatrixXcd> es;
    es.compute(S);
    cout << "   EigenSolver info: ";
    check_and_report_eigen_info(cout, es.info());
    cout << "   Egenvalues of S matrix:\n"
         << es.eigenvalues().format(IOFormat(StreamPrecision, 0, " ", "\n", "     ", "", "", "")) << "\n\n";

    const double threshold = Control_data::s_eigenval_threshold * es.eigenvalues()(es.eigenvalues().size() - 1);

    int vecs_to_cut = 0;
    while (es.eigenvalues()(vecs_to_cut) < threshold) {
        ++vecs_to_cut;
    }

    cout << "   Cutting " + to_string(vecs_to_cut) + " linear dependent vectors.\n\n";
    MatrixXcd U = es.eigenvectors().rightCols(es.eigenvalues().size() - vecs_to_cut);

#ifdef PHOTO_DEBUG
    cout << "   Transformation matrix:\n" << U << "\n\n";
#endif

    H   = U.adjoint() * H * U;
    S   = es.eigenvalues().tail(es.eigenvalues().size() - vecs_to_cut).asDiagonal();
    Dx  = U.adjoint() * Dx * U;
    Dy  = U.adjoint() * Dy * U;
    Dz  = U.adjoint() * Dz * U;
    Gx  = U.adjoint() * Gx * U;
    Gy  = U.adjoint() * Gy * U;
    Gz  = U.adjoint() * Gz * U;
    CAP = U.adjoint() * CAP * U;

    return U;
}
