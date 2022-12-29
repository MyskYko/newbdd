#include "Transduction.h"

using namespace std;

bool Transduction::IsFoConeShared_rec(vector<int> & vVisits, int i, int visitor) const {
  if(vVisits[i] == visitor) {
    return false;
  }
  if(vVisits[i]) {
    return true;
  }
  vVisits[i] = visitor;
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    if(IsFoConeShared_rec(vVisits, vvFos[i][j], visitor)) {
      return true;
    }
  }
  return false;
}
bool Transduction::IsFoConeShared(int i) const {
  vector<int> vVisits(nObjsAlloc);
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    if(IsFoConeShared_rec(vVisits, vvFos[i][j], j + 1)) {
      return true;
    }
  }
  return false;
}

void Transduction::BuildFoConeCompl(int i, vector<NewBdd::Node> & vPoFsCompl) const {
  if(nVerbose > 3) {
    cout << "\t\t\tBuild with complemented " << i << endl;
  }
  vector<NewBdd::Node> vFsCompl = vFs;
  vFsCompl[i] = ~vFs[i];
  vector<bool> vUpdatesCompl(nObjsAlloc);
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    vUpdatesCompl[vvFos[i][j]] = true;
  }
  for(list<int>::const_iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    if(vUpdatesCompl[*it]) {
      NewBdd::Node x = vFsCompl[*it];
      Build(*it, vFsCompl);
      if(x != vFsCompl[*it]) {
        for(unsigned j = 0; j < vvFos[*it].size(); j++) {
          vUpdatesCompl[vvFos[*it][j]] = true;
        }
      }
    }
  }
  for(unsigned j = 0; j < vPos.size(); j++) {
    int i0 = vvFis[vPos[j]][0] >> 1;
    bool c0 = vvFis[vPos[j]][0] & 1;
    vPoFsCompl.push_back(vFsCompl[i0] ^ c0);
  }
}

bool Transduction::MspfCalcG(int i) {
  NewBdd::Node g = vGs[i];
  vector<NewBdd::Node> vPoFsCompl;
  BuildFoConeCompl(i, vPoFsCompl);
  vGs[i] = NewBdd::Const1(bdd);
  for(unsigned j = 0; j < vPos.size(); j++) {
    NewBdd::Node x = ~(vPoFs[j] ^ vPoFsCompl[j]);
    x = x | vvCs[vPos[j]][0];
    vGs[i] = vGs[i] & x;
  }
  return vGs[i] != g;
}

int Transduction::MspfCalcC(int i, int block_i0) {
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    // x = Not(And(other FIs))
    NewBdd::Node x = NewBdd::Const1(bdd);
    for(unsigned jj = 0; jj < vvFis[i].size(); jj++) {
      if(j != jj) {
        int i0 = vvFis[i][jj] >> 1;
        bool c0 = vvFis[i][jj] & 1;
        x = x & (vFs[i0] ^ c0);
      }
    }
    x = ~x;
    // c = Or(x, g[i])
    NewBdd::Node c = x | vGs[i];
    x = NewBdd::Node();
    // Or(c, f[i_j]) == const1 -> redundant
    int i0 = vvFis[i][j] >> 1;
    bool c0 = vvFis[i][j] & 1;
    if(i0 != block_i0 && (c | (vFs[i0] ^ c0)).IsConst1()) {
      if(nVerbose > 4) {
        cout << "\t\t\t\tMspf remove wire " << i0 << "(" << c0 << ")" << " -> " << i << endl;
      }
      Disconnect(i, i0, j);
      return RemoveRedundantFis(i, block_i0, j) + 1;
    } else if(vvCs[i][j] != c) {
      vvCs[i][j] = c;
      vPfUpdates[i0] = true;
    }
  }
  return 0;
}

int Transduction::Mspf(bool fSort, int block, int block_i0) {
  if(nVerbose > 2) {
    cout << "\t\tMspf";
    if(block_i0 != -1) {
      cout << " (block " << block_i0 << " -> " << block << ")";
    } else if(block != -1) {
      cout << " (block " << block << ")";
    }
    cout << endl;
  }
  assert(all_of(vUpdates.begin(), vUpdates.end(), [](bool i) { return !i; }));
  vFoConeShared.resize(nObjsAlloc);
  if(state != PfState::mspf) {
    for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
      vPfUpdates[*it] = true;
    }
    state = PfState::mspf;
  }
  int count = 0;
  for(list<int>::reverse_iterator it = vObjs.rbegin(); it != vObjs.rend();) {
    if(vvFos[*it].empty()) {
      if(nVerbose > 3) {
        cout << "\t\t\tRemove unused " << *it << endl;
      }
      count += RemoveFis(*it);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    if(!vFoConeShared[*it] && !vPfUpdates[*it] && (vvFos[*it].size() == 1 || !IsFoConeShared(*it))) {
      it++;
      continue;
    }
    if(nVerbose > 3) {
      cout << "\t\t\tMspf " << *it << endl;
    }
    if(vvFos[*it].size() == 1 || !IsFoConeShared(*it)) {
      if(vFoConeShared[*it]) {
        vFoConeShared[*it] = false;
        NewBdd::Node g = vGs[*it];
        CalcG(*it);
        if(g == vGs[*it] && !vPfUpdates[*it]) {
          it++;
          continue;
        }
      } else {
        CalcG(*it);
      }
    } else {
      vFoConeShared[*it] = true;
      if(!MspfCalcG(*it) && !vPfUpdates[*it]) {
        it++;
        continue;
      }
    }
    if(fSort && (block != *it || block_i0 != -1)) {
      SortFis(*it);
    }
    if(int diff = (block == *it)? MspfCalcC(*it, block_i0): MspfCalcC(*it)) {
      count += diff;
      assert(!vvFis[*it].empty());
      if(vvFis[*it].size() == 1) {
        count += Replace(*it, vvFis[*it][0]);
        vObjs.erase(--(it.base()));
      }
      Build();
      it = vObjs.rbegin();
      continue;
    }
    vPfUpdates[*it] = false;
    it++;
  }
  for(unsigned j = 0; j < vPis.size(); j++) {
    vPfUpdates[vPis[j]] = false;
  }
  assert(all_of(vUpdates.begin(), vUpdates.end(), [](bool i) { return !i; }));
  assert(all_of(vPfUpdates.begin(), vPfUpdates.end(), [](bool i) { return !i; }));
  return count;
}

bool Transduction::MspfDebug() {
  vector<NewBdd::Node> vGsOld = vGs;
  vector<vector<NewBdd::Node> > vvCsOld = vvCs;
  state = PfState::none;
  Mspf();
  return vGsOld != vGs || vvCsOld != vvCs;
}
