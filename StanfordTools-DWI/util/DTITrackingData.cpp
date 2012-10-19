/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "DTITrackingData.h"
#include <DTITractParamsFile.h>
#include <DTIRandomSeedGeneratorMask.h>
#include <DTIVolumeIO.h>

using namespace std;

void DTITrackingData::setupFromOptionsFile (const char* infoFilename, bool bVoxelwiseCompute, bool bVerbose)
{
  _params = DTITractParamsFile::load1(infoFilename);
  if(_params==NULL) {
    cerr << "Failed to load tracking parameters." << endl;
    exit(0);
  }
  unsigned int xMax,yMax,zMax,lMax;
  _pdfVol = NULL;
  _wmVol = NULL;
  _voiMaskVol = NULL;
  
  // PDF I/O
  if(bVerbose)
    cerr << "Fetching PDF volume from disk...(file: " << _params->pdfFilename << ")" << endl;
  _pdfVol = DTIVolumeIO::loadVectorVolumeNifti (_params->pdfFilename.c_str());
  if(bVerbose && _pdfVol == NULL) 
    std::cerr << "Error reading PDF volume, continuing without." << std::endl;
  
  _pdfVol->getDimension(xMax,yMax,zMax,lMax);
  if(bVerbose)
    cerr << "Volume dimensions: " << xMax << " x " << yMax << " x " << zMax << endl;
  
  //  WM/GM I/O
  if(bVerbose)
    cerr << "Fetching WM/GM from disk...(file: " << _params->wmFilename << ")" << endl;
  _wmVol = DTIVolumeIO::loadScalarVolumeNifti (_params->wmFilename.c_str());
   
//   // Exclusion Mask I/O
//   if(bVerbose)
//     cerr << "Fetching exclusion mask volume from disk...(file: " << _params->exMaskFilename << ")" << endl;
//   _exMaskVol = DTIVolumeIO::loadScalarVolumeNifti (_params->exMaskFilename.c_str());
//   if(bVerbose && _exMaskVol == NULL)    
//     std::cerr << "No exclusion mask specified." << std::endl;

  // ROI Mask I/O
  if(bVerbose)
    cerr << "Fetching ROI mask volume from disk...(file: " << _params->voiMaskFilename << ")" << endl;
  _voiMaskVol = DTIVolumeIO::loadScalarVolumeNifti (_params->voiMaskFilename.c_str());
  if(bVerbose && _voiMaskVol == NULL)
    std::cerr << "Error reading ROI volume, continuing without." << std::endl;
    
//    DTIVector v = _pdfVol->getVector(21,24,36);
//    std::cerr <<"pdf: " << v[0] << std::endl;

//   if( bVoxelwiseCompute ) {
//      double vox_size[3];
//      _wmVol->getVoxelSize(vox_size[0],vox_size[1],vox_size[2]);
//      _params->startROISize[0] = vox_size[0];
//      _params->startROISize[1] = vox_size[1];
//      _params->startROISize[2] = vox_size[2];
//   }

  _num_way_voi=0;
  _start_voi=1;
  _stop_voi=2;
  _sourceGenerator = new DTIRandomSeedGeneratorMask(_start_voi);
  _endGenerator = new DTIRandomSeedGeneratorMask(_stop_voi);

  // XXX Must move this to input!!!
  bool bDoAllPair = bVoxelwiseCompute;

  if(_voiMaskVol!=NULL) {
    for (unsigned int z = 0; z < zMax; z++) {
      for (unsigned int y = 0; y < yMax; y++) {
	for (unsigned int x = 0; x < xMax; x++) {
	  int nROI = (int)_voiMaskVol->getScalar(x,y,z);
 	  DTIVector v(3);
 	  v[0]=x; v[1]=y; v[2]=z;
	  if(bDoAllPair) {
	    if(nROI>0) {
	      _sourceGenerator->addLoc(v);  
	      _voiMaskVol->setScalar(_stop_voi,x,y,z);
	    }
	  } else {
	    if(nROI==_start_voi) {
	      _sourceGenerator->addLoc(v);  
	    }else if(!bVoxelwiseCompute && nROI==_stop_voi) {
	      _endGenerator->addLoc(v);  
	    }else {
	      int nNewNum = nROI-2;
	      if(nNewNum>_num_way_voi)
		_num_way_voi == nNewNum;
	    }
	  }
	}
      }
    }
  }
}

void
DTITrackingData::cleanup()
{
   delete _pdfVol;
   delete _wmVol;
   delete _voiMaskVol;
   delete _sourceGenerator;
   delete _endGenerator;
   delete _params;
}
