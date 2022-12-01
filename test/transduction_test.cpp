#include <iostream>
#include <chrono>
#include <string>
#include <random>
#include <ctime>

#include "Transduction.h"

void Print(Transduction const & t, std::chrono::steady_clock::time_point const & start, std::string name) {
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << std::left << std::setw(20) << name << " : " << std::right << std::setw(10) << elapsed_seconds.count() << "s ";
  t.PrintStats();
}

int main(int argc, char ** argv) {
  if(argc <= 2) {
    std::cout << "Specify input and output aig names" << std::endl;
    return 1;
  }
  // read
  std::string aigname = argv[1];
  std::string outname = argv[2];
  aigman aig;
  aig.read(aigname);
  // prepare tests
  int N = 100;
  int M = 14;
#ifdef CSPF_ONLY
  N = 10;
  M = 9;
#endif
  std::srand(time(NULL));
  std::vector<int> tests;
  for(int i = 0; i < N; i++) {
    tests.push_back(std::rand() % M);
  }
  std::cout << "Tests = {";
  std::string delim;
  for(unsigned i = 0; i < tests.size(); i++) {
    std::cout << delim << tests[i];
    delim = ", ";
  }
  std::cout << "}" << std::endl;
  // init
  Transduction t(aig);
  int nodes = aig.nGates;
  int count = t.CountWires();
  // transduction
  auto start = std::chrono::steady_clock::now();
  for(unsigned i = 0; i < tests.size(); i++) {
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
      count -= t.Resub();
      break;
    case 6:
      count -= t.ResubMono();
      break;
    case 7:
      count -= t.Merge();
      break;
    case 8:
      count -= t.MergeDecomposeEager();
      break;
    case 9:
      count -= t.Mspf();
      break;
    case 10:
      count -= t.Resub(true);
      break;
    case 11:
      count -= t.ResubMono(true);
      break;
    case 12:
      count -= t.Merge(true);
      break;
    case 13:
      count -= t.MergeDecomposeEager(true);
      break;
    default:
      std::cout << "Wrong test pattern!" << std::endl;
      return 1;
    }
    Print(t, start, "Test " + std::to_string(tests[i]));
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
