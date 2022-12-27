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
  int M = 13;
#ifdef CSPF_ONLY
  N = 10;
  M = 8;
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
      if(t.State() == Transduction::PfState::cspf) {
        assert(!t.CspfDebug());
      } else if(t.State() == Transduction::PfState::mspf) {
        assert(!t.MspfDebug());
      }
      break;
    case 1:
      count -= t.TrivialDecompose();
      if(t.State() == Transduction::PfState::cspf) {
        assert(!t.CspfDebug());
      } else if(t.State() == Transduction::PfState::mspf) {
        assert(!t.MspfDebug());
      }
      break;
    case 2:
      count -= t.Decompose();
      if(t.State() == Transduction::PfState::cspf) {
        count -= t.Cspf(true);
        assert(!t.CspfDebug());
      } else if(t.State() == Transduction::PfState::mspf) {
        count -= t.Mspf();
        assert(!t.MspfDebug());
      }
      break;
    case 3:
      count -= t.Cspf(true);
      assert(!t.CspfDebug());
      break;
    case 4:
      count -= t.Resub();
      assert(!t.CspfDebug());
      break;
    case 5:
      count -= t.ResubMono();
      assert(!t.CspfDebug());
      break;
    case 6:
      count -= t.Merge();
      assert(!t.CspfDebug());
      break;
    case 7:
      count -= t.MergeDecomposeEager();
      assert(!t.CspfDebug());
      break;
    case 8:
      count -= t.Mspf();
      assert(!t.MspfDebug());
      break;
    case 9:
      count -= t.Resub(true);
      assert(!t.MspfDebug());
      break;
    case 10:
      count -= t.ResubMono(true);
      assert(!t.MspfDebug());
      break;
    case 11:
      count -= t.Merge(true);
      assert(!t.MspfDebug());
      break;
    case 12:
      count -= t.MergeDecomposeEager(true);
      assert(!t.MspfDebug());
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
