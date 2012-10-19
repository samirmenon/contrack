/***********************************************************************
* AUTHOR: sherbond <sherbond>
*   FILE: SubjectData.cpp
*   DATE: Thu Oct 23 16:59:05 2008
*  DESCR: 
***********************************************************************/
#include "SubjectData.h"
#include <DTIVolumeIO.h>
#include <fstream>
#include <functional>
#include <algorithm>
using namespace std;

bool VolumeInfo::LoadFromFile(std::string filename)
{
	// load nifti file
	PDTIScalarVolume vol(DTIVolumeIO::loadScalarVolumeNifti(filename.c_str()));
	if(vol!=NULL) 
	{
		// find the name of the file from the path
		string name = filename;
		int n = name.find_last_of('/');
		if(n == -1)n = name.find_last_of('\\');
		if(n != -1)name.erase(name.begin(),name.begin()+n+1);
		n = name.find(".nii.gz");
		if(n != -1)name.erase(name.begin()+n,name.end());

		_name	= name;
		_path	= filename;
		_vol	= vol;
		return true;
	}
	return false;
}
void VolumeInfo::Deserialize(std::istream &s, int version)
{
	char path[2048];
	s.getline(path,2048);

	//Convert to absolute path for consistency
	LoadFromFile(AbsolutePath(path));
}
void VolumeInfo::Serialize  (std::ostream &s, int version)
{
	s<< RelativePath(_path) <<endl;
}

const  string SubjectData::_vStdFiles[4] = {"/t1.nii.gz", "/b0.nii.gz", "/brainMask.nii.gz", "/pddDispersion.nii.gz"};
string SubjectData::_sQValFile = "/../raw/dti_g13_b800_aligned_avg.bvals";
string SubjectData::_sQVecFile = "/../raw/dti_g13_b800_aligned_avg.bvecs";
string SubjectData::_sRawFile  = "/../raw/dti_g13_b800_aligned_avg.nii.gz";
double SubjectData::_tau = 40;
double SubjectData::_qmag = sqrt(0.02);


bool SubjectData::LoadStandardVolumes()
{
	// First clear any previous volumes
	clear();
	_volRaw.~shared_ptr();
	// Attempt to find standard subject data
	for(int i = 0; i<4; i++) 
		LoadVolume(_subjectDir + string(_vStdFiles[i]));

	return size()?true : false;
}


bool SubjectData::LoadVolume(std::string filename)
{
	cout << "Loading " << filename << "..." << endl;
	VolumeInfo vol;
	if( vol.LoadFromFile(filename) )
	{
		push_back(vol);
		return true;
	}
	return false;
}

void SubjectData::LoadQVecs(string strValFile, string strVecFile)
{
	// locals
	string buffer;
	float d;

	// bvals
	ifstream streamVal(strValFile.c_str(), std::ios::in);
	// all bvals are on one line
	getline(streamVal,buffer);
	istringstream ss(buffer);
	while( ss >> d ) {
		_qvals.push_back(d);
	}
	streamVal.close();

	// update qmag
	vector<float>::iterator ii = find_if( _qvals.begin(), _qvals.end(), bind2nd(greater<double>(), 0));
	if( ii == _qvals.end() ){
		cerr << "All bvals <= 0." << endl;
		exit(-1);
	}
	_qmag = sqrt(*ii/_tau);

	// bvals
	vector<float> vx;
	vector<float> vy;
	vector<float> vz;
	ifstream streamVec(strVecFile.c_str(), std::ios::in);
	// x
	getline(streamVec,buffer);
	istringstream ssx(buffer);
	while( ssx >> d ) {
		vx.push_back(d);
	}
	// y
	getline(streamVec,buffer);
	istringstream ssy(buffer);
	while( ssy >> d ) {
		vy.push_back(d);
	}
	// z
	getline(streamVec,buffer);
	istringstream ssz(buffer);
	while( ssz >> d ) {
		vz.push_back(d);
	}

	for (int vv=0; vv<(int)_qvals.size(); vv++) {
		DTIVector vec(3);
		vec[0]=vx[vv]; vec[1]=vy[vv]; vec[2]=vz[vv];
		_qvecs.push_back(vec.copy());
	}
	streamVec.close();
}

void SubjectData::Serialize(std::ostream &s, int version)
{
	s<<"--- Volumes ---"<<endl;
	s<<"Num Volumes :"<<size()<<endl;

	// save information about each volume
	for(int i = 0; i < (int)size(); i++)
		(*this)[i].Serialize(s, version);
	s<<endl;
}

void SubjectData::Deserialize(std::istream &s, int version)
{
	//Delete all volumes
	clear();
	int num;

	s>>"--- Volumes ---">>endl;
	s>>"Num Volumes :">>num>>endl;
	for(int i = 0; i < num; i++)
	{
		VolumeInfo vol;
		
		//deserialize each volume
		vol.Deserialize(s,version);
		push_back(vol);
	}
	s>>endl;
}
