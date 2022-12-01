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

int RepeatResub(Transduction & t, std::chrono::steady_clock::time_point const & start, bool fCspfOnly, bool fMore) {
  int count = 0;
  int diff;
  do {
    diff = 0;
    for(int fMspf = 0; fMspf < 2 - (int)fCspfOnly; fMspf++) {
      int diff2;
      do {
        diff2 = t.ResubMono(fMspf);
        diff += diff2;
        Print(t, start, "ResubMono(" + std::to_string(fMspf) + ")");
      } while(diff2);
      do {
        diff2 = t.Resub(fMspf);
        diff += diff2;
        Print(t, start, "Resub(" + std::to_string(fMspf) + ")");
      } while(diff2);
    }
    count += diff;
  } while(fMore && diff);
  return count;
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
  // init
  int nodes = aig.nGates;
  aigman aigout = aig;
  bool fCspfOnly = false;
#ifdef CSPF_ONLY
  fCspfOnly = true;
#endif
  // transduction
  auto start = std::chrono::steady_clock::now();
  for(int fEager = 0; fEager < 2; fEager++) {
    for(int fFirst = 0; fFirst < 2; fFirst++) {
      for(int fMspf = 0; fMspf < 2 - (int)fCspfOnly; fMspf++) {
        for(int fMore = 0; fMore < 2; fMore++) {
          std::cout << "Heuristic : Eager " << fEager << " First " << fFirst << " Mspf " << fMspf << " More " << fMore << std::endl;
          Transduction t(aig);
          int nodes_ = t.CountNodes();
          int count = t.CountWires();
          if(fFirst) {
            count -= fEager? t.MergeDecomposeEager(fMspf): t.Merge(fMspf) + t.Decompose();
            Print(t, start, "MergeDecompose");
          }
          while(true) {
            count -= RepeatResub(t, start, fCspfOnly, fMore);
            if(nodes_ <= t.CountNodes()) {
              break;
            }
            nodes_ = t.CountNodes();
            if(nodes > nodes_) {
              nodes = nodes_;
              t.Aig(aigout);
            }
            count -= fEager? t.MergeDecomposeEager(fMspf): t.Merge(fMspf) + t.Decompose();
            Print(t, start, "MergeDecompose");
          }
          assert(count == t.CountWires());
        }
      }
    }
  }
  // write
  std::cout << nodes << std::endl;
  aigout.write(outname);
  return 0;
}
