#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <eigen3/Eigen/Dense>

#include "basis.h"
#include "control_data.h"
#include "disk_reader.h"
#include "constants.h"

using namespace std;
using namespace Eigen;

int main(int argc, char *argv[]) {
    if (!(argc == 3 || argc == 2)) {
        cout << " Proper usage: ./photo <input name> <settings>\n";
        return EXIT_SUCCESS;
    }

    const auto start = chrono::system_clock::now();

    const string input = argv[1];
    string setting     = "n";
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
        const string xgtopw_input_path = control.out_path + "/" + control.job_name + ".inp";
        std::ofstream outfile(xgtopw_input_path);
        if (!outfile.is_open())
            throw runtime_error("Cannot open out file!");

        punch_xgtopw_header(outfile);
        outfile << "$BASIS\n"
                << b << "$END\n";
        outfile.close();

        return EXIT_SUCCESS;
    }

    int basis_length = 0;
    switch (control.representation) {
        case Representation::cartesian:
            basis_length = b.functions_number_crt();
            break;

        case Representation::spherical:
            basis_length = b.functions_number_sph();
            break;
    }

    Disk_reader reader(basis_length, control.resources_path + "/" + control.file1E);

    const double opt_energy = control.opt_omega_eV / au_to_ev;
    MatrixXcd Dx, Dy, Dz;  // gauge integrals

    switch (control.gauge) {
        case Gauge::length:
            Dx = reader.load_Dipx();
            Dy = reader.load_Dipy();
            Dz = reader.load_Dipz();

            break;
        case Gauge::velocity:
            Dx = reader.load_Gradx() / opt_energy;
            Dy = reader.load_Grady() / opt_energy;
            Dz = reader.load_Gradz() / opt_energy;
        default:
            throw runtime_error("Currently only length and velocity gauge are supported!");
    }

    const auto S = reader.load_S();
    const auto H = reader.load_H();

    const auto end                                 = chrono::system_clock::now();
    const chrono::duration<double> elapsed_seconds = end - start;
    cout << " Wall time: " << setprecision(5) << fixed << elapsed_seconds.count() << " s\n";
    cout << "=========================================================================="
         << "\n"
         << "\n";
    return EXIT_SUCCESS;
}