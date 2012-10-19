/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_FIBER_TRACT_FACTORY_H
#define DTI_FIBER_TRACT_FACTORY_H

#include "typedefs.h"

#include "DTIPathwayFactoryInterface.h"

class DTIPathwayFactory : public DTIPathwayFactoryInterface {
 public:

  DTIPathwayInterface *createPathway (DTIPathwayAlgorithm algo);

  DTIPathwayFactory() {}
  virtual ~DTIPathwayFactory() {}
};

#endif
