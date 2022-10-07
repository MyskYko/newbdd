#include <iostream>
#include <algorithm>
#include <cassert>

#include "Transduction.h"

using namespace std;

Transduction::Transduction(aigman const & aig, int nVerbose) : nVerbose(nVerbose) {
  if(nVerbose > 2) {
    cout << "\t\tImport aig" << endl;
  }
  nObjs = aig.nObjs + aig.nPos;
  vvFis.resize(nObjs);
  vvFos.resize(nObjs);
  for(int i = 0; i < aig.nPis; i++) {
    vPis.push_back(i + 1);
  }
  for(int i = aig.nPis + 1; i < aig.nObjs; i++) {
    if(nVerbose > 3) {
      cout << "\t\t\tImport aig node " << i << endl;
    }
    for(int ii = i + i;  ii <= i + i + 1; ii++) {
      Connect(i, aig.vObjs[ii]);
    }
    vObjs.push_back(i);
  }
  for(int i = 0; i < aig.nPos; i++) {
    if(nVerbose > 3) {
      cout << "\t\t\tImport aig po " << i << endl;
    }
    Connect(i + aig.nObjs, aig.vPos[i]);
    vPos.push_back(i + aig.nObjs);
  }

  bdd = new NewBdd::Man(aig.nPis);
  bdd->SetParameters(0, 12);
  bdd->SetOneCounts(true);
  vFs.resize(nObjs);
  vFs[0] = bdd->Const0();
  for(int i = 0; i < aig.nPis; i++) {
    vFs[i + 1] = bdd->IthVar(i);
  }
  Build();
  bdd->Reorder();
  bdd->SetParameters(1);

  vGs.resize(nObjs);
  vvCs.resize(nObjs);
  for(int i = 0; i < aig.nPos; i++) {
    vGs[i + aig.nObjs] = bdd->Const0();
    vvCs[i + aig.nObjs].push_back(bdd->Const0());
  }

  /*  
  #include "AigBdd.h"
  vector<NewBdd::Node> vNodes;
  for(unsigned i = 0; i < vPos.size(); i++) {
    int i0 = vPos[i] >> 1;
    if(vPos[i] & 1) {
      vNodes.push_back(bdd->Not(vFs[i0]));
    } else {
      vNodes.push_back(vFs[i0]);
    }
  }
  aigman aig2;
  Bdd2Aig(*bdd, aig2, vNodes);
  aig2.write("c.aig");
  */
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

void Transduction::SortObjs(list<int>::iterator const & it) {
  for(unsigned j = 0; j < vvFis[*it].size(); j++) {
    int i0 = vvFis[*it][j] >> 1;
    if(vvFis[i0].empty()) {
      continue;
    }
    list<int>::iterator it_i0 = find(it, vObjs.end(), i0);
    if(it_i0 != vObjs.end()) {
      if(nVerbose > 6) {
        cout << "\t\t\t\t\t\tmove " << i0 << " before " << *it << endl;
      }
      vObjs.erase(it_i0);
      it_i0 = vObjs.insert(it, i0);
      SortObjs(it_i0);
    }
  }
}

void Transduction::Connect(int i, int f, bool fSort) {
  int i0 = f >> 1;
  if(nVerbose > 5) {
    cout << "\t\t\t\t\tConnect " << i0 << " to " << i << endl;
  }
  vvFis[i].push_back(f);
  vvFos[i0].push_back(i);
  if(fSort && !vvFos[i].empty() && !vvFis[i0].empty()) {
    list<int>::iterator it = find(vObjs.begin(), vObjs.end(), i);
    list<int>::iterator it_i0 = find(it, vObjs.end(), i0);
    if(it_i0 != vObjs.end()) {
      if(nVerbose > 6) {
        cout << "\t\t\t\t\t\tmove " << i0 << " before " << *it << endl;
      }
      vObjs.erase(it_i0);
      it_i0 = vObjs.insert(it, i0);
      SortObjs(it_i0);
    }
  }
}

void Transduction::Disconnect(int i, int i0, unsigned j) {
  if(nVerbose > 5) {
    cout << "\t\t\t\t\tDisconnect " << i0 << " from " << i << endl;
  }
  vector<int>::iterator it = find(vvFos[i0].begin(), vvFos[i0].end(), i);
  vvFos[i0].erase(it);
  vvFis[i].erase(vvFis[i].begin() + j);
}


void Transduction::RemoveFis(int i) {
  if(nVerbose > 4) {
    cout << "\t\t\t\tRemove node " << i << endl;
  }
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    int i0 = vvFis[i][j] >> 1;
    vector<int>::iterator it = find(vvFos[i0].begin(), vvFos[i0].end(), i);
    vvFos[i0].erase(it);
  }
  vvFis[i].clear();
}
int Transduction::FindFi(int i, int i0) const {
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    if((vvFis[i][j] >> 1) == i0) {
      return j;
    }
  }
  abort();
}
void Transduction::ReplaceNode(int i, int c) {
  if(nVerbose > 4) {
    cout << "\t\t\t\tReplace node " << i << " by " << (c >> 1) << endl;
  }
  assert(i != (c >> 1));
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    int k = vvFos[i][j];
    int l = FindFi(k, i);
    vvFis[k][l] = c ^ (vvFis[k][l] & 1);
    vvFos[c >> 1].push_back(k);
  }
  vvFos[i].clear();
  RemoveFis(i);
}

void Transduction::TrivialMerge() {
  if(nVerbose > 2) {
    cout << "\t\tTrivial merge" << endl;
  }
  for(list<int>::reverse_iterator it = vObjs.rbegin(); it != vObjs.rend();) {
    if(nVerbose > 3) {
      cout << "\t\t\tTrivial merge node " << *it << endl;
    }
    if(vvFos[*it].empty()) {
      RemoveFis(*it);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    for(unsigned j = 0; j < vvFis[*it].size(); j++) {
      int i0 = vvFis[*it][j] >> 1;
      int c0 = vvFis[*it][j] & 1;
      if(!vvFis[i0].empty() && vvFos[i0].size() == 1 && !c0) {
        Disconnect(*it, i0, j--);
        for(unsigned jj = 0; jj < vvFis[i0].size(); jj++) {
          int f = vvFis[i0][jj];
          if(find(vvFis[*it].begin(), vvFis[*it].end(), f) == vvFis[*it].end()) {
            Connect(*it, f);
          }
        }
      }
    }
    it++;
  }
}

void Transduction::TrivialDecompose() {
  if(nVerbose > 2) {
    cout << "\t\tTrivial decompose" << endl;
  }
  int pos = vPis.size() + 1;
  for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    if(nVerbose > 3) {
      cout << "\t\t\tTrivial decompose node " << *it << endl;
    }
    if(vvFis[*it].size() > 2) {
      int f0 = vvFis[*it].back();
      Disconnect(*it, f0 >> 1, vvFis[*it].size() - 1);
      int f1 = vvFis[*it].back();
      Disconnect(*it, f1 >> 1, vvFis[*it].size() - 1);
      while(!vvFis[pos].empty() || !vvFos[pos].empty()) {
        pos++;
        if(pos == nObjs) {
          nObjs++;
          vvFis.resize(nObjs);
          vvFis.resize(nObjs);
          vFs.resize(nObjs);
          vGs.resize(nObjs);
          vvCs.resize(nObjs);
          break;
        }
      }
      vObjs.insert(it, pos);
      Connect(pos, f0);
      Connect(pos, f1);
      Connect(*it, pos << 1);
    }
  }
  Build();
}

void Transduction::BuildNode(int i, vector<NewBdd::Node> & vFs_) {
  if(nVerbose > 3) {
    cout << "\t\t\tBuild node " << i << endl;
  }
  vFs_[i] = bdd->Const1();
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    int i0 = vvFis[i][j] >> 1;
    int c0 = vvFis[i][j] & 1;
    vFs_[i] = bdd->And(vFs_[i], bdd->NotCond(vFs_[i0], c0));
  }
}
void Transduction::Build() {
  if(nVerbose > 2) {
    cout << "\t\tBuild" << endl;
  }
  for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    BuildNode(*it, vFs);
  }
}

double Transduction::Rank(int i) const {
  if(vvFis[i].empty()) {
    return numeric_limits<double>::max();
  }
  assert(vPis.size() + ceil(log2(vvFos[i].size())) < 1024);
  double a = pow(2.0, vPis.size()) * vvFos[i].size();
  double b = bdd->OneCount(vFs[i]);
  //double b = bdd->ZeroCount(vFs[i]);
  assert(abs(b) < numeric_limits<double>::max() - abs(a));
  return a + b;
}
void Transduction::SortFisNode(int i) {
  sort(vvFis[i].begin(), vvFis[i].end(), RankComparator(*this));
  if(nVerbose > 4) {
    cout << "\t\t\t\tSort fanins node " << i << endl;
    for(unsigned j = 0; j < vvFis[i].size(); j++) {
      cout << "\t\t\t\t\tFanin " << j << " node " << (vvFis[i][j] >> 1) << " rank " << Rank(vvFis[i][j] >> 1) << endl;
    }
  }
}
void Transduction::SortFis() {
  for(int i : vObjs) {
    SortFisNode(i);
  }
}

void Transduction::RemoveRedundantFis(int i) {
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    NewBdd::Node x = bdd->Const1();
    for(unsigned jj = 0; jj < vvFis[i].size(); jj++) {
      if(j == jj) {
        continue;
      }
      int i0 = vvFis[i][jj] >> 1;
      int c0 = vvFis[i][jj] & 1;
      x = bdd->And(x, bdd->NotCond(vFs[i0], c0));
    }
    x = bdd->Not(x);
    x = bdd->Or(x, vGs[i]);
    int i0 = vvFis[i][j] >> 1;
    int c0 = vvFis[i][j] & 1;
    x = bdd->Or(x, bdd->NotCond(vFs[i0], c0));
    if(bdd->IsConst1(x)) {
      if(nVerbose > 4) {
        cout << "\t\t\t\tRemove wire " << i0 << " -> " << i << endl;
      }
      Disconnect(i, i0, j--);
    }
  }
}

void Transduction::CalcG(int i) {
  vGs[i] = bdd->Const1();
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    int k = vvFos[i][j];
    int l = FindFi(k, i);
    vGs[i] = bdd->And(vGs[i], vvCs[k][l]);
  }
  if(bdd->IsConst1(bdd->Or(vFs[i], vGs[i]))) {
    ReplaceNode(i, 1);
  } else if(bdd->IsConst1(bdd->Or(bdd->Not(vFs[i]), vGs[i]))) {
    ReplaceNode(i, 0);
  }
}
void Transduction::CalcC(int i) {
  SortFisNode(i);
  RemoveRedundantFis(i);
  vvCs[i].clear();
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    // x = Not(And(FIs with larger rank))
    NewBdd::Node x = bdd->Const1();
    for(unsigned jj = j + 1; jj < vvFis[i].size(); jj++) {
      int i0 = vvFis[i][jj] >> 1;
      int c0 = vvFis[i][jj] & 1;
      x = bdd->And(x, bdd->NotCond(vFs[i0], c0));
    }
    x = bdd->Not(x);
    // y = And(Not(f[i]), f[i_j])
    int i0 = vvFis[i][j] >> 1;
    int c0 = vvFis[i][j] & 1;
    NewBdd::Node f_i_j = bdd->NotCond(vFs[i0], c0);
    NewBdd::Node y = bdd->And(bdd->Not(vFs[i]), f_i_j);
    // c = Or(x, y, g[i])
    NewBdd::Node c = bdd->Or(x, y);
    x = y = NewBdd::Node();
    c = bdd->Or(c, vGs[i]);
    // Or(c, f[i_j]) == const1 -> redundant
    if(bdd->IsConst1(bdd->Or(c, f_i_j))) {
      if(nVerbose > 4) {
        cout << "\t\t\t\tRemove wire " << i0 << " -> " << i << endl;
      }
      Disconnect(i, i0, j--);
    } else {
      vvCs[i].push_back(c);
    }
  }
}

void Transduction::Cspf() {
  if(nVerbose > 2) {
    cout << "\t\tCspf" << endl;
  }
  for(list<int>::reverse_iterator it = vObjs.rbegin(); it != vObjs.rend();) {
    if(nVerbose > 3) {
      cout << "\t\t\tCspf node " << *it << endl;
    }
    if(vvFos[*it].empty()) {
      RemoveFis(*it);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    CalcG(*it);
    if(vvFis[*it].empty()) {
      assert(vvFos[*it].empty());
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    CalcC(*it);
    assert(!vvFis[*it].empty());
    if(vvFis[*it].size() == 1) {
      ReplaceNode(*it, vvFis[*it][0]);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    it++;
  }
  Build();
}

bool Transduction::TryConnect(int i, int f) {
  if(find(vvFis[i].begin(), vvFis[i].end(), f) == vvFis[i].end()) {
    NewBdd::Node x = bdd->Or(bdd->Not(vFs[i]), vGs[i]);
    x = bdd->Or(x, bdd->NotCond(vFs[f >> 1], f & 1));
    if(bdd->IsConst1(x)) {
      Connect(i, f, true);
      return true;
    }
  }
  return false;
}

void Transduction::MarkFoCone_rec(int i) {
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    int k = vvFos[i][j];
    if(!vMarks[k]) {
      vMarks[k] = true;
      MarkFoCone_rec(k);
    }
  }
}

void Transduction::Resub() {
  if(nVerbose) {
    cout << "Resubstitution" << endl;
  }
  list<int> targets = vObjs;
  for(list<int>::reverse_iterator it = targets.rbegin(); it != targets.rend(); it++) {
    if(nVerbose > 1) {
      cout << "\tResubstitute node " << *it << endl;
      cout << "\t aigs " << CountNodes() << " ands " << CountGates() << " wires " << CountWires() << endl;
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    vMarks.clear();
    vMarks.resize(nObjs);
    vMarks[*it] = true;
    MarkFoCone_rec(*it);
    for(unsigned i = 0; i < vPis.size(); i++) {
      int f = vPis[i] << 1;
      if(!TryConnect(*it, f)) {
        TryConnect(*it, f ^ 1);
      }
    }
    for(list<int>::iterator it2 = targets.begin(); it2 != targets.end(); it2++) {
      if(vMarks[*it2] || vvFos[*it2].empty()) {
        continue;
      }
      int f = *it2 << 1;
      if(!TryConnect(*it, f)) {
        TryConnect(*it, f ^ 1);
      }
    }
    Cspf();
    //return;
    //CspfFiCone();
  }
}

int Transduction::CountGates() {
  return vObjs.size();
}

int Transduction::CountWires() {
  int count = 0;
  for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    count += vvFis[*it].size();
  }
  return count;
}

int Transduction::CountNodes() {
  return CountWires() - CountGates();
}
