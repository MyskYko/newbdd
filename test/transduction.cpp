#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

#include "Transduction.h"

std::chrono::steady_clock::time_point start;

void Print(Transduction const & t, std::string name) {
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << std::left << std::setw(20) << name << " : " << std::right << std::setw(10) << elapsed_seconds.count() << "s ";
  t.PrintStats();
}

int RepeatResub(Transduction & t, bool fMono, bool fMspf) {
  int count = 0;
  while(int diff = fMono? t.ResubMono(fMspf): t.Resub(fMspf)) {
    count += diff;
    Print(t, (fMono? "ResubMono(": "Resub(") + std::to_string(fMspf) + ")");
  }
  Print(t, (fMono? "ResubMono(": "Resub(") + std::to_string(fMspf) + ")");
  return count;
}

int RepeatResubInner(Transduction & t, bool fMspf, bool fInner) {
  int count = 0;
  while(int diff = RepeatResub(t, true, fMspf) + RepeatResub(t, false, fMspf)) {
    count += diff;
    if(!fInner) {
      break;
    }
  }
  return count;
}

int RepeatResubOuter(Transduction & t, bool fMspfResub, bool fInner, bool fOuter) {
  int count = 0;
  while(int diff = fMspfResub? RepeatResubInner(t, false, fInner) + RepeatResubInner(t, true, fInner): RepeatResubInner(t, false, fInner)) {
    count += diff;
    if(!fOuter) {
      break;
    }
  }
  return count;
}

struct Param {
  int SortType = 0;
  int fEagerMerge = 0;
  int fFirstMerge = 0;
  int fMspfMerge = 0;
  bool fMspfResub = 0;
  int fInner = 0;
  int fOuter = 0;
  void Print() const {
    std::cout << "SortType " << SortType << " ";
    std::cout << "EagerMerge " << fEagerMerge << " ";
    std::cout << "FirstMerge " << fFirstMerge << " ";
    std::cout << "MspfMerge " << fMspfMerge << " ";
    std::cout << "MspfResub " << fMspfResub << " ";
    std::cout << "Inner " << fInner << " ";
    std::cout << "Outer " << fOuter << std::endl;
  }
};

void Run(aigman &aig, Param const &p) {
  p.Print();
  Transduction t(aig, p.SortType);
  int nodes = t.CountNodes();
  int count = t.CountWires();
  if(p.fFirstMerge) {
    count -= p.fEagerMerge? t.MergeDecomposeEager(p.fMspfMerge): t.Merge(p.fMspfMerge) + t.Decompose();
    Print(t, "MergeDecompose");
  }
  count -= RepeatResubOuter(t, p.fMspfResub, p.fInner, p.fOuter);
  if(nodes > t.CountNodes()) {
    nodes = t.CountNodes();
    t.Aig(aig);
    assert(count == t.CountWires());
  }
  while(true) {
    count -= p.fEagerMerge? t.MergeDecomposeEager(p.fMspfMerge): t.Merge(p.fMspfMerge) + t.Decompose();
    Print(t, "MergeDecompose");
    count -= RepeatResubOuter(t, p.fMspfResub, p.fInner, p.fOuter);
    if(nodes > t.CountNodes()) {
      nodes = t.CountNodes();
      t.Aig(aig);
      assert(count == t.CountWires());
    } else {
      break;
    }
  }
}

int main(int argc, char ** argv) {
  if(argc <= 2) {
    std::cout << "Specify input and output aig names" << std::endl;
    return 1;
  }
  // init
  std::string aigname = argv[1];
  std::string outname = argv[2];
  aigman aig;
  aig.read(aigname);
  aigman aigout = aig;
  // transduction
  Param p;
#ifndef CSPF_ONLY
  p.fMspfResub = true;
#endif
  start = std::chrono::steady_clock::now();
  for(p.SortType = 0; p.SortType < 4; p.SortType++)
  for(p.fEagerMerge = 0; p.fEagerMerge < 2; p.fEagerMerge++)
  for(p.fFirstMerge = 0; p.fFirstMerge < 2; p.fFirstMerge++)
#ifndef CSPF_ONLY
  for(p.fMspfMerge = 0; p.fMspfMerge < 2; p.fMspfMerge++)
#endif
  for(p.fInner = 0; p.fInner < 2; p.fInner++)
  for(p.fOuter = 0; p.fOuter < 2; p.fOuter++)
    {
      aigman aigtmp = aig;
      Run(aigtmp, p);
      if(aigtmp.nGates < aigout.nGates) {
        aigout = aigtmp;
      }
    }
  // write
  std::cout << aigout.nGates << std::endl;
  aigout.write(outname);
  return 0;
}
