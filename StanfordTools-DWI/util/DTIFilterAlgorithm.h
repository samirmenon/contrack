/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FILTER_ALGO_H
#define FILTER_ALGO_H

#include "DTIFilter.h"
#include "typedefs.h"

class DTIFilterAlgorithm : public DTIFilter {
 public:

  DTIFilterAlgorithm();
  virtual ~DTIFilterAlgorithm();
  void setFilter (DTIPathwayAlgorithm algo) { _algo = algo; }
  DTIPathwayAlgorithm getFilter() { return _algo; }
  virtual bool matches (DTIPathway *pathway);
 private:
  DTIPathwayAlgorithm _algo;
};

#endif
