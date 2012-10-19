#include "DTIQueryProcessor.h"

#include "DTIFilterTree.h"
#include "DTIFilterPathwayScalar.h"
#include "DTIFilterAlgorithm.h"

DTIQueryProcessor::DTIQueryProcessor (ROIManager *mgr)
{
  _roi_filters = new DTIFilterTree();
  _roi_filters->setROIManager (mgr);
  _algo_filter = new DTIFilterAlgorithm();
  _algo_filter->setFilter(DTI_PATHWAY_ALGORITHM_ANY);
}

DTIQueryProcessor::~DTIQueryProcessor ()
{
  delete _roi_filters;
  delete _algo_filter;
}

bool
DTIQueryProcessor::doesPathwayMatch (DTIPathway *pathway) 
{
  bool passesProperties = doesPathwayMatchProperties(pathway);
  if (passesProperties) {
    //    bool result = this->_roi_filters->doesPathwayMatch(pathway);
    bool result = true;
    // dakers 8/4/11 - temporarily disabling ROI filters since their
    // behavior is confusing in the refine selection window.
    if (result) return true;
  }
  return false;
}
  
bool
DTIQueryProcessor::setROIFilter (const char *str) 
{
  // return false if function is invalid...
  int result = _roi_filters->setFunc (str);
  if (!result) {
    return false;
  }
  else {
    return true;
  }
}

const char *
DTIQueryProcessor::getROIFilter () 
{
  return _roi_filters->getFunctionString();
}

bool
DTIQueryProcessor::doesPathwayMatchProperties (DTIPathway *pathway)
{
  for (std::vector<DTIFilterPathwayScalar *>::iterator iter = _pathway_property_filters.begin(); iter != _pathway_property_filters.end(); iter++) {
    DTIFilterPathwayScalar *filter = *iter;
    bool result = filter->matches(pathway);
    if (result == false) return false;
  }
  if (_algo_filter != NULL && !_algo_filter->matches(pathway)) 
    return false;
  return true;
}
 
void 
DTIQueryProcessor::appendScalarFilter (DTIFilterPathwayScalar *filter)
{
  _pathway_property_filters.push_back(filter);
}

void 
DTIQueryProcessor::resetROIFilter (ROIManager *mgr)
{
  delete _roi_filters;
  _roi_filters = new DTIFilterTree ();
  _roi_filters->setROIManager(mgr);
}

void 
DTIQueryProcessor::replaceScalarFilter (int rowIndex, PathwayProperty property, double minValue, double maxValue) 
{
  _pathway_property_filters[rowIndex]->setProperty (property);
  _pathway_property_filters[rowIndex]->setMinValue(minValue);
  _pathway_property_filters[rowIndex]->setMaxValue(maxValue);
  
}










