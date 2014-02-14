#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

/* ********************************************************************** */
/*  THIS IS BACKPROP NODE */
/* ********************************************************************** */

#include <iostream>
#include <stdio.h> // printf
#include <map>
#include <list>
#include "Base.h"
#include "FunSurf.h"

#define WeightAmp 2.0;

namespace IoType {
  enum IoType {Intra=0, GlobalIO=1, NbrIO=2};
}
/* ********************************************************************** */
class Node;
typedef Node *NodePtr;
typedef std::vector<NodePtr> NodeVec;
typedef double WeightType;
/* ********************************************************************** */
class Link;
typedef Link *LinkPtr;
class Link {
public:
  WeightType Weight;
  double FireVal;
  double Corrector;
  NodePtr USNode,DSNode;
  Link() {
    this->FireVal=0.0;
    this->USNode=NULL; this->DSNode=NULL;
    Randomize_Weight();
  }
  ~Link() {
    bool nop = true;
  }
  inline double GetFire() {
    return this->FireVal*this->Weight;
  }
  inline double GetCorrector() {
    return this->Corrector*this->Weight;
  }
  void Randomize_Weight(){
    this->Weight = (frand()-0.5) * WeightAmp;// to do: do this with a distribution change
  }
  void Print_Me() {
    printf("  Link ");
    printf("USNode:%p, DSNode:%p ", this->USNode, this->DSNode);
    printf("Weight:%lf \n", this->Weight);
  }
};
typedef std::vector<LinkPtr> LinkVec;
/* ********************************************************************** */
class Node {
public:
  LinkVec Working_Ins, Working_Outs;
  double RawFire, FireVal, PrevFire;
  double Corrector;
  double LRate;
  double MinCorr, MaxCorr;
  FunSurfGridPtr fsurf;
  /* ********************************************************************** */
  Node() {
    Init();
    this->LRate = 0.0;
  }
  /* ********************************************************************** */
  ~Node() {
    int cnt;
    for (cnt=0; cnt<this->Working_Ins.size(); cnt++) {
      delete this->Working_Ins.at(cnt);
    }
    this->Working_Ins.clear();// probably not necessary
    this->Working_Outs.clear();
  }
  /* ********************************************************************** */
  void Init(){
    this->FireVal = ((frand()*2.0)-1.0)*0.001;
    this->PrevFire = ((frand()*2.0)-1.0)*0.001;
    MinCorr = INT32_MAX;
    MaxCorr = INT32_MIN;
  }
  /* ********************************************************************** */
  void Attach_FunSurf(FunSurfGridPtr fsurf0) {
    this->fsurf = fsurf0;
  }
  /* ********************************************************************** */
  void Collect_And_Fire() {
    LinkPtr ups;
    double Sum=0;
    size_t siz = this->Working_Ins.size();
    for (int cnt=0; cnt<siz; cnt++) {
      ups = this->Working_Ins.at(cnt);
      Sum+=ups->GetFire();
    }
    this->RawFire = Sum;
    this->FireVal = ActFun(Sum);
  }
  /* ********************************************************************** */
  double ActFun(double xin) {
    double OutVal;
    OutVal = xin / sqrt(1.0 + xin * xin);/* symmetrical sigmoid function in range -1.0 to 1.0. */
    return OutVal;
    /* General formula: double power = 2.0; OutVal = xin / Math.pow(1 + Math.abs(Math.pow(xin, power)), 1.0 / power); */
  }
  /* ********************************************************************** */
  void Push_Fire() {
    LinkPtr downs;
    double MyFire=this->FireVal;
    size_t siz = this->Working_Outs.size();
    for (int cnt=0; cnt<siz; cnt++) {
      downs = this->Working_Outs.at(cnt);
      downs->FireVal = MyFire;
    }
  }
  /* ********************************************************************** */
  void Push_Correctors_Backward() {
    LinkPtr ups;
    size_t siz = this->Working_Ins.size();
    for (int cnt=0; cnt<siz; cnt++) {
      ups = this->Working_Ins.at(cnt);
      ups->Corrector = this->Corrector;
    }
  }
  /* ********************************************************************** */
  void Pull_Correctors() {
    double ClipRad = 20.0;
    LinkPtr downs;
    double Corr, TestCorr;
    double SurfParams[2];
    double MyFire=this->FireVal;
    double Fire_Deriv = 0.0;
    Corr = 0.0;
    size_t siz = this->Working_Outs.size();
    for (int cnt=0; cnt<siz; cnt++) {
      downs = this->Working_Outs.at(cnt);
      Corr += downs->GetCorrector();
      TestCorr += downs->GetCorrector();
    }
    switch (TW::TrainWay) {
    case TW::Classic :
      Fire_Deriv = sigmoid_deriv_postfire(MyFire);
      this->Corrector = Fire_Deriv * Corr;
      break;
    case TW::Surf :
      SurfParams[0] = ActFun(Corr/4.0);
      SurfParams[1] = MyFire;
      this->Corrector = this->fsurf->Eval(SurfParams);
      this->Corrector *= 4.0;
      break;
    }

    if (MinCorr>TestCorr) {
      MinCorr = TestCorr;
    }
    if (MaxCorr<TestCorr) {
      MaxCorr = TestCorr;
    }
  }
  /* ********************************************************************** */
  void Apply_Corrector(double CorrDelta) {
    LinkPtr ups;
    double num, fire;
    double ModCorrDelta = CorrDelta * this->LRate;
    size_t siz = this->Working_Ins.size();
    /*
    first we get the whole input vector and unitize it (sum of squares?). that becomes the recognition vector.
    then mult the recog vector by the corrector (and by lrate) and add it to my input weights.
    */
    double SumSq=0.0;
    for (int cnt=0; cnt<siz; cnt++) {
//      ups = this->Working_Ins.at(cnt); SumSq += ups->FireVal * ups->FireVal;
      fire = this->Working_Ins.at(cnt)->FireVal;
      SumSq += fire * fire;
    }
    if (this->LRate==123.0){
        bool nop = true;
    }
    for (int cnt=0; cnt<siz; cnt++) {
      ups = this->Working_Ins.at(cnt);
      num = ups->FireVal / SumSq;
      ups->Weight += num * ModCorrDelta;
    }
  }
  /* ********************************************************************** */
  void Print_Me() {
    printf(" Node FireVal:%lf, MinCorr:%lf, MaxCorr:%lf, LRate:%lf, this:%p, ", this->FireVal, MinCorr, MaxCorr, LRate, this);
    size_t siz = this->Working_Ins.size();
    printf(" numlinks:%li\n", siz);
    return;// snox
    for (int cnt=0; cnt<siz; cnt++) {
      LinkPtr lnk = this->Working_Ins.at(cnt);
      lnk->Print_Me();
    }
  }
  /* ********************************************************************** */
  void ConnectIn(NodePtr other) {// attach upstream node to me
    LinkPtr ln = new Link();
    ConnectIn(other, ln);
  }
  /* ********************************************************************** */
  void Randomize_Weights() {
    LinkPtr ups;
    Init();// clear metrics, etc.
    size_t siz = this->Working_Ins.size();
    for (int cnt=0; cnt<siz; cnt++) {
      ups = this->Working_Ins.at(cnt);
      ups->Randomize_Weight();
    }
  }
  /* ********************************************************************** */
  void ConnectIn(NodePtr other, LinkPtr ln) {// attach upstream node to me
    this->Working_Ins.push_back(ln);// this approach uses less memory, fewer allocations/frees and is probably faster.
    other->Working_Outs.push_back(ln);
    ln->USNode = other; ln->DSNode = this;
  }
//class Sigmoid {   public:
  /* *************************************************************************************************** */
  static double sigmoid_deriv_raw(double Value) {
    double vsq = Value*Value;// pre sym sigmoid deriv (from raw sum before actfun) genuine derivative
    double denom1 = pow((vsq + 1.0),(3/2));
    double retval = ( 1.0/sqrt(vsq + 1.0) ) - ( vsq / denom1 );
    return retval;
    /*
    http://www.quickmath.com/webMathematica3/quickmath/calculus/differentiate/basic.jsp#c=differentiate_basicdifferentiate&v1=%28x+%2F+sqrt%281.0+%2B+x*x%29%29&v2=x
    pre sym sigmoid deriv (from raw sum before actfun):
    ( 1.0/sqrt(x*x + 1.0) ) - ( x*x / (x*x + 1.0)^(3/2) )
    */
  }
  /* *************************************************************************************************** */
  static double sigmoid_deriv_postfire(double Value) {
    double MovedValue = (1.0+Value)/2.0;// first map range -1 ... +1 to 0 ... +1
    double retval = 2.0 * MovedValue * (1.0-MovedValue);// APPROXIMATE post sym sigmoid deriv (from fire value after actfun):
    return retval;
    /*
    APPROXIMATE post sym sigmoid deriv (from fire value after actfun):
    4*(x+0.5) * (1.0-(x+0.5))

    output * (1 - output)  derivative of ASYM sigmoid function, after actfun (hillock function)
    */
  }
//};
};

#endif // NODE_H_INCLUDED


