#ifndef SEED_BOX_GENERATOR_H
#define SEED_BOX_GENERATOR_H

#include <typedefs.h>
#include "DTISeedGenerator.h"

class DTIRandomSeedGenerator : public DTISeedGenerator {

 public:
  DTIRandomSeedGenerator(const DTIVector &pos, const DTIVector &dim);
  virtual ~DTIRandomSeedGenerator() {}

  virtual void getNextSeed (DTIVector &pt);
  virtual void initialize ();
  virtual bool finished() const { if(_progress>0.9999) return true; else return false; }
  DTIVector getPos(){return _pos;}
  DTIVector getSize(){return _dim;}
  virtual double getProgress() { return _progress; }
  virtual void setProgress(double p){ _progress = p; }


 protected:
  DTIVector _pos;
  DTIVector _dim;
  double _progress;
  
};

#endif
