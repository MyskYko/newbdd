#include <map>

#include "Transduction.h"

using namespace std;

int Transduction::RemoveRedundantFis(int i, unsigned j, int block_i0) {
  int count = 0;
  for(; j < vvFis[i].size(); j++) {
    if(block_i0 == (vvFis[i][j] >> 1)) {
      continue;
    }
    NewBdd::Node x = NewBdd::Const1(bdd);
    for(unsigned jj = 0; jj < vvFis[i].size(); jj++) {
      if(j != jj) {
        int i0 = vvFis[i][jj] >> 1;
        bool c0 = vvFis[i][jj] & 1;
        x = x & (vFs[i0] ^ c0);
      }
    }
    x = ~x | vGs[i];
    int i0 = vvFis[i][j] >> 1;
    bool c0 = vvFis[i][j] & 1;
    x = x | (vFs[i0] ^ c0);
    if(x.IsConst1()) {
      if(nVerbose > 4) {
        cout << "\t\t\t\tRemove wire " << i0 << "(" << c0 << ")" << " -> " << i << endl;
      }
      Disconnect(i, i0, j--);
      count++;
    }
  }
  return count;
}

void Transduction::CalcG(int i) {
  vGs[i] = NewBdd::Const1(bdd);
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    int k = vvFos[i][j];
    unsigned l = FindFi(k, i);
    vGs[i] = vGs[i] & vvCs[k][l];
  }
}

int Transduction::CalcC(int i) {
  int count = 0;
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    // x = Not(And(FIs with larger rank))
    NewBdd::Node x = NewBdd::Const1(bdd);
    for(unsigned jj = j + 1; jj < vvFis[i].size(); jj++) {
      int i0 = vvFis[i][jj] >> 1;
      bool c0 = vvFis[i][jj] & 1;
      x = x & (vFs[i0] ^ c0);
    }
    x = ~x;
    // x = Or(x, g[i])
    NewBdd::Node c = x | vGs[i];
    x = NewBdd::Node();
    int i0 = vvFis[i][j] >> 1;
    bool c0 = vvFis[i][j] & 1;
    // Or(x, f[i_j]) == const1 -> redundant
    if((c | (vFs[i0] ^ c0)).IsConst1()) {
      if(nVerbose > 4) {
        cout << "\t\t\t\tRemove wire " << i0 << "(" << c0 << ")" << " -> " << i << endl;
      }
      Disconnect(i, i0, j--);
      count++;
    } else if(vvCs[i][j] != c) {
      vvCs[i][j] = c;
      vPfUpdates[i0] = true;
    }
  }
  return count;
}

int Transduction::Cspf(bool fRRF, int block) {
  if(nVerbose > 2) {
    cout << "\t\tCspf" << endl;
  }
  if(state != PfState::cspf) {
    for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
      vPfUpdates[*it] = true;
    }
    state = PfState::cspf;
  }
  int count = 0;
  for(list<int>::reverse_iterator it = vObjs.rbegin(); it != vObjs.rend();) {
    if(nVerbose > 3) {
      cout << "\t\t\tCspf " << *it << endl;
    }
    if(vvFos[*it].empty()) {
      count += RemoveFis(*it);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    if(!vPfUpdates[*it]) {
      it++;
      continue;
    }
    CalcG(*it);
    if(fRRF && *it != block) {
      SortFis(*it);
      count += RemoveRedundantFis(*it);
    }
    count += CalcC(*it);
    vPfUpdates[*it] = false;
    assert(!vvFis[*it].empty());
    if(vvFis[*it].size() == 1) {
      count += Replace(*it, vvFis[*it][0]);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    it++;
  }
  for(unsigned j = 0; j < vPis.size(); j++) {
    vPfUpdates[vPis[j]] = false;
  }
  Build(false);
  assert(all_of(vPfUpdates.begin(), vPfUpdates.end(), [](bool i) { return !i; }));
  return count;
}

bool Transduction::CspfDebug() {
  vector<NewBdd::Node> vGsOld = vGs;
  vector<vector<NewBdd::Node> > vvCsOld = vvCs;
  state = PfState::none;
  Cspf();
  return vGsOld != vGs || vvCsOld != vvCs;
}
