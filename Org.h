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
  const static int NumScores = 2;
  double Score[NumScores];
  uint32_t FinalFail;
  struct Lugar *home;// my location
  static const bool Baselining = false;
#ifdef Nested
  FunSurfGridPtr FSurf;
#endif
  /* ********************************************************************** */
  Org() : Org(2, 4) {
#ifdef Nested
    FSurf = new FunSurfGrid(2, 4);
#endif
  }
  /* ********************************************************************** */
  Org(uint32_t NumDims0, uint32_t Rez0) : FunSurfGrid(NumDims0, Rez0) {
    for (int cnt=0; cnt<NumScores; cnt++) {
      this->Score[cnt] = 0.0;
    }
    this->home = NULL;
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
    org->Rand_Init();
    if (Baselining) {
      org->Create_Sigmoid_Deriv_Surface();// snox for testing
    }
    return org;
  }
  /* ********************************************************************** */
  void Rand_Init() {
    double MutAmp = 2.0;
    double HalfAmp = MutAmp/2.0;
    uint32_t siz = this->NumCells;
    uint32_t cnt;
    for (cnt=0; cnt<siz; cnt++) {
      Space[cnt] = frand()*MutAmp-HalfAmp;
    }
  }
  /* ********************************************************************** */
  void Mutate_Me(double MRate) {
    if (Baselining) {return;}
    double MutAmp = 0.01;
    double HalfAmp = MutAmp/2.0;
    uint32_t siz = this->NumCells;
    uint32_t cnt;
    for (cnt=0; cnt<siz; cnt++) {
      if (frand()<MRate) {
        Space[cnt] += frand()*MutAmp-HalfAmp;
      }
    }
  }
  /* ********************************************************************** */
  OrgPtr Spawn() {
    OrgPtr child;
    uint32_t siz = this->NumCells;
    child = new Org(this->NumDims, this->Rez);
    uint32_t cnt;
    for (cnt=0; cnt<siz; cnt++) {
      child->Space[cnt] = this->Space[cnt];
    }
    return child;
  }
  /* ********************************************************************** */
  void Compile_Me() {
  }
  /* ********************************************************************** */
  void Clear_Score() {
    for (int cnt=0; cnt<NumScores; cnt++) {
      this->Score[cnt]=0.0;
    }
  }
  /* ********************************************************************** */
//  void Print_Me() override {
//    printf("Org\n");
//  }
  /* ********************************************************************** */
  void Print_Score() {
    printf(" Score:%lf, %lf\n", this->Score[0], this->Score[1]);
  }
  /* ********************************************************************** */
  int Compare_Score(OrgPtr other) {
    //double left, right;
    int cnt = 0;
    double *ScoreMe, *ScoreYou;
    ScoreMe=this->Score; ScoreYou=other->Score;
    while (cnt<NumScores) {
      if (ScoreMe[cnt]<ScoreYou[cnt]) {return 1;}
      if (ScoreMe[cnt]>ScoreYou[cnt]) {return -1;}
      cnt++;
    }
    return 0;
  }
};

#endif // ORG_H_INCLUDED
