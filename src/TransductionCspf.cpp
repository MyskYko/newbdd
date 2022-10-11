#include "Transduction.h"

using namespace std;

int Transduction::RemoveRedundantFis(int i) {
  int count = 0;
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    NewBdd::Node x = bdd->Const1();
    for(unsigned jj = 0; jj < vvFis[i].size(); jj++) {
      if(j != jj) {
        int i0 = vvFis[i][jj] >> 1;
        int c0 = vvFis[i][jj] & 1;
        x = bdd->And(x, bdd->NotCond(vFs[i0], c0));
      }
    }
    x = bdd->Not(x);
    x = bdd->Or(x, vGs[i]);
    int i0 = vvFis[i][j] >> 1;
    int c0 = vvFis[i][j] & 1;
    x = bdd->Or(x, bdd->NotCond(vFs[i0], c0));
    if(bdd->IsConst1(x)) {
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
  vGs[i] = bdd->Const1();
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    int k = vvFos[i][j];
    unsigned l = FindFi(k, i);
    vGs[i] = bdd->And(vGs[i], vvCs[k][l]);
  }
}

int Transduction::CalcC(int i) {
  int count = 0;
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
        cout << "\t\t\t\tRemove wire " << i0 << "(" << c0 << ")" << " -> " << i << endl;
      }
      Disconnect(i, i0, j--);
      count++;
    } else {
      vvCs[i].push_back(c);
    }
  }
  return count;
}

int Transduction::Cspf(int block) {
  if(nVerbose > 2) {
    cout << "\t\tCspf" << endl;
  }
  int count = 0;
  vector<bool> vUpdates(nObjs);
  for(list<int>::reverse_iterator it = vObjs.rbegin(); it != vObjs.rend();) {
    if(nVerbose > 3) {
      cout << "\t\t\tCspf " << *it << endl;
    }
    if(vvFos[*it].empty()) {
      count += RemoveFis(*it);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    CalcG(*it);
    if(*it != block) {
      SortFis(*it);
      if(int diff = RemoveRedundantFis(*it)) {
        count += diff;
        vUpdates[*it] = true;
      }
    }
    if(int diff = CalcC(*it)) {
      count += diff;
      vUpdates[*it] = true;
    }
    assert(!vvFis[*it].empty());
    if(vvFis[*it].size() == 1) {
      for(unsigned j = 0; j < vvFos[*it].size(); j++) {
        vUpdates[vvFos[*it][j]] = true;
      }
      count += Replace(*it, vvFis[*it][0]);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    it++;
  }
  Update(vUpdates);
  return count;
}

int Transduction::CspfEager(int block) {
  if(nVerbose > 2) {
    cout << "\t\tCspf eager" << endl;
  }
  int count = 0;
  while(int diff = Cspf(block)) {
    count += diff;
  }
  return count;
}

int Transduction::CspfFiCone(int i, int block) {
  if(nVerbose > 2) {
    cout << "\t\tCspf fanin cone " << i << endl;
  }
  int count = 0;
  vector<bool> vMarks(nObjs);
  vMarks[i] = true;
  MarkFiCone_rec(vMarks, i);
  for(list<int>::reverse_iterator it = vObjs.rbegin(); it != vObjs.rend();) {
    if(!vMarks[*it]) {
      it++;
      continue;
    }
    if(nVerbose > 3) {
      cout << "\t\t\tCspf " << *it << endl;
    }
    if(vvFos[*it].empty()) {
      count += RemoveFis(*it);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    CalcG(*it);
    if(*it != block) {
      SortFis(*it);
      count += RemoveRedundantFis(*it);
    }
    count += CalcC(*it);
    assert(!vvFis[*it].empty());
    if(vvFis[*it].size() == 1) {
      count += Replace(*it, vvFis[*it][0]);
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
    it++;
  }
  Build();
  return count;
}
