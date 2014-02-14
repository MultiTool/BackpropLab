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
  double avgnumwinners = 0.0;
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
    if (true) {
      BPNet->Create_Simple();
    } else {
      BPNet->Create_Any_Depth();
    }
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
    size_t siz, pcnt;
    siz = forestv.size();
    for (pcnt=0; pcnt<siz; pcnt++) {
      delete forestv.at(pcnt);
    }
    delete BPNet;

    siz = TrainingSets.size();
    for (pcnt=0; pcnt<siz; pcnt++) {
      delete TrainingSets.at(pcnt);
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
    double goal;
    double ScoreBefore;
    double WinCnt;
    IOPairPtr Pair;
    do {
      BPNet->Randomize_Weights();
      ScoreBefore = Dry_Run_Test(16, TSet);
    } while (ScoreBefore==1.0);
    BPNet->Attach_FunSurf(FSurf);
    WinCnt=0.0;
    //MaxNeuroGens/=TSet->size();
    for (GenCnt=0; GenCnt<MaxNeuroGens; GenCnt++) {
      //TSet->Shuffle();
      //for (int paircnt=0; paircnt<TSet->size(); paircnt++) {
      //Pair = TSet->at(paircnt);
      Pair = TSet->at(GenCnt%TSet->size());

      goal = Pair->goalvec.at(0);
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
      // BPNet->Backprop(goal);
      BPNet->Backprop(&(Pair->goalvec));
      //}
    }
    double PrimaryScore = 0;
    if (FinalFail>(MaxNeuroGens-DoneThresh)){
      PrimaryScore = 0.0;
    }else{
      PrimaryScore = 1.0 - ( ((double)FinalFail)/(double)(MaxNeuroGens-DoneThresh) );// oneify
    }
    FSurf->FinalFail = FinalFail;
    FSurf->Score[0] *= PrimaryScore;//1.0 - ( ((double)FinalFail)/(double)MaxNeuroGens );// oneify
    double Remainder = MaxNeuroGens-GenCnt;// if nobody won *earlier*, then score by average goodness of output
    double temp = ( (WinCnt+Remainder)/((double)MaxNeuroGens) ) - ScoreBefore;
    if (temp<0.0){temp=0.0;}
    FSurf->Score[1] *= temp;//oneify
  }
  double fred;
  /*

  scoring issue for multiple training sets
do we just change training sets once for each generation?
or, do we expose each generation to all of the training sets, and try to make a sum score?


  */
  /* ********************************************************************** */
  void Gen(uint32_t evogens, uint32_t gencnt) { // each generation
    uint32_t popsize = this->forestv.size();
    LugarPtr lugar;
    OrgPtr parent, child, candidate;
    uint32_t pcnt;
    LugarPtr place;
    int numwinners;
    int tcnt;
    tcnt = gencnt % TrainingSets.size();
    printf("Pop.Gen(), ");
    //printf(" tcnt:%li, tcnt:%s\n", tcnt, TrainingSets.at(tcnt)->Name);
    printf("\n");
    this->BPNet->Print_Specs();
    for (pcnt=0; pcnt<popsize; pcnt++) {
      lugar = forestv[pcnt];
      candidate = lugar->tenant;
      candidate->Clear_Score();
      candidate->Oneify_Score();
      // this->Run_Test(candidate);
      for (tcnt=0; tcnt<TrainingSets.size(); tcnt++) {
        //TrainingSets.at(tcnt)->Shuffle();
        this->Run_Test(candidate, TrainingSets.at(tcnt));
        //if (tcnt==1){ numwinners = NumWinners(); }
      }
      //fred = candidate->Score[0]; printf("fred:%lf\n", fred);
      //candidate->Rescale_Score(1.0/((double)TrainingSets.size()));

      //fred = candidate->Score[0]; printf("fred:%lf\n", fred);
      // printf("candidate->Score:%lf, %lf\n", candidate->Score[0], candidate->Score[1]);
      bool nop = true;
    }
    double SurvivalRate = 0.5;
    Sort();
    OrgPtr bestbeast = ScoreDexv[0];
    OrgPtr leastbeast = ScoreDexv[this->popsz-2];
    double avgbeast = AvgBeast();
    numwinners = NumWinners();
    avgnumwinners = (avgnumwinners*0.98) + ((double)numwinners)*0.02;
    bestbeast->Print_Me(); printf("\n");
    printf("bestbeast->Score:%lf, %lf\n", bestbeast->Score[0], bestbeast->Score[1]);
    printf("avgbeast Score:%lf, numwinners:%li, avgnumwinners:%lf\n", avgbeast, numwinners, avgnumwinners);
    printf("leastbeast->Score:%lf, %lf\n", leastbeast->Score[0], leastbeast->Score[1]);
    printf("TrainingSets:%li, TrainWay:%s\n", TrainingSets.size(), TW::TrainWayNames[TW::TrainWay]);

    Birth_And_Death(SurvivalRate);

    if (32<(evogens - gencnt)) {
      Mutate(0.8, 0.8);
      printf("Mutation \n");
    } else {
      printf("NO MUTATION \n");// coasting, no evo
    }
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
      //if (ScoreDexv[cnt]->FinalFail < (MaxNeuroGens-DoneThresh)) { wincnt++; }
      if (ScoreDexv[cnt]->Score[0]>0.0) { wincnt++; }
      //printf(" FinalFail:%li\n", ScoreDexv[cnt]->FinalFail);
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
    std::random_shuffle(ScoreDexv.begin(), ScoreDexv.end());
    std::sort (ScoreDexv.begin(), ScoreDexv.end(), DescendingScore);
  }
  /* ********************************************************************** */
  void Birth_And_Death(double SurvivalRate) {
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
  void Init_Training_Sets() {
    TrainSetPtr tset;
    IOPairPtr match;

    tset = new TrainSet(); TrainingSets.push_back(tset);
    { // first XOR
      strcpy(tset->Name, "XOR");
      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back(-1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back( 1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back(-1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back( 1.0); match->goalvec.push_back(-1.0);
    }
    //return;

    tset = new TrainSet(); TrainingSets.push_back(tset);
    { // AND
      strcpy(tset->Name, "AND");
      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back(-1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back( 1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back(-1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back( 1.0); match->goalvec.push_back( 1.0);
    }
    //return;

    tset = new TrainSet(); TrainingSets.push_back(tset);
    { // OR
      strcpy(tset->Name, "OR");
      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back(-1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back( 1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back(-1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back( 1.0); match->goalvec.push_back( 1.0);
    }

    tset = new TrainSet(); TrainingSets.push_back(tset);
    { // NXOR
      strcpy(tset->Name, "NXOR");
      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back(-1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back( 1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back(-1.0); match->goalvec.push_back(-1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back( 1.0); match->goalvec.push_back( 1.0);
    }
    return;

    tset = new TrainSet(); TrainingSets.push_back(tset);
    { // NAND
      strcpy(tset->Name, "NAND");
      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back(-1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back(-1.0); match->invec.push_back( 1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back(-1.0); match->goalvec.push_back( 1.0);

      match = new IOPair(); tset->push_back(match);
      match->invec.push_back(1.0); match->invec.push_back( 1.0); match->invec.push_back( 1.0); match->goalvec.push_back(-1.0);
    }

  }

};

#endif // POP_H_INCLUDED

