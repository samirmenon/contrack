#ifndef DTI_PATHWAY_INTERSECTION_VOLUME_H
#define DTI_PATHWAY_INTERSECTION_VOLUME_H

#include "DTIVolume.h"
class DTIPathway;

class DTIPathwayIntersectionVolume : public DTIVolume<float> {
 public:
  DTIPathwayIntersectionVolume (DTIVolume<float> *currentBackground);
  void munchPathway (DTIPathway *pathway);
  void rasterizeSegment (const double a[3], const double b[3]);
  void normalize();
 protected:
  void setVoxel (double x, double y, double z);
 private:
  double _max_intersect;

};

#endif
