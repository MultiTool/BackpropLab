#ifndef FUNSURF_H_INCLUDED
#define FUNSURF_H_INCLUDED

#include "Base.h"

typedef std::vector<double> NumVec;
typedef NumVec *NumVecPtr;

// http://stackoverflow.com/questions/7070346/c-best-way-to-get-integer-division-and-remainder

/* ********************************************************************** */
class FunSurf;// function surface class
typedef FunSurf *FunSurfPtr;
typedef std::vector<FunSurfPtr> FunSurfVec;
class FunSurf {
public:
  uint32_t NumDims;
  /* ********************************************************************** */
  FunSurf(uint32_t NumDims0) {//} : FunSurf(Rez0) {
    this->NumDims = NumDims0;
  }
  /* ********************************************************************** */
  ~FunSurf() {
  }
  /* ********************************************************************** */
  virtual double Eval(double *HitVec) {
    return 1.0;// dummy
  }
  /* ********************************************************************** */
  virtual double Eval(NumVecPtr invec) {
    int vecsiz = invec->size();
    uint32_t mincomp = NumDims<vecsiz?NumDims:vecsiz;// only compare to the smallest dimensions
    return 1.0;// dummy
  }
  /* ********************************************************************** */
  void Map_To_Grid(std::vector<double> *ctrpoint) {
  }
  /* ********************************************************************** */
  virtual void Print_Me() {
  }
};


/* ******************************************************************************************************************************************** */
class FunSurfGrid;
typedef FunSurfGrid *FunSurfGridPtr;
typedef std::vector<FunSurfGridPtr> FunSurfGridVec;
class FunSurfGrid : public FunSurf {
public:
  uint32_t Rez, LastRez;
  double SubRez;
  uint32_t NumCorners;
  uint32_t NumCells;
  uint32_t *Strides;
  uint32_t *CornerStrides;
  double *Space;
  uint32_t *ctrloc;
  const double DoubleFudge = Fudge*2;
  /* ********************************************************************** */
//  FunSurfGrid(uint32_t Rez0) {
//    Rez = Rez0;
//    SubRez = Rez-1;
//  }
  /* ********************************************************************** */
  FunSurfGrid(uint32_t NumDims0, uint32_t Rez0) : FunSurf(NumDims0) {//: FunSurfGrid(Rez0) {
    Rez = Rez0; SubRez = Rez-1;
    LastRez = Rez-1;
    this->NumCorners = 1<<NumDims;
    this->NumCells = pow(Rez, NumDims);
    Strides = allocsafe(uint32_t, NumDims);
    ctrloc = allocsafe(uint32_t, NumDims);
    CornerStrides = allocsafe(uint32_t, NumCorners);
    Space = allocsafe(double, NumCells);
    Calc_Corner_Strides();
    Create_Seed_Surface();
  }
  /* ********************************************************************** */
  ~FunSurfGrid() {
    freesafe(Space);
    freesafe(CornerStrides);
    freesafe(Strides);
    freesafe(ctrloc);
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
  void Create_Dummy_Ramp() {
    // double fred = Node::sigmoid_deriv_postfire(3.0);
    int LastDim = this->NumDims-1;
    double cellval;
    double temp;
    double slope = 1.0/(double)this->NumDims;
    uint32_t dimdex, rem;
    //div_t split;
    for (int cellcnt=0; cellcnt<this->NumCells; cellcnt++) {
      rem = cellcnt;
      cellval = 0.0;
      for (int dcnt=LastDim; dcnt>=0; dcnt--) {
        //split = div(rem, Strides[dcnt]); dimdex = split.quot; rem = split.rem;
        dimdex = rem / Strides[dcnt];
        // dimdex = cellcnt / Strides[dcnt];
        //printf("cellcnt:%li, dimdex:%li\n", cellcnt, dimdex);
        temp = slope*( ((double)dimdex)/((double)Rez-1)*2.0-1.0 );
        cellval += temp;//((double)dimdex)*slope;
        rem = rem % Strides[dcnt];
      }
      //printf("\n");
      //this->Space[cellcnt] = cellval-(0.5*(double)(this->Rez));
      this->Space[cellcnt] = cellval;
    }
  }
  /* ********************************************************************** */
  void Print_Me() override {
    printf("Space (NumCells=%li):", this->NumCells);
    for (int cellcnt=0; cellcnt<this->NumCells; cellcnt++) {
      if (cellcnt%Rez==0) {
        printf("\n");
      }
      if (cellcnt%(Rez*Rez)==0) {
        printf("\n");
      }
      double cellval = this->Space[cellcnt];
      printf("%10.5lf, ", cellval);
    }
  }
  /* ********************************************************************** */
  double Eval(double *HitVec) override {
    // the purpose here is to linearly interpolate the value of a point inside a 1*1*1etc N-dimesional cube, based on the corner values.
    uint32_t bitoffset;

    double PointVal;
    uint32_t NbrDexs[NumCorners];
    uint32_t NbrDex;
    double weights[NumDims][2];
    double ctrpoint_mapped[NumDims];
    // ctrloc is the 0,0,0... origin of the 1.0*1.0 box where the HitVec hit. AKA floor() of all HitVec coords
    uint32_t orgdex = 0;

    for (int dcnt=0; dcnt<NumDims; dcnt++) {
      double coord = HitVec[dcnt];
      coord = (coord + 1.0)/2.0;
      coord = Fudge + (coord * (SubRez-DoubleFudge));//coord = coord * 7.0; // 8;
      // coord = Fudge + ( (coord + 1.0) * 0.5 * (SubRez-DoubleFudge));//coord = coord * 7.0; // 8;
      ctrpoint_mapped[dcnt] = coord;
      ctrloc[dcnt] = floor(coord);
      //printf("coord:%lf, ctrloc:%li\n ", coord, ctrloc[dcnt]);
      // floor of coord cannot be less than 0, ceiling of coord cannot be greater than or equal to 7.
      // must be (0<= coord <=7)
    }
    // convert ND coordinates into a 1D array index
    for (int dcnt=0; dcnt<NumDims; dcnt++) {
      orgdex += ctrloc[dcnt]*Strides[dcnt];// darn, we'll have to do this for EVERY index.
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
      offset = ctrpoint_mapped[dcnt] - ctrloc[dcnt];// ctrpoint is assumed to be already mapped to array index space
      weights[dcnt][0] = 1.0 - offset; weights[dcnt][1] = offset;
    }
    double NbrVal;
    PointVal = 0.0;

    for (int pcnt=0; pcnt<NumCorners; pcnt++) {
      // this first loop hits every corner of the hypercube
      NbrDex = NbrDexs[pcnt];
      if (NbrDex>=this->NumCells) {
        printf("Error! NbrDex[%li] is too big for NumCells[%li]. ", NbrDex, NumCells);
        throw 1234;
      }
      NbrVal = Space[NbrDex];
      //NbrVal = Space[NbrDexs[pcnt]];
      //printf("NbrDex:%li, NbrVal:%lf, ", NbrDex, NbrVal); printf("\n");
      // create weights for this PointVal
      for (int dcnt=0; dcnt<NumDims; dcnt++) {
        bitoffset = (pcnt >> dcnt) & 0x1;
        NbrVal *= weights[dcnt][bitoffset];
        //printf("weights[dcnt][bitoffset]:%lf, dcnt:%li, bitoffset:%li ", weights[dcnt][bitoffset], dcnt, bitoffset);
        //printf("\n");
      }
      PointVal += NbrVal;
    }
    // here, PointVal is the final altitude answer.
    return PointVal;
  }
  /* ********************************************************************** */
  double Eval(NumVecPtr invec) override {
    int vecsiz = invec->size();
    return Map_To_Grid(invec);
  }
  /* ********************************************************************** */
  double Map_To_Grid(NumVecPtr ctrpoint) {
    // the purpose here is to linearly interpolate the value of a point inside a 1*1*1etc N-dimesional cube, based on the corner values.
    uint32_t bitoffset;

    double PointVal;
    uint32_t NbrDexs[NumCorners];
    double weights[NumDims][2];
    double ctrpoint_mapped[NumDims];
    // ctrloc is the 0,0,0... origin of the 1.0*1.0 box where the ctrpoint hit. AKA floor() of all ctrpoint coords
    uint32_t orgdex = 0;

    uint32_t orgint;

    for (int dcnt=0; dcnt<NumDims; dcnt++) {
      double coord = ctrpoint->at(dcnt);
      coord = (coord + 1.0)/2.0;
      coord = Fudge + (coord * (SubRez-DoubleFudge));//coord = coord * 7.0; // 8;

      // coord = Fudge + ( (coord + 1.0) * 0.5 * (SubRez-DoubleFudge));//coord = coord * 7.0; // 8;
      ctrpoint_mapped[dcnt] = coord;
      orgint = floor(coord);
      // floor of coord cannot be less than 0, ceiling of coord cannot be greater than or equal to 7.
      // must be (0<= coord <=7)
      ctrloc[dcnt] = orgint;
    }
    // convert ND coordinates into a 1D array index
    for (int dcnt=0; dcnt<NumDims; dcnt++) {
      orgdex += ctrloc[dcnt]*Strides[dcnt];// darn, we'll have to do this for EVERY index.
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
      offset = ctrpoint_mapped[dcnt] - ctrloc[dcnt];// ctrpoint is assumed to be already mapped to array index space
      weights[dcnt][0] = 1.0 - offset; weights[dcnt][1] = offset;
    }
    double NbrVal;
    PointVal = 0.0;

    for (int pcnt=0; pcnt<NumCorners; pcnt++) {
      // this first loop hits every corner of the hypercube
      NbrVal = Space[NbrDexs[pcnt]];
      printf("NbrVal:%lf, ", NbrVal);
      printf("\n");
      // create weights for this PointVal
      for (int dcnt=0; dcnt<NumDims; dcnt++) {
        bitoffset = (pcnt >> dcnt) & 0x1;
        NbrVal *= weights[dcnt][bitoffset];
        printf("weights[dcnt][bitoffset]:%lf, dcnt:%li, bitoffset:%li ", weights[dcnt][bitoffset], dcnt, bitoffset);
        printf("\n");
      }
      PointVal += NbrVal;
    }

    // here, PointVal is the final altitude answer.
    return PointVal;
  }
  /* ********************************************************************** */
  void Create_Seed_Surface() {
    // only works for 2 dimensions, Rez of 4x4.
    if (NumDims!=2 || Rez!=4) {
      printf("Wrong dims or rez for Create_Seed_Surface. NumDims:%li, Rez:%li\n", NumDims, Rez);
      throw 20;
    }
    static const double WeirdSpace[16] = {// results of 14 training sets on 3 layer net for 16000 evogens:
      0.04867, 0.98687, 0.84539, -0.66855, /**/0.08880, -0.10979, 0.94619, -0.40974, /**/-0.36289, -0.09001, 0.16976, 0.18663, /**/-0.03881, -0.17056, 0.14299, 0.39953
    };
    static const double WeirdSpace2[16] = {
      -1.00920,   -0.03257,    0.03759,    0.75073, /**/
      -0.24873,   -0.27913,    0.27312,    0.58155, /**/
       0.16936,   -0.28706,    0.28773,    0.16862, /**/
      -0.92993,   -0.04704,    0.03760,    0.47212, /**/
    };
    static const double WeirdSpace3[16] = {// symmetrical classic sig deriv
      -0.06514,   -0.03257,    0.03759,    0.07518, /**/
      -0.54624,   -0.27913,    0.27312,    0.54624, /**/
      -0.57546,   -0.28706,    0.28773,    0.57546, /**/
      -0.09408,   -0.04704,    0.03760,    0.07520, /**/
    };
    for (int cnt=0; cnt<16; cnt++) {
      //Space[cnt]=WeirdSpace3[cnt]*8.0;
      Space[cnt]=WeirdSpace3[cnt];
    }
    /*
    // results of 14 training sets on 3 layer net for 16000 evogens:
    0.04867,    0.98687,    0.84539,   -0.66855,
    0.08880,   -0.10979,    0.94619,   -0.40974,
    -0.36289,   -0.09001,    0.16976,    0.18663,
    -0.03881,   -0.17056,    0.14299,    0.39953,
    */
  }
  /* ********************************************************************** */
  void Create_Sigmoid_Deriv_Surface() {
    // only works with 2d fun surf
    int LastDim = this->NumDims-1;
    double fireval, corrval, sigval;
    double slope = 1.0/(double)this->NumDims;
    uint32_t dimdex, rem;
    int dex;
    for (int firecnt=0; firecnt<this->Rez; firecnt++) {
      int base = Strides[1]*firecnt;
      fireval = ( ((double)firecnt)/((double)LastRez)*2.0-1.0 );
      sigval = sigmoid_deriv_postfire(fireval);
      for (int corrcnt=0; corrcnt<this->Rez; corrcnt++) {
        dex = base + corrcnt;
        corrval = ( ((double)corrcnt)/((double)LastRez)*2.0-1.0 );
        // to do: map firecnt and corrcnt to -1.0 to +1.0 range
        this->Space[dex] = sigval * corrval;
      }
    }
  }
  /* *************************************************************************************************** */
  static double sigmoid_deriv_postfire(double Value) {
    double MovedValue = (1.0+Value)/2.0;// first map range -1 ... +1 to 0 ... +1
    double retval = 2.0 * MovedValue * (1.0-MovedValue);// APPROXIMATE post sym sigmoid deriv (from fire value after actfun):
    return retval;
  }
};

#endif // FUNSURF_H_INCLUDED
