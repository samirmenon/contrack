/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef VOI_LISTENER_H
#define VOI_LISTENER_H

#include "DTI_typedefs.h"
#include <util/typedefs.h>

class VOIListener {

 protected:
  VOIListener() {}
  virtual ~VOIListener() {}

public:
  virtual void roiPanelTranslate(double x, double y, double z) = 0;
  virtual void roiPanelScale (double x, double y, double z) = 0;
  virtual void roiPanelLabel (const char *value) = 0;
  
  virtual void roiPanelSetVOI(int activeID) = 0;
  virtual void roiPanelMotion (VOIMotionType type, int id) = 0;
  virtual void roiPanelEnterPressed() = 0;

};

#endif
