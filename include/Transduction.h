#ifndef TRANSDUCTION_H
#define TRANSDUCTION_H

#include <list>

#include <aig.hpp>

#include "NewBdd.h"

class Transduction {
public:
  Transduction(aigman const & aig, int nVerbose = 0);
  ~Transduction();
  
  void Aig(aigman & aig) const;

  int CountGates() const;
  int CountWires() const;
  int CountNodes() const;

  int TrivialMerge();
  void TrivialDecompose();

  void Decompose();

  int Cspf(int block = -1);
  int CspfEager(int block = -1);

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

  void SortObjs(std::list<int>::iterator const & it);
  void Connect(int i, int f, bool fSort = false);
  unsigned FindFi(int i, int i0) const;
  void Disconnect(int i, int i0, unsigned j);
  void Disconnect(int i, int f);

  int RemoveFis(int i);
  int Replace(int i, int f);

  int TrivialMergeOne(int i);

  void BuildOne(int i, std::vector<NewBdd::Node> & vFs_);
  void Build();

  double Rank(int f) const;
  void SortFisOne(int i);
  void SortFis();

  int RemoveRedundantFis(int i);
  int CalcG(int i);
  int CalcC(int i);

  void MarkFiCone_rec(std::vector<bool> & vMarks, int i) const;
  void MarkFoCone_rec(std::vector<bool> & vMarks, int i) const;

  void BuildFoCone(int i);
  int CspfFiCone(int i, int block = -1);

  bool TryConnect(int i, int f);

  std::list<int> vObjsOld;
  std::vector<std::vector<int> > vvFisOld;
  std::vector<std::vector<int> > vvFosOld;
  std::vector<NewBdd::Node> vFsOld;
  std::vector<NewBdd::Node> vGsOld;
  std::vector<std::vector<NewBdd::Node> > vvCsOld;

  void Save();
  void Load();
  void ClearSave();

  struct RankComparator {
    Transduction const & t;
    RankComparator(Transduction const & t) : t(t) {}
    bool operator()(int a, int b) {
      return t.Rank(a) < t.Rank(b);
    }
  };
};

#endif
