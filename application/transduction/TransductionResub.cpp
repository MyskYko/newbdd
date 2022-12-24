#include <iostream>
#include <algorithm>
#include <cassert>

#include "Transduction.h"

using namespace std;

bool Transduction::TryConnect(int i, int f) {
  if(find(vvFis[i].begin(), vvFis[i].end(), f) == vvFis[i].end()) {
    int i0 = f >> 1;
    bool c0 = f & 1;
    NewBdd::Node x = ~vFs[i] | vGs[i] | (vFs[i0] ^ c0);
    if(x.IsConst1()) {
      Connect(i, f, true);
      return true;
    }
  }
  return false;
}

int Transduction::Resub(bool fMspf) {
  if(nVerbose) {
    cout << "Resubstitution" << endl;
  }
  int count = fMspf? Mspf(): Cspf();
  int nodes = CountNodes();
  Save();
  list<int> targets = vObjs;
  for(list<int>::reverse_iterator it = targets.rbegin(); it != targets.rend(); it++) {
    if(nVerbose > 1) {
      cout << "\tResubstitute " << *it << " : ";
      PrintStats();
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    int count_ = count;
    // merge
    count += TrivialMergeOne(*it);
    // resub
    bool fConnect = false;
    vector<bool> vMarks(nObjs);
    MarkFoCone_rec(vMarks, *it);
    list<int> targets2 = vObjs;
    for(list<int>::iterator it2 = targets2.begin(); it2 != targets2.end(); it2++) {
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
        count += Mspf();
      } else {
        vPfUpdates[*it] = true;
        count += Cspf();
      }
    }
    if(nodes < CountNodes()) {
      Load();
      count = count_;
      continue;
    }
    if(!vvFos[*it].empty() && vvFis[*it].size() > 2) {
      // decompose
      list<int>::iterator it2 = find(vObjs.begin(), vObjs.end(), *it);
      int pos = nObjs;
      count += TrivialDecomposeOne(it2, pos);
    }
    nodes = CountNodes();
    Save();
  }
  ClearSave();
  return count;
}

int Transduction::ResubMono(bool fMspf) {
  if(nVerbose) {
    cout << "Resubstitution monotonic" << endl;
  }
  int count = fMspf? Mspf(): Cspf();
  list<int> targets = vObjs;
  for(list<int>::reverse_iterator it = targets.rbegin(); it != targets.rend(); it++) {
    if(nVerbose > 1) {
      cout << "\tResubstitute monotonic " << *it << " : ";
      PrintStats();
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    // merge
    count += TrivialMergeOne(*it);
    // resub
    Save();
    for(unsigned i = 0; i < vPis.size(); i++) {
      if(vvFos[*it].empty()) {
        break;
      }
      int f = vPis[i] << 1;
      if(TryConnect(*it, f) || TryConnect(*it, f ^ 1)) {
        count--;
        int diff;
        if(fMspf) {
          Build();
          diff = Mspf(*it, f >> 1);
        } else {
          vPfUpdates[*it] = true;
          diff = Cspf();
        }
        if(diff) {
          count += diff;
          if(fMspf && !vvFos[*it].empty()) {
            vPfUpdates[*it] = true;
            count += Mspf();
          }
          Save();
        } else {
          Load();
          count++;
        }
      }
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    vector<bool> vMarks(nObjs);
    MarkFoCone_rec(vMarks, *it);
    list<int> targets2 = vObjs;
    for(list<int>::iterator it2 = targets2.begin(); it2 != targets2.end(); it2++) {
      if(vvFos[*it].empty()) {
        break;
      }
      if(!vMarks[*it2] && !vvFos[*it2].empty()) {
        int f = *it2 << 1;
        if(TryConnect(*it, f) || TryConnect(*it, f ^ 1)) {
          count--;
          int diff;
          if(fMspf) {
            Build();
            diff = Mspf(*it, f >> 1);
          } else {
            vPfUpdates[*it] = true;
            diff = Cspf();
          }
          if(diff) {
            count += diff;
            if(fMspf && !vvFos[*it].empty()) {
              vPfUpdates[*it] = true;
              count += Mspf();
            }
            Save();
          } else {
            Load();
            count++;
          }
        }
      }
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    // decompose
    if(vvFis[*it].size() > 2) {
      list<int>::iterator it2 = find(vObjs.begin(), vObjs.end(), *it);
      int pos = nObjs;
      count += TrivialDecomposeOne(it2, pos);
    }
  }
  ClearSave();
  return count;
}