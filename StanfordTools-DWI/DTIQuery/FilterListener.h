/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FILTER_PANEL_LISTENER_H
#define FILTER_PANEL_LISTENER_H

#include <util/typedefs.h>

class FilterListener {

 public:
  FilterListener() {}
  virtual ~FilterListener() {}
  
  virtual void filterPanelLength (double minLength, double maxLength) = 0;
  virtual void filterPanelMedianFA (double minFA, double maxFA) = 0;
  virtual void filterPanelAlgorithm (DTIPathwayAlgorithm algo) = 0;
  virtual bool filterPanelFilterText (const char *filter) = 0;
  virtual void filterPanelFilterOp (DTIQueryOperation op) = 0;
  virtual void filterPanelMeanCurvature (double minCurv, double maxCurv) = 0;
  virtual void filterPanelEnterPressed() = 0;
};

#endif
