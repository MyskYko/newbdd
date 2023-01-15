#include <chrono>
#include <random>
#include <ctime>

#include <argparse/argparse.hpp>

#include "Transduction.h"

using namespace std;

void Print(Transduction const & t, chrono::steady_clock::time_point const & start, string name) {
  auto end = chrono::steady_clock::now();
  chrono::duration<double> elapsed_seconds = end - start;
  cout << left << setw(20) << name << " : " << right << setw(10) << elapsed_seconds.count() << "s ";
  t.PrintStats();
}

int main(int argc, char ** argv) {
  argparse::ArgumentParser ap("tra_test");
  ap.add_argument("input");
  ap.add_argument("output");
  ap.add_argument("-c", "--cspf").default_value(false).implicit_value(true);
  ap.add_argument("-s", "--sorttype").default_value(0).scan<'i', int>();
  ap.add_argument("-i", "--pishuffle").default_value(0).scan<'i', int>();
  ap.add_argument("-r", "--random").default_value(true).implicit_value(false);
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
  bool fCspf = ap.get<bool>("--cspf");
  int SortType = ap.get<int>("--sorttype");
  int PiShuffle = ap.get<int>("--pishuffle");
  bool fRandom = ap.get<bool>("--random");
  srand(time(NULL));
  if(fRandom) {
    SortType = rand() % 4;
    PiShuffle = rand();
  }
  cout << "SortType = " << SortType << "; PiShuffle = " << PiShuffle << ";" << endl;
  // read
  aigman aig;
  aig.read(aigname);
  // prepare tests
  int N = 100;
  int M = 11;
  if(fCspf) {
    N = 10;
    M = 7;
  }
  vector<int> Tests;
  for(int i = 0; i < N; i++) {
    Tests.push_back(rand() % M);
  }
  cout << "Tests = {";
  string delim;
  for(unsigned i = 0; i < Tests.size(); i++) {
    cout << delim << Tests[i];
    delim = ", ";
  }
  cout << "};" << endl;
  // init
  Transduction t(aig, 0, SortType);
  if(PiShuffle) {
    t.ShufflePis(PiShuffle);
  }
  int nodes = aig.nGates;
  int count = t.CountWires();
  // transduction
  auto start = chrono::steady_clock::now();
  for(unsigned i = 0; i < Tests.size(); i++) {
    switch(Tests[i]) {
    case 0:
      count -= t.TrivialMerge();
      if(t.State() == PfState::cspf) {
        assert(!t.CspfDebug());
      } else if(t.State() == PfState::mspf) {
        assert(!t.MspfDebug());
      }
      break;
    case 1:
      count -= t.TrivialDecompose();
      if(t.State() == PfState::cspf) {
        assert(!t.CspfDebug());
      } else if(t.State() == PfState::mspf) {
        assert(!t.MspfDebug());
      }
      break;
    case 2:
      count -= t.Decompose();
      if(t.State() == PfState::cspf) {
        count -= t.Cspf(true);
        assert(!t.CspfDebug());
      } else if(t.State() == PfState::mspf) {
        count -= t.Mspf();
        assert(!t.MspfDebug());
      }
      break;
    case 3:
      count -= t.Cspf(true);
      assert(!t.CspfDebug());
      break;
    case 4:
      count -= t.Resub();
      assert(!t.CspfDebug());
      break;
    case 5:
      count -= t.ResubMono();
      assert(!t.CspfDebug());
      break;
    case 6:
      count -= t.Merge();
      assert(!t.CspfDebug());
      break;
    case 7:
      count -= t.Mspf(true);
      assert(!t.MspfDebug());
      break;
    case 8:
      count -= t.Resub(true);
      assert(!t.MspfDebug());
      break;
    case 9:
      count -= t.ResubMono(true);
      assert(!t.MspfDebug());
      break;
    case 10:
      count -= t.Merge(true);
      assert(!t.MspfDebug());
      break;
    default:
      cout << "Wrong test pattern!" << endl;
      return 1;
    }
    Print(t, start, "Test " + to_string(Tests[i]));
    if(!t.Verify()) {
      cout << "Circuits are not equivalent!" << endl;
      return 1;
    }
    if(count != t.CountWires()) {
      cout << "Wrong wire count!" << endl;
      return 1;
    }
    if(t.CountNodes() < nodes) {
      nodes = t.CountNodes();
      t.Aig(aig);
    }
  }
  // write
  cout << nodes << endl;
  aig.write(outname);
  return 0;
}
