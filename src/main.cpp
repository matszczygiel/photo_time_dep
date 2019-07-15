#include <chrono>
#include <fstream>
#include <iostream>
#include <iomanip>


#include "basis.h"
#include "control_data.h"

using namespace std;

int main(int argc, char *argv[]) {
    const auto start = chrono::system_clock::now();

    ifstream file("test.inp");
    if (!file.is_open())
        throw runtime_error("Invalid input basis file.");

    const auto control = Control_data::parse_input_file(file);


    b.read(file);
    file.close();

    b.truncate_at(Shell::F);
    cout << b.functions_number_crt() << '\n';
    cout << b.functions_number_sph() << '\n';

    cout << b << '\n';

    const auto end = chrono::system_clock::now();
    const chrono::duration<double> elapsed_seconds = end - start;
    cout << " Wall time: " << setprecision(5) << fixed << elapsed_seconds.count() << " s\n";
    cout << "=========================================================================="
         << "\n"
         << "\n";

    return EXIT_SUCCESS;
}