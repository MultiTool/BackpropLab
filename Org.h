#ifndef ORG_H_INCLUDED
#define ORG_H_INCLUDED

#include "Base.h"
#include "FunSurf.h"

#undef Nested

/* ********************************************************************** */
class Org;
typedef Org *OrgPtr;
typedef std::vector<OrgPtr> OrgVec;
class Org : public FunSurfGrid {
public:
  double Score;
  struct Lugar *home;// my location
#ifdef Nested
  FunSurfGridPtr FSurf;
#endif
  /* ********************************************************************** */
  Org() : Org(2, 4) {
    this->Score = 0.0;
    this->home = NULL;
#ifdef Nested
    FSurf = new FunSurfGrid(2, 4);
#endif
  }
  /* ********************************************************************** */
  Org(uint32_t NumDims0, uint32_t Rez0) : FunSurfGrid(NumDims0, Rez0) {
  }
  /* ********************************************************************** */
  ~Org() {
#ifdef Nested
    delete FSurf;
#endif
  }
  /* ********************************************************************** */
  static OrgPtr Abiogenate() {
    OrgPtr org = new Org();
    org->Mutate_Me();
    org->Create_Sigmoid_Deriv_Surface();// snox for testing
    return org;
  }
  /* ********************************************************************** */
  void Mutate_Me() {
    double MutAmp = 0.01;
    double HalfAmp = MutAmp/2.0;
    uint32_t siz = this->NumCells;
    uint32_t cnt;
    for (cnt=0;cnt<siz;cnt++){
      Space[cnt] += frand()*MutAmp-HalfAmp;
    }
  }
  /* ********************************************************************** */
  OrgPtr Spawn() {
    OrgPtr child;
    uint32_t siz = this->NumCells;
    child = new Org(this->NumDims, this->Rez);
    child->Score = 0.0;
    uint32_t cnt;
    for (cnt=0;cnt<siz;cnt++) {
      child->Space[cnt] = this->Space[cnt];
    }
    return child;
  }
  /* ********************************************************************** */
  void Compile_Me() {
  }
  /* ********************************************************************** */
  void Print_Me() {
    printf("Org\n");
  }
};

#endif // ORG_H_INCLUDED
