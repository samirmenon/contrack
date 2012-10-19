#ifndef __EXTENDED_FSLIO_H__
#define __EXTENDED_FSLIO_H__

#include "fslio.h"

#ifdef _WIN32
typedef unsigned int uint;
#endif

#ifdef __cplusplus
extern "C" {
#endif

  double *****FslGetBufferAsScaledDouble_5D(FSLIO *fslio);
  int convertBufferToScaledFloatWithOffset(float *outbuf, void *inbuf, uint xDimIn, uint yDimIn, uint zDimIn, uint tDimIn, uint uDimIn, uint xOff, uint yOff, uint zOff, uint tOff, uint uOff, uint xDim, uint yDim, uint zDim, uint tDim, uint uDim, float slope, float inter, int nifti_datatype, float *minVal, float *maxVal );
  int reverseBufferToScaledFloatWithOffset(float* mem_buf, void *nifti_buf, uint xDimMem, uint yDimMem, uint zDimMem, uint tDimMem, uint uDimMem, uint xOffMem, uint yOffMem, uint zOffMem, uint tOffMem, uint uOffMem, uint xDimNifti, uint yDimNifti, uint zDimNifti, uint tDimNifti, uint uDimNifti, float slope, float inter, int nifti_datatype, float *minVal, float *maxVal );
  short  ****FslGetBufferAsScaledShort(FSLIO *fslio);

#ifdef __cplusplus
}
#endif

#endif
