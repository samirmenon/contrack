/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/


#ifndef DTI_FIBER_TRACT_INTERFACE_H
#define DTI_FIBER_TRACT_INTERFACE_H

#include "typedefs.h"
#include <vector>
#include <list>

struct DTIPathwayStatisticHeader;
class DTIPathwayFactoryInterface;

class DTIPathwayInterface {
 public:

  // XXX Move this into general statistic
  std::vector <double> _path_grow_weight;

  // Append a segment to the path (adds to END of list)
  virtual void append (const DTIVector &vec);

  // Inserts a segment before the given position (0 = first element)
  virtual void insertBefore (const DTIVector &vec, int position);

  // Prepend a segment to the path (adds to BEGINNING of list)
  virtual void prepend (const DTIVector &vec);

  // Remove the node from the path neighboring nodes will be joined by
  // an edge (0 = first element)
  virtual void remove(int position);
  virtual void remove(int p1, int p2);

  // retrieve the x,y,z coords of a point in the pathway, by its index.
  virtual void getPoint (int index, double pt[3]) const;
  virtual void getPoint (int index, float pt[3]) const;
  virtual DTIVector getPointV (int index) const;

  // get the direction from this point to the next in the index
  virtual void getDirection (int index, double pt[3]) const;
  virtual void getDirection (int index, float pt[3]) const;
  virtual DTIVector getDirectionV (int index) const;

  // get the instantaneous symmetric tangent about this point
  virtual DTIVector getTangentV(int index) const;

  // set the x,y,z coords of a point in the pathway, by its index.
  virtual void setPoint (int index, const double pt[3]);
  virtual void setPoint (int index, const DTIVector &vec);
  
  virtual double getStepSize() const;

  // retrieve a point statistic for a point in the pathway, by the point
  // index and the type of statistic:
  virtual double getPointStatistic (int index, PathwayProperty statIndex);
  virtual double getPathStatistic (PathwayProperty statIndex);
  virtual int getNumPathStatistics(){return (int)_path_stat_vector.size();}

  virtual void initializePathStatistics (int numStatistics, std::vector<DTIPathwayStatisticHeader* > *headers, bool initializePerPoint);
  virtual void setPathStatistic (int statIndex, double stat);
  virtual void setPointStatistic (int pointIndex, int statIndex, double stat);
  //  virtual void appendPointStatistic (int statIndex, double stat);

  //  PathwayProperty getLuminanceCacheIndex() { return _point_stat_array.size(); }
  
  virtual void xformPathway(TNT::Array2D<float> xform);

  // Get number of points in the pathway.
  virtual int getNumPoints() const;

  // Set/Get number of times this exact pathway appears in the database
  void setCount(int c) {_count = c;} 
  int getCount() const {return _count;}
  
  // Sets the index of the seed point:
  virtual void setSeedPointIndex (int index) { _seed_point_index = index; }
  int getSeedPointIndex() { return _seed_point_index;}

  // XXX This is a quick hack, because I need reference counting for
  // my metropolis sampler, but don't want to implement it fully
  void addReference(){ _ref_count++; }
  void remReference(){ _ref_count--; if(_ref_count<0) std::cerr << "_ref_count < 0" << std::endl;}
  int getReferenceCount(){return _ref_count;}

  DTIPathwayInterface *deepCopy(DTIPathwayFactoryInterface *pathFactory);

  DTIPathwayInterface *resample (DTIPathwayFactoryInterface *pathFactory, double stepMM);

  DTIPathwayInterface(DTIPathwayAlgorithm algo);
  virtual ~DTIPathwayInterface();

 protected:
  int _seed_point_index;
  int _count;
  int _ref_count;

  DTIPathwayAlgorithm _algo;

  std::vector <DTIGeometryVector *> _point_vector;
  std::vector<std::vector <double> > _point_stat_array;
  std::vector <double> _path_stat_vector;
  std::vector <float*> _aux_buf_points;
 
};

#endif
