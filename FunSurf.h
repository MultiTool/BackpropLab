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
  uint32_t Rez;
  double SubRez;

  /* ********************************************************************** */
  FunSurf(uint32_t NumDims0) : FunSurf() {
    this->NumDims = NumDims0;
  }
  /* ********************************************************************** */
  FunSurf() {
    Rez = 8;
    SubRez = Rez-1;
  }
  /* ********************************************************************** */
  ~FunSurf() {
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
  uint32_t NumCorners;
  uint32_t NumCells;
  uint32_t *Strides;
  uint32_t *CornerStrides;
  double *Space;
  uint32_t *ctrloc;
  const double DoubleFudge = Fudge*2;
  /* ********************************************************************** */
  FunSurfGrid() {
    Rez = 8;
    SubRez = Rez-1;
  }
  /* ********************************************************************** */
  FunSurfGrid(uint32_t NumDims0) : FunSurfGrid() {
    this->NumDims = NumDims0;
    this->NumCorners = 1<<NumDims;
    this->NumCells = pow(Rez, NumDims);
    Strides = allocsafe(uint32_t, NumDims);
    ctrloc = allocsafe(uint32_t, NumDims);
    CornerStrides = allocsafe(uint32_t, NumCorners);
    Space = allocsafe(double, NumCells);
    Calc_Corner_Strides();
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
    int LastDim = this->NumDims-1;
    double cellval;
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
        cellval += ((double)dimdex)*slope;
        rem = rem % Strides[dcnt];
      }
      //printf("\n");
      this->Space[cellcnt] = cellval;
    }
  }
  /* ********************************************************************** */
  void Print_Me() override {
    printf("Space:");
    for (int cellcnt=0; cellcnt<this->NumCells; cellcnt++) {
      if (cellcnt%Rez==0) {
        printf("\n");
      }
      if (cellcnt%(Rez*Rez)==0) {
        printf("\n");
      }
      double cellval = this->Space[cellcnt];
      printf("%lf, ", cellval);
    }
  }
  /* ********************************************************************** */
  double Eval(NumVecPtr invec) override {
    int vecsiz = invec->size();
    uint32_t mincomp = NumDims<vecsiz?NumDims:vecsiz;// only compare to the smallest dimensions
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
  void Map_To_GridX(std::vector<double> *ctrpoint) {
    // ctrpoint is -1 to +1 space, map to 0 to 8 space
    std::vector<uint32_t> ctrloc;
    uint32_t orgint;

    for (int dcnt=0; dcnt<NumDims; dcnt++) {
      double coord = ctrpoint->at(dcnt);
      coord = (coord + 1.0)/2.0;
      coord = Fudge + (coord * (SubRez-DoubleFudge));//coord = coord * 7.0; // 8;

      // coord = Fudge + ( (coord + 1.0) * 0.5 * (SubRez-DoubleFudge));//coord = coord * 7.0; // 8;

      orgint = floor(coord);
      // floor of coord cannot be less than 0, ceiling of coord cannot be greater than or equal to 7.
      // must be (0<= coord <=7)
      ctrloc.at(dcnt) = orgint;
    }
  }
};

#endif // FUNSURF_H_INCLUDED
