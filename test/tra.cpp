#include <argparse/argparse.hpp>

#include "Transduction.h"

using namespace std;

int main(int argc, char **argv) {
  argparse::ArgumentParser ap("tra");
  ap.add_argument("input");
  ap.add_argument("output");
  ap.add_argument("-c", "--cspf").default_value(false).implicit_value(true);
  ap.add_argument("-t", "--transformation").default_value(6).scan<'i', int>();
  ap.add_argument("-s", "--sorttype").default_value(0).scan<'i', int>();
  ap.add_argument("-i", "--pishuffle").default_value(0).scan<'i', int>();
  ap.add_argument("-p", "--parameter").default_value(0).scan<'i', int>();
  ap.add_argument("-r", "--random").default_value(0).scan<'i', int>();
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
  bool fMspf = !ap.get<bool>("--cspf");
  int Transformation = ap.get<int>("--transformation");
  int SortType = ap.get<int>("--sorttype");
  int PiShuffle = ap.get<int>("--pishuffle");
  int Parameter = ap.get<int>("--parameter");
  int Random = ap.get<int>("--random");
  int nVerbose = ap.get<int>("--verbose");
  if(Random) {
    srand(Random);
    SortType = rand() % 4;
    PiShuffle = rand();
    Parameter = rand() % 16;
  }
  aigman aig;
  aig.read(aigname);
  Transduction t(aig, nVerbose, SortType);
  if(PiShuffle) {
    t.ShufflePis(PiShuffle);
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
    count -= t.RepeatResub(false, fMspf);
    break;
  case 5:
    count -= t.RepeatResub(true, fMspf);
    break;
  case 6: {
    bool fFirstMerge = Parameter % 2;
    bool fMspfMerge = fMspf? (Parameter / 2) % 2: false;
    bool fInner = (Parameter / 4) % 2;
    bool fOuter = (Parameter / 8) % 2;
    count -= t.Optimize(fFirstMerge, fMspfMerge, fMspf, fInner, fOuter);
    break;
  }
  default:
    std::cout << "Invalid transformation method" << std::endl;
    return -1;
  }
  if(!t.Verify()) {
    std::cout << "Circuits are not equivalent!" << std::endl;
    return 1;
  }
  if(count != t.CountWires()) {
    std::cout << "Wrong wire count!" << std::endl;
    std::cout << count << " != " << t.CountWires() << std::endl;
    return 1;
  }
  t.Aig(aig);
  aig.write(outname);
  return 0;
}
