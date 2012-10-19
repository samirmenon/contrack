#ifndef VTK_ROI_FACTORY_H
#define VTK_ROI_FACTORY_H

#include "ROIFactory.h"
class vtkRenderer;
class ROIManager;

class vtkROIFactory : public ROIFactory
{
 public:
  vtkROIFactory(vtkRenderer *renderer, ROIManager *manager) {_renderer = renderer; _voi_manager = manager;}
  virtual PDTIFilterROI DeserializeROI(std::istream &s, int version);

 private:
  vtkRenderer *_renderer;
  ROIManager *_voi_manager;
};
#endif
