#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <iostream>
#include <fstream>
#include <algorithm> //required for std::swap
#include <limits>
#include <string.h>

#define byteSwapAny(x) byteSwap((unsigned char *) &x,sizeof(x))

static bool isLittleEndian() {
  unsigned char EndianTest[2] = { 1, 0 };
  short x;
  x = *(short *) EndianTest;
  return x == 1;
}

static void byteSwap(unsigned char * b, int n)
{
   register int i = 0;
   register int j = n-1;
   while (i<j)
   {
      std::swap(b[i], b[j]);
      i++, j--;
   }
}

template <class T>
static void writeScalar(T scalar, std::ostream &theStream) {
  theStream.write ((char *) &scalar, sizeof(T));
}

template <class T>
static void writeScalarBE(T scalar, std::ostream &theStream) {
  if(isLittleEndian())
    byteSwapAny(scalar);
  theStream.write ((char *) &scalar, sizeof(T));
}

template <class T>
static T readScalar (std::istream &theStream) {
  T scalar;
  theStream.read ((char *) &scalar, sizeof(T));
  return scalar;
}

template <class T>
static T readScalarBE (std::ifstream &theStream) {
  T scalar;
  theStream.read ((char *) &scalar, sizeof(T));
  if(isLittleEndian())
    byteSwapAny(scalar);
  return scalar;
}

static void deleteVoidBuffer (void* buffer, size_t nBytes) {
  if( nBytes == sizeof(double) ) {
    //delete [] (double*)buffer;
    free( (double*)buffer);
  }
  else if( nBytes == sizeof(float) ) {
    //delete [] (float*)buffer;
    free( (float*)buffer);
  }
  else if( nBytes == sizeof(int) ) {
    //delete [] (int*)buffer;
    free( (int*)buffer);
  }
  else if( nBytes == sizeof(short) ) {
    //delete [] (short*)buffer;
    free( (short*)buffer);
  }
  else if( nBytes == sizeof(char) ) {
    //delete [] (char*)buffer;
    free( (char*)buffer);
  }
  else {
    std::cerr << "Cannot delete buffer of unknown type!" << std::endl;
  }
}

// Text parsing
static void IgnoreLine(std::istream& is) 
{
  is.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
}

static void IgnoreBeforeChar(std::istream& is, char c) 
{
  is.ignore(std::numeric_limits<std::streamsize>::max(),c);
}

template <class T>
static T GetQStateScalar(std::istream &is) {
  IgnoreBeforeChar(is,':');
  T scalar;
  is >> scalar;
  IgnoreLine(is);
}

template <class T>
static void GetQStateVector3(std::istream &is, T v[3]) {
  IgnoreBeforeChar(is,':');
  char c;
  is >> v[0] >> c >> v[1] >> c >> v[2];
  IgnoreLine(is);
}

std::istringstream* FileToString(std::ifstream &file);
void StringToFile(std::ofstream &file, std::ostringstream &localStream);
std::string RelativePath(char *currDir, std::string path);
std::string RelativePath(std::string path);
std::string AbsolutePath(char *currDir, std::string path);
std::string AbsolutePath(std::string path);
std::string FileNameToPath(std::string filename);
std::string PathToFileName(std::string filename);

//Read a const string from the stream
inline std::istream & operator>>(std::istream &in , const char *str){static char dummy[1024]; return in.get(dummy, strlen(str)+1);}
//Read end of line using std::endl
inline std::istream & operator>>(std::istream &in , std::ostream& (*fn)(std::ostream&)){static char c; return in.get(c);}

//! An interface for object serialization
/*! This interfaces serializes the state of the object rather the actual data
	For eg. in case of gui elements it saves the position, visibility etc.
	For fibers it saves their assignments and list of pdb's loaded. It doesn't
	save the actual fiber data as one might expect. This is because of the limitation
	of the pdb file format. This might change in future.
*/
class ISerializable
{
public:
	virtual void Deserialize(std::istream &s, int version = 0) = 0;
    virtual void Serialize  (std::ostream &s, int version = 0) = 0;
};



#endif
