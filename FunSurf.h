#ifndef FUNSURF_H_INCLUDED
#define FUNSURF_H_INCLUDED

#include "Base.h"

typedef std::vector<double> NumVec;
typedef NumVec *NumVecPtr;

/* ********************************************************************** */
class FunSurf;// function surface class
typedef FunSurf *FunSurfPtr;
typedef std::vector<FunSurfPtr> FunSurfVec;
class FunSurf {
public:
  uint32_t NumDims;
  uint32_t Rez;
  double SubRez;

  uint32_t NumCorners;
  uint32_t *Strides;
  uint32_t *CornerStrides;
  const double DoubleFudge = Fudge*2;
  /* ********************************************************************** */
  FunSurf(uint32_t NumDims0) : FunSurf() {
    this->NumDims = NumDims0;
    NumCorners = 1<<NumDims;
    Strides = allocsafe(uint32_t, NumDims);
    CornerStrides = allocsafe(uint32_t, NumCorners);
    Calc_Corner_Strides();
  }
  /* ********************************************************************** */
  FunSurf() {
    Rez = 8;
    SubRez = Rez-1;
  }
  /* ********************************************************************** */
  ~FunSurf() {
    freesafe(CornerStrides);
    freesafe(Strides);
  }
  /* ********************************************************************** */
  virtual double Eval(NumVecPtr invec) {
    int vecsiz = invec->size();
    uint32_t mincomp = NumDims<vecsiz?NumDims:vecsiz;// only compare to the smallest dimensions
    return 1.0;// dummy
  }
  /* ********************************************************************** */
  void Calc_Corner_Strides() {
    uint32_t tempdex;
    uint32_t bitoffset;
    uint32_t stride = 1;// array stride for an N-dimensional array
    for (int dcnt=0; dcnt<NumDims; dcnt++) {
      Strides[dcnt] = stride;
      stride*=Rez;
    }
    //dim0 + dim1*Rez + dim2*Rez*Rez + dim3*Rez*Rez*Rez +
    for (int pcnt=0; pcnt<NumCorners; pcnt++) {
      tempdex = 0;
      for (int dcnt=0; dcnt<NumDims; dcnt++) {
        bitoffset = (pcnt >> dcnt) & 0x1;
        if (bitoffset==1) { tempdex += Strides[dcnt]; }
      }
      CornerStrides[pcnt] = tempdex;
    }
  }
  /* ********************************************************************** */
  void Map_To_Grid(std::vector<double> *ctrpoint) {
    // ctrpoint is -1 to +1 space, map to 0 to 8 space
    std::vector<uint32_t> ctrloc;
    uint32_t orgint;
    double coord = ctrpoint->at(0);
    coord = (coord + 1.0)/2.0;
    //coord = coord * 7.0; // 8;
    coord = Fudge + (coord * (SubRez-DoubleFudge));
    orgint = floor(coord);
    // floor of coord cannot be less than 0, ceiling of coord cannot be greater than or equal to 7.
    // must be (0<= coord <=7)
  }
  /* ********************************************************************** */
  void test(std::vector<uint32_t> *ctrloc, std::vector<double> *ctrpoint) {
    // the purpose here is to linearly interpolate the value of a point inside a 1*1*1etc N-dimesional cube, based on the corner values.
    uint32_t bitoffset;
    double Space[Rez^NumDims];
    double PointVal;
    uint32_t NbrDexs[NumCorners];
    double weights[NumDims][2];
    // ctrloc is the 0,0,0... origin of the 1.0*1.0 box where the ctrpoint hit. AKA floor() of all ctrpoint coords
    uint32_t orgdex = 0;

    // convert ND coordinates into a 1D array index
    for (int dcnt=0; dcnt<NumDims; dcnt++) {
      orgdex += ctrloc->at(dcnt)*Strides[dcnt];// darn, we'll have to do this for EVERY index.
    }
    // get 1D indexes of neighboring corners
    for (int pcnt=0; pcnt<NumCorners; pcnt++) {
      NbrDexs[pcnt] = orgdex + CornerStrides[pcnt];
    }

    /*
    add 1 stride[dim] to each dimension of a point to get the point's neighbor in that dimension.  NOT GOOD ON EDGES.
    */
    double coord, offset;
    for (int dcnt=0; dcnt<NumDims; dcnt++) {
      // generate dim * 2 array  here
      //coord = ctrpoint->at(dcnt);// ctrpoint is already mapped to array index space
      offset = ctrpoint->at(dcnt) - ctrloc->at(dcnt);// ctrpoint is assumed to be already mapped to array index space
      weights[dcnt][0] = 1.0 - offset; weights[dcnt][1] = offset;
    }
    double NbrVal;

    for (int pcnt=0; pcnt<NumCorners; pcnt++) {
      // this first loop hits every corner of the hypercube
      NbrVal = Space[NbrDexs[pcnt]];
      PointVal = NbrVal; // ?? somehow get the value of this point
      // create weights for this PointVal
      for (int dcnt=0; dcnt<NumDims; dcnt++) {
        bitoffset = (pcnt >> dcnt) & 0x1;
        PointVal *= weights[dcnt][bitoffset];
      }
    }
    // here, PointVal is the final altitude answer.
  }
};

#endif // FUNSURF_H_INCLUDED

