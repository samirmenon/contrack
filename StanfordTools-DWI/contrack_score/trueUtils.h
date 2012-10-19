#ifndef TRUE_UTILS_H
#define TRUE_UTILS_H
#include <DTIVolumeIO.h>

inline void addProcToFilename(string &ifile, int myid)
{
  if(myid>=0) {
    int nExt = ifile.find_last_of(".");
    string strExt; strExt.assign(ifile.begin()+nExt,ifile.end());
    int nRoot = nExt;
    if(ifile.find_last_of("_")!=string::npos)
      nRoot = ifile.find_last_of("_");
    string ofile; ofile.assign(ifile.begin(),ifile.begin()+nRoot);
    
    ostringstream ss;
    ss << ofile;
    ss << "_" << myid;
    ss << strExt;
    ifile = ss.str();
  }
}

inline void getInitPIDOutFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "init";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".pid";
  filename = ss.str();
}

inline void getInitScoreOutFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "init";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".score";
  filename = ss.str();
}

inline void getGenPIDFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "sub";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".pid";
  filename = ss.str();
}

inline void getGenScoreFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "sub";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".score";
  filename = ss.str();
}

inline void getTempSortPIDFilename(string &filename, const string &prefix, int myid, int iter)
{
  ostringstream ss;
  ss << prefix << "sub_sort";
  if(myid>=0)
    ss << "_" << myid << "_" << iter;
  ss << ".pid";
  filename = ss.str();
}

inline void getTempSortScoreFilename(string &filename, const string &prefix, int myid, int iter)
{
  ostringstream ss;
  ss << prefix << "sub_sort";
  if(myid>=0)
    ss << "_" << myid << "_" << iter;
  ss << ".score";
  filename = ss.str();
}

inline void getVoxelPIDFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "vox";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".pid";
  filename = ss.str();
}

inline void getVoxelScoreFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "vox";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".score";
  filename = ss.str();
}

inline void getModelFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "model";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".pid";
  filename = ss.str();
}

inline void getModelScoreFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "model";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".score";
  filename = ss.str();
}

inline void getModelDeltaAddFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "md_add";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".pid";
  filename = ss.str();
}

inline void getModelDeltaInFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "md_in";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".pid";
  filename = ss.str();
}

inline void getModelDeltaRemFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "md_rem";
  if(myid>=0)
    ss << "_" << myid;
  ss << ".pid";
  filename = ss.str();
}

 inline void getVoxelIntPIDFilename(string &filename, const string &prefix, unsigned int x, unsigned int y, unsigned int z, int myid) 
 { 
   ostringstream ss; 
   ss << prefix << "vox_" << x << "_" << y << "_" << z; 
   if(myid>=0) 
     ss << "_" << myid; 
   ss << ".pid"; 
   filename = ss.str(); 
 } 

inline void getVoxelIntPIDFilename(string &filename, const string &prefix, unsigned int voxNum, int myid)
{
  ostringstream ss;
  ss << prefix;
  if(myid>=0)
    ss << "/p" << myid << "/";
  ss  << "vox_" << voxNum << ".pid";
  filename = ss.str();
}

inline void getVoxelIntOffFilename(string &filename, const string &prefix, int myid)
{
  ostringstream ss;
  ss << prefix << "vox";
  if(myid>=0)
    ss << "_" << myid;
  ss  << ".off";
  filename = ss.str();
}

template <class T>
inline void uniqueNA(vector<T> &v)
{
  int nI = v.size();
  for(int ii=nI-1; ii>0; ii--) {
    if(find(v.begin(),v.begin()+ii,v[ii])!=v.begin()+ii)
      v.erase(v.begin()+ii);
  }
}

template <class T>
inline void SaveSubvolumeIntoVolume(const string &strVolTemplateFile, const string &strOutFile, const DTIVolume<T>* volSub, const vector<int> &vSubCoords)
{
  DTIScalarVolume* volWhole = DTIVolumeIO::loadScalarVolumeNifti (strVolTemplateFile.c_str());
  volWhole->setAllValsDefault();
  uint iDim,jDim,kDim,lDim;
  volSub->getDimension(iDim,jDim,kDim,lDim);
  for(unsigned int kk=0; kk<kDim; kk++) 
    for(unsigned int jj=0; jj<jDim; jj++) 
      for(unsigned int ii=0; ii<iDim; ii++) 
	for(unsigned int ll=0; ll<lDim; ll++) { 
	  float f = (float) volSub->getScalar(ii,jj,kk,ll);
	  volWhole->setScalar(f,ii+vSubCoords[0],jj+vSubCoords[2],kk+vSubCoords[4],ll);
	}

  DTIVolumeIO::saveVolumeNifti(volWhole,strOutFile.c_str());  
  delete volWhole;
}


template <class T>
inline void SaveSubvolumeIntoVolumeOld(const string &strVolTemplateFile, const string &strOutFile, const DTIVolume<T>* volSub, const vector<int> &vSubCoords)
{
  DTIScalarVolume* volWhole = DTIVolumeIO::loadScalarVolumeNifti (strVolTemplateFile.c_str());
  volWhole->setAllValsDefault();
  uint iDim,jDim,kDim;
  volSub->getDimension(iDim,jDim,kDim);
  for(unsigned int kk=0; kk<kDim; kk++) {
    for(unsigned int jj=0; jj<jDim; jj++) {
      for(unsigned int ii=0; ii<iDim; ii++) {
	float f = (float) volSub->getScalar(ii,jj,kk);
	volWhole->setScalar(f,ii+vSubCoords[0],jj+vSubCoords[2],kk+vSubCoords[4]);
      }
    }
  }
  DTIVolumeIO::saveVolumeNifti(volWhole,strOutFile.c_str());  
  delete volWhole;
}

#endif
