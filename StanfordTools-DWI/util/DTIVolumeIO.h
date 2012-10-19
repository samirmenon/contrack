/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_VOLUME_IO_H
#define DTI_VOLUME_IO_H

#include <cstddef>
#include <DTIVolume.h>

class DTIVolumeIO {
 private:
  DTIVolumeIO() {}
  virtual ~DTIVolumeIO() {}
 public:
  static DTIScalarVolume *loadScalarVolumeNifti(const char* filename, int* subcoord=NULL);
  static DTIVectorVolume *loadVectorVolumeNifti(const char* filename, int* subcoord=NULL);
  static DTITensorVolume *loadTensorVolumeNifti(const char* filename, int* subcoord=NULL);
  static void saveVolumeNifti (DTIVolume<float> *vol, const char* filename);
  static void saveVolumeNifti3D (DTIVolume<float> *vol, const char* filename);

 private:
  static int loadVolumeNifti(const char* filename, DTIVolume<float>* vol, int* subcoord=NULL);
  static int loadVolumeNiftiNew(const char* filename, DTIVolume<float>* vol, int* subcoord=NULL);
  static void setDimsWithCoords(int* subcoord, int &xdim, int &ydim, int &zdim, int offset[3]);
};

#endif
