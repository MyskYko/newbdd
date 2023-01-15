#include <argparse/argparse.hpp>

#include "AigBdd.h"

using namespace std;

int main(int argc, char ** argv) {
  argparse::ArgumentParser ap("aig2bdd");
  ap.add_argument("input");
  ap.add_argument("-g", "--garbage_collect").default_value(false).implicit_value(true);
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
  bool fGc = ap.get<bool>("-g");
  bool fReo = ap.get<bool>("-r");
  aigman aig;
  aig.read(aigname);
  aig.supportfanouts();
  NewBdd::Man bdd(aig.nPis);
  bdd.SetParameters(fGc, fReo? 10: -1);
  vector<NewBdd::Node> vNodes;
  Aig2Bdd(aig, bdd, vNodes);
  cout << NewBdd::Node::CountNodes(vNodes) << endl;
  return 0;
}
