#include "supervisor.cpp"
#include "activators.cpp"
#include <iostream>


using namespace std;

int main() {
    Function<double> *s = new Sigmoid<double>(), *l = new Linear<double>();
    Supervisor super(1000, {64, 16, 1}, s, l);
    super.PlayCompetition();
    super.BenchmarkBestRandom();
    delete s; delete l;
}