#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include <vector>
class DTIFilterTree;
class DTIFilter;
class ROIManager;
class DTIPathway;
class DTIFilterPathwayScalar;
class DTIFilterAlgorithm;

#include "typedefs.h"

class DTIQueryProcessor {

 public:

  DTIQueryProcessor(ROIManager *mgr);
  virtual ~DTIQueryProcessor();

  bool doesPathwayMatch (DTIPathway *pathway);
  bool setROIFilter (const char *str);
  const char *getROIFilter ();
  void resetROIFilter(ROIManager *mgr);

  void appendScalarFilter (DTIFilterPathwayScalar *filter);
  void replaceScalarFilter (int rowIndex, PathwayProperty property, double minValue, double maxValue);
  void setAlgoFilter (DTIFilterAlgorithm *algorithm) { _algo_filter = algorithm; }
  
  DTIFilterPathwayScalar *getScalarFilter (int rowIndex) { return _pathway_property_filters[rowIndex]; }
  DTIFilterAlgorithm *getAlgoFilter () { return _algo_filter; }

 protected:

  bool doesPathwayMatchProperties (DTIPathway *pathway);
 
  
 private:
  
  std::vector<DTIFilterPathwayScalar *>_pathway_property_filters;  
  DTIFilterAlgorithm *_algo_filter;
  DTIFilterTree *_roi_filters;

  // could cache the results of filters here too.
  // i.e. keep a DTIPathwayDatabase* here.

};

#endif
