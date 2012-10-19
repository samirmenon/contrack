/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_VOLUME_H
#define DTI_VOLUME_H
#include <DTIMath.h>
#include <tnt/tnt_array1d.h>
#include <tnt/tnt_array2d.h>
#include <cstddef>
#include <cstring>
#include <cassert>

using namespace TNT;

template <class T>
class DTIVolume {
  friend class DTIVolumeIO;

 protected:
  DTIVolume(const T defaultval);
 public:
  // Constructor. Initializes the volume and allocates memory.
  DTIVolume (unsigned int iDim, unsigned int jDim, unsigned int kDim, unsigned int lDim, double xSizeMM, double ySizeMM, double zSizeMM, const T defaultval); 
  DTIVolume (const DTIVolume* vol); 

  // Destructor
  virtual ~DTIVolume();

  void setAllValsDefault();

  // Get the dimensions of the volume.

  DTIVolume<T> *GenerateMaskVolume();

  void getDimension (unsigned int &iDim, unsigned int &jDim, unsigned int &kDim, unsigned int &lDim) const {iDim=_dim[0];jDim=_dim[1];kDim=_dim[2];lDim=_dim[3];}
  void getDimension (unsigned int &iDim, unsigned int &jDim, unsigned int &kDim) const {unsigned int foo; getDimension(iDim,jDim,kDim,foo);}

  // Get the voxel dimensions (in mm)
  void getVoxelSize (double &x, double &y, double &z) const {x=_vox_size[0];y=_vox_size[1];z=_vox_size[2];}
  void getVoxelSize (float &x, float &y, float &z) const {x=_vox_size[0];y=_vox_size[1];z=_vox_size[2];}

  // Are the voxel dimensions the same?
  bool isIsotropic(T &size) { if (_vox_size[0] == _vox_size[1] && _vox_size[1] == _vox_size[2]) { size = _vox_size[0]; return true; } else {return false; }}

  void sub2index(unsigned int i, unsigned int j, unsigned int k, unsigned int &index) const;
  void sub2index(float i, float j, float k, unsigned int &index) const { sub2index(DTIMath::interpNN(i), DTIMath::interpNN(j), DTIMath::interpNN(k), index); }
  void index2sub(unsigned int index, unsigned int &i, unsigned int &j, unsigned int &k) const;

  // Check to see if a point is inside the volume.
  bool inBounds (unsigned int i, unsigned int j, unsigned int k, unsigned int l) const;
  bool inBounds (unsigned int i, unsigned int j, unsigned int k) const { return inBounds(i,j,k,(unsigned int)0);}
  bool inBounds (float i, float j, float k, float l) const {return inBounds(DTIMath::interpNN(i),DTIMath::interpNN(j),DTIMath::interpNN(k),DTIMath::interpNN(l));}
  bool inBounds (float i, float j, float k) const  {return inBounds(i,j,k,(float)0);}

  // Scalar access
  void setScalar (const T& value, unsigned int i, unsigned int j, unsigned int k, unsigned int l=0);
  void setScalar (const T& value, float i, float j, float k, float l=0) {setScalar(value,DTIMath::interpNN(i),DTIMath::interpNN(j),DTIMath::interpNN(k),DTIMath::interpNN(l));}
  T getScalar (unsigned int i, unsigned int j, unsigned int k, unsigned int l=0) const;
  T getScalar (float i, float j, float k, float l=0) const {return getScalar(DTIMath::interpNN(i),DTIMath::interpNN(j),DTIMath::interpNN(k),DTIMath::interpNN(l));}

  // Vector access
  void setVector (const Array1D<T> &value, unsigned int i, unsigned int j, unsigned int k);
  void setVector (const Array1D<T> &value, float i, float j, float k){setVector(value,DTIMath::interpNN(i),DTIMath::interpNN(j),DTIMath::interpNN(k));}
  Array1D<T> getVector (unsigned int i, unsigned int j, unsigned int k) const;
  Array1D<T> getVector (float i, float j, float k) const {return getVector(DTIMath::interpNN(i),DTIMath::interpNN(j),DTIMath::interpNN(k));}

  // Tensor access
  void setTensor (const Array2D<T> &value, unsigned int i, unsigned int j, unsigned int k);
  void setTensor (const Array2D<T> &value, float i, float j, float k){setTensor(value,DTIMath::interpNN(i),DTIMath::interpNN(j),DTIMath::interpNN(k));}
  Array2D<T> getTensor (unsigned int i, unsigned int j, unsigned int k) const;
  Array2D<T> getTensor (float i, float j, float k) const {return getTensor(DTIMath::interpNN(i),DTIMath::interpNN(j),DTIMath::interpNN(k));}

  // Matrix access
  void setMatrix (const Array2D<T> &value, unsigned int i, unsigned int j, unsigned int k);
  void setMatrix (const Array2D<T> &value, float i, float j, float k){setMatrix(value,DTIMath::interpNN(i),DTIMath::interpNN(j),DTIMath::interpNN(k));}
  Array2D<T> getMatrix (unsigned int i, unsigned int j, unsigned int k) const;
  Array2D<T> getMatrix (float i, float j, float k) const {return getMatrix(DTIMath::interpNN(i),DTIMath::interpNN(j),DTIMath::interpNN(k));}
  void setRowsCols(int nR, int nC){_nR=nR; _nC=nC;}
  void getRowsCols(int& nR, int& nC){nR=_nR; nC=_nC;}

  void setQformOffset (T *val);

  double *getTransformMatrix ();
  void getTransformVector(T *val);
  const double *getTransformMatrixConst () const;
  double *getInverseTransformMatrix();
  double *getSformMatrix () { return &(_stoxyz[0][0]); }
  double *getInverseSformMatrix() { return &(_stoijk[0][0]); }
  double *getQformMatrix () { return &(_qtoxyz[0][0]); }
  double *getInverseQformMatrix() { return &(_qtoijk[0][0]); }
  int getQformCode() const { return _qform_code; }
  int getSformCode() const { return _sform_code; }
  T& getCalcMinVal() { return _calcminval; }
  T& getCalcMaxVal() { return _calcmaxval; }
  T getCalcMinValConst() const { return _calcminval; }
  T getCalcMaxValConst() const { return _calcmaxval; }
  void setCalcMinVal(T& calcminval) { _calcminval = calcminval; }
  void setCalcMaxVal(T& calcmaxval) { _calcmaxval = calcmaxval; } 

  // DTIQuery special call
  T *getDataPointer () const { return _data; }

 protected:
  unsigned int getArrayOffset(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const;
  void getSubFromArrayOffset(unsigned int offset, unsigned int& i, unsigned int& j, unsigned int& k, unsigned int& l) const;
  double _vox_size[3];
  int _dim[4];  
  int _nR,_nC;
  double _qtoxyz[4][4];
  double _qtoijk[4][4];
  double _stoxyz[4][4];
  double _stoijk[4][4];
  int _qform_code;
  int _sform_code;
  T _defaultval;
  T _calcminval;
  T _calcmaxval;

  T *_data;
};

template <class T>
DTIVolume<T>::DTIVolume(const T defaultval)
{
  for(int ii=0; ii<3; ii++) _vox_size[ii]=0;
  for(int ii=0; ii<4; ii++) _dim[ii]=0;
  _data = NULL;
  _defaultval = defaultval;
  setCalcMinVal (_defaultval);
  setCalcMaxVal (_defaultval);
  _nR = -1;
  _nC = -1;
}

template <class T>
DTIVolume<T>::DTIVolume(unsigned int iDim, unsigned int jDim, unsigned int kDim, unsigned int lDim, double xSizeMM, double ySizeMM, double zSizeMM, const T defaultval)
{  
  _defaultval = defaultval;
  _vox_size[0] = xSizeMM; _vox_size[1] = ySizeMM; _vox_size[2] = zSizeMM;
  _dim[0] = iDim;_dim[1] = jDim;_dim[2] = kDim;_dim[3] = lDim;
  _data = new T [iDim*jDim*kDim*lDim];
  //memset (_data, 0, sizeof (T) * iDim*jDim*kDim*lDim);
      
  setCalcMinVal (_defaultval);
  setCalcMaxVal (_defaultval);
  _nR = -1;
  _nC = -1;

  _sform_code = 0;
  _qform_code = 0;

  // Initialize to be the identity matrix:
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      _qtoxyz[i][j] = (i == j) ? 1.0 : 0.0;
      _qtoijk[i][j] = (i == j) ? 1.0 : 0.0;
      _stoxyz[i][j] = (i == j) ? 1.0 : 0.0;
      _stoijk[i][j] = (i == j) ? 1.0 : 0.0;
    }
  }
}


template <class T>
DTIVolume<T> *DTIVolume<T>::GenerateMaskVolume()
{
  DTIVolume<T> *newVol = new DTIVolume<T>(this);
  delete[] newVol->_data;
  newVol->_dim[3] = 1;
  newVol->_data = new T[_dim[0]*_dim[1]*_dim[2]];
  memset (newVol->_data, 0, sizeof(T) * _dim[0]*_dim[1]*_dim[2]);
  return newVol;
}

template <class T>
DTIVolume<T>::DTIVolume(const DTIVolume<T> *other)
{
  _defaultval = other->_defaultval;
  // don't copy these parts
  setCalcMinVal (_defaultval);
  setCalcMaxVal (_defaultval);

  for(int ii=0; ii<3; ii++) _vox_size[ii]=other->_vox_size[ii];
  for(int ii=0; ii<4; ii++) _dim[ii]=other->_dim[ii];
  _data = new T[_dim[0]*_dim[1]*_dim[2]*_dim[3]];

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      _qtoxyz[i][j] = other->_qtoxyz[i][j];
      _qtoijk[i][j] = other->_qtoijk[i][j];
      _stoxyz[i][j] = other->_stoxyz[i][j];
      _stoijk[i][j] = other->_stoijk[i][j];
    }
  }
  _qform_code = other->_qform_code;
  _sform_code = other->_sform_code;
  _nR = other->_nR;
  _nC = other->_nC;
}

template <class T>
void 
DTIVolume<T>::sub2index(unsigned int i, unsigned int j, unsigned int k, unsigned int &index) const
{
  index = getArrayOffset(i, j, k, 0);
}

template <class T>
void 
DTIVolume<T>::index2sub(unsigned int index, unsigned int &i, unsigned int &j, unsigned int &k) const
{
  unsigned int l;
  getSubFromArrayOffset(index, i, j, k, l);
  assert(l==0);
}

template <class T>
DTIVolume<T>::~DTIVolume()
{
  delete []_data;
}

template <class T>
void
DTIVolume<T>::setAllValsDefault()
{
   for(unsigned int ii=0; ii<_dim[0]; ii++) 
     for(unsigned int jj=0; jj<_dim[1]; jj++) 
       for(unsigned int kk=0; kk<_dim[2]; kk++)  
 	for(unsigned int ll=0; ll<_dim[3]; ll++)  
 	  setScalar(_defaultval,ii,jj,kk,ll); 
}



template <class T>
bool DTIVolume<T>::inBounds(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
{
  unsigned int imax,jmax,kmax,lmax;
  getDimension(imax,jmax,kmax,lmax);
  if (i < 0 || i > imax-1 ||
      j < 0 || j > jmax-1 ||
      k < 0 || k > kmax-1 ||
      l < 0 || l > lmax-1)
    return false;
  else
    return true;
}

template <class T>
void DTIVolume<T>::getTransformVector(T *val)
{
	double *vmx  = getTransformMatrix();
	val[0] = vmx[3]; val[1] = vmx[7]; val[2] = vmx[11];
}

template <class T>
void DTIVolume<T>::setQformOffset (T *val)
{
  this->_qtoxyz[0][3] = val[0];
  this->_qtoxyz[1][3] = val[1];
  this->_qtoxyz[2][3] = val[2];
}

template <class T>
double * DTIVolume<T>::getTransformMatrix()
{
  // first choice is sform matrix
  if (_sform_code > 0)
    return getSformMatrix();
  else
    return getQformMatrix();
}

template <class T>
const double * DTIVolume<T>::getTransformMatrixConst() const
{
  // first choice is sform matrix
  if (_sform_code > 0)
    return getSformMatrix();
  else
    return getQformMatrix();
}

template <class T>
double * DTIVolume<T>::getInverseTransformMatrix()
{
  // first choice is sform matrix
  if (_sform_code > 0)
    return getInverseSformMatrix();
  else
    return getInverseQformMatrix();
}

template <class T>
unsigned int DTIVolume<T>::getArrayOffset(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const 
{
  return l + k * _dim[3] * _dim[0] * _dim[1] + j * _dim[3] * _dim[0] + i*_dim[3];
}

template <class T>
void DTIVolume<T>::getSubFromArrayOffset(unsigned int offset, unsigned int& i, unsigned int& j, unsigned int& k, unsigned int& l) const
{
  // Peel off a layer at a time
  unsigned int kFactor = _dim[3] * _dim[0] * _dim[1];
  unsigned int jFactor = _dim[3] * _dim[0];
  unsigned int iFactor = _dim[3];
  unsigned int left = offset;

  k = floor( (float)left/(float)kFactor );
  left = left % kFactor; 

  j = floor( (float)left/(float)jFactor );
  left = left % jFactor; 

  i = floor( (float)left/(float)iFactor );
  left = left % iFactor; 

  l = left;
}

////////////////////////////////////////////////////////////////////////
// Get/Set Code
////////////////////////////////////////////////////////////////////////

template <class T>
void 
DTIVolume<T>::setScalar(const T& value, unsigned int i, unsigned int j, unsigned int k, unsigned int l)
{
  unsigned int iDim, jDim, kDim, lDim;
  getDimension (iDim, jDim, kDim, lDim);
  assert (i < iDim && i >= 0 &&
	  j < jDim && j >= 0 &&
	  k < kDim && k >= 0 &&
	  l < lDim && l >= 0);
  _data[getArrayOffset(i,j,k,l)] = value;
}

template <class T>
T 
DTIVolume<T>::getScalar(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
{
  unsigned int iDim, jDim, kDim, lDim;
  getDimension (iDim, jDim, kDim, lDim);
  assert (i < iDim && i >= 0 &&
	  j < jDim && j >= 0 &&
	  k < kDim && k >= 0 &&
	  l < lDim && l >= 0);
  return _data[getArrayOffset(i,j,k,l)];
}

template <class T>
void
DTIVolume<T>::setVector(const Array1D<T> &value, unsigned int i, unsigned int j, unsigned int k)
{
  for(int ll=0; ll<_dim[3]; ll++)
    setScalar(value[ll],i,j,k,ll);
}

template <class T>
Array1D<T>
DTIVolume<T>::getVector(unsigned int i, unsigned int j, unsigned int k) const
{
  Array1D<T> value(_dim[3]);
  for(int ll=0; ll<_dim[3]; ll++)
    value[ll] = getScalar(i,j,k,ll);
  return value;
}

template <class T>
void
DTIVolume<T>::setTensor(const Array2D<T> &value, unsigned int i, unsigned int j, unsigned int k)
{
  // Tensor orders following:
  // Dxx, Dxy, Dyy, Dxz, Dyz, Dzz (lower triangular - see NIFTI docs)
  int _tensorOrder[6] = {0,1,2,3,4,5};
  setScalar(value[0][0],i,j,k,_tensorOrder[0]);
  setScalar(value[0][1],i,j,k,_tensorOrder[1]);
  setScalar(value[1][1],i,j,k,_tensorOrder[2]);
  setScalar(value[2][0],i,j,k,_tensorOrder[3]);
  setScalar(value[2][1],i,j,k,_tensorOrder[4]);
  setScalar(value[2][2],i,j,k,_tensorOrder[5]);
}

template <class T>
Array2D<T>
DTIVolume<T>::getTensor(unsigned int i, unsigned int j, unsigned int k) const
{
  // Tensor orders following:
  // Dxx, Dxy, Dyy, Dxz, Dyz, Dzz (lower triangular - see NIFTI docs)
  int _tensorOrder[6] = {0,1,2,3,4,5};
  Array2D<T> t(3,3);
  t[0][0] = getScalar(i,j,k,_tensorOrder[0]);
  t[0][1] = getScalar(i,j,k,_tensorOrder[1]);
  t[1][0] = getScalar(i,j,k,_tensorOrder[1]);
  t[1][1] = getScalar(i,j,k,_tensorOrder[2]);
  t[0][2] = getScalar(i,j,k,_tensorOrder[3]);
  t[2][0] = getScalar(i,j,k,_tensorOrder[3]);
  t[1][2] = getScalar(i,j,k,_tensorOrder[4]);
  t[2][1] = getScalar(i,j,k,_tensorOrder[4]);
  t[2][2] = getScalar(i,j,k,_tensorOrder[5]);
  return t;
}


template <class T>
void
DTIVolume<T>::setMatrix(const Array2D<T> &value, unsigned int i, unsigned int j, unsigned int k)
{
  assert(value.dim1()==_nR);
  assert(value.dim2()==_nC);
  for(int rr=0; rr<_nR; rr++) {
    for(int cc=0; cc<_nC; cc++) {
      unsigned int ll = rr*_nC + cc;
      setScalar(value[rr][cc],i,j,k,ll);
    }
  }
}

template <class T>
Array2D<T>
DTIVolume<T>::getMatrix(unsigned int i, unsigned int j, unsigned int k) const
{
  Array2D<T> value(_nR,_nC);
  for(int rr=0; rr<_nR; rr++) {
    for(int cc=0; cc<_nC; cc++) { 
      unsigned int ll = rr*_nC + cc;
      value[rr][cc] = getScalar(i,j,k,ll);
    }
  }
  return value;
}

// Volume Types
typedef DTIVolume<float> DTIScalarVolume;
typedef DTIVolume<float> DTIVectorVolume;
typedef DTIVolume<float> DTITensorVolume;
typedef DTIVolume<float> DTIMatrixVolume;

#ifdef _WIN32
	#include <memory>
	typedef std::shared_ptr<DTIVectorVolume> DTIVectorVolumePtr;
	typedef std::shared_ptr<DTIScalarVolume> PDTIScalarVolume;
#else
	#include <tr1/memory>
	typedef std::tr1::shared_ptr<DTIVectorVolume> DTIVectorVolumePtr;
	typedef std::tr1::shared_ptr<DTIScalarVolume> PDTIScalarVolume;
#endif

#endif
