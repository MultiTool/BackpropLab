#ifndef TRAININGSETS_H_INCLUDED
#define TRAININGSETS_H_INCLUDED

/* ********************************************************************** */
class IOPair;
typedef IOPair *IOPairPtr;
typedef std::vector<IOPairPtr> IOPairVec;
class IOPair {
public:
  std::vector<double> invec, goalvec;
};
class TrainSet;
typedef TrainSet *TrainSetPtr;
//typedef std::vector<TrainSetPtr> TrainingSetList;
class TrainSet : public IOPairVec {
public:
  char Name[300];
  ~TrainSet() {
    this->Clear();
  };
  void Clear() {
    // IOPairPtr pptr;
    for (int cnt=0; cnt<this->size(); cnt++) {
      delete this->at(cnt);
    }
    this->clear();
  }
  void Shuffle() {
    std::random_shuffle(this->begin(), this->end());
  }
  void Random_Truth(uint32_t Num_InBits) {
    this->Clear();// Create a random truth table with Num_InBits inputs and 1 output.
    IOPairPtr match;
    // Num_InBits = 2;
    uint32_t InTable_Size = 1<<Num_InBits;// 2 to the Num_InBits
    uint32_t OutPossibilities = 1<<InTable_Size;// 2 to the InTable_Size
    OutPossibilities-=2;// eliminate 000... and 111... as outputs
    uint32_t outstatebits = 1 + (rand() % OutPossibilities);
    double outstate, instate;
    for (uint32_t instatecnt=0; instatecnt<InTable_Size; instatecnt++) {
      match = new IOPair(); this->push_back(match);
      outstate = BitInt::TransBit(outstatebits, instatecnt);
      for (uint32_t inbitcnt=0; inbitcnt<Num_InBits; inbitcnt++) {
        instate = BitInt::TransBit(instatecnt, inbitcnt);
        match->invec.push_back(instate);
      }
      match->goalvec.push_back(outstate);
    }
  }
};
/* ********************************************************************** */
class TrainingSetList : public std::vector<TrainSetPtr> {
public:
  ~TrainingSetList() {
      Clear();
  };
  void Clear() {
    size_t siz = this->size();
    for (int pcnt=0; pcnt<siz; pcnt++) {
      delete this->at(pcnt);
    }
    this->clear();
  }
  void Shuffle() {
    std::random_shuffle(this->begin(),this->end());
  }
  void All_Truth(uint32_t Num_InBits) {
    IOPairPtr match;// Generate ALL the truth tables that have Num_InBits inputs and 1 output.
    uint32_t InTable_Size = 1<<Num_InBits;// 2 to the Num_InBits
    uint32_t OutPossibilities = 1<<InTable_Size;// 2 to the InTable_Size
    OutPossibilities-=1;// eliminate 000... and 111... as outputs
    for (uint32_t OutStateBits=1; OutStateBits<OutPossibilities-0; OutStateBits++) {
      TrainSetPtr tset = new TrainSet(); this->push_back(tset);
      double outstate, instate;
      for (uint32_t instatecnt=0; instatecnt<InTable_Size; instatecnt++) {
        match = new IOPair(); tset->push_back(match);
        outstate = BitInt::TransBit(OutStateBits, instatecnt);
        match->invec.push_back(1.0);// dummy bit for constant current
        for (uint32_t inbitcnt=0; inbitcnt<Num_InBits; inbitcnt++) {
          instate = BitInt::TransBit(instatecnt, inbitcnt);
          match->invec.push_back(instate);
        }
        match->goalvec.push_back(outstate);
      }
    }
  }
  static TrainingSetList *All_Truth_Factory(uint32_t Num_InBits) {
    IOPairPtr match;// Generate ALL the truth tables that have Num_InBits inputs and 1 output.
    uint32_t InTable_Size = 1<<Num_InBits;// 2 to the Num_InBits
    uint32_t OutPossibilities = 1<<InTable_Size;// 2 to the InTable_Size
    OutPossibilities-=2;// eliminate 000... and 111... as outputs
    TrainingSetList *tsl = new TrainingSetList();
    for (uint32_t OutStateBits=1; OutStateBits<OutPossibilities-0; OutStateBits++) {
      TrainSetPtr tset = new TrainSet(); tsl->push_back(tset);
      double outstate, instate;
      for (uint32_t instatecnt=0; instatecnt<InTable_Size; instatecnt++) {
        match = new IOPair(); tset->push_back(match);
        outstate = BitInt::TransBit(OutStateBits, instatecnt);
        for (uint32_t inbitcnt=0; inbitcnt<Num_InBits; inbitcnt++) {
          instate = BitInt::TransBit(instatecnt, inbitcnt);
          match->invec.push_back(instate);
        }
        match->goalvec.push_back(outstate);
      }
    }
    return tsl;
  }
};

#endif // TRAININGSETS_H_INCLUDED
