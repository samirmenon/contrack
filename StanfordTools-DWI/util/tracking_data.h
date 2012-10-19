/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef TRACKINGDATA_H
#define TRACKINGDATA_H

#include <vector>
#include <typedefs.h>
#include <DTIScalarVolume.h>
#include <DTIVectorVolume.h>
#include <DTITractParamsFile.h>

class TrackingData {
public:
void setupFromOptionsFile (const char* infoFilename, bool bVoxelwiseCompute, bool bVerbose=true);
void cleanup();

DTIScalarVolume *_voiMaskVol;
DTIScalarVolume *_faVol;
DTIVectorVolume *_pdfVol;
DTIScalarVolume *_exMaskVol;
DTITractParamsFile* _params;
std::vector<DTIVector> _vecStartLocs;

};

#endif
