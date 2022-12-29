#include <set>
#include <map>

#include "Transduction.h"

using namespace std;

int Transduction::TrivialMergeOne(int i) {
  if(nVerbose > 3) {
    cout << "\t\t\tTrivial merge " << i << endl;
  }
  int count = 0;
  vector<int> vFisOld = vvFis[i];
  vector<NewBdd::Node> vCsOld = vvCs[i];
  vvFis[i].clear();
  vvCs[i].clear();
  for(unsigned j = 0; j < vFisOld.size(); j++) {
    int i0 = vFisOld[j] >> 1;
    int c0 = vFisOld[j] & 1;
    if(vvFis[i0].empty() || vvFos[i0].size() > 1 || c0) {
      if(nVerbose > 5) {
        cout << "\t\t\t\t\tFanin " << j << " : " << i0 << "(" << c0 << ")" << endl;
      }
      vvFis[i].push_back(vFisOld[j]);
      vvCs[i].push_back(vCsOld[j]);
      continue;
    }
    vPfUpdates[i] = vPfUpdates[i] | vPfUpdates[i0];
    vvFos[i0].erase(std::find(vvFos[i0].begin(), vvFos[i0].end(), i));
    count++;
    vector<int>::iterator itfi = vFisOld.begin() + j;
    vector<NewBdd::Node>::iterator itc = vCsOld.begin() + j;
    for(unsigned jj = 0; jj < vvFis[i0].size(); jj++) {
      int f = vvFis[i0][jj];
      vector<int>::iterator it = find(vvFis[i].begin(), vvFis[i].end(), f);
      if(it == vvFis[i].end()) {
        vvFos[f >> 1].push_back(i);
        itfi = vFisOld.insert(itfi, f);
        itc = vCsOld.insert(itc, vvCs[i0][jj]);
        itfi++;
        itc++;
        count--;
      } else {
        assert(state == PfState::none);
      }
    }
    count += RemoveFis(i0, false);
    vObjs.erase(find(vObjs.begin(), vObjs.end(), i0));
    vFisOld.erase(itfi);
    vCsOld.erase(itc);
    j--;
  }
  return count;
}
int Transduction::TrivialMerge() {
  if(nVerbose > 2) {
    cout << "\t\tTrivial merge" << endl;
  }
  int count = 0;
  for(list<int>::reverse_iterator it = vObjs.rbegin(); it != vObjs.rend();) {
    count += TrivialMergeOne(*it);
    it++;
  }
  return count;
}

int Transduction::TrivialDecomposeOne(list<int>::iterator const & it, int & pos) {
  if(nVerbose > 3) {
    cout << "\t\t\tTrivial decompose " << *it << endl;
  }
  assert(vvFis[*it].size() > 2);
  int count = 2 - vvFis[*it].size();
  while(vvFis[*it].size() > 2) {
    int f0 = vvFis[*it].back();
    NewBdd::Node c0 = vvCs[*it].back();
    Disconnect(*it, f0 >> 1, vvFis[*it].size() - 1, false, false);
    int f1 = vvFis[*it].back();
    NewBdd::Node c1 = vvCs[*it].back();
    Disconnect(*it, f1 >> 1, vvFis[*it].size() - 1, false, false);
    CreateNewGate(pos);
    Connect(pos, f1, false, false, c1);
    Connect(pos, f0, false, false, c0);
    if(!vPfUpdates[*it]) {
      if(state == PfState::cspf) {
        vGs[pos] = vGs[*it];
      } else if(state == PfState::mspf) {
        NewBdd::Node x = NewBdd::Const1(bdd);
        for(unsigned j = 0; j < vvFis[*it].size(); j++) {
          int i0 = vvFis[*it][j] >> 1;
          bool c0 = vvFis[*it][j] & 1;
          x = x & (vFs[i0] ^ c0);
        }
        x = ~x;
        vGs[pos] = x | vGs[*it];
      }
    }
    Connect(*it, pos << 1, false, false, vGs[pos]);
    vObjs.insert(it, pos);
    Build(pos, vFs);
  }
  return count;
}
int Transduction::TrivialDecompose() {
  if(nVerbose > 2) {
    cout << "\t\tTrivial decompose" << endl;
  }
  int count = 0;
  int pos = vPis.size() + 1;
  for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    if(vvFis[*it].size() > 2) {
      count += TrivialDecomposeOne(it, pos);
    }
  }
  return count;
}

int Transduction::Merge(bool fMspf) {
  if(nVerbose) {
    cout << "Merge" << endl;
  }
  int count = fMspf? Mspf(true): Cspf(true);
  list<int> targets = vObjs;
  for(list<int>::reverse_iterator it = targets.rbegin(); it != targets.rend(); it++) {
    if(nVerbose > 1) {
      cout << "\tMerge " << *it << endl;
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    count += TrivialMergeOne(*it);
    bool fConnect = false;
    for(unsigned i = 0; i < vPis.size(); i++) {
      int f = vPis[i] << 1;
      if(TryConnect(*it, f) || TryConnect(*it, f ^ 1)) {
        fConnect |= true;
        count--;
      }
    }
    vector<bool> vMarks(nObjsAlloc);
    MarkFoCone_rec(vMarks, *it);
    for(list<int>::iterator it2 = targets.begin(); it2 != targets.end(); it2++) {
      if(!vMarks[*it2] && !vvFos[*it2].empty()) {
        int f = *it2 << 1;
        if(TryConnect(*it, f) || TryConnect(*it, f ^ 1)) {
          fConnect |= true;
          count--;
        }
      }
    }
    if(fConnect) {
      if(fMspf) {
        Build();
        count += Mspf(true, *it);
      } else {
        vPfUpdates[*it] = true;
        count += Cspf(true, *it);
      }
      if(!vvFos[*it].empty()) {
        vPfUpdates[*it] = true;
        count += fMspf? Mspf(true): Cspf(true);
      }
    }
  }
  return count;
}

int Transduction::Decompose() {
  if(nVerbose) {
    cout << "Decompose" << endl;
  }
  int count = 0;
  int pos = vPis.size() + 1;
  for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    set<int> s1(vvFis[*it].begin(), vvFis[*it].end());
    assert(s1.size() == vvFis[*it].size());
    list<int>::iterator it2 = it;
    for(it2++; it2 != vObjs.end(); it2++) {
      set<int> s2(vvFis[*it2].begin(), vvFis[*it2].end());
      set<int> s;
      set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(s, s.begin()));
      if(s.size() > 1) {
        if(s == s1) {
          if(s == s2) {
            if(nVerbose > 1) {
              cout << "\tReplace " << *it2 << " by " << *it << endl;
            }
            count += Replace(*it2, *it << 1, false);
            it2 = vObjs.erase(it2);
            it2--;
          } else {
            if(nVerbose > 1) {
              cout << "\tDecompose " << *it2 << " by " << *it << endl;
            }
            for(set<int>::iterator it3 = s.begin(); it3 != s.end(); it3++) {
              unsigned j = find(vvFis[*it2].begin(), vvFis[*it2].end(), *it3) - vvFis[*it2].begin();
              Disconnect(*it2, *it3 >> 1, j, false);
            }
            count += s.size();
            if(find(vvFis[*it2].begin(), vvFis[*it2].end(), *it << 1) == vvFis[*it2].end()) {
              Connect(*it2, *it << 1, false, false);
              count--;
            }
            vPfUpdates[*it2] = true;
          }
          continue;
        }
        if(s == s2) {
          it = vObjs.insert(it, *it2);
          vObjs.erase(it2);
        } else {
          CreateNewGate(pos);
          if(nVerbose > 1) {
            cout << "\tCreate " << pos << " for intersection of " << *it << " and " << *it2  << endl;
          }
          if(nVerbose > 2) {
            cout << "\t\tIntersection :";
            for(set<int>::iterator it3 = s.begin(); it3 != s.end(); it3++) {
              cout << " " << (*it3 >> 1) << "(" << (*it3 & 1) << ")";
            }
            cout << endl;
          }
          for(set<int>::iterator it3 = s.begin(); it3 != s.end(); it3++) {
            Connect(pos, *it3, false, false);
          }
          count -= s.size();
          it = vObjs.insert(it, pos);
          Build(pos, vFs);
          vPfUpdates[*it] = true;
        }
        s1 = s;
        it2 = it;
      }
    }
    if(vvFis[*it].size() > 2) {
      if(nVerbose > 1) {
        cout << "\tTrivial decompose " << *it << endl;
      }
      count += TrivialDecomposeOne(it, pos);
    }
  }
  return count;
}

int Transduction::MergeDecomposeEager(bool fMspf) {
  int count = Merge(fMspf) + Decompose() + (fMspf? Mspf(true): Cspf(true));
  Save();
  while(true) {
    int diff = Merge(fMspf) + Decompose() + (fMspf? Mspf(true): Cspf(true));
    if(diff <= 0) {
      Load();
      break;
    }
    count += diff;
    Save();
  }
  ClearSave();
  return count;
}
