#include <set>
#include <map>

#include "Transduction.h"

using namespace std;

int Transduction::TrivialMergeOne(int i, bool fErase) {
  if(nVerbose > 3) {
    cout << "\t\t\tTrivial merge " << i << endl;
  }
  int count = 0;
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    int i0 = vvFis[i][j] >> 1;
    int c0 = vvFis[i][j] & 1;
    if(!vvFis[i0].empty() && vvFos[i0].size() == 1 && !c0) {
      Disconnect(i, i0, j--, false);
      count++;
      for(unsigned jj = 0; jj < vvFis[i0].size(); jj++) {
        int f = vvFis[i0][jj];
        vector<int>::iterator it = find(vvFis[i].begin(), vvFis[i].end(), f);
        if(it == vvFis[i].end()) {
          Connect(i, f, false, false, vvCs[i0][jj]);
          count--;
        } else {
          unsigned l = it - vvFis[i].begin();
          if(state == PfState::cspf && vvCs[i][l].Valid() && vvCs[i0][jj].Valid()) {
            vvCs[i][l] = bdd->And(vvCs[i][l], vvCs[i0][jj]);
          } else {
            vvCs[i][l] = NewBdd::Node();
          }
        }
      }
      count += RemoveFis(i0, false);
      if(fErase) {
        vObjs.erase(find(vObjs.begin(), vObjs.end(), i0));
      }
    }
  }
  if(state != PfState::mspf) {
    vPfUpdates[i] = true;
  }
  return count;
}
int Transduction::TrivialMerge() {
  if(nVerbose > 2) {
    cout << "\t\tTrivial merge" << endl;
  }
  int count = 0;
  for(list<int>::reverse_iterator it = vObjs.rbegin(); it != vObjs.rend();) {
    if(vvFos[*it].empty()) {
      assert(vvFis[*it].empty());
      it = list<int>::reverse_iterator(vObjs.erase(--(it.base())));
      continue;
    }
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
    Connect(pos, f0, false, false, c0);
    Connect(pos, f1, false, false, c1);
    Connect(*it, pos << 1, false, false);
    vObjs.insert(it, pos);
    Build(pos);
  }
  vPfUpdates[*it] = true;
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
      SortFis(*it);
      count += TrivialDecomposeOne(it, pos);
    }
  }
  return count;
}

int Transduction::Merge(bool fMspf) {
  if(nVerbose) {
    cout << "Merge" << endl;
  }
  int count = fMspf? Mspf(): CspfEager();
  list<int> targets = vObjs;
  for(list<int>::reverse_iterator it = targets.rbegin(); it != targets.rend(); it++) {
    if(nVerbose > 1) {
      cout << "\tMerge " << *it << endl;
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    count += TrivialMergeOne(*it, true);
    if(!fMspf) {
      count += CspfEager();
    }
    bool fConnect = false;
    for(unsigned i = 0; i < vPis.size(); i++) {
      int f = vPis[i] << 1;
      if(TryConnect(*it, f) || TryConnect(*it, f ^ 1)) {
        fConnect |= true;
        count--;
      }
    }
    vector<bool> vMarks(nObjs);
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
      Build();
      count += fMspf? Mspf(): CspfEager();
    }
  }
  return count;
}

int Transduction::Decompose() {
  if(nVerbose > 2) {
    cout << "\t\tDecompose" << endl;
  }
  int count = 0;
  int pos = vPis.size() + 1;
  for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    set<int> s1(vvFis[*it].begin(), vvFis[*it].end());
    if(s1.size() < vvFis[*it].size()) {
      count += vvFis[*it].size() - s1.size();
      for(unsigned j = 0; j < vvFis[*it].size(); j++) {
        for(unsigned jj = j + 1; jj < vvFis[*it].size(); jj++) {
          if(vvFis[*it][j] == vvFis[*it][jj]) {
            Disconnect(*it, vvFis[*it][jj] >> 1, jj, false);
            jj--;
          }
        }
      }
      assert(s1.size() == vvFis[*it].size());
      assert(vPfUpdates[*it]);
    }
  }
  for(list<int>::iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    if(nVerbose > 3) {
      cout << "\t\t\tDecompose " << *it << endl;
    }
    set<int> s1(vvFis[*it].begin(), vvFis[*it].end());
    list<int>::iterator it2 = it;
    for(it2++; it2 != vObjs.end(); it2++) {
      if(nVerbose > 4) {
        cout << "\t\t\t\tDecompose " << *it2 << " by " << *it << endl;
      }
      set<int> s2(vvFis[*it2].begin(), vvFis[*it2].end());
      set<int> s;
      set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(s, s.begin()));
      if(s.size() > 1) {
        if(nVerbose > 5) {
          cout << "\t\t\t\t\tIntersection";
          for(set<int>::iterator it3 = s.begin(); it3 != s.end(); it3++) {
            cout << " " << (*it3 >> 1) << "(" << (*it3 & 1) << ")";
          }
          cout << endl;
        }
        if(s == s1) {
          if(s == s2) {
            count += Replace(*it2, *it << 1, false);
            it2 = vObjs.erase(it2);
            it2--;
          } else {
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
          for(set<int>::iterator it3 = s.begin(); it3 != s.end(); it3++) {
            Connect(pos, *it3, false, false);
          }
          count -= s.size();
          it = vObjs.insert(it, pos);
          Build(pos);
        }
        if(nVerbose > 3) {
          cout << "\t\t\tDecompose switch to " << *it << endl;
        }
        s1 = s;
        it2 = it;
      }
    }
    if(vvFis[*it].size() > 2) {
      SortFis(*it);
      count += TrivialDecomposeOne(it, pos);
    }
  }
  return count;
}

int Transduction::MergeDecomposeEager(bool fMspf) {
  int count = Merge(fMspf) + Decompose() + (fMspf? Mspf(): CspfEager());
  Save();
  while(true) {
    int diff = Merge(fMspf) + Decompose() + (fMspf? Mspf(): CspfEager());
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
