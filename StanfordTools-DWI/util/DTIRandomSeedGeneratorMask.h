#ifndef SEED_GENERATOR_MASK_H
#define SEED_GENERATOR_MASK_H

#include <vector>
#include <typedefs.h>
#include "DTISeedGenerator.h"

//class DTIVolume;

class DTIRandomSeedGeneratorMask : public DTISeedGenerator {

 public:
  DTIRandomSeedGeneratorMask(int index){_index=index;}
  //DTIRandomSeedGeneratorMask(const DTIVolume<double>* vol, int index);
  virtual ~DTIRandomSeedGeneratorMask() {}

  // Inherited
  virtual void getNextSeed (DTIVector &pt);
  virtual void initialize ();
  virtual bool finished() const { if(_progress>0.9999) return true; else return false; }
  virtual double getProgress() { return _progress; }
  virtual void setProgress(double p){ _progress = p; }
  // Don't use these
  virtual DTIVector getPos(){DTIVector v(3); v[0]=-1;v[1]=-1;v[2]=-1; return v;}
  virtual DTIVector getSize(){DTIVector v(3); v[0]=-1;v[1]=-1;v[2]=-1; return v;}

  // Specific for this class
  void setIndex(int n){_index = n;}
  int getIndex(){return _index;}
  void addLoc(DTIVector& v){_vecLocs.push_back(v.copy());}
  int getNumStartLocs(){return _vecLocs.size();}
  DTIVector getLoc(int id){return _vecLocs[id].copy();}


 protected:
  std::vector<DTIVector> _vecLocs;
  int _index;
  double _progress;
  
};

#endif
