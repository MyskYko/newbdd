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
  int PiShuffle = ap.get<int>("--pishuffle");
  int nVerbose = ap.get<int>("--verbose");
  aigman aig;
  aig.read(aigname);
  do {
    aigman aigout = aig;
    for(int sorttype = 0; sorttype <= SortType; sorttype++)
    for(int pishuffle = 0; pishuffle <= PiShuffle; pishuffle++) {
      Transduction t(aig, nVerbose, sorttype);
      if(pishuffle) {
        t.ShufflePis(pishuffle);
      }
      int count = t.CountWires();
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
        count -= t.Merge(fMspf) + t.Decompose() + (fMspf? t.Mspf(true): t.Cspf(true));
        break;
      case 4:
        count -= t.RepeatResub(true, fMspf);
        break;
      case 5:
        count -= t.RepeatResub(false, fMspf);
        break;
      case 6: {
        int diff = 0;
        TransductionBackup orig, best;
        t.Save(orig);
        best = orig;
        for(int fFirstMerge = 0; fFirstMerge < 2; fFirstMerge++)
        for(int fMspfMerge = 0; fMspfMerge < 1 + fMspf; fMspfMerge++)
        for(int fInner = 0; fInner < 2; fInner++)
        for(int fOuter = 0; fOuter < 2; fOuter++) {
          t.Load(orig);
          int diff_ = t.Optimize(fFirstMerge, fMspfMerge, fMspf, fInner, fOuter);
          if(diff_ > diff) {
            t.Save(best);
            diff = diff_;
          }
        }
        t.Load(best);
        count -= diff;
        break;
      }
      default:
        std::cout << "Invalid transformation method" << std::endl;
        return -1;
      }
      t.PrintStats();
      if(!t.Verify()) {
        std::cout << "Circuits are not equivalent!" << std::endl;
        return 1;
      }
      if(count != t.CountWires()) {
        std::cout << "Wrong wire count!" << std::endl;
        std::cout << count << " != " << t.CountWires() << std::endl;
        return 1;
      }
      if(t.CountNodes() < aigout.nGates) {
        t.Aig(aigout);
      }
    }
    if(aigout.nGates < aig.nGates) {
      aig = aigout;
    } else {
      break;
    }
  } while(fRepeat);
  aig.write(outname);
  return 0;
}
