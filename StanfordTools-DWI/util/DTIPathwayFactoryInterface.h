/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_FIBER_TRACT_FACTORY_INTERFACE_H
#define DTI_FIBER_TRACT_FACTORY_INTERFACE_H

#include "typedefs.h"

class DTIPathwayInterface;

class DTIPathwayFactoryInterface {
 public:

  virtual DTIPathwayInterface *createPathway (DTIPathwayAlgorithm algo) = 0;

 protected:
  DTIPathwayFactoryInterface() {}
  virtual ~DTIPathwayFactoryInterface() {}
};

#endif
