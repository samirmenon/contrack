/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FILTER_PATHWAY_SCALAR_H
#define FILTER_PATHWAY_SCALAR_H

#include "DTIFilter.h"
#include "typedefs.h"

class DTIFilterPathwayScalar : public DTIFilter {
 public:

  DTIFilterPathwayScalar(PathwayProperty property);
  virtual ~DTIFilterPathwayScalar();
  void setMinValue (double minimum);
  void setMaxValue (double maximum);
  double getMinValue() { return _min_value; }
  double getMaxValue() { return _max_value; }
  virtual bool matches (DTIPathway *pathway);
  PathwayProperty getProperty() { return _property; }
  void setProperty (PathwayProperty prop) { _property = prop; }

 private:
  double _max_value;
  double _min_value;
  PathwayProperty _property;
};

#endif
