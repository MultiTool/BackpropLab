#ifndef CLUSTER_H_INCLUDED
#define CLUSTER_H_INCLUDED

#include "Node.h"
#include "TrainingSets.h"

/* ********************************************************************** */
class Cluster;
typedef Cluster *ClusterPtr;
typedef std::vector<ClusterPtr> ClusterVec;
class Cluster {
public:
  NodeVec NodeList;
  NodeVec *NodeListPtr;
  /* ********************************************************************** */
  Cluster() {
    NodeListPtr = &(NodeList);
  }
  /* ********************************************************************** */
  Cluster(int Num_Nodes) : Cluster() {
    NodeListPtr = &(NodeList);
    Fill_With_Nodes(Num_Nodes);
  }
  /* ********************************************************************** */
  ~Cluster() {
    NodePtr ndp;
    for (int ncnt=0; ncnt<this->NodeList.size(); ncnt++) {
      ndp = this->NodeList.at(ncnt);
      delete ndp;
    }
  }
  /* ********************************************************************** */
  void Fill_With_Nodes(int Num_Nodes) {
    NodePtr ndp;
    int cnt;
    this->NodeList.resize(Num_Nodes);
    for (cnt=0; cnt<Num_Nodes; cnt++) {
      ndp = new Node();
      this->NodeList.at(cnt) = ndp;
    }
  }
  /* ********************************************************************** */
  void Connect_Other_Cluster(ClusterPtr upstreamer) {
    // Connect upstream cluster all-to-all to this one's inputs
    size_t siz0 = this->NodeList.size();
    size_t siz1 = upstreamer->NodeList.size();
    NodePtr dsn, usn;
    int cnt0, cnt1;
    for (cnt0=0; cnt0<siz0; cnt0++) {
      dsn = this->NodeList.at(cnt0);
      for (cnt1=0; cnt1<siz1; cnt1++) {
        usn = upstreamer->NodeList.at(cnt1);
        dsn->ConnectIn(usn);
      }
    }
  }
  /* ********************************************************************** */
  void Set_Learning_Rate(double LRate) {
    NodePtr ndp;
    for (int ncnt=0; ncnt<this->NodeList.size(); ncnt++) {
      ndp = this->NodeList.at(ncnt);
      ndp->LRate = LRate;
    }
  }
  /* ********************************************************************** */
  void Attach_FunSurf(FunSurfGridPtr fsurf0) {
    NodePtr ndp;
    for (int ncnt=0; ncnt<this->NodeList.size(); ncnt++) {
      ndp = this->NodeList.at(ncnt);
      ndp->Attach_FunSurf(fsurf0);
    }
  }
  /* ********************************************************************** */
  void Push_Fire() {
    NodePtr ndp;
    int cnt;
    size_t siz = this->NodeList.size();
    for (cnt=0; cnt<siz; cnt++) {
      ndp = this->NodeList.at(cnt);
      ndp->Push_Fire();
    }
    // right here the fire vals reside in the links
  }
  /* ********************************************************************** */
  void Collect_And_Fire() {
    NodePtr ndp;
    int cnt;
    // right here the fire vals reside in the links
    size_t siz = this->NodeList.size();
    for (cnt=0; cnt<siz; cnt++) {
      ndp = this->NodeList.at(cnt);
      ndp->Collect_And_Fire();
    }
  }
  /* ********************************************************************** */
  void Fire_Gen() {
    Push_Fire();
    // right here the fire vals reside in the links
    Collect_And_Fire();
  }
  /* ********************************************************************** */
  void Push_Correctors_Backward() {
    NodePtr ndp;
    int cnt;
    size_t siz = this->NodeList.size();
    for (cnt=0; cnt<siz; cnt++) {
      ndp = this->NodeList.at(cnt);
      ndp->Push_Correctors_Backward();
    }
  }
  /* ********************************************************************** */
  void Pull_Correctors() {
    NodePtr ndp;
    int cnt;
    size_t siz = this->NodeList.size();
    for (cnt=0; cnt<siz; cnt++) {
      ndp = this->NodeList.at(cnt);
      ndp->Pull_Correctors();
    }
  }
  /* ********************************************************************** */
  void Apply_Correctors() {
    NodePtr ndp;
    int cnt;
    size_t siz = this->NodeList.size();
    for (cnt=0; cnt<siz; cnt++) {
      ndp = this->NodeList.at(cnt);
      ndp->Apply_Corrector(ndp->Corrector);
    }
  }
  /* ********************************************************************** */
  void Randomize_Weights() {
    NodePtr ndp;
    size_t cnt;
    size_t siz = this->NodeList.size();
    for (cnt=0; cnt<siz; cnt++) {
      ndp = this->NodeList.at(cnt);
      ndp->Randomize_Weights();
    }
  }
  /* ********************************************************************** */
  void Print_Me(int ClusterNum) {
    size_t cnt;
    NodePtr ndp;
    printf(" --------------------------------\n");
    printf(" Cluster ClusterNum:%li, this:%p, ", ClusterNum, this);
    size_t siz = this->NodeList.size();
    printf(" num nodes:%li\n", siz);
    for (cnt=0; cnt<siz; cnt++) {
      ndp = this->NodeList.at(cnt);
      ndp->Print_Me();
    }
    printf("\n");
  }
  /* ********************************************************************** */
  void Load_Inputs(std::vector<double> *invec) {
    size_t siz = invec->size();
    if (siz<this->NodeList.size()) { siz = this->NodeList.size(); }
    for (size_t cnt=0; cnt<siz; cnt++) {
      this->NodeList.at(cnt)->FireVal = invec->at(cnt);
    }
  }
  /* ********************************************************************** */
  void Load_Correctors(std::vector<double> *goalvec) {
    size_t siz = goalvec->size();
    if (siz<this->NodeList.size()) { siz = this->NodeList.size(); }
    double FireVal;
    NodePtr node;
    for (size_t cnt=0; cnt<siz; cnt++) {
      node = this->NodeList.at(cnt);
      FireVal = node->FireVal;
      node->Corrector = goalvec->at(cnt)-FireVal;
    }
  }
};

#endif // CLUSTER_H_INCLUDED
