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

  void Cspf();

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

  void Connect(int i, int f);
  void Disconnect(int i, int i0, unsigned j);

  void RemoveFis(int i);
  int FindFi(int i, int i0) const;
  void ReplaceNode(int i, int c);

  void TrivialMerge();

  void BuildNode(int i, std::vector<NewBdd::Node> & vFs_);
  void Build();

  double Rank(int i) const;
  void SortFisNode(int i);
  void SortFis();

  void RemoveRedundantFis(int i);

  void CalcG(int i);
  void CalcC(int i);

  struct RankComparator {
    Transduction const & t;
    RankComparator(Transduction const & t) : t(t) {}
    bool operator()(int a, int b) {
      return t.Rank(a >> 1) < t.Rank(b >> 1);
    }
  };
};

#endif
