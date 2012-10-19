/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef TYPEDEFS_ARRAY_H
#define TYPEDEFS_ARRAY_H

#include <tnt/tnt_vec.h>
#include <tnt/tnt_array3d.h>
#include <tnt/tnt_cmat.h>


typedef double NumberType;
typedef TNT::Array1D<uint> DTIVoxel;

//typedef TNT::Array1D<float> DTIVector;
//typedef TNT::Array1D<short> DTIShortVector;
//typedef TNT::Array1D<float> DTIGeometryVector;
typedef TNT::Array1D<float> DTIArray;
typedef TNT::Array2D<float> DTITensor;
typedef TNT::Array2D<float> DTIMatrix;

template <class T>
bool operator== (const TNT::Array1D<T> &A, const TNT::Array1D<T> &B) {
  bool b = true;
  for (int i = 0; i < A.dim1(); i++)
    b = b && A[i] == B[i];
  return b;
}

template <class T>
TNT::Array1D<T> operator* (const TNT::Array1D<T> &A, const double & B) {

  TNT::Array1D<T> C(A.dim1());
  for (int i = 0; i < A.dim1(); i++) {
    C[i] = A[i] * B;
  }
  return C;
}

template <class T>
TNT::Array1D<T> operator* (const double &B, const TNT::Array1D<T> &A) {

  TNT::Array1D<T> C(A.dim1());
  for (int i = 0; i < A.dim1(); i++) {
    C[i] = A[i] * B;
  }
  return C;
}


template <class T>
TNT::Array1D<T> operator* (const T & B, const TNT::Array1D<T> &A) {

  TNT::Array1D<T> C(A.dim1());
  for (int i = 0; i < A.dim1(); i++) {
    C[i] = A[i] * B;
  }
  return C;
}

template <class T>
TNT::Array1D<T> operator* (const TNT::Array2D<T> &A, const TNT::Array1D<T> &B) {

  TNT::Array1D<T> C(A.dim1());
  for (int i = 0; i < A.dim1(); i++) {
    C[i]=0;
    for (int j = 0; j < A.dim2(); j++) {
      C[i] += A[i][j] * B[j];
    }
  }
  return C;
}

template <class T>
TNT::Array1D<T> operator* (const TNT::Array1D<T> &A, const TNT::Array2D<T> &B) {

  TNT::Array1D<T> C(B.dim2());
  for (int i = 0; i < B.dim1(); i++) {
    C[i]=0;
    for (int j = 0; j < B.dim2(); j++) {
      C[i] += A[j] * B[j][i];
    }
  }
  return C;
}

template <class T>
TNT::Array2D<T> operator* (const T & B, const TNT::Array2D<T> &A) {

  TNT::Array2D<T> C(A.dim1(),A.dim2());
  for (int i = 0; i < A.dim1(); i++) {
    for (int j = 0; j < A.dim2(); j++) {
    C[i][j] = A[i][j] * B;
    }
  }
  return C;
}

template <class T>
TNT::Array2D<T> operator/ (const TNT::Array2D<T> &A, const T & B) {

  TNT::Array2D<T> C(A.dim1(),A.dim2());
  for (int i = 0; i < A.dim1(); i++) {
    for (int j = 0; j < A.dim2(); j++) {
    C[i][j] = A[i][j] / B;
    }
  }
  return C;
}

template <class T>
T dproduct (const TNT::Array1D<T> &A, const TNT::Array1D<T> &B) {
  
  T sum = 0.0;
  for (int i = 0; i < A.dim1(); i++) {
    sum += A[i] * B[i];
  }
  return sum;
}

template <class T>
TNT::Array1D<T> operator- (const TNT::Array1D<T> &A) {

  TNT::Array1D<T> B(A.dim1());
  for (int i = 0; i < A.dim1(); i++) {
    B[i] = -A[i];
  }
  return B;
}

template <class T>
T abs (const TNT::Array1D<T> &A) {

  T mag = 0;
  for (int i = 0; i < A.dim1(); i++) {
    mag += A[i]*A[i]; 
  }
  return sqrt (mag);
}

template <class T>
TNT::Array1D<T> norm (const TNT::Array1D<T> &A) {

  double mag = abs(A);
  TNT::Array1D<T> B(A.dim1());
  for (int i = 0; i < A.dim1(); i++) {
    B[i] = A[i]/mag;
  }
  return B;
}

template <class T>
void writeVector (const TNT::Array1D<T> &v, std::ofstream &theStream) {
  for(int ii=0; ii<v.dim(); ii++)
    writeScalar <T> (v[ii],theStream);
}

template <class T>
TNT::Array1D<T> readVector (int nSize, std::ifstream &theStream) {
  TNT::Array1D<T> v(nSize);
  for( int ii=0; ii<nSize; ii++) 
    v[ii] = readScalar <T> (theStream);
  return v.copy();
}


// Array types
//New vector class
#define FOR(EXPR) for(int i = 0; i < Len; i++){ EXPR; }

//! Template class for a generic vector size
/*! Had to be MACROed because C++ doesn't allow 
	inheritance in partial template specialization ... sucks
*/
#define DEFINE_VECTOR(CLASS)\
	CLASS(T *in)			{ FOR(v[i]=in[i]); }											\
	CLASS(int size)			{;}																\
	CLASS(int size, T* in)		{ FOR(v[i]=in[i]); }										\
	CLASS 		copy		()const{return CLASS((T*)v); }									\
	CLASS& 		operator=	(const CLASS &in){FOR(v[i]=in.v[i]); return *this;}				\
	T& 		operator []	(int idx){assert(idx > -1 && idx < Len); return v[idx];}			\
	const T& 	operator []	(int idx)const {assert(idx > -1 && idx < Len); return v[idx];}	\
	operator T*			(){return v;}														\
	operator const T*		(){return v;}													\
	CLASS 		operator - 	(const CLASS &a)const {CLASS b; FOR(b[i]=v[i]-a[i]); return b;}	\
	CLASS& 		operator -=	(const CLASS &a){ FOR(v[i]-=a[i]); return *this; }				\
	CLASS 		operator - 	(){CLASS b; FOR(b[i]=-v[i]); return b;}							\
	CLASS 		operator + 	(const CLASS &a)const {CLASS b; FOR(b[i]=v[i]+a[i]); return b;}	\
	CLASS& 		operator +=	(const CLASS &a){ FOR(v[i]-=a[i]); return *this; }				\
	CLASS 		operator * 	(T a){CLASS b; FOR(b[i]=v[i]*a); return b;}						\
	CLASS 		operator * 	(const CLASS &a)const {CLASS b; FOR(b[i]=v[i]*a[i]); return b;}	\
	CLASS 		operator / 	(T a){CLASS b; FOR(b[i]=v[i]/a); return b;}						\
	CLASS 		operator / 	(const CLASS &a)const {CLASS b; FOR(b[i]=v[i]/a[i]); return b;}	\
	int 		dim1		() const {return Len;}											\
																							\
	void Deserialize(std::istream &s, int version = 0)										\
	{	s>>"("; FOR(s>>v[i]; if(i!=Len-1) s>>",";); s>>")"; }								\
    void Serialize  (std::ostream &s, int version = 0)										\
	{	s<<"("; FOR(s<<v[i]; if(i!=Len-1) s<<",";); s<<")"; }								\

template <class T, int Len=2> struct _Point2 : public ISerializable							
{
	union{ T v[Len]; struct{T x , y;};};
	_Point2(T a=0,T b=0){x=a; y=b;}
	DEFINE_VECTOR(_Point2);
};

template <class T, int Len =3> struct Vector3 :public ISerializable
{
	union{ T v[Len]; struct{T x, y, z;};};
	Vector3(T a=0,T b=0,T c=0){x=a; y=b; z=c;}
	//Vector3(T* a):_Vector<T,3>(a),x(v[0]),y(v[1]),z(v[2]){}
	DEFINE_VECTOR(Vector3);
};
typedef _Point2<int	  ,2> Point2i;
typedef _Point2<float ,2> Point2f;
typedef _Point2<double,2> Point2d;

template <class T> T         		dproduct(const Vector3<T,3> &a, const Vector3<T,3> &b){ T sum=0; int Len=3;FOR(sum+=a[i]*b[i]); return sum; }
template <class T> T         		abs		(const Vector3<T,3> &a){return sqrt( dproduct(a, a) );}
template <class T> Vector3<T,3> 	norm 	(const Vector3<T,3> &a){ double mag = abs(a); Vector3<T,3> b;  int Len=3;FOR(b[i]=a[i]/mag); return b;}	
template <class T> Vector3<T,3> 	min		(const Vector3<T,3> &a, const Vector3<T,3> &b){ Vector3<T,3> c; int Len=3;FOR( c[i]=a[i]<b[i]?a[i]:b[i]); return c;}	
template <class T> Vector3<T,3> 	max		(const Vector3<T,3> &a, const Vector3<T,3> &b){ Vector3<T,3> c; int Len=3;FOR( c[i]=a[i]>b[i]?a[i]:b[i]); return c;}	
#undef FOR


template <class T> Vector3<T> operator* (const TNT::Array2D<T> &A, const Vector3<T> &B)
{
	Vector3<T> C;
	for (int i = 0; i < A.dim1(); i++) {
		C[i]=0;
		for (int j = 0; j < A.dim2(); j++)
			C[i] += A[i][j] * B[j];
	}
	return C;
}
typedef Vector3<float> 	DTIVector;
typedef Vector3<short> 	DTIShortVector;
typedef Vector3<float> 	DTIGeometryVector;
typedef Vector3<double> Vector3d;

template <class T>
void print_matrix(T *a, int m, int n)
{
	for(int i = 0; i < m; i++)
	{
		for(int j = 0; j < n; j++)
			std::cerr<<a[i*n+j]<<", ";
		std::cerr<<std::endl;
	}
}
#endif
