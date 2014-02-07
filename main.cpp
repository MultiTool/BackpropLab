#include <iostream>

#include "Base.h"
#include "FunSurf.h"
#include "Node.h"
#include "Cluster.h"
#include "Stack.h"
#include "Pop.h"

using namespace std;

/*
next
get real stats on
percent of times corr squish works with depth of 30 (-5) clusters
percent of times corr classic works with depth of 30 (-5) clusters

how steep a squish helps, and how steep hurts.

get stats on fail rate of 100 (-5) classic vs squish.

next
create 2d funsurf, program it with whole field of corrector * sigderiv, plug it into the network.
use on tiny network.

ways to burn function into funsurf:
pass back a pointer to the function. not great for unknown number of variables. would need an array parameter.
hard code (paste) sigderiv function into funsurf - hacky but easy. do this.
put sig deriv function at base of dependencies. meh, doesn't belong at base.

need a quick translation from dimdex backward to real function input coord.

*/
const uint32_t ndims = 2;
int main() {
  if (false) {
    double alt;
    NumVec nv;

    FunSurfGrid fs(ndims, 8);
    //fs.Create_Dummy_Ramp();
    fs.Create_Sigmoid_Deriv_Surface();
    if (false) {
      //nv.push_back(0.5);nv.push_back(0.5);nv.push_back(0.5);nv.push_back(0.5);
      nv.push_back(0.0); nv.push_back(0.0); nv.push_back(0.0); nv.push_back(0.0);
      alt = fs.Eval(&nv);
    }
    double invec[ndims];
    printf("\n");
    for (double step=-1.0; step<=1.0; step+=0.03) {
      for (int cnt=0; cnt<ndims; cnt++) {
        invec[cnt] = step;
      }
      alt = fs.Eval(invec);
      printf("step:%lf, alt:%lf\n", step, alt);
    }
    printf("\n");
    fs.Print_Me();
    printf("\n");

    double base = 1.0;
    printf("floor(base):%lf\n", floor(base));
    printf("floor(base-DBL_EPSILON):%lf\n", floor(base-DBL_EPSILON));
    printf("Fudge:%lf\n", Fudge/Fudge);
    printf("0.0/0.0:%lf\n", 0.0/0.0);
    return 0;
  }

  int numgens = 4000;
  numgens = 1000;
  int finalfail = 0;
  uint32_t num0, num1;
  double in0, in1;
  cout << "Hello world!" << endl;
  srand(time(NULL));

  if (true) {
    Pop pop;
    pop.Gen();
    return 0;
  }
  double goal;
  Stack stk;
  stk.Create_Simple();

  FunSurfGrid fs(2, 4);
  fs.Create_Sigmoid_Deriv_Surface();
  stk.Attach_FunSurf(&fs);

  for (int gcnt=0; gcnt<numgens; gcnt++) {
    num0 = Pop::Bit2Int(gcnt, 0);
    num1 = Pop::Bit2Int(gcnt, 1);
    in0 = Pop::TransInt(num0);
    in1 = Pop::TransInt(num1);// in0 = TransBit(gcnt, 0); in1 = TransBit(gcnt, 1);
    goal = Pop::TransInt(num0 ^ num1);
    stk.Load_Inputs(in0, in1, 1.0);
    stk.Fire_Gen();
    double fire = stk.OutLayer->NodeList.at(0)->FireVal;
    if ((numgens-6)<gcnt) {
      printf("gcnt:%li, ", gcnt);
      printf("in0:%lf, in1:%lf, ", in0, in1);
      printf("goal:%lf, fire:%lf, delta:%lf, ", goal, fire, goal-fire);
    }
    if ( (goal>0 && fire>0) || (goal<0 && fire<0) ) {
      if ((numgens-6)<gcnt) { printf("Success!"); }
    } else {
      if ((numgens-6)<gcnt) { printf("Fail :( "); }
      finalfail = gcnt;
    }

    stk.Backprop(goal);

    if ((numgens-6)<gcnt) { printf("\n", fire); }
  }
  printf("\n");
  fs.Print_Me(); printf("\n\n");
  stk.Print_Me(); printf("\n");
  printf("numgens:%li, finalfail:%li\n", numgens, finalfail);
  return 0;
}
