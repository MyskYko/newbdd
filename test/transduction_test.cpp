#include <iostream>

#include "Transduction.h"

int main(int argc, char ** argv) {
  if(argc == 1) {
    std::cout << "Specify aig name" << std::endl;
    return 1;
  }
  aigman aig;
  aig.read(argv[1]);
  Transduction t(aig, 2);
  int count = t.CountWires();
  int diff = 0;
  // diff += t.TrivialMerge();
  // diff += t.TrivialDecompose();
  // diff += t.CspfEager();
  // diff += t.Resub();
  diff += t.ResubMono();
  diff += t.TrivialMerge();
  diff += t.Decompose();
  std::cout << count << " " << diff << " " << t.CountWires() << std::endl;
  if(count - diff != t.CountWires()) {
    std::cout << "Wrong wire count!" << std::endl;
  }
  t.Aig(aig);
  aig.write("b.aig");
  return 0;
}
