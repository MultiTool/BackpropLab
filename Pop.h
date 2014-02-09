#ifndef POP_H_INCLUDED
#define POP_H_INCLUDED

//#include "Org.h"
#include "Lugar.h"
#include "Base.h"
#include "FunSurf.h"
#include "Node.h"
#include "Cluster.h"
#include "Stack.h"

#define popmax 1000
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
    BPNet->Create_Simple();
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
    do {
      BPNet->Randomize_Weights();
      ScoreBefore = Dry_Run_Test(16);
    } while (ScoreBefore==1.0);
    BPNet->Attach_FunSurf(FSurf);
    double WinCnt, LoseCnt;
    WinCnt=0.0;
    LoseCnt=0.0;
    for (GenCnt=0; GenCnt<MaxGens; GenCnt++) {
      num0 = Bit2Int(GenCnt, 0);
      num1 = Bit2Int(GenCnt, 1);
      in0 = TransInt(num0);
      in1 = TransInt(num1);// in0 = TransBit(GenCnt, 0); in1 = TransBit(GenCnt, 1);
      goal = TransInt(num0 ^ num1);
      BPNet->Load_Inputs(in0, in1, 1.0);
      BPNet->Fire_Gen();
      double fire = BPNet->OutLayer->NodeList.at(0)->FireVal;
      if (goal*fire>0) {
        WinCnt++;
      } else {
        LoseCnt++;
        FinalFail = GenCnt;
      }
      if ((GenCnt-FinalFail)>DoneThresh) {
        break;
      }
      BPNet->Backprop(goal);
    }
    FSurf->Score[0] = 1.0 - ( ((double)FinalFail)/(double)MaxGens );
    double Remainder = MaxGens-GenCnt;
    FSurf->Score[1] = ( (WinCnt+Remainder)/((double)MaxGens) ) - ScoreBefore;
    /*
    how should we score?
    1. to save time, if an org wins X times in a row, the testing ends.
    2. ratio of wins/Total should be scored against a baseline of wins/Total from before training. ratio? subtraction? (ScoreAfter/ScoreBefore)?
    3. if ScoreAfter just a ratio over history, then it rewards early success as well as average success.
    4. the only problem is if we quit early. then average success is cut short. just averge in a filler 1.0/1.0 for the remainder.
    (WinCnt+remainder)/(Total+remainder)

    */
    if (true) {
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
  void Gen() { // new generation
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
      printf("candidate->Score:%lf, %lf\n", candidate->Score[0], candidate->Score[1]);
    }
    this->Sort();
    /*
    place holder
    first we need to score and sort the parents, then we create children
    */
    // LugarVec forestv_unref = this->forestv;
    for (pcnt=0; pcnt<popsize; pcnt++) {
      lugar = forestv[pcnt];
      parent = lugar->tenant;
      child = parent->Spawn();
      lugar->Attach_Next_Tenant(child);
    }
    for (pcnt=0; pcnt<popsize; pcnt++) {// delete the parents and replace them.
      lugar = forestv[pcnt];
      lugar->Rollover_Tenant();
    }
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
  void Glean() {
    Sort();
    size_t siz = ScoreDexv.size();
    int NumSurvivors = siz / 2;
    int topcnt, cnt;
    topcnt = 0;
    for (cnt=NumSurvivors; cnt<siz; cnt++) {
      delete ScoreDexv[cnt];
      ScoreDexv[cnt] = ScoreDexv[topcnt]->Spawn();
      topcnt++; if (topcnt>=NumSurvivors) {topcnt=0;}
    }

    LugarVec forestv_unref = this->forestv;
    siz = forestv_unref.size();
    for (cnt=0; cnt<siz; cnt++) {
      forestv_unref[cnt]->tenant = ScoreDexv[cnt];
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
  void Mutate() {
    size_t siz = this->forestv.size();
    for (int cnt=0; cnt<siz; cnt++) {
      LugarPtr lugar = this->forestv.at(cnt);
      OrgPtr org = lugar->tenant;
      org->Mutate_Me();
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

