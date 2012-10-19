/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior
University. All rights reserved. **/

#ifndef Q_SUBJECTDATA_H
#define Q_SUBJECTDATA_H

#include <typedefs.h>
#include <DTIVolume.h>
#include <memory>

//! Stores the information relating to nifti volume
class VolumeInfo : public ISerializable
{
friend class SubjectData;
protected:
    VolumeInfo(){}
    bool LoadFromFile(std::string filename);

public:
    VolumeInfo(PDTIScalarVolume vol, std::string name, std::string path):_vol(vol),_name(name), _path(path){}
    //! The nifti volume
    PROPERTY(PDTIScalarVolume, _vol, Volume);
    //! Name of the volume
    PROPERTY(std::string, _name, Name);
    //! Absolute path of the volume on disk
    PROPERTY(std::string, _path, Path);

    //--- ISerializable interface ---//
    void Deserialize(std::istream &s, int version = 0);
    void Serialize  (std::ostream &s, int version = 0);
};

//! This class stores the nifti images associated with a subject
class SubjectData : public std::vector<VolumeInfo>, public ISerializable
{
public:
    //! Loads the standard volumes which are t1, b0, brainmash and pddDispersion
    bool LoadStandardVolumes();
    //! Loads a single volume specified by the path in \param filename
    bool LoadVolume(std::string filename);
    //! Not used
    void LoadQVecs(std::string strValFile, std::string strVecFile);

    //! Path to the subject's data directory
    PROPERTY(std::string, _subjectDir, SubjDir);
    //! Not used
    PROPERTY(DTIVectorVolumePtr, _volRaw, RawVolume);

    const std::vector<DTIVector> &GetQVecs()const{return _qvecs;}
    const std::vector<float> &GetQVals()const{return _qvals;}

    //--- ISerializable interface ---//
    void Deserialize(std::istream &s, int version = 0);
    void Serialize  (std::ostream &s, int version = 0);

    const static std::string _vStdFiles[4];
private:

    // trueData stuff
    static double _tau;    /// diffustion time (ms)
    static double _qmag;   /// magnitude of 1/micron^2
    static std::string _sQVecFile;
    static std::string _sQValFile;
    static std::string _sRawFile;
    static std::string _sPredFile;

    std::vector<DTIVector> _qvecs; /// scanning directions
    std::vector<float> _qvals;
};

#endif

