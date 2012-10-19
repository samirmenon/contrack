/***********************************************************************
 * AUTHOR: David Akers <dakers>
 *   FILE: .//DTIRandomSeedGenerator.cpp
 *   DATE: Thu Jun  9 13:26:23 2005
 *  DESCR: 
 ***********************************************************************/
#include "DTIRandomSeedGenerator.h"
#include <DTIMath.h>

/***********************************************************************
 *  Method: DTIRandomSeedGenerator::DTIRandomSeedGenerator
 *  Params: const DTIVector &pos, const DTIVector &dim
 * Effects: 
 ***********************************************************************/
DTIRandomSeedGenerator::DTIRandomSeedGenerator(const DTIVector &pos, const DTIVector &dim) : DTISeedGenerator()
{
  _pos = pos.copy();
  _dim = dim.copy();
  _progress = 0;
}


/***********************************************************************
 *  Method: DTIRandomSeedGenerator::getNextSeed
 *  Params: DTIVector &pt
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIRandomSeedGenerator::getNextSeed(DTIVector &pt)
{
  for(int ii=0; ii<3; ii++) {
    pt[ii] = _pos[ii] - _dim[ii]/2.0;
    while( pt[ii] <= _pos[ii] - _dim[0]/2.0 || 
	   pt[ii] >= _pos[ii] + _dim[0]/2.0 ) {
      pt[ii] = _pos[ii] + _dim[ii]*DTIMath::randzeroone() - _dim[ii]/2.0;
    } 
  }
}


/***********************************************************************
 *  Method: DTIRandomSeedGenerator::initialize
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIRandomSeedGenerator::initialize()
{
}


