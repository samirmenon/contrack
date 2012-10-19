/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIPathwayInterface.cpp
 *    DATE: Mon Jun 21 23:47:52 PDT 2004
 *************************************************************************/
#include "DTIPathwayInterface.h"
#include "DTIPathwayDatabase.h"
#include "DTIPathwayStatisticHeader.h"
#include <tnt/tnt_array1d_utils.h>
#include "DTIPathwayFactoryInterface.h"
#include "DTIStats.h"
#include <math.h>
#include <iostream>
using namespace std;

/*************************************************************************
 * Function Name: DTIPathwayInterface::append
 * Parameters: const DTIVector &vec
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayInterface::append(const DTIVector &vec)
{
  //float *v = new float[3];
  //  v[0] = vec[0];
  //  v[1] = vec[1];
  //  v[2] = vec[2];

  DTIGeometryVector *copy = new DTIGeometryVector(3);
  (*copy)[0] = vec[0];
  (*copy)[1] = vec[1];
  (*copy)[2] = vec[2];
  _point_vector.push_back (copy);

  //XXX 
  _path_grow_weight.push_back(0);
}


/*************************************************************************
 * Function Name: DTIPathwayInterface::prepend
 * Parameters: const DTIVector &vec
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayInterface::prepend(const DTIVector &vec)
{
  DTIGeometryVector *copy = new DTIGeometryVector(3);
  (*copy)[0] = vec[0];
  (*copy)[1] = vec[1];
  (*copy)[2] = vec[2];
  _point_vector.insert (_point_vector.begin(), copy);

  // XXX
  _path_grow_weight.insert (_path_grow_weight.begin(),0);
}

/*************************************************************************
 * Function Name: DTIPathwayInterface::remove
 * Parameters: int position
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayInterface::remove(int position)
{
  if((int)_point_vector.size() > position) {
    // Delete the actual data for the point
    delete _point_vector[position];
    // Remove the point from the list
    _point_vector.erase(_point_vector.begin()+position);
  }
  // Remove any stats for this point
  if((int)_point_stat_array.size() > position)
    _point_stat_array.erase(_point_stat_array.begin()+position);
  if((int)_path_stat_vector.size() > position)
  _path_stat_vector.erase(_path_stat_vector.begin()+position);
  //delete _aux_buf_points[position];
  //_aux_buf_points.erase(_aux_buf_points.begin()+position);

  _path_grow_weight.erase(_path_grow_weight.begin()+position);
}

/*************************************************************************
 * Function Name: DTIPathwayInterface::remove
 * Parameters: int pStart, int pEnd
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayInterface::remove(int pStart, int pEnd)
{
  assert(pStart<pEnd);
  assert(pEnd<=(int)_point_vector.size());
  // Remove points  
  for(int pp=pStart; pp<pEnd; pp++) {
    delete _point_vector[pp];
    if(!_point_stat_array.empty()) 
      _point_stat_array[pp].clear();
  }
  _point_vector.erase(_point_vector.begin()+pStart,_point_vector.begin()+pEnd);
  if(!_point_stat_array.empty())
    _point_stat_array.erase(_point_stat_array.begin()+pStart,_point_stat_array.begin()+pEnd);
  if(!_path_stat_vector.empty())
    _path_stat_vector.erase(_path_stat_vector.begin()+pStart,_path_stat_vector.begin()+pEnd);
}


/*************************************************************************
 * Function Name: DTIPathwayInterface::getPoint
 * Parameters: int index, double pt[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayInterface::getPoint(int index, double pt[3]) const
{
  //assert (index <= getNumPoints()-1 && index >= 0);
  if( !(index <= getNumPoints()-1 && index >= 0) ) {
    std::cerr<< index << std::endl;
  }
  DTIGeometryVector v = (*_point_vector[index]);
  pt[0] = v[0];
  pt[1] = v[1];
  pt[2] = v[2];
}

/*************************************************************************
 * Function Name: DTIPathwayInterface::getPoint
 * Parameters: int index, float pt[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayInterface::getPoint(int index, float pt[3]) const
{
  //assert (index <= getNumPoints()-1 && index >= 0);
  if( !(index <= getNumPoints()-1 && index >= 0) ) {
    std::cerr<< index << std::endl;
  }
  DTIGeometryVector v = (*_point_vector[index]);
  pt[0] = v[0];
  pt[1] = v[1];
  pt[2] = v[2];
}

/*************************************************************************
 * Function Name: DTIPathwayInterface::getPointV
 * Parameters: int index
 * Returns: DTIVector 
 * Effects: 
 *************************************************************************/
DTIVector
DTIPathwayInterface::getPointV(int index) const
{
  //assert (index <= getNumPoints()-1 && index >= 0);
  if( !(index <= getNumPoints()-1 && index >= 0) ) {
    std::cerr<< index << std::endl;
  }
  DTIGeometryVector v = (*_point_vector[index]);
  DTIVector vec(3);
  vec[0] = v[0];
  vec[1] = v[1];
  vec[2] = v[2];
  return vec.copy();
}

/*************************************************************************
 * Function Name: DTIPathwayInterface::xformPathway
 * Parameters: TNT::Array2D<float>
 * Returns: void
 * Effects: 
 *************************************************************************/
void 
DTIPathwayInterface::xformPathway(TNT::Array2D<float> xform)
{
  DTIGeometryVector v(4);
  for (int ii = 0; ii < (int)_point_vector.size(); ii++) {
    v[0] = (*_point_vector[ii])[0];
    v[1] = (*_point_vector[ii])[1];
    v[2] = (*_point_vector[ii])[2];
    v[3] = 1;
    v = xform*v;
    (*_point_vector[ii])[0] = v[0];
    (*_point_vector[ii])[1] = v[1];
    (*_point_vector[ii])[2] = v[2];
  }
}

/*************************************************************************
 * Function Name: DTIPathwayInterface::getDirection
 * Parameters: int index, double pt[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayInterface::getDirection(int index, double pt[3]) const
{
  // XXX No assert in -mno-cygwin
  //  assert (index + 1 <= getNumPoints()-1 && index >= 0);
  DTIGeometryVector v1 = (*_point_vector[index]).copy();
  DTIGeometryVector v2 = (*_point_vector[index+1]).copy();
  v2 -= v1;
  double dist = sqrt( v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]);
  pt[0] = v2[0] / dist;
  pt[1] = v2[1] / dist;
  pt[2] = v2[2] / dist;
}

/*************************************************************************
 * Function Name: DTIPathwayInterface::getStepSize
 * Parameters:
 * Returns: double
 * Effects: 
 *************************************************************************/
double
DTIPathwayInterface::getStepSize() const
{
  int index = 0;
  // XXX No assert in -mno-cygwin
  //  assert (index + 1 <= getNumPoints()-1 && index >= 0);
  double dist = 0;
  if( _point_vector.size() >1 ) {
    // There are multiple points so there are steps, so we can figure out the step size
    DTIGeometryVector v1 = (*_point_vector[index]).copy();
    DTIGeometryVector v2 = (*_point_vector[index+1]).copy();
    v2 -= v1;
    dist = sqrt( v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]);
  }
  return dist;
}


/*************************************************************************
 * Function Name: DTIPathwayInterface::getDirection
 * Parameters: int index, float pt[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayInterface::getDirection(int index, float pt[3]) const
{
  // XXX No assert in -mno-cygwin
  //  assert (index + 1 <= getNumPoints()-1 && index >= 0);
  DTIGeometryVector v1 = (*_point_vector[index]).copy();
  DTIGeometryVector v2 = (*_point_vector[index+1]).copy();
  v2 -= v1;
  double dist = sqrt( v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]);
  pt[0] = v2[0] / dist;
  pt[1] = v2[1] / dist;
  pt[2] = v2[2] / dist;
}

/*************************************************************************
 * Function Name: DTIPathwayInterface::getDirectionV
 * Parameters: int index
 * Returns: DTIVector
 * Effects: 
 *************************************************************************/
DTIVector
DTIPathwayInterface::getDirectionV(int index) const
{
  if(!(index + 1 <= getNumPoints()-1 && index >= 0)) {
    std::cerr << "Accessing bad index of pathway. Exiting." << std::endl;
    exit(0);
  }
  DTIGeometryVector v1 = (*_point_vector[index]).copy();
  DTIGeometryVector v2 = (*_point_vector[index+1]).copy();
  v2 -= v1;
  double dist = sqrt( v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]);
  DTIVector vec(3);
  vec[0] = v2[0] / dist;
  vec[1] = v2[1] / dist;
  vec[2] = v2[2] / dist;
  return vec.copy();
}



/*************************************************************************
 * Function Name: DTIPathwayInterface::getNumPoints
 * Parameters: 
 * Returns: int
 * Effects: 
 *************************************************************************/
int
DTIPathwayInterface::getNumPoints() const
{
  return (int)_point_vector.size();
}



/***********************************************************************
 *  Method: DTIPathwayInterface::getPointStatistic
 *  Params: int pointIndex, int statIndex
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIPathwayInterface::getPointStatistic(PointProperty pointIndex, PathwayProperty statIndex)
{
  //  assert (pointIndex < _point_stat_array[statIndex].size());
  if (pointIndex >= (int)_point_stat_array[statIndex].size()) {
    return 0.0;
  }
  return (_point_stat_array[statIndex])[pointIndex];
}


/***********************************************************************
 *  Method: DTIPathwayInterface::getPathStatistic
 *  Params: int statIndex
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIPathwayInterface::getPathStatistic(PathwayProperty statIndex)
{
  return _path_stat_vector[statIndex];
}





/***********************************************************************
 *  Method: DTIPathwayInterface::initializePathStatistics
 *  Params: int numStatistics
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayInterface::initializePathStatistics(int numStatistics, std::vector <DTIPathwayStatisticHeader *> *statHeaders, bool initializePerPoint)
{
  _path_stat_vector.clear();
  _point_stat_array.clear();


  // points MUST be loaded first.
  double statInit = 0.0;
  for (int i = 0; i < numStatistics; i++) {
    _path_stat_vector.push_back (statInit);
  }

  if (!initializePerPoint) {
    return;
  }
  // XXX why was this luminance feature here
  // luminance cache:
  //_path_stat_vector.push_back (statInit);

  std::vector<double> initVec;
  for (int i = 0; i < getNumPoints(); i++) {
    initVec.push_back (0);
  }

  for (int i = 0; i < numStatistics; i++) {
    if ((*statHeaders)[i]->_is_computed_per_point) {
      _point_stat_array.push_back (initVec);
    }
    else {
      std::vector<double> dummy;
      _point_stat_array.push_back(dummy);
    }
  }  
  _point_stat_array.push_back (initVec);
  // luminance cache
}


/***********************************************************************
 *  Method: DTIPathwayInterface::setPathStatistic
 *  Params: int statIndex, double stat
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayInterface::setPathStatistic(PathwayProperty statIndex, double stat)
{
  if (statIndex >= 0) {
    _path_stat_vector[statIndex] = stat;
  }
}


/***********************************************************************
 *  Method: DTIPathwayInterface::setPointStatistic
 *  Params: int pointIndex, int statIndex, double stat
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayInterface::setPointStatistic(PointProperty pointIndex, PathwayProperty statIndex, double stat)
{
  //  cerr << "starting" << endl;
  //  cerr << "stat index: " << statIndex << endl;
  //  cerr << "size: " << _point_stat_array[statIndex].size() << endl;
  //  cerr << "point index: " << pointIndex << endl;
  if (statIndex >= 0) {
    //assert (pointIndex < _point_stat_array[statIndex].size());
    (_point_stat_array[statIndex])[pointIndex] = stat;
  }
  //  cerr << "ending" << endl;
}

#if 0

/***********************************************************************
 *  Method: DTIPathwayInterface::appendPointStatistic
 *  Params: int statIndex, double stat
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayInterface::appendPointStatistic(PathwayProperty statIndex, double stat)
{
  _point_stat_array[statIndex].push_back(stat);
}
#endif

/***********************************************************************
 *  Method: DTIPathwayInterface::insertAfter
 *  Params: const DTIVector &vec, int position
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayInterface::insertBefore(const DTIVector &vec, int position)
{

  float *v = new float[3];
  v[0] = vec[0];
  v[1] = vec[1];
  v[2] = vec[2];

  DTIGeometryVector *copy = new DTIGeometryVector(v);
  int curPos = 0;
  for (std::vector<DTIGeometryVector *>::iterator iter = _point_vector.begin();
       iter != _point_vector.end(); iter++) {
    if (curPos == position) {
      _point_vector.insert (iter, copy);
      break;
    }
    curPos++;
  }

  // XXX 
  _path_grow_weight.insert( _path_grow_weight.begin()+position,0);
}

/***********************************************************************
 *  Method: DTIPathwayInterface::setPoint
 *  Params: int index, const double pt[3]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayInterface::setPoint(int index, const double pt[3]) 
{
  //assert (index <= getNumPoints()-1 && index >= 0);
  DTIGeometryVector *v = _point_vector[index];
  (*v)[0] = pt[0];
  (*v)[1] = pt[1];
  (*v)[2] = pt[2];
}

/***********************************************************************
 *  Method: DTIPathwayInterface::setPoint
 *  Params: int index, const DTIVector &vec
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayInterface::setPoint(int index, const DTIVector &vec) 
{
  //if(index > getNumPoints()-1 || index < 0)
  //assert (index <= getNumPoints()-1 && index >= 0);

  DTIGeometryVector *v = _point_vector[index];
  (*v)[0] = vec[0];
  (*v)[1] = vec[1];
  (*v)[2] = vec[2];
}

/***********************************************************************
 *  Method: DTIPathwayInterface::deepCopy
 *  Params: 
 * Returns: DTIPathwayInterface *
 * Effects: 
 ***********************************************************************/
DTIPathwayInterface *
DTIPathwayInterface::deepCopy(DTIPathwayFactoryInterface *pathFactory)
{
  // should maybe use the copy constructor instead.

  DTIPathwayInterface *newPathway = pathFactory->createPathway(_algo);

  // add all points, stats, etc.
  int i;
  for (i = 0; i < (int)_point_vector.size(); i++) {
    DTIGeometryVector *vec = _point_vector[i];
    DTIGeometryVector *newGeom = new DTIGeometryVector(3);
    (*newGeom)[0] = (*vec)[0];
    (*newGeom)[1] = (*vec)[1];
    (*newGeom)[2] = (*vec)[2];
    newPathway->_point_vector.push_back(newGeom);
  }

  // XXX
  for (i = 0; i < (int)_path_grow_weight.size(); i++) {
    newPathway->_path_grow_weight.push_back(_path_grow_weight[i]);
  }

  // xxx does not yet copy statistics... assertion below tests for this.
  //XXXX ignoring for now
  //assert (_path_stat_vector.size() == 0);

  newPathway->_seed_point_index = _seed_point_index;
  return newPathway;
}


/***********************************************************************
 *  Method: DTIPathwayInterface::~DTIPathwayInterface
 *  Params: 
 * Effects: 
 ***********************************************************************/
DTIPathwayInterface::~DTIPathwayInterface()
{
  //  cerr << "pathway destroyed: " << this << endl;

  // XXX must clean-up pathway data  

  // XXX watch out for DTIGeometryVector
  
  _point_stat_array.clear();
  _path_stat_vector.clear();
  int k;
  for(k=0; k<(int)_point_vector.size(); k++) {
    delete _point_vector[k];
    //delete _aux_buf_points[k];
  }
  _point_vector.clear();
  //_aux_buf_points.clear();

//  std::vector <DTIGeometryVector *> _point_vector;
//   std::vector<std::vector <double> > _point_stat_array;
//   std::vector <double> _path_stat_vector;

// float *v = new float[3];
//     v[0] = (*vec)[0];
//     v[1] = (*vec)[1];
//     v[2] = (*vec)[2];
//     DTIGeometryVector *newGeom = new DTIGeometryVector(3, v);

// XXX
  _path_grow_weight.clear();
}



/***********************************************************************
 *  Method: DTIPathwayInterface::DTIPathwayInterface
 *  Params: DTIPathwayAlgorithm algo
 * Effects: 
 ***********************************************************************/
DTIPathwayInterface::DTIPathwayInterface(DTIPathwayAlgorithm algo)
{ 
  //  cerr << "pathway created: " << this << endl;

  _algo = algo;
  _ref_count = 1;
}


void getSampleInfo (double arcLength, int &minIndex, double &minArcLength, double &interpolation, DTIPathwayInterface *pathway) 

{
  /* minIndex and minArcLength are both in/out parameters! */

  // (this would be inefficient if we had to walk the whole path every time,
  // so we use the previous point and arclength to make our job easier.)

  double arcLengthRemaining = arcLength - minArcLength;
  
  // static double p1Data[3];
//   static double p2Data[3];
  static float p1Data[3];
  static float p2Data[3];
  static DTIVector *p1 = new DTIVector(p1Data);
  static DTIVector *p2 = new DTIVector(p2Data);
  //assert (minIndex+1 < pathway->getNumPoints());
  pathway->getPoint(minIndex, *p1);
  pathway->getPoint(minIndex+1, *p2);  
  double distance = abs(*p1 - *p2);
  while (arcLengthRemaining > distance) {
    arcLengthRemaining -= distance;
    minArcLength += distance;
    DTIVector *swap = p1;
    p1 = p2;
    p2 = swap;
    if (minIndex+2 >= pathway->getNumPoints()) {
      cerr << "arcLength: " << arcLength << endl;
      cerr << "path length: " << DTIStats::computeLength(pathway,-1,0) << endl;
      cerr << "minArcLength: " << minArcLength << endl;
      cerr << "minIndex: " << minIndex << endl;
      cerr << "numPoints: " << pathway->getNumPoints() << endl;
      exit(0);
      //assert (false);
    }
    minIndex++;
    pathway->getPoint (minIndex+1, *p2);
    distance = abs(*p1 - *p2);
  }
  //delete p1;
  //delete p2;
  interpolation = arcLengthRemaining / distance;
}

/***********************************************************************
 *  Method: DTIPathwayInterface::resample
 *  Params: DTIPathwayFactoryInterface *pathFactory
 * Returns: DTIPathwayInterface *
 * Effects: 
 ***********************************************************************/
DTIPathwayInterface *
DTIPathwayInterface::resample(DTIPathwayFactoryInterface *pathFactory, double stepMm)
{
  DTIPathwayInterface *newPathway = pathFactory->createPathway(_algo);
  double pathLength = DTIStats::computeLength (this, -1, 0);

  int numPoints = (int) ceil (pathLength/stepMm) + 1; // endpoint

  DTIVector resampledPoint(3);

  DTIVector interpStart(3);
  DTIVector interpEnd(3);
  //DTIVector oldPoint(3);
  double arcLength = 0;
  double minArcLength = 0;
  int minIndex = 0;

  for (int i = 0; i < numPoints; i++) {
    if (i == 0) {
      getPoint (0, resampledPoint);
    }
    if (i == numPoints-1) {
      getPoint (getNumPoints()-1, resampledPoint);
    }
    else {
      arcLength = (double) i / (numPoints-1) * pathLength;
      //cerr << "arclength: " << arcLength << endl;
      double interpolation;
      getSampleInfo (arcLength, minIndex, minArcLength, interpolation, this); 
      getPoint (minIndex, interpStart);
      getPoint (minIndex+1, interpEnd);
      resampledPoint = (interpStart*(1-interpolation) + interpEnd*(interpolation));
      //if (i != 0) {cerr << "distance: " << DTIStats::computeDistance (resampledPoint, oldPoint) << endl;}
      //oldPoint = resampledPoint.copy();
    }
    newPathway->append(resampledPoint);
  }
  return newPathway;

}


/***********************************************************************
 *  Method: DTIPathwayInterface::getTangentV
 *  Params: int index
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIPathwayInterface::getTangentV(int index) const
{
  DTIVector vec(3);
  if (index==0) {
    vec = this->getDirectionV(index);
  } else if(index==(_point_vector.size()-1)) {
    vec = -(this->getDirectionV(index-1));
  } else {
    DTIVector vp = (*_point_vector[index-1]).copy();
    DTIVector v  = (*_point_vector[index]).copy();
    DTIVector vn = (*_point_vector[index+1]).copy();
    vec = norm((v - vp)+(vn - v)); // don't need to multiply by 1/2 for norm
  }
  return vec.copy();
}
