#include "Transduction.h"

using namespace std;

bool Transduction::TryConnect(int i, int f) {
  if(find(vvFis[i].begin(), vvFis[i].end(), f) == vvFis[i].end()) {
    int i0 = f >> 1;
    bool c0 = f & 1;
    NewBdd::Node x = ~vFs[i] | vGs[i] | (vFs[i0] ^ c0);
    if(x.IsConst1()) {
      if(nVerbose > 3) {
        cout << "\t\t\tConnect " << (f >> 1) << "(" << (f & 1) << ")" << std::endl;
      }
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
  int count = fMspf? Mspf(true): Cspf(true);
  int nodes = CountNodes();
  TransductionBackup b;
  Save(b);
  list<int> targets = vObjs;
  for(list<int>::reverse_iterator it = targets.rbegin(); it != targets.rend(); it++) {
    if(nVerbose > 1) {
      cout << "\tResubstitute " << *it << endl;
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    int count_ = count;
    // merge
    count += TrivialMergeOne(*it);
    // resub
    bool fConnect = false;
    vector<bool> vMarks(nObjsAlloc);
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
    if(nodes < CountNodes()) {
      Load(b);
      count = count_;
      continue;
    }
    if(!vvFos[*it].empty() && vvFis[*it].size() > 2) {
      // decompose
      list<int>::iterator it2 = find(vObjs.begin(), vObjs.end(), *it);
      int pos = nObjsAlloc;
      count += TrivialDecomposeOne(it2, pos);
    }
    nodes = CountNodes();
    Save(b);
  }
  return count;
}

int Transduction::ResubMono(bool fMspf) {
  if(nVerbose) {
    cout << "Resubstitution mono" << endl;
  }
  int count = fMspf? Mspf(true): Cspf(true);
  list<int> targets = vObjs;
  for(list<int>::reverse_iterator it = targets.rbegin(); it != targets.rend(); it++) {
    if(nVerbose > 1) {
      cout << "\tResubstitute mono " << *it << endl;
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    // merge
    count += TrivialMergeOne(*it);
    // resub
    TransductionBackup b;
    Save(b);
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
          diff = Mspf(true, *it, f >> 1);
        } else {
          vPfUpdates[*it] = true;
          diff = Cspf(true, *it, f >> 1);
        }
        if(diff) {
          count += diff;
          if(!vvFos[*it].empty()) {
            vPfUpdates[*it] = true;
            count += fMspf? Mspf(true): Cspf(true);
          }
          Save(b);
        } else {
          Load(b);
          count++;
        }
      }
    }
    if(vvFos[*it].empty()) {
      continue;
    }
    vector<bool> vMarks(nObjsAlloc);
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
            diff = Mspf(true, *it, f >> 1);
          } else {
            vPfUpdates[*it] = true;
            diff = Cspf(true, *it, f >> 1);
          }
          if(diff) {
            count += diff;
            if(!vvFos[*it].empty()) {
              vPfUpdates[*it] = true;
              count += fMspf? Mspf(true): Cspf(true);
            }
            Save(b);
          } else {
            Load(b);
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
      int pos = nObjsAlloc;
      count += TrivialDecomposeOne(it2, pos);
    }
  }
  return count;
}
