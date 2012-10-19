#ifndef ROI_FACTORY_H
#define ROI_FACTORY_H

#include "DTIVolume.h"

class ROIFactory
{
 public:
  virtual PDTIFilterROI DeserializeROI(std::istream &s, int version) = 0;

};
#endif
