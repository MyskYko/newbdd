#include <iostream>
#include <string>
#include <random>

#include "Transduction.h"

int main(int argc, char ** argv) {
  if(argc == 1) {
    std::cout << "Specify aig name" << std::endl;
    return 1;
  }
  std::string aigname = argv[1];
  std::string outname = aigname + ".out.aig";
  aigman aig;
  aig.read(aigname);
  int nodes = aig.nGates;
  std::srand(123);
  int N = 100;
  int M = 10;
  for(int j = 0; j < M; j++) {
    Transduction t(aig, 0);
    int count = t.CountWires();
    int diff = 0;
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
      if(t.CountNodes() < nodes) {
        nodes = t.CountNodes();
        aigman aig2;
        t.Aig(aig2);
        aig2.write(outname);
      }
    }
  }
  return 0;
}
