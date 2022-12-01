#include "Transduction.h"

using namespace std;

Transduction::Transduction(aigman const & aig, int SortType, int nVerbose) : SortType(SortType), state(PfState::none), nVerbose(nVerbose) {
  if(nVerbose > 2) {
    cout << "\t\tImport aig" << endl;
  }
  // allocation
  bdd = new NewBdd::Man(aig.nPis);
  nObjs = aig.nObjs + aig.nPos;
  vvFis.resize(nObjs);
  vvFos.resize(nObjs);
  vFs.resize(nObjs);
  vGs.resize(nObjs);
  vvCs.resize(nObjs);
  vUpdates.resize(nObjs);
  vPfUpdates.resize(nObjs);
  // import
  vector<int> v(aig.nObjs, -1);
  // constant
  vFs[0] = bdd->Const0();
  v[0] = 0;
  // inputs
  for(int i = 0; i < aig.nPis; i++) {
    vPis.push_back(i + 1);
    vFs[i + 1] = bdd->IthVar(i);
    v[i + 1] = (i + 1) << 1;
  }
  // nodes
  for(int i = aig.nPis + 1; i < aig.nObjs; i++) {
    if(nVerbose > 3) {
      cout << "\t\t\tImport node " << i << endl;
    }
    if(aig.vObjs[i + i] == aig.vObjs[i + i + 1]) {
      v[i] = v[aig.vObjs[i + i] >> 1] ^ (aig.vObjs[i + i] & 1);
    } else {
      for(int ii = i + i;  ii <= i + i + 1; ii++) {
        Connect(i, v[aig.vObjs[ii] >> 1] ^ (aig.vObjs[ii] & 1));
      }
      vObjs.push_back(i);
      v[i] = i << 1;
    }
  }
  // outputs
  for(int i = 0; i < aig.nPos; i++) {
    if(nVerbose > 3) {
      cout << "\t\t\tImport po " << i << endl;
    }
    vPos.push_back(i + aig.nObjs);
    Connect(vPos[i], v[aig.vPos[i] >> 1] ^ (aig.vPos[i] & 1));
    vvCs[vPos[i]][0] = bdd->Const0();
  }
  // build bdd
  bdd->SetParameters(1, 12);
  Build();
  bdd->Reorder();
  bdd->SetParameters(1);
  // check and store outputs
  bool fRemoved = false;
  for(unsigned i = 0; i < vPos.size(); i++) {
    int i0 = vvFis[vPos[i]][0] >> 1;
    int c0 = vvFis[vPos[i]][0] & 1;
    NewBdd::Node x = bdd->NotCond(vFs[i0], c0);
    if(i0) {
      if(bdd->IsConst1(bdd->Or(x, vvCs[vPos[i]][0]))) {
        Disconnect(vPos[i], i0, 0, false);
        Connect(vPos[i], 1, false, false);
        x = bdd->Const1();
        fRemoved |= vvFos[i0].empty();
      } else if(bdd->IsConst1(bdd->Or(bdd->Not(x), vvCs[vPos[i]][0]))) {
        Disconnect(vPos[i], i0, 0, false);
        Connect(vPos[i], 0, false, false);
        x = bdd->Const0();
        fRemoved |= vvFos[i0].empty();
      }
    }
    vPoFs.push_back(x);
  }
  // remove unused nodes
  if(fRemoved) {
    for(list<int>::reverse_iterator it = vObjs.rbegin(); it != vObjs.rend();) {
      if(vvFos[*it].empty()) {
        RemoveFis(*it);
        it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
        continue;
      }
      it++;
    }
  }
}
Transduction::~Transduction() {
  vFs.clear();
  vGs.clear();
  vvCs.clear();
  vPoFs.clear();
  delete bdd;
}

void Transduction::Aig(aigman & aig) const {
  aig.clear();
  aig.nPis = vPis.size();
  aig.nObjs = aig.nPis + 1;
  aig.vObjs.resize(aig.nObjs * 2);
  vector<int> values(nObjs);
  for(int i = 0; i < aig.nPis; i++) {
    values[i + 1] = (i + 1) << 1;
  }
  for(list<int>::const_iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    assert(vvFis[*it].size() > 1);
    for(int i = 0; i < 2; i++) {
      int i0 = vvFis[*it][i] >> 1;
      int c0 = vvFis[*it][i] & 1;
      aig.vObjs.push_back(values[i0] ^ c0);
    }
    aig.nGates++;
    int r = aig.nObjs << 1;
    aig.nObjs++;
    for(unsigned i = 2; i < vvFis[*it].size(); i++) {
      aig.vObjs.push_back(r);
      int i0 = vvFis[*it][i] >> 1;
      int c0 = vvFis[*it][i] & 1;
      aig.vObjs.push_back(values[i0] ^ c0);
      aig.nGates++;
      r = aig.nObjs << 1;
      aig.nObjs++;
    }
    values[*it] = r;
  }
  for(unsigned i = 0; i < vPos.size(); i++) {
    int i0 = vvFis[vPos[i]][0] >> 1;
    int c0 = vvFis[vPos[i]][0] & 1;
    aig.vPos.push_back(values[i0] ^ c0);
    aig.nPos++;
  }
}

void Transduction::SortObjs_rec(list<int>::iterator const & it) {
  for(unsigned j = 0; j < vvFis[*it].size(); j++) {
    int i0 = vvFis[*it][j] >> 1;
    if(!vvFis[i0].empty()) {
      list<int>::iterator it_i0 = find(it, vObjs.end(), i0);
      if(it_i0 != vObjs.end()) {
        if(nVerbose > 6) {
          cout << "\t\t\t\t\t\tMove " << i0 << " before " << *it << endl;
        }
        vObjs.erase(it_i0);
        it_i0 = vObjs.insert(it, i0);
        SortObjs_rec(it_i0);
      }
    }
  }
}

void Transduction::MarkFiCone_rec(vector<bool> & vMarks, int i) const {
  if(vMarks[i]) {
    return;
  }
  vMarks[i] = true;
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    MarkFiCone_rec(vMarks, vvFis[i][j] >> 1);
  }
}
void Transduction::MarkFoCone_rec(vector<bool> & vMarks, int i) const {
  if(vMarks[i]) {
    return;
  }
  vMarks[i] = true;
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    MarkFoCone_rec(vMarks, vvFos[i][j]);
  }
}

void Transduction::Build(int i, vector<NewBdd::Node> & vFs_) const {
  if(nVerbose > 3) {
    cout << "\t\t\tBuild " << i << endl;
  }
  vFs_[i] = bdd->Const1();
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    int i0 = vvFis[i][j] >> 1;
    int c0 = vvFis[i][j] & 1;
    vFs_[i] = bdd->And(vFs_[i], bdd->NotCond(vFs_[i0], c0));
  }
}
void Transduction::Build(int i) {
  Build(i, vFs);
}
void Transduction::Build() {
  if(nVerbose > 2) {
    cout << "\t\tBuild" << endl;
  }
  for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    if(vUpdates[*it]) {
      NewBdd::Node x = vFs[*it];
      Build(*it);
      if(x != vFs[*it]) {
        for(unsigned j = 0; j < vvFos[*it].size(); j++) {
          vUpdates[vvFos[*it][j]] = true;
        }
      }
    }
  }
  for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    vPfUpdates[*it] = vPfUpdates[*it] || vUpdates[*it];
    vUpdates[*it] = false;
  }
  for(unsigned j = 0; j < vPos.size(); j++) {
    vUpdates[vPos[j]] = false;
  }
  assert(all_of(vUpdates.begin(), vUpdates.end(), [](bool i) { return !i; }));
}

double Transduction::Rank(int f) const {
  int i = f >> 1;
  switch(SortType) {
#ifdef COUNT_ONES
  case 1:
    return bdd->OneCount(bdd->NotCond(vFs[i], f & 1));
  case 2:
    return bdd->OneCount(vFs[i]);
  case 3:
    return bdd->ZeroCount(vFs[i]);
#endif
  default:
    return -distance(vObjs.begin(), find(vObjs.begin(), vObjs.end(), i));
  }
}
bool Transduction::RankCompare(int a, int b) const {
  int a0 = a >> 1;
  int b0 = b >> 1;
  if(vvFis[a0].empty() && vvFis[b0].empty()) {
    return a0 < b0;
  }
  if(vvFis[a0].empty() && !vvFis[b0].empty()) {
    return false;
  }
  if(!vvFis[a0].empty() && vvFis[b0].empty()) {
    return true;
  }
  if(vvFos[a0].size() > vvFos[b0].size()) {
    return false;
  }
  if(vvFos[a0].size() < vvFos[b0].size()) {
    return true;
  }
  return Rank(a) < Rank(b);
}
bool Transduction::SortFis(int i) {
  if(nVerbose > 4) {
    cout << "\t\t\t\tSort fanins " << i << endl;
  }
  bool fSort = false;
  for(int p = 1; p < (int)vvFis[i].size(); p++) {
    int f = vvFis[i][p];
    NewBdd::Node c = vvCs[i][p];
    int q = p - 1;
    for(; q >= 0 && RankCompare(f, vvFis[i][q]); q--) {
      vvFis[i][q + 1] = vvFis[i][q];
      vvCs[i][q + 1] = vvCs[i][q];
    }
    if(q + 1 != p) {
      fSort = true;
      vvFis[i][q + 1] = f;
      vvCs[i][q + 1] = c;
    }
  }
  if(nVerbose > 5) {
    for(unsigned j = 0; j < vvFis[i].size(); j++) {
      cout << "\t\t\t\t\tFanin " << j << " : " << (vvFis[i][j] >> 1) << "(" << (vvFis[i][j] & 1) << ")" << endl;
    }
  }
  return fSort;
}
