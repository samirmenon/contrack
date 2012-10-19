/***********************************************************************
 * AUTHOR: David Akers and Anthony Sherbondy
 *   FILE: DTIPathwayIntersectionVolume.cpp
 *   DATE: Thu Jul  7 13:28:22 2005
 *  DESCR: 
 ***********************************************************************/
#include "DTIPathwayIntersectionVolume.h"
#include "DTIPathway.h"
#include <math.h>
#include <iostream>
#include <DTIMath.h>
using namespace std;


/***********************************************************************
 *  Method: DTIPathwayIntersectionVolume::munchPathway
 *  Params: DTIPathway *pathway
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIntersectionVolume::munchPathway(DTIPathway *pathway)
{
  for (int i = 0; i < pathway->getNumPoints(); i++) {
    static double oldPt[3];
    static double pt[3];
    oldPt[0] = pt[0];
    oldPt[1] = pt[1];
    oldPt[2] = pt[2];
    pathway->getPoint (i, pt);
    if (i > 0) {
      rasterizeSegment (oldPt, pt);
    }
  }
}

#if 0

/***********************************************************************
 *  Method: DTIPathwayIntersectionVolume::munchPoint
 *  Params: const double pt[3]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIntersectionVolume::munchPoint(const double ptInput[3])
{
  unsigned int xSize, ySize, zSize, cSize;
  getDimension(xSize, ySize, zSize, cSize);
  
  double *inverseMx = getInverseTransformMatrix();

  double pt[4] = {ptInput[0], ptInput[1], ptInput[2], 1};
  double ptVoxSpace[4];

  ptVoxSpace[0] = inverseMx[0]*pt[0] + inverseMx[1]*pt[1]
    + inverseMx[2]*pt[2] + inverseMx[3];
  ptVoxSpace[1] = inverseMx[4]*pt[0] + inverseMx[5]*pt[1]
    + inverseMx[6]*pt[2] + inverseMx[7];
  ptVoxSpace[2] = inverseMx[8]*pt[0] + inverseMx[9]*pt[1]
    + inverseMx[10]*pt[2] + inverseMx[11];

  float x = ptVoxSpace[0];
  float y = ptVoxSpace[1];
  float z = ptVoxSpace[2];

  assert (x < xSize);
  assert (y < ySize);
  assert (z < zSize);
  assert (x >= 0);
  assert (y >= 0);
  assert (z >= 0);
  //  cerr << "DTIPathwayIntersectionVolume::munchPoint(const double pt[3]) is disabled" << endl;
  double oldCount = getScalar (x, y, z, 0);
  setScalar (x, y, z, 0, oldCount+1);

  //  (*_fa_array)[xCoord][yCoord][zCoord] = (*_fa_array)[xCoord][yCoord][zCoord] + 1;
  //  if ((*_fa_array)[xCoord][yCoord][zCoord] > _max_intersect) {
  //    _max_intersect = (*_fa_array)[xCoord][yCoord][zCoord];
  //  }
}
#endif

/***********************************************************************
 *  Method: DTIPathwayIntersectionVolume::normalize
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIntersectionVolume::normalize()
{
  float minValue=0;
  float maxValue; 
  unsigned int xSize, ySize, zSize, cDim;
  getDimension(xSize, ySize, zSize, cDim);
  for (unsigned int x = 0; x < xSize; x++) {
    for (unsigned int y = 0; y < ySize; y++) {
      for (unsigned int z = 0; z < zSize; z++) {
	float value = this->getScalar (x,y,z,0);
	if (value > maxValue || (x==0&&y==0&&z==0)) {
	  maxValue = value;
	}
      }
    }
  }
  setCalcMinVal (minValue);
  setCalcMaxVal (maxValue);
}


/***********************************************************************
 *  Method: DTIPathwayIntersectionVolume::DTIPathwayIntersectionVolume
 *  Params: const unsigned int intersectionVolumeDimensions[3], const unsigned int pathwayDimensions[3], const double pathwayVoxSize[3]
 * Effects: 
 ***********************************************************************/
DTIPathwayIntersectionVolume::DTIPathwayIntersectionVolume(DTIVolume<float> *currentBackground) : DTIVolume<float> (currentBackground)
{
  _max_intersect = 0;
}







void swapPoints (double a[3], double b[3]) {
  static double c[3];
  c[0] = b[0];
  c[1] = b[1];
  c[2] = b[2];
  b[0] = a[0];
  b[1] = a[1];
  b[2] = a[2];
  a[0] = c[0];
  a[1] = c[1];
  a[2] = c[2];
}

/***********************************************************************
 *  Method: DTIPathwayIntersectionVolume::rasterizeSegment
 *  Params: const double a[3], const double b[3]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIntersectionVolume::rasterizeSegment(const double a[3], const double b[3])
{
  double aCopy[3] = {a[0], a[1], a[2]};
  double bCopy[3] = {b[0], b[1], b[2]};

  //  cerr << "rasterizeSegment (" << aCopy[0] << ", " << aCopy[1] << ", " << aCopy[2] << "  " << bCopy[0] << ", " << bCopy[1] << ", " << bCopy[2] << ")" << endl;

  double dx = (aCopy[0]-bCopy[0]);
  double dy = (aCopy[1]-bCopy[1]);
  double dz = (aCopy[2]-bCopy[2]);

  bool iterateX = false;
  bool iterateY = false;
  bool iterateZ = false;

  if (fabs(dx) >= fabs(dy) && fabs(dx) >= fabs(dz)) {
    iterateX = true;
    if (aCopy[0] > bCopy[0]) {
      swapPoints (aCopy,bCopy);
      dy = -dy;
      dz = -dz;
    }
  }
  else if (fabs(dy) >= fabs(dx) && fabs(dy) >= fabs(dz)) {
    iterateY = true;
    if (aCopy[1] > bCopy[1]) {
      swapPoints (aCopy,bCopy);
      dx = -dx;
      dz = -dz;
    }
  }
  else if (fabs(dz) >= fabs(dx) && fabs(dz) >= fabs(dy)) {
    iterateZ = true;
    if (aCopy[2] > bCopy[2]) {
      swapPoints (aCopy,bCopy);
      dx = -dx;
      dy = -dy;
    }
  }

  double x = aCopy[0];
  double y = aCopy[1];
  double z = aCopy[2];

  bool sign;

  if (iterateX) {
    // iterate over x:
    double yStep = dy/dx;
    double zStep = dz/dx;
    while (x < bCopy[0]) {
      // calculate y, z:
      setVoxel (x,y,z);
      if (x > bCopy[0]-1.0) {
	x = bCopy[0];
      }
      else {
	x += 1.0;
      }
      y += yStep;
      z += zStep;
    }
  }
  else if (iterateY) {
    // iterate over y:
    double xStep = dx/dy;
    double zStep = dz/dy;
    while (y < bCopy[1]) {
      // calculate y, z:
      setVoxel (x,y,z);
      if (y > bCopy[1]-1) {
	y = bCopy[1];
      }
      else {
	y += 1.0;
      }
      x += xStep;
      z += zStep;
    }
  }
  else if (iterateZ) {
    // iterate over z:
    double xStep = dx/dz;
    double yStep = dy/dz;
    while (z < bCopy[2]) {
      // calculate x, y:
      setVoxel (x,y,z);
      if (z > bCopy[2]-1) {
	z = bCopy[2];
      }
      else {
	z += 1.0;
      }
      x += xStep;
      y += yStep;
    }
  }
}


/***********************************************************************
 *  Method: DTIPathwayIntersectionVolume::setVoxel
 *  Params: double x, double y, double z
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIntersectionVolume::setVoxel(double x, double y, double z)
{
  unsigned int xDim, yDim, zDim, cDim;
  getDimension(xDim, yDim, zDim, cDim);

  static int lastX = -1000;
  static int lastY = -1000;
  static int lastZ = -1000;

  double *inverseMx = getInverseTransformMatrix();

  double pt[4] = {x, y, z, 1};
  double ptVoxSpace[4];

  ptVoxSpace[0] = inverseMx[0]*pt[0] + inverseMx[1]*pt[1]
    + inverseMx[2]*pt[2] + inverseMx[3];
  ptVoxSpace[1] = inverseMx[4]*pt[0] + inverseMx[5]*pt[1]
    + inverseMx[6]*pt[2] + inverseMx[7];
  ptVoxSpace[2] = inverseMx[8]*pt[0] + inverseMx[9]*pt[1]
    + inverseMx[10]*pt[2] + inverseMx[11];

  unsigned int xCoord = DTIMath::interpNN(ptVoxSpace[0]);
  unsigned int yCoord = DTIMath::interpNN(ptVoxSpace[1]);
  unsigned int zCoord = DTIMath::interpNN(ptVoxSpace[2]);

  if (xCoord == lastX && yCoord == lastY && zCoord == lastZ) {
    return;
  }
  else {
    lastX = xCoord;
    lastY = yCoord;
    lastZ = zCoord;
  }
  assert (xCoord < xDim);
  assert (yCoord < yDim);
  assert (zCoord < zDim);
  assert (xCoord >= 0);
  assert (yCoord >= 0);
  assert (zCoord >= 0);

  float oldCount = getScalar (xCoord, yCoord, zCoord);
  setScalar (oldCount+1, xCoord, yCoord, zCoord);
}


