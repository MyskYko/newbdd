#include <argparse/argparse.hpp>

#include "AigBdd.h"

using namespace std;

int main(int argc, char ** argv) {
  argparse::ArgumentParser ap("aig2bdd");
  ap.add_argument("input");
  ap.add_argument("-v", "--verbose").default_value(0).scan<'i', int>();
  ap.add_argument("-g", "--garbage_collect").default_value(0).scan<'i', int>();
  ap.add_argument("-r", "--reorder").default_value(false).implicit_value(true);
  try {
    ap.parse_args(argc, argv);
  }
  catch (const runtime_error& err) {
    cerr << err.what() << endl;
    cerr << ap;
    return 1;
  }
  string aigname = ap.get<string>("input");
  int nVerbose = ap.get<int>("-v");
  int nGbc = ap.get<int>("-g");
  bool fReo = ap.get<bool>("-r");
  aigman aig;
  aig.read(aigname);
  aig.supportfanouts();
  NewBdd::Man bdd(aig.nPis, 0, nVerbose);
  bdd.SetParameters(nGbc, fReo? 10: -1);
  vector<NewBdd::Node> vNodes;
  Aig2Bdd(aig, bdd, vNodes);
  cout << NewBdd::Node::CountNodes(vNodes) << endl;
  return 0;
}
