#include <aig.hpp>

#include "NewBdd.h"

void Aig2Bdd(aigman const & aig, NewBdd::Man & bdd, std::vector<NewBdd::Node> & vNodes, bool fVerbose = 0);

void Bdd2Aig(NewBdd::Man const & bdd, aigman & aig, std::vector<NewBdd::Node> const & vNodes);
