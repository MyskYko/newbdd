#include <iostream>
#include <algorithm>
#include <cassert>

#include "Transduction.h"

using namespace std;

Transduction::Transduction(aigman const & aig, int nVerbose) : nVerbose(nVerbose) {
  nObjs = aig.nObjs + aig.nPos;
  vvFis.resize(nObjs);
  vvFos.resize(nObjs);
  for(int i = 0; i < aig.nPis; i++) {
    vPis.push_back(i + 1);
  }
  for(int i = aig.nPis + 1; i < aig.nObjs; i++) {
    for(int ii = i + i;  ii <= i + i + 1; ii++) {
      int i0 = aig.vObjs[ii];
      vvFis[i].push_back(i0);
      vvFos[i0 >> 1].push_back(i);
    }
    vObjs.push_back(i);
  }
  for(int i = 0; i < aig.nPos; i++) {
    int i0 = aig.vPos[i];
    vvFis[i + aig.nObjs].push_back(i0);
    vvFos[i0 >> 1].push_back(i + aig.nObjs);
    vPos.push_back(i + aig.nObjs);
  }

  TrivialMerge();

  bdd = new NewBdd::Man(aig.nPis);
  bdd->SetParameters(0, 12);
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
    if(vvFis[*it].empty()) {
      continue;
    }
    assert(vvFis[*it].size() != 1);
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
        if(nVerbose > 3) {
          cout << "\t\t\tDisconnect " << i0 <<"(" << c0 << ")" << " from " << *it << endl;
        }
        vector<int>::iterator it2 = find(vvFos[i0].begin(), vvFos[i0].end(), *it);
        vvFos[i0].erase(it2);
        vvFis[*it][j] = -1;
        for(unsigned jj = 0; jj < vvFis[i0].size(); jj++) {
          int c = vvFis[i0][jj];
          if(find(vvFis[*it].begin(), vvFis[*it].end(), c) == vvFis[*it].end()) {
            if(nVerbose > 3) {
              cout << "\t\t\tConnect " << (c >> 1) << "(" << (c & 1) << ")" << " to " << *it << endl;
            }
            vvFis[*it].push_back(c);
            vvFos[c >> 1].push_back(*it);
          }
        }
      }
    }
    vector<int> vFisNew;
    for(unsigned j = 0; j < vvFis[*it].size(); j++) {
      if(vvFis[*it][j] != -1) {
        vFisNew.push_back(vvFis[*it][j]);
      }
    }
    vvFis[*it] = vFisNew;
    it++;
  }
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
  return vvFos[i].size();
}
void Transduction::SortFisNode(int i) {
  sort(vvFis[i].begin(), vvFis[i].end(), RankComparator(*this));
}
void Transduction::SortFis() {
  for(int i : vObjs) {
    SortFisNode(i);
  }
}

void Transduction::RemoveFis(int i) {
  if(nVerbose > 3) {
    cout << "\t\t\tRemove node " << i << endl;
  }
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    int i0 = vvFis[i][j] >> 1;
    vector<int>::iterator it = find(vvFos[i0].begin(), vvFos[i0].end(), i);
    assert(it != vvFos[i0].end());
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
  if(nVerbose > 3) {
    cout << "\t\t\tReplace node " << i << " by " << (c >> 1) << "(" << (c & 1) << ")" << endl;
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
  // if ( fRemove && RemoveRedundantFIs( id ) )
  //   {
  //     return;
  //   }
  SortFisNode(i);
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
      if(nVerbose > 3) {
        cout << "\t\t\tRemove wire " << i0 << "(" << c0 << ")" << " -> " << i << endl;
      }
      vector<int>::iterator it = find(vvFos[i0].begin(), vvFos[i0].end(), i);
      vvFos[i0].erase(it);
      vvFis[i].erase(vvFis[i].begin() + j);
      j--;
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
    CalcC(*it);
    if(vvFis[*it].size() == 1) {
      ReplaceNode(*it, vvFis[*it][0]);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    it++;
  }
  Build();
}
