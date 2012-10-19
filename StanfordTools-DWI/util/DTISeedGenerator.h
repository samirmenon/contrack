#ifndef SEED_GENERATOR_H
#define SEED_GENERATOR_H

#include <typedefs.h>

class DTISeedGenerator {

 public:
  DTISeedGenerator() {}
  virtual ~DTISeedGenerator() {}

  virtual void getNextSeed (DTIVector &pt) = 0;
  virtual void initialize () = 0;
  virtual bool finished() const = 0;
  virtual DTIVector getPos()=0;
  virtual DTIVector getSize()=0;

  virtual double getProgress()=0;
  virtual void setProgress(double d)=0;
 
};

#endif
