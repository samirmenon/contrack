/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_TRACKINGDATA_H
#define DTI_TRACKINGDATA_H

#include <vector>
#include <typedefs.h>
#include <DTIVolume.h>
class DTITractParamsFile;
class DTIRandomSeedGeneratorMask;

class DTITrackingData {
 public:
  void setupFromOptionsFile (const char* infoFilename, bool bVoxelwiseCompute=false, bool bVerbose=false);
  void cleanup();
  
  DTIScalarVolume *_voiMaskVol;
  DTIScalarVolume *_wmVol;
  DTIVectorVolume *_pdfVol;
  DTITractParamsFile* _params;
  int _num_way_voi;
  int _start_voi;
  int _stop_voi;
  DTIRandomSeedGeneratorMask* _sourceGenerator;
  DTIRandomSeedGeneratorMask* _endGenerator;
};

#endif
