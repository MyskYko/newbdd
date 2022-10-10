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
      Disconnect(i, i0, j--);
      count++;
      for(unsigned jj = 0; jj < vvFis[i0].size(); jj++) {
        int f = vvFis[i0][jj];
        if(find(vvFis[i].begin(), vvFis[i].end(), f) == vvFis[i].end()) {
          Connect(i, f);
          count--;
        }
      }
      count += RemoveFis(i0);
      if(fErase) {
        vObjs.erase(find(vObjs.begin(), vObjs.end(), i0));
      }
    }
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
      count += RemoveFis(*it);
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
    Disconnect(*it, f0 >> 1, vvFis[*it].size() - 1);
    int f1 = vvFis[*it].back();
    Disconnect(*it, f1 >> 1, vvFis[*it].size() - 1);
    CreateNewGate(pos);
    Connect(pos, f0);
    Connect(pos, f1);
    Connect(*it, pos << 1);
    vObjs.insert(it, pos);
    Build(pos);
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
      SortFis(*it);
      count += TrivialDecomposeOne(it, pos);
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
    if(nVerbose > 3) {
      cout << "\t\t\tDecompose " << *it << endl;
    }
    set<int> s1(vvFis[*it].begin(), vvFis[*it].end());
    if(s1.size() < vvFis[*it].size()) {
      count += vvFis[*it].size() - s1.size();
      RemoveFis(*it);
      for(set<int>::iterator it3 = s1.begin(); it3 != s1.end(); it3++) {
        Connect(*it, *it3);
      }
    }
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
            count += Replace(*it2, *it << 1);
            it2 = vObjs.erase(it2);
            it2--;
          } else {
            for(set<int>::iterator it3 = s.begin(); it3 != s.end(); it3++) {
              Disconnect(*it2, *it3);
            }
            Connect(*it2, *it << 1);
            count += s.size() - 1;
          }
          continue;
        }
        if(s == s2) {
          it = vObjs.insert(it, *it2);
          vObjs.erase(it2);
        } else {
          CreateNewGate(pos);
          for(set<int>::iterator it3 = s.begin(); it3 != s.end(); it3++) {
            Connect(pos, *it3);
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
