#include <iostream>
#include <random>
#include <ctime>

#include "Transduction.h"

int main(int argc, char ** argv) {
  if(argc == 1) {
    std::cout << "Specify aig name" << std::endl;
    return 1;
  }
  aigman aig;
  aig.read(argv[1]);
  Transduction t(aig, 0);
  int count = t.CountWires();
  int diff = 0;
  std::srand(std::time(nullptr));
  int N = 100;
  for(int i = 0; i < N; i++) {
    int r = std::rand() % 10;
    std::cout << "Test " << r << " : ";
    switch(r) {
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
      diff += t.Resub(r % 2);
      break;
    case 8:
    case 9:
      diff += t.ResubMono(r % 2);
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
  }
  t.Aig(aig);
  aig.write("b.aig");
  return 0;
}
