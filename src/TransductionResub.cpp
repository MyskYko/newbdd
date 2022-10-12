#include <iostream>
#include <algorithm>
#include <cassert>

#include "Transduction.h"

using namespace std;

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

int Transduction::Resub(bool fMspf) {
  if(nVerbose) {
    cout << "Resubstitution" << endl;
  }
  int count = fMspf? Mspf(): CspfEager();
  list<int> targets = vObjs;
  for(list<int>::reverse_iterator it = targets.rbegin(); it != targets.rend(); it++) {
    if(nVerbose > 1) {
      cout << "\tResubstitute " << *it << " : ";
      PrintStats();
    }
    if(vvFos[*it].empty()) {
      continue;
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
    vMarks[*it] = true;
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

int Transduction::ResubMono(bool fMspf) {
  if(nVerbose) {
    cout << "Resubstitution monotonic" << endl;
  }
  int count = fMspf? Mspf(): CspfEager();
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
    count += TrivialMergeOne(*it, true);
    if(!fMspf) {
      count += CspfEager();
    }
    // resub
    Save();
    for(unsigned i = 0; i < vPis.size(); i++) {
      if(vvFos[*it].empty()) {
        break;
      }
      int f = vPis[i] << 1;
      if(TryConnect(*it, f) || TryConnect(*it, f ^ 1)) {
        count--;
        Build();
        if(int diff = fMspf? Mspf(*it, f >> 1): Cspf(*it)) {
          count += diff;
          count += fMspf? Mspf(): CspfEager();
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
    vMarks[*it] = true;
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
          Build();
          if(int diff = fMspf? Mspf(*it, f >> 1): Cspf(*it)) {
            count += diff;
            count += fMspf? Mspf(): CspfEager();
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
      count += fMspf? Mspf(): CspfEager();
    }
  }
  ClearSave();
  return count;
}
