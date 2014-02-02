#ifndef FUNSURF_H_INCLUDED
#define FUNSURF_H_INCLUDED

typedef std::vector<double> NumVec;
typedef NumVec *NumVecPtr;

/* ********************************************************************** */
class FunSurf;// function surface class
typedef FunSurf *FunSurfPtr;
typedef std::vector<FunSurfPtr> FunSurfVec;
class FunSurf {
public:
  uint32_t NumDims;

  /* ********************************************************************** */
  FunSurf(uint32_t NumDims0) : FunSurf() {
    this->NumDims = NumDims0;
  }
  /* ********************************************************************** */
  FunSurf() {
  }
  /* ********************************************************************** */
  virtual double Eval(NumVecPtr invec) {
    int vecsiz = invec->size();
    uint32_t mincomp = NumDims<vecsiz?NumDims:vecsiz;// only compare to the smallest dimensions
    return 1.0;// dummy
  }
};

#endif // FUNSURF_H_INCLUDED
