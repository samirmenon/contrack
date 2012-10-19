/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "tracking_data.h"
#include <DTIVolumeIO.h>

using namespace std;

void TrackingData::setupFromOptionsFile (const char* infoFilename, bool bVoxelwiseCompute, bool bVerbose)
{

  _params = DTITractParamsFile::load060822(infoFilename);
  
  double dummyACPC[3]; // ACPC offset
  unsigned int xMax,yMax,zMax;
  _pdfVol = NULL;
  _faVol = NULL;
  _exMaskVol = NULL;
  _voiMaskVol = NULL;
  
  // PDF I/O
  if(bVerbose)
    cerr << "Fetching PDF volume from disk...(file: " << _params->pdfFilename << ")" << endl;
  _pdfVol = DTIVolumeIO::loadVectorVolumeNifti (_params->pdfFilename.c_str(), dummyACPC);
  if(bVerbose && _pdfVol == NULL) 
    std::cerr << "Error reading PDF volume, continuing without." << std::endl;
  
  _pdfVol->getDimension(xMax,yMax,zMax);
  if(bVerbose)
    cerr << "Volume dimensions: " << xMax << " x " << yMax << " x " << zMax << endl;
  
  //  WM/GM I/O
  if(bVerbose)
    cerr << "Fetching WM/GM from disk...(file: " << _params->faFilename << ")" << endl;
  _faVol = DTIVolumeIO::loadScalarVolumeNifti (_params->faFilename.c_str(), dummyACPC);
   
  // Exclusion Mask I/O
  if(bVerbose)
    cerr << "Fetching exclusion mask volume from disk...(file: " << _params->exMaskFilename << ")" << endl;
  _exMaskVol = DTIVolumeIO::loadScalarVolumeNifti (_params->exMaskFilename.c_str(), dummyACPC);
  if(bVerbose && _exMaskVol == NULL)    
    std::cerr << "No exclusion mask specified." << std::endl;

  // ROI Mask I/O
  if(bVerbose)
    cerr << "Fetching ROI mask volume from disk...(file: " << _params->voiMaskFilename << ")" << endl;
  _voiMaskVol = DTIVolumeIO::loadScalarVolumeNifti (_params->voiMaskFilename.c_str(), dummyACPC);
  if(bVerbose && _voiMaskVol == NULL)
    std::cerr << "Error reading ROI volume, continuing without." << std::endl;
    
//    DTIVector v = _pdfVol->getVector(21,24,36);
//    std::cerr <<"pdf: " << v[0] << std::endl;

  if( bVoxelwiseCompute ) {
     double vox_size[3];
     _faVol->getVoxelSize(vox_size[0],vox_size[1],vox_size[2]);
     _params->startROISize[0] = vox_size[0];
     _params->startROISize[1] = vox_size[1];
     _params->startROISize[2] = vox_size[2];
  }
    
  if( _voiMaskVol != NULL || _exMaskVol != NULL ) {
    float lowGMValue = 0.00001;
    // Setting fa to -fa wherever the voi mask is 1
    for (int z = 0; z < zMax; z++) {
      for (int y = 0; y < yMax; y++) {
	for (int x = 0; x < xMax; x++) {
	  if (_faVol->getScalar(x,y,z) == 0) {
	    _faVol->setScalar(x,y,z,lowGMValue);
	  }
	  if (_voiMaskVol!=NULL && _voiMaskVol->getScalar(x,y,z) > 0) {
	    // Setup indices for all ROI voxels
	    //if( bVoxelwiseCompute && _voiMaskVol->getScalar(x,y,z) == 1 && _faVol->getScalar(x,y,z)>lowGMValue) {
	    if( bVoxelwiseCompute && _faVol->getScalar(x,y,z)>lowGMValue) {
	      DTIVector paramsVec = _pdfVol->getVector(x,y,z).copy();
	      if( !(paramsVec[0] == 0 && paramsVec[1] == 0 && paramsVec[2] == 0) ) {
	        DTIVector v(3);
	        v[0]=x; v[1]=y; v[2]=z;
	        _vecStartLocs.push_back(v);    
	      }
	    }
	    _faVol->setScalar(x,y,z,-_faVol->getScalar(x,y,z));
	  }
	  if (_exMaskVol!=NULL && _exMaskVol->getScalar(x,y,z)>0) {
	  	_faVol->setScalar(x,y,z,lowGMValue);
	  }
	}
      }
    }
  }
}

void
TrackingData::cleanup()
{
//   delete _pdfVol;
//   delete _faVol;
//   delete _exMaskVol;
//   delete _voiMaskVol;
//   _vecStartLocs.clear();
//   delete _params;
}
