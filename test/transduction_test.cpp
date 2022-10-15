#include <iostream>
#include <string>
#include <random>
#include <ctime>

#include "Transduction.h"

int main(int argc, char ** argv) {
  if(argc == 1) {
    std::cout << "Specify aig name" << std::endl;
    return 1;
  }
  // read
  std::string aigname = argv[1];
  std::string outname = aigname + ".out.aig";
  aigman aig;
  aig.read(aigname);
  // init
  int nodes = aig.nGates;
  int N = 100;
  std::srand(time(NULL));
  std::vector<int> tests;
  for(int i = 0; i < N; i++) {
    tests.push_back(std::rand() % 10);
  }
  std::cout << "Tests = {";
  std::string delim;
  for(unsigned i = 0; i < tests.size(); i++) {
    std::cout << delim << tests[i];
    delim = ", ";
  }
  std::cout << "}" << std::endl;
  // transduction
  Transduction t(aig, 0);
  int count = t.CountWires();
  int diff = 0;
  for(unsigned i = 0; i < tests.size(); i++) {
    std::cout << "Test " << tests[i] << " : ";
    switch(tests[i]) {
    case 0:
      diff += t.TrivialMerge();
      break;
    case 1:
      diff += t.TrivialDecompose();
      break;
    case 2:
      diff += t.Decompose();
      break;
    case 3:
      diff += t.Cspf();
      break;
    case 4:
      diff += t.CspfEager();
      break;
    case 5:
      diff += t.Mspf();
      break;
    case 6:
    case 7:
      diff += t.Resub(tests[i] % 2);
      break;
    case 8:
    case 9:
      diff += t.ResubMono(tests[i] % 2);
      break;
    default:
      return 1;
    }
    t.PrintStats();
    if(!t.Verify()) {
      std::cout << "Circuits are not equivalent!" << std::endl;
      return 1;
    }
    if(count - diff != t.CountWires()) {
      std::cout << "Wrong wire count!" << std::endl;
      return 1;
    }
    if(t.CountNodes() < nodes) {
      nodes = t.CountNodes();
      t.Aig(aig);
    }
  }
  std::cout << "Results : nodes " << nodes << std::endl;
  // write
  aig.write(outname);
  return 0;
}
