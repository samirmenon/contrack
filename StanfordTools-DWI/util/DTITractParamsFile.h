/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_TRACT_PARAMS_FILE_H
#define DTI_TRACT_PARAMS_FILE_H

#include "typedefs.h"
#include <fstream>
#include <vector>

class DTITractParamsFile {

public:

  DTITractParamsFile();
  unsigned int nDesiredSamples;
  unsigned int nBurnIn;
  double       dStepSizeMm;
  unsigned int nSkipSamples;
  unsigned int nMaxChainLength;
  unsigned int nMinChainLength;
  bool         bUseAbsorption;
  bool         bComputeFA;
  bool 	       bUseWayMask;
  DTIVector startROIPos;
  DTIVector startROISize;
  bool bStartValidCortex;
  bool bStartIsSeedRegion;
  DTIVector endROIPos;
  DTIVector endROISize;
  bool bEndValidCortex;
  bool bEndIsSeedRegion;
  DTIMCMethod method;
  DTIPathWeight pathWeightType;
  unsigned int nTrialsPerNode;
  unsigned int nTrialsPerNodeLengthChange;
  unsigned int nStartPathTries;
  std::string olaDirsFilename;
  std::string olaLikeLUTFilename;
  std::string tensorsFilename;
  std::string wmFilename;
  std::string pdfFilename;
  std::string voiMaskFilename;
  std::string exMaskFilename;
  unsigned int nSaveOutSpacing;
  
  // Mutation Probabilities
  double translateMut;
  double eccbClosedMut;
  double ccbClosedMut;
  double eccbLargeNMut;
  double eccbLargeSMut;
  double eccbLargeAMut;
  double eccbNMut;
  double eccbSMut;
  double eccbAMut;
  double ccbNMut;
  double ccbSMut;
  double ccbAMut;
  double epMut;
  double spMut;
  double rotateMut;

  // Tempering Info
  double tempSwapProb;
  std::vector<double> invTempsVec;

  // Prior Stuff: Absorption and Smoothness
  double wmThresh;
  double absRateNormal;
  double absRatePenalty;
  double stdSmoothness;
  double kGenSmooth;
  double angleCutoff;
  double shapeLinearityMidCl;
  double shapeLinearityWidthCl;
  double shapeUniformS;

  static double s2kSmooth(double s);

  static DTITractParamsFile* load1(const char* filename);
  static DTITractParamsFile* load0(const char* filename);
  void save1(const char* filename);
  void save0(const char* filename);


private:

  static void getLine(std::istream& stream, unsigned int &i);
  static void getLine(std::istream& stream, double &d);
  static void getLine(std::istream& stream, std::vector<double> &v);
  static void getLine(std::istream& stream, std::string &str);
  static void getLine(std::istream& stream, bool &b);
  static void getLine(std::istream& stream, DTIVector &v); 
  static void getLine(std::istream& stream, DTIMCMethod &m);
  static void getLine(std::istream& stream, DTIPathWeight &w);

  static void setLine(std::ostream& stream, const char* string, bool b);
  static void setLine(std::ostream& stream, const char* string,  std::vector<double> &v);
  static void setLine(std::ostream& stream, const char* string, DTIMCMethod &m);
  static void setLine(std::ostream& stream, const char* string, DTIPathWeight &w);
  static void setLine(std::ostream& stream, const char* string, DTIVector &v); 

};

#endif


