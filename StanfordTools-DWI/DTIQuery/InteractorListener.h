/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef INTERACTOR_WINDOW_LISTENER_H
#define INTERACTOR_WindowLISTENER_H

class DTIPathwayDatabase;

class InteractorListener {

 public:
  InteractorListener() {}
  virtual ~InteractorListener() {}

  virtual void interactorWindowVOIUpdate (double x, double y, double z,
	  double xsize, double ysize, double zsize, int activeID, const char *label, int numVOIs, int *roiIDs, VOIMotionType motionType, int motionID) = 0;

  virtual void interactorWindowSliderUpdate (double minLength, double maxLength, double minFA, double maxFA, double minCurv, double maxCurv) = 0;

  virtual void interactorFilterStringUpdate (DTIQueryOperation op, const char *newstr) = 0;

  virtual void interactorWindowDatabaseLoaded (DTIPathwayDatabase *database) = 0;

  virtual void interactorWindowUnitsChanged (int units) = 0;
  
  virtual void interactorWindowAlgoUpdate (DTIPathwayAlgorithm algo) = 0;
};


#endif
