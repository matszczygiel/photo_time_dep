#include <fstream>
#include <iostream>

#include "basis.h"

using namespace std;

int main() {
    Basis b;
    ifstream file("test.inp");
    if (!file.is_open())
        throw runtime_error("Invalid input basis file.");

    b.read(file);
    file.close();

    ofstream file_w("test_wrt.inp");

    file_w << "$BASIS\n";
    file_w << b;
    file_w << "$END\n";

    file_w.close();
}