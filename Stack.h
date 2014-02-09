#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

/* ********************************************************************** */
class Stack;// for layered 1-way networks
typedef Stack *StackPtr;
class Stack {
public:
  ClusterVec Layers;
  ClusterPtr InLayer, OutLayer;
  /* ********************************************************************** */
  Stack() {
  }
  /* ********************************************************************** */
  ~Stack() {
    for (int lcnt=0; lcnt<Layers.size(); lcnt++) {
      delete Layers.at(lcnt);
    }
  }
  /* ********************************************************************** */
  void Attach_FunSurf(FunSurfGridPtr fsurf0) {
    size_t cnt;
    ClusterPtr cluster;
    size_t siz = this->Layers.size();
    for (cnt=0; cnt<siz; cnt++) {
      cluster = this->Layers.at(cnt);
      cluster->Attach_FunSurf(fsurf0);
    }
  }
  /* ********************************************************************** */
  void Create_Simple() {
    ClusterPtr clprev, clnow;
    double LRate = 0.5;
    InLayer = clnow = new Cluster(3); Layers.push_back(clnow);
    clnow->Set_Learning_Rate(LRate);
    clprev = clnow;
    for (int lcnt=0; lcnt<1; lcnt++) {
      clnow = new Cluster(2); Layers.push_back(clnow);
      clnow->Connect_Other_Cluster(clprev);
      clnow->Set_Learning_Rate(LRate);
      clprev = clnow;
    }
    OutLayer = clnow = new Cluster(1); Layers.push_back(clnow);
    clnow->Connect_Other_Cluster(clprev);
    clnow->Set_Learning_Rate(LRate);
  }
  /* ********************************************************************** */
  void Create_Any_Depth() {
    ClusterPtr clprev, clnow;
    double LRate = 0.5;
    InLayer = clnow = new Cluster(3); Layers.push_back(clnow);
    clnow->Set_Learning_Rate(LRate);
    clprev = clnow;
    int largo = 200;
    // largo = 1;
    largo = 10;
    //largo = 20;
    //largo = 30; largo = 100;
    for (int lcnt=0; lcnt<largo; lcnt++) {
      //clnow = new Cluster(4); Layers.push_back(clnow);
      //clnow = new Cluster(2+largo); Layers.push_back(clnow);
      clnow = new Cluster(largo/4); Layers.push_back(clnow);
      clnow->Connect_Other_Cluster(clprev);
      if(true) {
        if (lcnt<largo-5) {
          LRate = 0.5;
        } else {
          LRate = 0.0;
        }
      }
      clnow->Set_Learning_Rate(LRate);
      clprev = clnow;
    }
    OutLayer = clnow = new Cluster(1); Layers.push_back(clnow);
    clnow->Connect_Other_Cluster(clprev);
    clnow->Set_Learning_Rate(0.0);
  }
  /* ********************************************************************** */
  void Load_Inputs(double in0, double in1, double in2) {
    InLayer->NodeList.at(0)->FireVal = in0;
    InLayer->NodeList.at(1)->FireVal = in1;
    InLayer->NodeList.at(2)->FireVal = in2;
  }
  /* ********************************************************************** */
  void Fire_Gen() {
    int lcnt;
    ClusterPtr clnow;
    clnow = Layers.at(0);
    for (lcnt=1; lcnt<Layers.size(); lcnt++) {
      clnow->Push_Fire();// emit
      clnow = Layers.at(lcnt);
      clnow->Collect_And_Fire();// absorb
    }
  }
  /* ********************************************************************** */
  void Backprop(double goal) {
    int lcnt;
    ClusterPtr clnow;

    double FireVal = OutLayer->NodeList.at(0)->FireVal;
    OutLayer->NodeList.at(0)->Corrector = goal-FireVal;

    int LastLayer = Layers.size()-1;
    clnow = Layers.at(LastLayer);
    for (lcnt=LastLayer-1; lcnt>=0; lcnt--) {
      clnow->Push_Correctors_Backward();
      clnow = Layers.at(lcnt);
      clnow->Pull_Correctors();// absorb
    }

    for (lcnt=LastLayer; lcnt>=0; lcnt--) {
      clnow = Layers.at(lcnt);
      clnow->Apply_Correctors();
    }
  }
  /* ********************************************************************** */
  void Randomize_Weights() {
    ClusterPtr clnow;
    size_t lcnt;
    size_t siz = this->Layers.size();
    for (lcnt=0; lcnt<siz; lcnt++) {
      clnow = Layers.at(lcnt);
      clnow->Randomize_Weights();
    }
  }
  /* ********************************************************************** */
  void Print_Me() {
    size_t cnt;
    ClusterPtr cluster;
    printf(" Stack this:%p, ", this);
    size_t siz = this->Layers.size();
    printf(" num Clusters:%li\n", siz);
    for (cnt=0; cnt<siz; cnt++) {
      cluster = this->Layers.at(cnt);
      cluster->Print_Me(cnt);
    }
  }
};

#endif // STACK_H_INCLUDED
