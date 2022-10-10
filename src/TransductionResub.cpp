#include <iostream>
#include <algorithm>
#include <cassert>

#include "Transduction.h"

using namespace std;

int Transduction::Resub() {
  if(nVerbose) {
    cout << "Resubstitution" << endl;
  }
  int count = CspfEager();
  list<int> targets = vObjs;
  for(list<int>::reverse_iterator it = targets.rbegin(); it != targets.rend(); it++) {
    if(nVerbose > 1) {
      cout << "\tResubstitute " << *it;
      cout << " : nodes " << CountNodes() << " gates " << CountGates() << " wires " << CountWires() << endl;
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
      count += CspfEager();
    }
  }
  return count;
}

int Transduction::ResubMono() {
  if(nVerbose) {
    cout << "Resubstitution monotonic" << endl;
  }
  int count = CspfEager();
  list<int> targets = vObjs;
  for(list<int>::reverse_iterator it = targets.rbegin(); it != targets.rend(); it++) {
    if(nVerbose > 1) {
      cout << "\tResubstitute monotonic " << *it;
      cout << " : nodes " << CountNodes() << " gates " << CountGates() << " wires " << CountWires() << endl;
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    // merge
    count += TrivialMergeOne(*it, true);
    count += CspfEager();
    // resub
    Save();
    for(unsigned i = 0; i < vPis.size(); i++) {
      if(vvFos[*it].empty()) {
        break;
      }
      int f = vPis[i] << 1;
      if(TryConnect(*it, f) || TryConnect(*it, f ^ 1)) {
        count--;
        //int diff = CspfFiCone(*it, *it);
        BuildFoCone(*it);
        int diff = Cspf(*it);
        //int diff = CspfEager(*it);
        if(diff) {
          count += diff;
          count += CspfEager();
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
          //int diff = CspfFiCone(*it, *it);
          BuildFoCone(*it);
          int diff = Cspf(*it);
          //int diff = CspfEager(*it);
          if(diff) {
            count += diff;
            count += CspfEager();
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
      while(vvFis[*it].size() > 2) {
        int f0 = vvFis[*it].back();
        Disconnect(*it, f0 >> 1, vvFis[*it].size() - 1);
        int f1 = vvFis[*it].back();
        Disconnect(*it, f1 >> 1, vvFis[*it].size() - 1);
        int i = nObjs++;
        vvFis.resize(nObjs);
        vvFos.resize(nObjs);
        vFs.resize(nObjs);
        vGs.resize(nObjs);
        vvCs.resize(nObjs);
        vObjs.insert(find(vObjs.begin(), vObjs.end(), *it), i);
        Connect(i, f0);
        Connect(i, f1);
        Connect(*it, i << 1);
        count--;
      }
      Build();
    }
    int diff = Cspf();
    assert(!diff);
  }
  ClearSave();
  return count;
}
