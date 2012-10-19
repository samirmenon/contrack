/***********************************************************************
 * AUTHOR: Anthony Sherbondy
 *   FILE: DTIRandomSeedGeneratorMask.cpp
 *   DATE: Thu Jun  9 13:26:23 2005
 *  DESCR: 
 ***********************************************************************/
#include "DTIRandomSeedGeneratorMask.h"
#include <DTIVolume.h>
#include <DTIMath.h>

/***********************************************************************
 *  Method: DTIRandomSeedGeneratorMask::DTIRandomSeedGeneratorMask
 *  Params: const DTIVector &pos, const DTIVector &dim
 * Effects: 
 ***********************************************************************/
// DTIRandomSeedGeneratorMask::DTIRandomSeedGeneratorMask(const DTIVolume* vol, int index) : DTISeedGenerator()
// {
//   if(vol!=NULL) {
//     unsigned int zMax,yMax,xMax,lMax;
//     //float xsize,ysize,zsize;
//     //vol->getVoxelSize(xsize,ysize,zsize);
//     //_size[0] = xsize; _size[1] = ysize; _size[2] = zsize;
//     vol->getDimension(xMax,yMax,zMax,lMax);
//     for (unsigned int z = 0; z < zMax; z++) {
//       for (unsigned int y = 0; y < yMax; y++) {
// 	for (unsigned int x = 0; x < xMax; x++) {
// 	  if (vol->getScalar(x,y,z,0) == index) {
//  	    // Setup indices for all ROI voxels
//  	    DTIVector v(3);
//  	    v[0]=x; v[1]=y; v[2]=z;
//  	    _vecLocs.push_back(v);    
//  	  }
// 	}
//       }
//     }
//   }
//   _progress = 0;
// }


/***********************************************************************
 *  Method: DTIRandomSeedGeneratorMask::getNextSeed
 *  Params: DTIVector &pt
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIRandomSeedGeneratorMask::getNextSeed(DTIVector &pt)
{
  if(_vecLocs.size()>0) {
    int rI = (int)DTIMath::round((_vecLocs.size()-1)*DTIMath::randzeroone());
    DTIVector pos = _vecLocs[rI];
    for(int ii=0; ii<3; ii++) {
      pt[ii]=pos[ii]+1;
      while(pt[ii]>=pos[ii]+1)
	pt[ii] = pos[ii] + DTIMath::randzeroone();
    }
  } else {
    pt[0]=-1;pt[1]=-1;pt[2]=-1;
  }
}


/***********************************************************************
 *  Method: DTIRandomSeedGeneratorMask::initialize
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIRandomSeedGeneratorMask::initialize()
{
}


