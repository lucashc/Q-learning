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
  super->sortPlayersByScore();
  super->players[0]->saveNetwork(filename);
  cout << "Best player saved" << endl;
  exit(signum);
}

int main() {
  	signal(SIGINT, gracefulExit);
    Function<double> *s = new Sigmoid<double>(), *l = new Linear<double>();
    super = new Supervisor(640, {64, 32, 1}, s, l);
    super->evolve(-1);
    delete s; delete l; delete super;
}
