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
  uint32_t MaxNeuroGens = 2000;
  uint32_t DoneThresh = 32;//64; //32; //64;// 128;//16;
  std::vector<TrainSetPtr> TrainingSets;
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
    //BPNet->Create_Any_Depth();
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
    Init_Training_Sets();
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
  void Init_Training_Sets() {
    TrainSetPtr tset;
    IOPairPtr match;

    tset = new TrainSet(); TrainingSets.push_back(tset);
    { // first XOR
      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back(-1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back( 1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back(-1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back( 1.0); match->goalvec.push_back(-1.0);
    }

    tset = new TrainSet(); TrainingSets.push_back(tset);
    { // AND?
      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back(-1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back( 1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back(-1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back( 1.0); match->goalvec.push_back( 1.0);
    }

    tset = new TrainSet(); TrainingSets.push_back(tset);
    { // OR
      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back(-1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back( 1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back(-1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back( 1.0); match->goalvec.push_back( 1.0);
    }
  }
  /* ********************************************************************** */
  double Dry_Run_Test(uint32_t MaxNeuroGens) {
    uint32_t GenCnt;
    uint32_t num0, num1;
    double in0, in1;
    double goal;
    double WinCnt;
    WinCnt=0.0;
    for (GenCnt=0; GenCnt<MaxNeuroGens; GenCnt++) {
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
    return WinCnt/((double)MaxNeuroGens);
  }
  /* ********************************************************************** */
  void Run_Test(OrgPtr FSurf) {
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
    for (GenCnt=0; GenCnt<MaxNeuroGens; GenCnt++) {
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
    FSurf->FinalFail = FinalFail;
    FSurf->Score[0] = 1.0 - ( ((double)FinalFail)/(double)MaxNeuroGens );
    double Remainder = MaxNeuroGens-GenCnt;// if nobody won *earlier*, then score by average goodness of output
    FSurf->Score[1] = ( (WinCnt+Remainder)/((double)MaxNeuroGens) ) - ScoreBefore;
    if (false) {
      printf("\n");
      if (false) {
        FSurf->Print_Me();
        printf("\n\n");
        BPNet->Print_Me();
        printf("\n");
      }
      printf("MaxNeuroGens:%li, FinalFail:%li\n", MaxNeuroGens, FinalFail);
    }
  }
  /* ********************************************************************** */
  double Dry_Run_Test(uint32_t MaxNeuroGens, TrainSetPtr TSet) {
    uint32_t GenCnt;
    double goal;
    double WinCnt;
    IOPairPtr Pair;
    WinCnt=0.0;
    for (GenCnt=0; GenCnt<MaxNeuroGens; GenCnt++) {
      Pair = TSet->at(GenCnt%TSet->size());
      goal = Pair->goalvec.at(0);
      // BPNet->Load_Inputs(Pair->invec.at(0), Pair->invec.at(1), 1.0);
      BPNet->Load_Inputs(&(Pair->invec));
      BPNet->Fire_Gen();
      double fire = BPNet->OutLayer->NodeList.at(0)->FireVal;
      if (goal*fire>0) { WinCnt++; }
    }
    return WinCnt/((double)MaxNeuroGens);
  }
  /* ********************************************************************** */
  void Run_Test(OrgPtr FSurf, TrainSetPtr TSet) {
    uint32_t FinalFail = 0;
    uint32_t GenCnt;
    uint32_t num0, num1;
    double in0, in1;
    double goal;
    double ScoreBefore;
    double WinCnt;
    IOPairPtr Pair;
    do {
      BPNet->Randomize_Weights();
      ScoreBefore = Dry_Run_Test(16);
    } while (ScoreBefore==1.0);
    FSurf->Clear_Score();
    BPNet->Attach_FunSurf(FSurf);
    WinCnt=0.0;
    for (GenCnt=0; GenCnt<MaxNeuroGens; GenCnt++) {
      Pair = TSet->at(GenCnt%TSet->size());
      goal = Pair->goalvec.at(0);
      //BPNet->Load_Inputs(Pair->invec.at(0), Pair->invec.at(1), 1.0);
      BPNet->Load_Inputs(&(Pair->invec));
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
    FSurf->FinalFail = FinalFail;
    FSurf->Score[0] = 1.0 - ( ((double)FinalFail)/(double)MaxNeuroGens );
    double Remainder = MaxNeuroGens-GenCnt;// if nobody won *earlier*, then score by average goodness of output
    FSurf->Score[1] = ( (WinCnt+Remainder)/((double)MaxNeuroGens) ) - ScoreBefore;
    if (false) {
      printf("\n");
      if (false) {
        FSurf->Print_Me();
        printf("\n\n");
        BPNet->Print_Me();
        printf("\n");
      }
      printf("MaxNeuroGens:%li, FinalFail:%li\n", MaxNeuroGens, FinalFail);
    }
  }
  double avgnumwinners = 0.0;
  /* ********************************************************************** */
  void Gen(uint32_t evogens, uint32_t gencnt) { // each generation
    printf("Pop.Gen()\n");
    uint32_t popsize = this->forestv.size();
    LugarPtr lugar;
    OrgPtr parent, child, candidate;
    uint32_t pcnt;
    LugarPtr place;
    this->BPNet->Print_Specs();
    for (pcnt=0; pcnt<popsize; pcnt++) {
      lugar = forestv[pcnt];
      candidate = lugar->tenant;
      // this->Run_Test(candidate);
      this->Run_Test(candidate, TrainingSets.at(0));// xor
      // printf("candidate->Score:%lf, %lf\n", candidate->Score[0], candidate->Score[1]);
    }
    double SurvivalRate = 0.5;
    Sort();
    OrgPtr bestbeast = ScoreDexv[0];
    OrgPtr leastbeast = ScoreDexv[this->popsz-2];
    double avgbeast = AvgBeast();
    int numwinners = NumWinners();
    avgnumwinners = (avgnumwinners*0.98) + ((double)numwinners)*0.02;
    bestbeast->Print_Me(); printf("\n");
    printf("bestbeast->Score:%lf, %lf\n", bestbeast->Score[0], bestbeast->Score[1]);
    printf("avgbeast Score:%lf, numwinners:%li, avgnumwinners:%lf\n", avgbeast, numwinners, avgnumwinners);
    printf("leastbeast->Score:%lf, %lf\n", leastbeast->Score[0], leastbeast->Score[1]);
    Birth_And_Death(SurvivalRate);

    if (16<(evogens - gencnt)) {
      Mutate(0.8, 0.8);
      printf("Mutation \n");
    } else {
      printf("NO MUTATION \n");
    }
    //Mutate_Sorted(0.8, 0.8);
  }
  /* ********************************************************************** */
  double AvgBeast() {
    size_t siz = ScoreDexv.size();
    double sum = 0.0;
    for (int cnt=0; cnt<siz; cnt++) {
      sum += ScoreDexv[cnt]->Score[0];
    }
    sum /= (double)siz;
    return sum;
  }
  /* ********************************************************************** */
  size_t NumWinners() {
    size_t siz = ScoreDexv.size();// only works if sorted descending already
    size_t wincnt = 0;
    for (int cnt=0; cnt<siz; cnt++) {
      // if (ScoreDexv[cnt]->FinalFail >= (MaxNeuroGens-DoneThresh)) { break; }
      //if (ScoreDexv[cnt]->Score[0]<0.01) { break; }
      if (ScoreDexv[cnt]->FinalFail < (MaxNeuroGens-DoneThresh)) { wincnt++; }
      //wincnt++;
    }
    return wincnt;
  }
  /* ********************************************************************** */
  static bool AscendingScore(OrgPtr b0, OrgPtr b1) {
    return b0->Compare_Score(b1) > 0;
  }
  static bool DescendingScore(OrgPtr b0, OrgPtr b1) {
    return b1->Compare_Score(b0) > 0;
  }
  void Sort() {
    if (false) {
      LugarVec forestv_unref = this->forestv;
      size_t siz = forestv_unref.size();
      int cnt;
      for (cnt=0; cnt<siz; cnt++) {
        ScoreDexv[cnt] = forestv_unref[cnt]->tenant;
      }
    }
    std::random_shuffle(ScoreDexv.begin(), ScoreDexv.end());
    // std::sort (ScoreDexv.begin(), ScoreDexv.end(), AscendingScore);
    std::sort (ScoreDexv.begin(), ScoreDexv.end(), DescendingScore);
  }
  /* ********************************************************************** */
  void Birth_And_Death(double SurvivalRate) {
    //  Sort();
    size_t siz = ScoreDexv.size();
    size_t NumSurvivors = siz * SurvivalRate;
    size_t topcnt, cnt;
    LugarPtr home;
    OrgPtr doomed, child;
    topcnt = 0;
    for (cnt=NumSurvivors; cnt<siz; cnt++) {
      doomed = ScoreDexv[cnt]; home = doomed->home;
      delete doomed;
      child = ScoreDexv[topcnt]->Spawn();
      home->Attach_Tenant(child); ScoreDexv[cnt] = child;
      topcnt++;
      if (topcnt>=NumSurvivors) {topcnt=0;}
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
  void Mutate_Sorted(double Pop_MRate, double Org_MRate) {
    size_t siz = this->forestv.size();
    for (int cnt=16; cnt<siz; cnt++) {
      if (frand()<Pop_MRate) {
        OrgPtr org = this->ScoreDexv[cnt];
        org->Mutate_Me(Org_MRate);
      }
    }
  }
  /* ********************************************************************** */
  void Mutate(double Pop_MRate, double Org_MRate) {
    OrgPtr org;
    size_t LastOrg;
    //size_t siz = this->forestv.size();
    size_t siz = this->ScoreDexv.size(); LastOrg = siz-1;
    for (int cnt=0; cnt<LastOrg; cnt++) {
      if (frand()<Pop_MRate) {
        //LugarPtr lugar = this->forestv.at(cnt);
        org = this->ScoreDexv[cnt];// lugar->tenant;
        org->Mutate_Me(Org_MRate);
      }
    }
    org = this->ScoreDexv[LastOrg];
    org->Rand_Init();
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

