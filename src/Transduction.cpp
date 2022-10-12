#include "Transduction.h"

using namespace std;

Transduction::Transduction(aigman const & aig, int nVerbose) : state(PfState::none), nVerbose(nVerbose) {
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
  // constant
  vFs[0] = bdd->Const0();
  // inputs
  for(int i = 0; i < aig.nPis; i++) {
    vPis.push_back(i + 1);
    vFs[i + 1] = bdd->IthVar(i);
  }
  // nodes
  for(int i = aig.nPis + 1; i < aig.nObjs; i++) {
    if(nVerbose > 3) {
      cout << "\t\t\tImport node " << i << endl;
    }
    for(int ii = i + i;  ii <= i + i + 1; ii++) {
      Connect(i, aig.vObjs[ii]);
    }
    vObjs.push_back(i);
  }
  // outputs
  for(int i = 0; i < aig.nPos; i++) {
    if(nVerbose > 3) {
      cout << "\t\t\tImport po " << i << endl;
    }
    Connect(i + aig.nObjs, aig.vPos[i]);
    vPos.push_back(i + aig.nObjs);
    vvCs[i + aig.nObjs][0] = bdd->Const0();
  }
  // build bdd
  bdd->SetParameters(1, 12);
  bdd->SetOneCounts(true);
  Build();
  bdd->Reorder();
  bdd->SetParameters(1);
  // replace constant outputs
  for(int i = 0; i < aig.nPos; i++) {
    int i0 = aig.vPos[i] >> 1;
    int c0 = aig.vPos[i] & 1;
    if(bdd->IsConst1(bdd->Or(vFs[i0], vvCs[i + aig.nObjs][0]))) {
      Disconnect(i + aig.nObjs, i0, 0, false);
      Connect(i + aig.nObjs, !c0, false, false);
    } else if(bdd->IsConst1(bdd->Or(bdd->Not(vFs[i0]), vvCs[i + aig.nObjs][0]))) {
      Disconnect(i + aig.nObjs, i0, 0, false);
      Connect(i + aig.nObjs, c0, false, false);
    }
  }
}
Transduction::~Transduction() {
  vFs.clear();
  vGs.clear();
  vvCs.clear();
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
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    int i0 = vvFis[i][j] >> 1;
    if(!vMarks[i0]) {
      vMarks[i0] = true;
      MarkFiCone_rec(vMarks, i0);
    }
  }
}
void Transduction::MarkFoCone_rec(vector<bool> & vMarks, int i) const {
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    int k = vvFos[i][j];
    if(!vMarks[k]) {
      vMarks[k] = true;
      MarkFoCone_rec(vMarks, k);
    }
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
  return bdd->OneCount(vFs[i]);
  //return bdd->OneCount(bdd->NotCond(vFs[i], f & 1));
  //return bdd->ZeroCount(vFs[i]);
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
