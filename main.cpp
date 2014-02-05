#include <iostream>

#include "Base.h"
#include "FunSurf.h"
#include "Node.h"
#include "Cluster.h"
#include "Stack.h"

using namespace std;

inline double TransBit(int val, int bitnum) {
  return ((double)((val >> bitnum)&0x1)) * 2.0 - 1.0;
}

inline double TransInt(int val) {
  return ((double)val) * 2.0 - 1.0;
}
inline uint32_t Bit2Int(int val, int bitnum) {
  return ((val >> bitnum)&0x1);
}

/*
next
get real stats on
percent of times corr squish works with depth of 30 (-5) clusters
percent of times corr classic works with depth of 30 (-5) clusters

how steep a squish helps, and how steep hurts.

get stats on fail rate of 100 (-5) classic vs squish.

*/

int main() {
  if (true) {
    NumVec nv;
    //nv.push_back(0.5);nv.push_back(0.5);nv.push_back(0.5);nv.push_back(0.5);
    nv.push_back(0.0);nv.push_back(0.0);nv.push_back(0.0);nv.push_back(0.0);
    FunSurfGrid fs(4);
    fs.Create_Dummy_Ramp();
    fs.Print_Me();
    printf("\n");
    double alt = fs.Eval(&nv);
    printf("alt:%lf\n", alt);
    double base = 1.0;
    printf("floor(base):%lf\n", floor(base));
    printf("floor(base-DBL_EPSILON):%lf\n", floor(base-DBL_EPSILON));
    printf("Fudge:%lf\n", Fudge/Fudge);
    printf("0.0/0.0:%lf\n", 0.0/0.0);
    return 0;
  }

  int numgens = 4000;
  int finalfail = 0;
  uint32_t num0, num1;
  double in0, in1;
  cout << "Hello world!" << endl;
  srand(time(NULL));

  double goal;
  Stack stk;
  stk.Create_Simple();
  for (int gcnt=0; gcnt<numgens; gcnt++) {
    num0 = Bit2Int(gcnt, 0);
    num1 = Bit2Int(gcnt, 1);
    in0 = TransInt(num0);
    in1 = TransInt(num1);// in0 = TransBit(gcnt, 0); in1 = TransBit(gcnt, 1);
    goal = TransInt(num0 ^ num1);
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
  stk.Print_Me();
  printf("numgens:%li, finalfail:%li\n", numgens, finalfail);
  return 0;
}
