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

  int TrivialMerge();
  int TrivialDecompose();
  int Decompose();

  int Cspf(int block = -1);
  int CspfEager(int block = -1);

  int Mspf(int block = -1);

  int Resub();
  int ResubMono();

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

  int nVerbose;

  inline void Connect(int i, int f, bool fSort = false);
  inline unsigned FindFi(int i, int i0) const;
  inline void Disconnect(int i, int i0, unsigned j);
  inline void Disconnect(int i, int f);

  inline int RemoveFis(int i);
  inline int Replace(int i, int f);
  inline void CreateNewGate(int & pos);

  void SortObjs_rec(std::list<int>::iterator const & it);

  void MarkFiCone_rec(std::vector<bool> & vMarks, int i) const;
  void MarkFoCone_rec(std::vector<bool> & vMarks, int i) const;

  void Build(int i, std::vector<NewBdd::Node> & vFs_) const;
  void Build(int i);
  void Build();

  void Update(std::vector<bool> & vUpdates);
  void Update(int i);

  double Rank(int f) const;
  bool RankCompare(int a, int b) const;
  struct RankComparator {
    Transduction const & t;
    RankComparator(Transduction const & t) : t(t) {}
    bool operator()(int a, int b) {
      return t.RankCompare(a, b);
    }
  };
  void SortFis(int i);

  int TrivialMergeOne(int i, bool fErase = false);
  int TrivialDecomposeOne(std::list<int>::iterator const & it, int & pos);

  int RemoveRedundantFis(int i);
  void CalcG(int i);
  int CalcC(int i);
  int CspfFiCone(int i, int block = -1);

  bool IsFoConeShared_rec(std::vector<int> & vVisits, int i, int visitor) const;
  bool IsFoConeShared(int i) const;
  void BuildFoConeCompl(int i, std::vector<NewBdd::Node> & vPoFsCompl) const;
  void MspfCalcG(int i);
  bool MspfCalcC(int i);

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

void Transduction::Connect(int i, int f, bool fSort) {
  int i0 = f >> 1;
  if(nVerbose > 5) {
    std::cout << "\t\t\t\t\tConnect " << i0 << "(" << (f & 1) << ")" << " to " << i << std::endl;
  }
  vvFis[i].push_back(f);
  vvFos[i0].push_back(i);
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
void Transduction::Disconnect(int i, int i0, unsigned j) {
  if(nVerbose > 5) {
    std::cout << "\t\t\t\t\tDisconnect " << i0 << "(" << (vvFis[i][j] & 1) << ")" << " from " << i << std::endl;
  }
  vvFos[i0].erase(std::find(vvFos[i0].begin(), vvFos[i0].end(), i));
  vvFis[i].erase(vvFis[i].begin() + j);
}
void Transduction::Disconnect(int i, int f) {
  unsigned j = std::find(vvFis[i].begin(), vvFis[i].end(), f) - vvFis[i].begin();
  Disconnect(i, f >> 1, j);
}

int Transduction::RemoveFis(int i) {
  if(nVerbose > 4) {
    std::cout << "\t\t\t\tRemove " << i << std::endl;
  }
  for(unsigned j = 0; j < vvFis[i].size(); j++) {
    int i0 = vvFis[i][j] >> 1;
    vvFos[i0].erase(std::find(vvFos[i0].begin(), vvFos[i0].end(), i));
  }
  int count = vvFis[i].size();
  vvFis[i].clear();
  vFs[i] = vGs[i] = NewBdd::Node();
  vvCs[i].clear();
  return count;
}
int Transduction::Replace(int i, int f) {
  if(nVerbose > 4) {
    std::cout << "\t\t\t\tReplace " << i << " by " << (f >> 1) << "(" << (f & 1) << ")" << std::endl;
  }
  assert(i != (f >> 1));
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    int k = vvFos[i][j];
    unsigned l = FindFi(k, i);
    vvFis[k][l] = f ^ (vvFis[k][l] & 1);
    vvFos[f >> 1].push_back(k);
  }
  vvFos[i].clear();
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
  }
}

void Transduction::Save() {
  vObjsOld = vObjs;
  vvFisOld = vvFis;
  vvFosOld = vvFos;
  vFsOld = vFs;
  vGsOld = vGs;
  vvCsOld = vvCs;
}
void Transduction::Load() {
  vObjs = vObjsOld;
  vvFis = vvFisOld;
  vvFos = vvFosOld;
  vFs = vFsOld;
  vGs = vGsOld;
  vvCs = vvCsOld;
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
