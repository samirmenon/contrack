/***********************************************************************
 * AUTHOR: sherbond <sherbond>
 *   FILE: DTIVolumeIO.cpp
 *   DATE: Sat Jun 14 23:54:11 2008
 *  DESCR: 
 ***********************************************************************/
#include "DTIVolumeIO.h"
// For NIFTI
#include <extended_fslio.h>
#include "nifti1_io.h"
#include <iostream>
using namespace std;

/***********************************************************************
 *  Method: DTIVolumeIO::loadScalarVolumeNifti
 *  Params: const char *filename, double offset[3], int *subcoord
 * Returns: DTIScalarVolume *
 * Effects: 
 ***********************************************************************/
DTIScalarVolume *
DTIVolumeIO::loadScalarVolumeNifti(const char *filename, int *subcoord)
{
  DTIScalarVolume* vol = new DTIScalarVolume(0.);
  if(loadVolumeNiftiNew(filename, vol, subcoord)<0) {
    // Failed to load the volume
    delete vol;
    vol = NULL;
  }
  return vol;
}


/***********************************************************************
 *  Method: DTIVolumeIO::loadVectorVolumeNifti
 *  Params: const char *filename, double offset[3], int *subcoord
 * Returns: DTIVectorVolume *
 * Effects: 
 ***********************************************************************/
DTIVectorVolume *
DTIVolumeIO::loadVectorVolumeNifti(const char *filename, int *subcoord)
{
  DTIVectorVolume* vol = new DTIVectorVolume(0.);
  if(loadVolumeNiftiNew(filename, vol, subcoord)<0) {
    // Failed to load the volume
    delete vol;
    vol = NULL;
  }
  return vol;
}


/***********************************************************************
 *  Method: DTIVolumeIO::loadTensorVolumeNifti
 *  Params: const char *filename, double offset[3], int *subcoord
 * Returns: DTITensorVolume *
 * Effects: 
 ***********************************************************************/
DTITensorVolume *
DTIVolumeIO::loadTensorVolumeNifti(const char *filename, int *subcoord)
{
  DTITensorVolume* vol = new DTITensorVolume(0.);
  vol->setRowsCols(3,3);
  if(loadVolumeNiftiNew(filename, vol, subcoord)<0) {
    // Failed to load the volume
    delete vol;
    vol = NULL;
  }
  return vol;
}


/***********************************************************************
 *  Method: DTIVolumeIO::loadVolumeNifti
 *  Params: const char *filename, double offset[3], DTIVolume<float> *vol, int *subcoord
 * Returns: void
 * Effects: 
 ***********************************************************************/
int
DTIVolumeIO::loadVolumeNifti(const char *filename, DTIVolume<float> *vol, int *subcoord)
{
  double *****fdata;
  FSLIO *fslio;
  void *buffer;
  // Have to do this because fsl function doesn't take const char*???
  char* fname = new char[1024];
  strcpy(fname,filename);

  // open nifti dataset 
  FslSetOverrideOutputType(FSL_TYPE_NIFTI_GZ);
  fslio = FslInit();
  buffer = FslReadAllVolumes(fslio,fname);
  if (buffer == NULL) {
    //fprintf(stderr, "\nError opening and reading %s.\n",fname);
    delete []fname;
    free(fslio);
    return -1;
  }
  
  int xdim = fslio->niftiptr->nx;
  int ydim = fslio->niftiptr->ny;
  int zdim = fslio->niftiptr->nz;
  int soffset[3];
  setDimsWithCoords(subcoord, xdim, ydim, zdim, soffset);
  double xSizeMM = fslio->niftiptr->dx;
  double ySizeMM = fslio->niftiptr->dy;
  double zSizeMM = fslio->niftiptr->dz;
  double cmin = fslio->niftiptr->cal_min;
  double cmax = fslio->niftiptr->cal_max;

  // Handle different 4D file formats
  unsigned int ldim=1;
  if (fslio->niftiptr->ndim > 4) {
    // Fourth dimension is reserved for time
    ldim = fslio->niftiptr->nu;
  }else if (fslio->niftiptr->ndim == 4) {
    // Support for non-standard 4D files or actually a time dimension
    ldim = fslio->niftiptr->nt;
  }

  // get data as doubles and scaled 
  fdata = FslGetBufferAsScaledDouble_5D(fslio);
  // XXX We are putting the 4th dimension first
  float *data = new float[xdim*ydim*zdim*ldim];
  int counter = 0;
  float minVal = fdata[0][0][0][0][0];
  float maxVal = minVal;
  for (int z = 0; z < zdim; z++) {
    for (int y = 0; y < ydim; y++) {
      for (int x = 0; x < xdim; x++) {
	for (int l = 0; l < ldim; l++) {
	  if (fslio->niftiptr->ndim > 4) {
	    data[counter] = fdata[l][0][z+soffset[2]][y+soffset[1]][x+soffset[0]];
	  }
	  else if (fslio->niftiptr->ndim == 4) {
	    data[counter] = fdata[0][l][z+soffset[2]][y+soffset[1]][x+soffset[0]];
	  }
	  else {
	    data[counter] = fdata[0][0][z+soffset[2]][y+soffset[1]][x+soffset[0]];
	  }
	  if (data[counter] < minVal) {
	    minVal = data[counter];
	  }
	  if (data[counter] > maxVal) {
	    maxVal = data[counter];
	  }
	  counter++;
	}
      }
    }
  }

  vol->_data = data;
  vol->_vox_size[0] = xSizeMM;
  vol->_vox_size[1] = ySizeMM;
  vol->_vox_size[2] = zSizeMM;
  vol->_dim[0] = xdim;
  vol->_dim[1] = ydim;
  vol->_dim[2] = zdim;
  vol->_dim[3] = ldim;

  //if (cmin == 0 && cmax == 0) 
  {
    cmin = minVal; 
    cmax = maxVal;
  }
  vol->_calcminval = cmin;
  vol->_calcmaxval = cmax;
  vol->_qform_code = (fslio->niftiptr->qform_code);
  vol->_sform_code = (fslio->niftiptr->sform_code);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      vol->_qtoxyz[i][j] = (fslio->niftiptr->qto_xyz).m[i][j];
      vol->_qtoijk[i][j] = (fslio->niftiptr->qto_ijk).m[i][j];
      vol->_stoxyz[i][j] = (fslio->niftiptr->sto_xyz).m[i][j];
      vol->_stoijk[i][j] = (fslio->niftiptr->sto_ijk).m[i][j];
    }
  }
  
  // cleanup
  nifti_image_free(fslio->niftiptr);
  delete[] fname;
  free(fslio);
  free(****fdata);
  free(***fdata);
  free(**fdata);
  free(*fdata);
  free(fdata);

  // Everything was OK
  return 0;
}

/***********************************************************************
 *  Method: DTIVolumeIO::loadVolumeNiftiNew
 *  Params: const char *filename, double offset[3], DTIVolume<float> *vol, int *subcoord
 * Returns: void
 * Effects: 
 ***********************************************************************/
int
DTIVolumeIO::loadVolumeNiftiNew(const char *filename, DTIVolume<float> *vol, int *subcoord)
{
  FSLIO *fslio;
  void *buffer;
  // Have to do this because fsl function doesn't take const char*???
  char* fname = new char[1024];
  strcpy(fname,filename);
  
  // open nifti dataset 
  FslSetOverrideOutputType(FSL_TYPE_NIFTI_GZ);
  fslio = FslInit();
  buffer = FslReadAllVolumes(fslio,fname);
  if (buffer == NULL) {
    //fprintf(stderr, "\nError opening and reading %s.\n",fname);
    delete []fname;
    free(fslio);
    return -1;
  }
  
  int xdim = fslio->niftiptr->nx;
  int ydim = fslio->niftiptr->ny;
  int zdim = fslio->niftiptr->nz;
  int soffset[3];
  setDimsWithCoords(subcoord, xdim, ydim, zdim, soffset);
  double xSizeMM = fslio->niftiptr->dx;
  double ySizeMM = fslio->niftiptr->dy;
  double zSizeMM = fslio->niftiptr->dz;
  double cmin = fslio->niftiptr->cal_min;
  double cmax = fslio->niftiptr->cal_max;

  // Handle different 4D file formats
  unsigned int ldim=1;
  if (fslio->niftiptr->ndim > 4) {
    // Fourth dimension is reserved for time
    ldim = fslio->niftiptr->nu;
  }else if (fslio->niftiptr->ndim == 4) {
    // Support for non-standard 4D files or actually a time dimension
    ldim = fslio->niftiptr->nt;
  }

  // XXX We are putting the 4th dimension first
  float *data = new float[xdim*ydim*zdim*ldim];
  float slope, inter;
  if (fslio->niftiptr->scl_slope == 0) {
    slope = 1.0;
    inter = 0.0;
  }
  else {
    slope = fslio->niftiptr->scl_slope;
    inter = fslio->niftiptr->scl_inter;
  }

  float minVal,maxVal;
  int nConvRet = convertBufferToScaledFloatWithOffset( data, 
						       fslio->niftiptr->data,
						       fslio->niftiptr->nx,fslio->niftiptr->ny,
						       fslio->niftiptr->nz,fslio->niftiptr->nt,
						       fslio->niftiptr->nu,
						       soffset[0],soffset[1],soffset[2],0,0,
						       xdim,ydim,zdim,ldim,1,
						       slope, inter, fslio->niftiptr->datatype,
						       &minVal, &maxVal);
  if(nConvRet!=0) {
    delete []fname;
    delete []data;
    free(fslio);
    return -1;
  }

//  //Voxel offset test
//  memset(data,0,sizeof(data[0])*((zdim-1)*xdim*ydim + (ydim-1)*xdim + (xdim-1)));
//
////  data[42 + 61*xdim + 31*xdim*ydim]=maxVal;
////  data[(xdim-1-42) + (ydim-1-61)*xdim + (zdim-1-31)*xdim*ydim]=maxVal;
//  data[83 + 121*xdim + 61*xdim*ydim]=maxVal;

  
  vol->_data = data;
  vol->_vox_size[0] = xSizeMM;
  vol->_vox_size[1] = ySizeMM;
  vol->_vox_size[2] = zSizeMM;
  vol->_dim[0] = xdim;
  vol->_dim[1] = ydim;
  vol->_dim[2] = zdim;
  vol->_dim[3] = ldim;

  //if (cmin == 0 && cmax == 0) 
  {
    cmin = minVal; 
    cmax = maxVal;
  }
  vol->_calcminval = cmin;
  vol->_calcmaxval = cmax;
  vol->_qform_code = (fslio->niftiptr->qform_code);
  vol->_sform_code = (fslio->niftiptr->sform_code);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      vol->_qtoxyz[i][j] = (fslio->niftiptr->qto_xyz).m[i][j];
      vol->_qtoijk[i][j] = (fslio->niftiptr->qto_ijk).m[i][j];
      vol->_stoxyz[i][j] = (fslio->niftiptr->sto_xyz).m[i][j];
      vol->_stoijk[i][j] = (fslio->niftiptr->sto_ijk).m[i][j];
    }
  }
  
  // cleanup
  nifti_image_free(fslio->niftiptr);
  delete[] fname;
  free(fslio);
  // Everything was OK
  return 0;
}


/***********************************************************************
 *  Method: DTIVolumeIO::saveVolumeNifti
 *  Params: DTIVolume<float> *vol, const char *filename, double offset[3]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIVolumeIO::saveVolumeNifti(DTIVolume<float> *vol, const char *filename)
{
  unsigned int xdim,ydim,zdim,ldim;
  vol->getDimension(xdim,ydim,zdim,ldim);

  float xSize, ySize, zSize;
  vol->getVoxelSize(xSize,ySize,zSize);

//   float cmin, cmax;
//   cmin = vol->getCalcMinVal();
//   cmax = vol->getCalcMaxVal();

  FSLIO* fslio;
  FslSetOverrideOutputType(FSL_TYPE_NIFTI_GZ);
  fslio = FslOpen(filename,"wb");
  //  fslio = FslXOpen(filename,"wb",FSL_TYPE_NIFTI_GZ);
  //... set the appropriate header information using FslSet calls ...
  fslio->niftiptr->nx = xdim;
  fslio->niftiptr->dim[0] = xdim;
  fslio->niftiptr->ny = ydim;
  fslio->niftiptr->dim[1] = ydim;
  fslio->niftiptr->nz = zdim;
  fslio->niftiptr->dim[2] = zdim;
//   fslio->niftiptr->nt = 1;
//   fslio->niftiptr->dim[3] = 1;
//   fslio->niftiptr->nu = ldim;
//   fslio->niftiptr->dim[4] = ldim;
  fslio->niftiptr->nt = ldim;
  fslio->niftiptr->dim[3] = ldim;

  if(ldim==1) {
    fslio->niftiptr->ndim = 3;
  } else {
    //fslio->niftiptr->ndim = 5;
    fslio->niftiptr->ndim = 4;
  }

 fslio->niftiptr->dx = xSize;
 fslio->niftiptr->dy = ySize;
 fslio->niftiptr->dz = zSize;

 //FslSetVoxDim(fslio, xSize, ySize, zSize,1);
  FslSetTimeUnits(fslio, "s");
  FslSetDataType(fslio, 16); //float

  long counter=0;
  float *data = vol->getDataPointer();
  float *nifti_data = new float[xdim*ydim*zdim*ldim];
  float minVal;
  float maxVal;
  float slope=1;
  float inter=0;
  vector<uint> soffset(3,0);

  int nConvRet = reverseBufferToScaledFloatWithOffset( data, 
						       nifti_data,
						       fslio->niftiptr->nx,fslio->niftiptr->ny,
						       fslio->niftiptr->nz,fslio->niftiptr->nt,
						       fslio->niftiptr->nu,
						       soffset[0],soffset[1],soffset[2],0,0,
						       xdim,ydim,zdim,ldim,1,
						       slope, inter, fslio->niftiptr->datatype,
						       &minVal, &maxVal);
  
  //FslSetIntensityScaling(fslio, slope, inter);
  //FslSetCalMinMax(fslio, minVal, maxVal);
  fslio->niftiptr->cal_min = minVal;
  fslio->niftiptr->cal_max = maxVal;
 
  // mat44 stdmat;
//   double *transformMx = vol->getTransformMatrix();

//   for (int i = 0; i < 4; i++) {
//     for (int j = 0; j < 4; j++) {
//       stdmat.m[i][j] = transformMx[i*4+j];
//     }
//   }

  fslio->niftiptr->qform_code = 2;//vol->_qform_code;
  fslio->niftiptr->sform_code = 2;//vol->_sform_code;
  fslio->niftiptr->qoffset_x = -xdim;
  fslio->niftiptr->qoffset_y = -ydim;
  fslio->niftiptr->qoffset_z = -zdim;

//   mat44 xform;
//   mat44 xformI;
//   xform.m[0][0] = 2; xform.m[0][1] = 0; xform.m[0][2] = 0; xform.m[0][3] = -xdim;
//   xform.m[1][0] = 0; xform.m[1][1] = 2; xform.m[1][2] = 0; xform.m[1][3] = -ydim;
//   xform.m[2][0] = 0; xform.m[2][1] = 0; xform.m[2][2] = 2; xform.m[2][3] = -zdim;
//   xform.m[3][0] = 0; xform.m[3][1] = 0; xform.m[3][2] = 0; xform.m[3][3] = 1;
//   xformI.m[0][0] = 0.5; xformI.m[0][1] = 0; xformI.m[0][2] = 0; xformI.m[0][3] = xdim;
//   xformI.m[1][0] = 0; xformI.m[1][1] = 0.5; xformI.m[1][2] = 0; xformI.m[1][3] = ydim;
//   xformI.m[2][0] = 0; xformI.m[2][1] = 0; xformI.m[2][2] = 0.5; xformI.m[2][3] = zdim;
//   xformI.m[3][0] = 0; xformI.m[3][1] = 0; xformI.m[3][2] = 0; xformI.m[3][3] = 1;
  
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      (fslio->niftiptr->qto_xyz).m[i][j] = vol->_qtoxyz[i][j];
      (fslio->niftiptr->qto_ijk).m[i][j] = vol->_qtoijk[i][j];
      (fslio->niftiptr->sto_xyz).m[i][j] = vol->_stoxyz[i][j];
      (fslio->niftiptr->sto_ijk).m[i][j] = vol->_stoijk[i][j];

      // (fslio->niftiptr->qto_xyz).m[i][j] = xform.m[i][j];
//       (fslio->niftiptr->qto_ijk).m[i][j] = xformI.m[i][j];
//       (fslio->niftiptr->sto_xyz).m[i][j] = xform.m[i][j];
//       (fslio->niftiptr->sto_ijk).m[i][j] = xformI.m[i][j];
    }
  }
  
  // Supposed to work, but doesn't...
  float qb,qc,qd,qx,qy,qz, qfac;
  nifti_mat44_to_quatern( fslio->niftiptr->qto_xyz, &qb, &qc, &qd, &qx, &qy, &qz, NULL, NULL, NULL, &qfac );
  fslio->niftiptr->quatern_b = qb;
  fslio->niftiptr->quatern_c = qc;
  fslio->niftiptr->quatern_d = qd;
  fslio->niftiptr->qoffset_x = qx;
  fslio->niftiptr->qoffset_y = qy;
  fslio->niftiptr->qoffset_z = qz;
  fslio->niftiptr->qfac = qfac;

  //FslSetStdXform(fslio, vol->getSformCode(), stdmat); // should copy this, not set it to 1
  //FslSetRigidXform (fslio, vol->getQformCode(), stdmat); // same as above
  FslWriteHeader(fslio);

  // From FslWriteVolumes
  long int bpv = fslio->niftiptr->nbyper;  /* bytes per voxel */
  long int nbytes = xdim*ydim*zdim*ldim*bpv;
  int retval = nifti_write_buffer(fslio->fileptr, nifti_data, nbytes);
  //FslWriteVolumes(fslio,data,ldim);
  FslClose(fslio);

  // Cleanup
  nifti_image_free(fslio->niftiptr);
  free(fslio);
  delete nifti_data;
}

/***********************************************************************
 *  Method: DTIVolumeIO::saveVolumeNifti
 *  Params: DTIVolume<float> *vol, const char *filename, double offset[3]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIVolumeIO::saveVolumeNifti3D(DTIVolume<float> *vol, const char *filename)
{
  // XXX Only saves 3D of 4D right now
  unsigned int xdim,ydim,zdim,ldim;
  vol->getDimension(xdim,ydim,zdim,ldim);

  float xSize, ySize, zSize;
  vol->getVoxelSize(xSize,ySize,zSize);

  float cmin, cmax;
  cmin = vol->getCalcMinVal();
  cmax = vol->getCalcMaxVal();

  long counter=0;
  float *data = new float[xdim*ydim*zdim];
  float minVal;
  float maxVal;
  for (unsigned int z = 0; z < zdim; z++) {
    for (unsigned int y = 0; y < ydim; y++) {
      for (unsigned int x = 0; x < xdim; x++) {
	unsigned int l = 0;
	data[counter] = vol->getScalar (x,y,z,l);
	if(x==0 && y==0 && z==0) {
	  minVal = data[counter];
	  maxVal = minVal;
	} else{
	  if (data[counter] < minVal)
	    minVal = data[counter];
	  if (data[counter] > maxVal)
	    maxVal = data[counter];
	}
	counter++;
      }
    }
  }
  if (cmin == 0 && cmax == 0) {
    // this indicates values were not set (according to NIFTI spec)... we'll calculate them ourselves, then...
    cmin = minVal;
    cmax = maxVal;
  }

  FSLIO* fslio;
  FslSetOverrideOutputType(FSL_TYPE_NIFTI_GZ);
  fslio = FslOpen(filename,"wb");
  //  fslio = FslXOpen(filename,"wb",FSL_TYPE_NIFTI_GZ);
  //... set the appropriate header information using FslSet calls ...
  FslSetDim(fslio, xdim, ydim, zdim, 1);
  FslSetDimensionality(fslio, 3);
  FslSetVoxDim(fslio, xSize, ySize, zSize,1);
  FslSetTimeUnits(fslio, "s");
  FslSetDataType(fslio, 16); //float
  //  FslSetIntensityScaling(fslio, slope, intercept);
  FslSetCalMinMax(fslio, cmin, cmax);
  mat44 stdmat;
  double *transformMx = vol->getTransformMatrix();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      stdmat.m[i][j] = transformMx[i*4+j];
    }
  }
  
  FslSetStdXform(fslio, vol->getSformCode(), stdmat); // should copy this, not set it to 1
  FslSetRigidXform (fslio, vol->getQformCode(), stdmat); // same as above
  FslWriteHeader(fslio);
  FslWriteVolumes(fslio,data,1);
  FslClose(fslio);

  // Cleanup
  nifti_image_free(fslio->niftiptr);
  free(fslio);
  delete data;
}

void 
DTIVolumeIO::setDimsWithCoords(int* subcoord, int &xdim, int &ydim, int &zdim, int offset[3])
{
  if (subcoord==NULL) {
    offset[0]=0; offset[1]=0; offset[2]=0;
  } else {
    if(subcoord[0] < 0)
      subcoord[0] = 0;
    if(subcoord[1] >= xdim)
      subcoord[1] = xdim-1;
    if(subcoord[2] < 0)
      subcoord[2] = 0;
    if(subcoord[3] >= ydim)
      subcoord[3] = ydim-1;
    if(subcoord[4] < 0)
      subcoord[4] = 0;
    if(subcoord[5] >= zdim)
      subcoord[5] = zdim-1;
    xdim = subcoord[1]-subcoord[0]+1;
    ydim = subcoord[3]-subcoord[2]+1;
    zdim = subcoord[5]-subcoord[4]+1;
    offset[0]=subcoord[0];
    offset[1]=subcoord[2];
    offset[2]=subcoord[4];
  }
}
