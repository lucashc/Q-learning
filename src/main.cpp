#include "supervisor.cpp"
#include "activators.cpp"
#include <iostream>
#include <csignal>
#include <string>

using namespace std;


Supervisor *super;

// Save best player
void gracefulExit(int signum) {
  cout << endl << "Shutting down, saving best player..." << endl;
  string filename;
  filename = "EmergencySave" + std::to_string(time(0)) + ".ssvn";
  super->sortPlayers();
  super->players[0]->saveNetwork(filename);
  cout << "Best player saved" << endl;
  exit(signum);
}

int main() {
    Function<double> *s = new Sigmoid<double>(), *l = new Linear<double>();
    super = new Supervisor(1000, {64, 16, 1}, s, l);
    super->playCompetition();
    super->benchmarkBestRandom();
    delete s; delete l; delete super;
}
