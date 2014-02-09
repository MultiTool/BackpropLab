#ifndef POP_H_INCLUDED
#define POP_H_INCLUDED

//#include "Org.h"
#include "Lugar.h"
#include "Base.h"
#include "FunSurf.h"
#include "Node.h"
#include "Cluster.h"
#include "Stack.h"

//#define popmax 1000
#define popmax 100
//#define popmax 10

/* ********************************************************************** */
class Pop;
typedef Pop *PopPtr;
class Pop {
public:
  uint32_t popsz;
  LugarVec forestv;
  OrgVec ScoreDexv; // for sorting
  StackPtr BPNet;// crucible
  /* ********************************************************************** */
  Pop() : Pop(popmax) {
  }
  /* ********************************************************************** */
  Pop(int popsize) {
    BPNet = new Stack();
    LugarPtr lugar;
    Org *org;
    int pcnt;
    //BPNet->Create_Simple();
    BPNet->Create_Any_Depth();
    this->popsz = popsize;
    forestv.resize(popsize);
    ScoreDexv.resize(popsize);
    for (pcnt=0; pcnt<popsize; pcnt++) {
      lugar = new Lugar();
      org = Org::Abiogenate();
      lugar->Attach_Tenant(org);
      forestv.at(pcnt) = lugar;
      ScoreDexv.at(pcnt) = org;
    }
  }
  /* ********************************************************************** */
  ~Pop() {
    int siz, pcnt;
    siz = forestv.size();
    for (pcnt=0; pcnt<siz; pcnt++) {
      delete forestv.at(pcnt);
    }
    delete BPNet;
  }
  /* ********************************************************************** */
  double Dry_Run_Test(uint32_t MaxGens) {
    uint32_t GenCnt;
    uint32_t num0, num1;
    double in0, in1;
    double goal;
    double WinCnt;
    WinCnt=0.0;
    for (GenCnt=0; GenCnt<MaxGens; GenCnt++) {
      num0 = Bit2Int(GenCnt, 0);
      num1 = Bit2Int(GenCnt, 1);
      in0 = TransInt(num0);
      in1 = TransInt(num1);
      goal = TransInt(num0 ^ num1);
      //goal = TransInt(num0 & num1);
      BPNet->Load_Inputs(in0, in1, 1.0);
      BPNet->Fire_Gen();
      double fire = BPNet->OutLayer->NodeList.at(0)->FireVal;
      if (goal*fire>0) { WinCnt++; }
    }
    return WinCnt/((double)MaxGens);
  }
  /* ********************************************************************** */
  void Run_Test(OrgPtr FSurf) {
    uint32_t MaxGens = 2000;
    uint32_t DoneThresh = 16;
    uint32_t FinalFail = 0;
    uint32_t GenCnt;
    uint32_t num0, num1;
    double in0, in1;
    double goal;
    double ScoreBefore;
    double WinCnt;
    do {
      BPNet->Randomize_Weights();
      ScoreBefore = Dry_Run_Test(16);
    } while (ScoreBefore==1.0);
    FSurf->Clear_Score();
    BPNet->Attach_FunSurf(FSurf);
    WinCnt=0.0;
    for (GenCnt=0; GenCnt<MaxGens; GenCnt++) {
      num0 = Bit2Int(GenCnt, 0); num1 = Bit2Int(GenCnt, 1);
      in0 = TransInt(num0); in1 = TransInt(num1);
      goal = TransInt(num0 ^ num1);
      //goal = TransInt(num0 & num1);
      BPNet->Load_Inputs(in0, in1, 1.0);
      BPNet->Fire_Gen();
      double fire = BPNet->OutLayer->NodeList.at(0)->FireVal;
      if (goal*fire>0) {
        WinCnt++;
      } else {
        FinalFail = GenCnt;
      }
      if ((GenCnt-FinalFail)>DoneThresh) {
        break;
      }
      BPNet->Backprop(goal);
    }
    FSurf->Score[0] = 1.0 - ( ((double)FinalFail)/(double)MaxGens );
    double Remainder = MaxGens-GenCnt;// if nobody won *earlier*, then score by average goodness of output
    FSurf->Score[1] = ( (WinCnt+Remainder)/((double)MaxGens) ) - ScoreBefore;
    if (false) {
      printf("\n");
      if (false) {
        FSurf->Print_Me();
        printf("\n\n");
        BPNet->Print_Me();
        printf("\n");
      }
      printf("numgens:%li, FinalFail:%li\n", MaxGens, FinalFail);
    }
  }
  /* ********************************************************************** */
  void Gen() { // each generation
    printf("Pop.Gen()\n");
    uint32_t popsize = this->forestv.size();
    LugarPtr lugar;
    OrgPtr parent, child, candidate;
    uint32_t pcnt;
    LugarPtr place;

    for (pcnt=0; pcnt<popsize; pcnt++) {
      lugar = forestv[pcnt];
      candidate = lugar->tenant;
      this->Run_Test(candidate);
      // printf("candidate->Score:%lf, %lf\n", candidate->Score[0], candidate->Score[1]);
    }
    Birth_And_Death();
    OrgPtr bestbeast = ScoreDexv[0];
    bestbeast->Print_Me(); printf("\n");
    printf("bestbeast->Score:%lf, %lf\n", bestbeast->Score[0], bestbeast->Score[1]);
    Mutate(0.8, 0.8);
  }
  /* ********************************************************************** */
  static bool AscendingScore(OrgPtr b0, OrgPtr b1) {
    return b0->Compare_Score(b1) > 0;
  }
  static bool DescendingScore(OrgPtr b0, OrgPtr b1) {
    return b1->Compare_Score(b0) > 0;
  }
  void Sort() {
    LugarVec forestv_unref = this->forestv;
    size_t siz = forestv_unref.size();
    int cnt;
    for (cnt=0; cnt<siz; cnt++) {
      ScoreDexv[cnt] = forestv_unref[cnt]->tenant;
    }
    std::random_shuffle(ScoreDexv.begin(), ScoreDexv.end());
    // std::sort (ScoreDexv.begin(), ScoreDexv.end(), AscendingScore);
    std::sort (ScoreDexv.begin(), ScoreDexv.end(), DescendingScore);
  }
  /* ********************************************************************** */
  void Birth_And_Death() {
    Sort();
    size_t siz = ScoreDexv.size();
    size_t NumSurvivors = siz / 2;
    size_t topcnt, cnt;
    LugarPtr home;
    OrgPtr doomed, child;
    topcnt = 0;
    for (cnt=NumSurvivors; cnt<siz; cnt++) {
      doomed = ScoreDexv[cnt]; home = doomed->home;
      delete doomed;
      child = ScoreDexv[topcnt]->Spawn();
      home->Attach_Tenant(child); ScoreDexv[cnt] = child;
      topcnt++; if (topcnt>=NumSurvivors) {topcnt=0;}
    }
  }
  /* ********************************************************************** */
  void Print_Sorted_Scores() {
    size_t siz = ScoreDexv.size();
    int cnt;
    for (cnt=0; cnt<siz; cnt++) {
      ScoreDexv[cnt]->Print_Score();
    }
  }
  /* ********************************************************************** */
  void Mutate(double Pop_MRate, double Org_MRate) {
    size_t siz = this->forestv.size();
    for (int cnt=0; cnt<siz; cnt++) {
      if (frand()<Pop_MRate) {
        LugarPtr lugar = this->forestv.at(cnt);
        OrgPtr org = lugar->tenant;
        org->Mutate_Me(Org_MRate);
      }
    }
  }
  /* ********************************************************************** */
  void Compile_Me() {
    size_t siz = this->forestv.size();
    for (int cnt=0; cnt<siz; cnt++) {
      LugarPtr lugar = this->forestv.at(cnt);
      OrgPtr org = lugar->tenant;
      org->Compile_Me();
    }
  }
  /* ********************************************************************** */
  inline static double TransBit(int val, int bitnum) {
    return ((double)((val >> bitnum)&0x1)) * 2.0 - 1.0;
  }
  inline static double TransInt(int val) {
    return ((double)val) * 2.0 - 1.0;
  }
  inline static uint32_t Bit2Int(int val, int bitnum) {
    return ((val >> bitnum)&0x1);
  }
};

#endif // POP_H_INCLUDED

