#include <iostream>

#include "Transduction.h"

int main(int argc, char ** argv) {
  if(argc == 1) {
    std::cout << "Specify aig name" << std::endl;
    return 1;
  }
  aigman aig;
  aig.read(argv[1]);
  Transduction t(aig, 4);
  t.Cspf();
  t.Aig(aig);
  aig.write("b.aig");
  return 0;
}
