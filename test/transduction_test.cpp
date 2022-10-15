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
  // prepare tests
  int N = 100;
  std::srand(time(NULL));
  std::vector<int> tests;
  for(int i = 0; i < N; i++) {
    tests.push_back(std::rand() % 11);
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
  int nodes = aig.nGates;
  int count = t.CountWires();
  for(unsigned i = 0; i < tests.size(); i++) {
    std::cout << "Test " << tests[i] << " : ";
    switch(tests[i]) {
    case 0:
      count -= t.TrivialMerge();
      break;
    case 1:
      count -= t.TrivialDecompose();
      break;
    case 2:
      count -= t.Decompose();
      break;
    case 3:
      count -= t.Cspf();
      break;
    case 4:
      count -= t.CspfEager();
      break;
    case 5:
      count -= t.Mspf();
      break;
    case 6:
    case 7:
      count -= t.Resub(tests[i] % 2);
      break;
    case 8:
    case 9:
      count -= t.ResubMono(tests[i] % 2);
      break;
    case 10:
      count -= t.Merge();
      break;
    default:
      std::cout << "Wrong test pattern!" << std::endl;
      return 1;
    }
    t.PrintStats();
    if(!t.Verify()) {
      std::cout << "Circuits are not equivalent!" << std::endl;
      return 1;
    }
    if(count != t.CountWires()) {
      std::cout << "Wrong wire count!" << std::endl;
      return 1;
    }
    if(t.CountNodes() < nodes) {
      nodes = t.CountNodes();
      t.Aig(aig);
    }
  }
  // write
  std::cout << nodes << std::endl;
  aig.write(outname);
  return 0;
}
