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
  vector<int> vVisits(nObjs);
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    if(IsFoConeShared_rec(vVisits, vvFos[i][j], j + 1)) {
      return true;
    }
  }
  return false;
}

void Transduction::BuildFoConeCompl(int i, vector<NewBdd::Node> & vPoFsCompl) const {
  if(nVerbose > 2) {
    cout << "\t\tBuild with complemented " << i << endl;
  }
  vector<NewBdd::Node> vFsCompl = vFs;
  vFsCompl[i] = bdd->Not(vFs[i]);
  vector<bool> vMarks(nObjs);
  MarkFoCone_rec(vMarks, i);
  for(list<int>::const_iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    if(vMarks[*it]) {
      Build(*it, vFsCompl);
    }
  }
  for(unsigned j = 0; j < vPos.size(); j++) {
    int i0 = vvFis[vPos[j]][0] >> 1;
    int c0 = vvFis[vPos[j]][0] & 1;
    vPoFsCompl.push_back(bdd->NotCond(vFsCompl[i0], c0));
  }
}

int Transduction::MspfCalcG(int i) {
  if(!IsFoConeShared(i)) {
    return CalcG(i);
  }
  vector<NewBdd::Node> vPoFsCompl;
  BuildFoConeCompl(i, vPoFsCompl);
  vGs[i] = bdd->Const1();
  for(unsigned j = 0; j < vPos.size(); j++) {
    int i0 = vvFis[vPos[j]][0] >> 1;
    int c0 = vvFis[vPos[j]][0] & 1;
    NewBdd::Node x = bdd->Not(bdd->Xor(bdd->NotCond(vFs[i0], c0), vPoFsCompl[j]));
    x = bdd->Or(x, vGs[vPos[j]]);
    vGs[i] = bdd->And(vGs[i], x);
  }
  if(bdd->IsConst1(bdd->Or(vFs[i], vGs[i]))) {
    return Replace(i, 1);
  }
  if(bdd->IsConst1(bdd->Or(bdd->Not(vFs[i]), vGs[i]))) {
    return Replace(i, 0);
  }
  return 0;
}

int Transduction::MspfCalcC(int i) {
  vvCs[i].clear();
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    // x = Not(And(other FIs))
    NewBdd::Node x = bdd->Const1();
    for(unsigned jj = 0; jj < vvFis[i].size(); jj++) {
      if(j != jj) {
        int i0 = vvFis[i][jj] >> 1;
        int c0 = vvFis[i][jj] & 1;
        x = bdd->And(x, bdd->NotCond(vFs[i0], c0));
      }
    }
    x = bdd->Not(x);
    // c = Or(x, g[i])
    NewBdd::Node c = bdd->Or(x, vGs[i]);
    x = NewBdd::Node();
    // Or(c, f[i_j]) == const1 -> redundant
    int i0 = vvFis[i][j] >> 1;
    int c0 = vvFis[i][j] & 1;
    NewBdd::Node f_i_j = bdd->NotCond(vFs[i0], c0);
    if(bdd->IsConst1(bdd->Or(c, f_i_j))) {
      if(nVerbose > 4) {
        cout << "\t\t\t\tRemove wire " << i0 << "(" << c0 << ")" << " -> " << i << endl;
      }
      Disconnect(i, i0, j--);
      return 1;
    } else {
      vvCs[i].push_back(c);
    }
  }
  return 0;
}

int Transduction::Mspf(int block) {
  if(nVerbose > 2) {
    cout << "\t\tMspf" << endl;
  }
  int count = 0;
  for(list<int>::reverse_iterator it = vObjs.rbegin(); it != vObjs.rend();) {
    if(nVerbose > 3) {
      cout << "\t\t\tMspf " << *it << endl;
    }
    if(vvFos[*it].empty()) {
      count += RemoveFis(*it);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    count += MspfCalcG(*it);
    if(vvFis[*it].empty()) {
      assert(vvFos[*it].empty());
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    int diff = MspfCalcC(*it);
    count += diff;
    if(diff) {
      assert(!vvFis[*it].empty());
      if(vvFis[*it].size() == 1) {
        count += Replace(*it, vvFis[*it][0]);
        vObjs.erase(--(it.base()));
      }
      Build();
      it = vObjs.rbegin();
      continue;
    }
    it++;
  }
  return count;
}