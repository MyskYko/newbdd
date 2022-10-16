#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

#include "Transduction.h"

void Print(Transduction const & t, std::chrono::steady_clock::time_point const & start, std::string name) {
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << std::left << std::setw(20) << name << " : " << std::right << std::setw(10) << elapsed_seconds.count() << "s ";
  t.PrintStats();
}

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
  Transduction t(aig, 0);
  int count = t.CountWires();
  auto start = std::chrono::steady_clock::now();
  // transduction
  while(true) {
    count -= t.Merge(true) + t.Decompose();
    Print(t, start, "MergeDecompose");
    for(int i = 0; i < 2; i++) {
      int diff;
      do {
        diff = 0;
        while(true) {
          int diff2 = t.ResubMono(i);
          diff += diff2;
          Print(t, start, "ResubMono(" + std::to_string(i) + ")");
          if(!diff2) {
            break;
          }
        }
        while(true) {
          int diff2 = t.Resub(i);
          diff += diff2;
          Print(t, start, "Resub(" + std::to_string(i) + ")");
          if(!diff2) {
            break;
          }
        }
        count -= diff;
      } while(diff);
    }
    if(nodes > t.CountNodes()) {
      nodes = t.CountNodes();
      t.Aig(aig);
    } else {
      break;
    }
  }
  assert(count == t.CountWires());
  // write
  std::cout << nodes << std::endl;
  aig.write(outname);
  return 0;
}
