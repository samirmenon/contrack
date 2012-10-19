/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FIBER_TRACT_DATABASE_H
#define FIBER_TRACT_DATABASE_H

class DTIPathway;
class DTIFilter;
class DTIFilterTree;
class ROIManager;
struct DTIPathwayStatisticHeader;
struct DTIPathwayAlgorithmHeader;
struct DTIPathwayStatisticThreshold;
class ComputeListener;
//class DTIVolumeInterface;
//class DTIScalarVolumeInterface;

#include "typedefs.h"

#include <list>
#include <vector>

enum StatConstraint {
  STAT_CONSTRAINT_NONE = 0,
  STAT_CONSTRAINT_LUMINANCE = 1,
  STAT_CONSTRAINT_STAT = 2
};

class DTIPathwayDatabase {
  friend class DTIPathwayIO;

 public:
  DTIPathwayDatabase();
  virtual ~DTIPathwayDatabase();

  DTIPathwayDatabase *copy();
  void addPathway (DTIPathway *pathway);
  void removePathway (DTIPathway *pathway);
  void removeAllPathways();
  int getNumFibers();
  DTIPathway *getPathway(int index);
  void replacePathway (int index, DTIPathway *newPathway);
  void clearAllPathways();

  int computeStatIndex (StatConstraint encoding, bool pointStatistic, int index);
  int getNumAlgorithms ();
  int getNumPathStatistics(StatConstraint encoding = STAT_CONSTRAINT_NONE);
  DTIPathwayStatisticHeader *getPathStatisticHeader (int id);
  DTIPathwayStatisticThreshold *getPathStatistic(int id);

  //! \return the index of the path based on the statistic name. -1 if no such statistic exist.
  int getStatisticIndex( std::string statistic_name);

  std::vector<DTIPathwayStatisticHeader *> *getPathStatisticHeaders() { return &_pathway_statistic_headers; }
  int getNumPointStatistics(StatConstraint encoding = STAT_CONSTRAINT_NONE);
  void addStatisticHeader (DTIPathwayStatisticHeader *header);
  void clearStatisticHeaders();
  DTIPathwayAlgorithmHeader *getPathAlgorithmHeader (int id);
  std::vector<DTIPathwayAlgorithmHeader *> *getPathAlgorithmHeaders() { return &_pathway_algorithm_headers; }
  void addAlgorithmHeader (DTIPathwayAlgorithmHeader *header);
/*   void computeAllStats(DTIVolumeInterface *tensors, ScalarVolumeInterface *faVol, ComputeListener *listener); */
/*   void setupStandardStatisticHeaders(); */
  
  void setTransformMatrix (const double mx[16]);
  const double *getTransformMatrix() { return &(_transform_mx[0]);}
  void getBoundingBox (double xRange[2], double yRange[2], double zRange[2]);
  double computeMinValue (PathwayProperty property);
  double computeMaxValue (PathwayProperty property);
  double getMinValue (PathwayProperty property);
  double getMaxValue (PathwayProperty property);

  void setVoxelSize (const double mmScale[3]);
  void setVoxelSize (const float mmScale[3]);
  void getVoxelSize (double mmScale[3]);
  void getVoxelSize (float mmScale[3]);
  void setSceneDimension (const unsigned int dim[3]); // voxel dimensions of scene
  void getSceneDimension (unsigned int dim[3]);

  static std::string getNullStatisticsName(){return "Blank";}
 private:
  const double *getMinValues();
  const double *getMaxValues();
  bool _recompute_min_needed;
  bool _recompute_max_needed;

  std::vector<DTIPathway *> _fiber_pathways;
  std::vector<DTIPathwayStatisticHeader *> _pathway_statistic_headers;
  std::vector<DTIPathwayAlgorithmHeader *> _pathway_algorithm_headers;
  std::vector< DTIPathwayStatisticThreshold *> _stats_threshold;

  double _transform_mx[16];
  double _mm_scale[3];
  unsigned int _scene_dim[3];
};


#endif
