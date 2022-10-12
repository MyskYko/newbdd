#ifndef TRANSDUCTION_H
#define TRANSDUCTION_H

#include <iostream>
#include <list>
#include <algorithm>
#include <cassert>

#include <aig.hpp>

#include "NewBdd.h"

class Transduction {
public:
  Transduction(aigman const & aig, int nVerbose = 0);
  ~Transduction();
  
  void Aig(aigman & aig) const;

  inline int CountGates() const;
  inline int CountWires() const;
  inline int CountNodes() const;
  inline void PrintStats() const;

  int TrivialMerge();
  int TrivialDecompose();
  int Decompose();

  int Cspf(int block = -1);
  int CspfEager(int block = -1);

  int Mspf(int block_i = -1, int block_i0 = -1);

  int Resub(bool fMspf = false);
  int ResubMono(bool fMspf = false);

private:
  int nObjs;
  std::vector<int> vPis;
  std::vector<int> vPos;
  std::list<int> vObjs;
  std::vector<std::vector<int> > vvFis;
  std::vector<std::vector<int> > vvFos;

  NewBdd::Man * bdd;
  std::vector<NewBdd::Node> vFs;
  std::vector<NewBdd::Node> vGs;
  std::vector<std::vector<NewBdd::Node> > vvCs;

  std::vector<bool> vUpdates;
  std::vector<bool> vPfUpdates;
  enum class PfState {none, cspf, mspf};
  PfState state;

  int nVerbose;

  inline void Connect(int i, int f, bool fSort = false, bool fUpdate = true, NewBdd::Node c = NewBdd::Node());
  inline unsigned FindFi(int i, int i0) const;
  inline void Disconnect(int i, int i0, unsigned j, bool fUpdate = true, bool fPfUpdate = true);

  inline int RemoveFis(int i, bool fPfUpdate = true);
  inline int Replace(int i, int f, bool fUpdate = true);
  inline void CreateNewGate(int & pos);

  void SortObjs_rec(std::list<int>::iterator const & it);

  void MarkFiCone_rec(std::vector<bool> & vMarks, int i) const;
  void MarkFoCone_rec(std::vector<bool> & vMarks, int i) const;

  void Build(int i, std::vector<NewBdd::Node> & vFs_) const;
  void Build(int i);
  void Build();

  double Rank(int f) const;
  bool RankCompare(int a, int b) const;
  bool SortFis(int i);

  int TrivialMergeOne(int i, bool fErase = false);
  int TrivialDecomposeOne(std::list<int>::iterator const & it, int & pos);

  int RemoveRedundantFis(int i);
  void CalcG(int i);
  int CalcC(int i);

  bool IsFoConeShared_rec(std::vector<int> & vVisits, int i, int visitor) const;
  bool IsFoConeShared(int i) const;
  void BuildFoConeCompl(int i, std::vector<NewBdd::Node> & vPoFsCompl) const;
  void MspfCalcG(int i);
  bool MspfCalcC(int i, int block_i, int block_i0);

  bool TryConnect(int i, int f);

private:
  std::list<int> vObjsOld;
  std::vector<std::vector<int> > vvFisOld;
  std::vector<std::vector<int> > vvFosOld;
  std::vector<NewBdd::Node> vFsOld;
  std::vector<NewBdd::Node> vGsOld;
  std::vector<std::vector<NewBdd::Node> > vvCsOld;

  inline void Save();
  inline void Load();
  inline void ClearSave();
};

int Transduction::CountGates() const {
  return vObjs.size();
}
int Transduction::CountWires() const {
  int count = 0;
  for(std::list<int>::const_iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    count += vvFis[*it].size();
  }
  return count;
}
int Transduction::CountNodes() const {
  return CountWires() - CountGates();
}
void Transduction::PrintStats() const {
  int gates = CountGates();
  int wires = CountWires();
  int nodes = wires - gates;
  std::cout << "nodes " << nodes << " gates " << gates << " wires " << wires << std::endl;
}

void Transduction::Connect(int i, int f, bool fSort, bool fUpdate, NewBdd::Node c) {
  int i0 = f >> 1;
  if(nVerbose > 5) {
    std::cout << "\t\t\t\t\tConnect " << i0 << "(" << (f & 1) << ")" << " to " << i << std::endl;
  }
  vvFis[i].push_back(f);
  vvFos[i0].push_back(i);
  if(fUpdate) {
    vUpdates[i] = true;
  }
  vvCs[i].push_back(c);
  if(fSort && !vvFos[i].empty() && !vvFis[i0].empty()) {
    std::list<int>::iterator it = std::find(vObjs.begin(), vObjs.end(), i);
    std::list<int>::iterator it_i0 = std::find(it, vObjs.end(), i0);
    if(it_i0 != vObjs.end()) {
      if(nVerbose > 6) {
        std::cout << "\t\t\t\t\t\tMove " << i0 << " before " << *it << std::endl;
      }
      vObjs.erase(it_i0);
      it_i0 = vObjs.insert(it, i0);
      SortObjs_rec(it_i0);
    }
  }
}
unsigned Transduction::FindFi(int i, int i0) const {
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    if((vvFis[i][j] >> 1) == i0) {
      return j;
    }
  }
  abort();
}
void Transduction::Disconnect(int i, int i0, unsigned j, bool fUpdate, bool fPfUpdate) {
  if(nVerbose > 5) {
    std::cout << "\t\t\t\t\tDisconnect " << i0 << "(" << (vvFis[i][j] & 1) << ")" << " from " << i << std::endl;
  }
  vvFos[i0].erase(std::find(vvFos[i0].begin(), vvFos[i0].end(), i));
  vvFis[i].erase(vvFis[i].begin() + j);
  vvCs[i].erase(vvCs[i].begin() + j);
  if(fUpdate) {
    vUpdates[i] = true;
  }
  if(fPfUpdate) {
    vPfUpdates[i0] = true;
  }
}

int Transduction::RemoveFis(int i, bool fPfUpdate) {
  if(nVerbose > 4) {
    std::cout << "\t\t\t\tRemove " << i << std::endl;
  }
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    int i0 = vvFis[i][j] >> 1;
    vvFos[i0].erase(std::find(vvFos[i0].begin(), vvFos[i0].end(), i));
    if(fPfUpdate) {
      vPfUpdates[i0] = true;
    }
  }
  int count = vvFis[i].size();
  vvFis[i].clear();
  vFs[i] = vGs[i] = NewBdd::Node();
  vvCs[i].clear();
  vUpdates[i] = vPfUpdates[i] = false;
  return count;
}
int Transduction::Replace(int i, int f, bool fUpdate) {
  if(nVerbose > 4) {
    std::cout << "\t\t\t\tReplace " << i << " by " << (f >> 1) << "(" << (f & 1) << ")" << std::endl;
  }
  assert(i != (f >> 1));
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    int k = vvFos[i][j];
    unsigned l = FindFi(k, i);
    vvFis[k][l] = f ^ (vvFis[k][l] & 1);
    vvFos[f >> 1].push_back(k);
    if(fUpdate) {
      vUpdates[k] = true;
    }
  }
  vvFos[i].clear();
  vPfUpdates[f >> 1] = true;
  return RemoveFis(i);
}
void Transduction::CreateNewGate(int & pos) {
  while(pos != nObjs && (!vvFis[pos].empty() || !vvFos[pos].empty())) {
    pos++;
  }
  if(pos == nObjs) {
    nObjs++;
    vvFis.resize(nObjs);
    vvFos.resize(nObjs);
    vFs.resize(nObjs);
    vGs.resize(nObjs);
    vvCs.resize(nObjs);
    vUpdates.resize(nObjs);
    vPfUpdates.resize(nObjs);
  }
}

void Transduction::Save() {
  vObjsOld = vObjs;
  vvFisOld = vvFis;
  vvFosOld = vvFos;
  vFsOld = vFs;
  vGsOld = vGs;
  vvCsOld = vvCs;
  assert(std::all_of(vUpdates.begin(), vUpdates.end(), [](bool i) { return !i; }));
  assert(std::all_of(vPfUpdates.begin(), vPfUpdates.end(), [](bool i) { return !i; }));
}
void Transduction::Load() {
  vObjs = vObjsOld;
  vvFis = vvFisOld;
  vvFos = vvFosOld;
  vFs = vFsOld;
  vGs = vGsOld;
  vvCs = vvCsOld;
  std::fill(vUpdates.begin(), vUpdates.end(), false);
  std::fill(vPfUpdates.begin(), vPfUpdates.end(), false);
}
void Transduction::ClearSave() {
  vObjsOld.clear();
  vvFisOld.clear();
  vvFosOld.clear();
  vFsOld.clear();
  vGsOld.clear();
  vvCsOld.clear();
}

#endif
