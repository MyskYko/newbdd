#include <argparse/argparse.hpp>

#include "Transduction.h"

using namespace std;

int main(int argc, char **argv) {
  argparse::ArgumentParser ap("tra");
  ap.add_argument("input");
  ap.add_argument("output");
  ap.add_argument("-r", "--repeat").default_value(false).implicit_value(true);
  ap.add_argument("-s", "--sorttype").default_value(0).scan<'i', int>();
  ap.add_argument("-t", "--transformation").default_value(0).scan<'i', int>();
  ap.add_argument("-m", "--mspf").default_value(false).implicit_value(true);
  ap.add_argument("-p", "--pishuffle").default_value(0).scan<'i', int>();
  ap.add_argument("-v", "--verbose").default_value(0).scan<'i', int>();
  try {
    ap.parse_args(argc, argv);
  }
  catch (const runtime_error& err) {
    cerr << err.what() << endl;
    cerr << ap;
    return 1;
  }
  string aigname = ap.get<string>("input");
  string outname = ap.get<string>("output");
  bool fRepeat = ap.get<bool>("--repeat");
  int SortType = ap.get<int>("--sorttype");
  int Transformation = ap.get<int>("--transformation");
  bool fMspf = ap.get<bool>("--mspf");
  int nPiShuffle = ap.get<int>("--pishuffle");
  int nVerbose = ap.get<int>("--verbose");
  aigman aig;
  aig.read(aigname);
  Transduction t(aig, nVerbose, SortType);
  if(nPiShuffle) {
    t.ShufflePis(nPiShuffle);
  }
  int count = t.CountWires();
  t.PrintStats();
  if(!fRepeat) {
    switch(Transformation) {
    case 0:
      count -= fMspf? t.Mspf(true): t.Cspf(true);
      break;
    case 1:
      count -= t.Resub(fMspf);
      break;
    case 2:
      count -= t.ResubMono(fMspf);
      break;
    case 3:
      count -= t.Merge(fMspf) + t.Decompose();
      count -= fMspf? t.Mspf(true): t.Cspf(true);
      break;
    default:
      std::cout << "Invalid transformation method" << std::endl;
      return -1;
    }
    t.PrintStats();    
    t.Aig(aig);
  } else {
    while(true) {
      int diff;
      switch(Transformation) {
      case 0:
        diff = fMspf? t.Mspf(true): t.Cspf(true);
        break;
      case 1:
        diff = t.Resub(fMspf);
        break;
      case 2:
        diff = t.ResubMono(fMspf);
        break;
      case 3:
        diff = t.Merge(fMspf) + t.Decompose();
        diff += fMspf? t.Mspf(true): t.Cspf(true);
        break;
      default:
        std::cout << "Invalid transformation method" << std::endl;
        return -1;
      }
      count -= diff;
      t.PrintStats();
      if(diff <= 0) {
        break;
      }
      t.Aig(aig);
    }
  }
  if(!t.Verify()) {
    std::cout << "Circuits are not equivalent!" << std::endl;
    return 1;
  }
  if(count != t.CountWires()) {
    std::cout << "Wrong wire count!" << std::endl;
    return 1;
  }
  aig.write(outname);
  return 0;
}
