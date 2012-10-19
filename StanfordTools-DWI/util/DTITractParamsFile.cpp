#include "DTITractParamsFile.h"
#include <fstream>
#include <string>
#include "DTIMath.h"

/***********************************************************************
 *  Method: DTITractParamsFile::DTITractParamsFile
 *  Params: 
 * Effects: 
 ***********************************************************************/
DTITractParamsFile::DTITractParamsFile()
{
  nDesiredSamples = 1000;
  nBurnIn = 1;
  dStepSizeMm = 1;
  nSkipSamples = 1;
  nMaxChainLength = 7;
  nMinChainLength = 7;
  bUseAbsorption = false;
  bComputeFA = false;
  bUseWayMask = false;
  startROIPos = DTIVector(3);
  startROISize = DTIVector(3);
  bStartValidCortex = true;
  bStartIsSeedRegion = true;
  endROIPos = DTIVector(3);
  endROISize = DTIVector(3);
  bEndValidCortex = true;
  bEndIsSeedRegion = true;
  method = SISR;
  pathWeightType = POST;
  nTrialsPerNode = 5;
  nTrialsPerNodeLengthChange = 20;
  nStartPathTries = 1;
  olaDirsFilename = "";
  olaLikeLUTFilename = "";
  tensorsFilename = "";
  wmFilename = "";
  pdfFilename = "";
  voiMaskFilename = "";
  exMaskFilename = "";
  nSaveOutSpacing = 100;
  
  // Mutation Probabilities
  translateMut = 0;
  eccbClosedMut = 0;
  ccbClosedMut = 1;
  eccbLargeNMut = 0;
  eccbLargeSMut = 0;
  eccbLargeAMut = 0;
  eccbNMut = 0;
  eccbSMut = 0;
  eccbAMut = 0;
  ccbNMut = 0;
  ccbSMut = 0;
  ccbAMut = 0;
  epMut = 0;
  spMut = 0;
  rotateMut = 0;

  // Tempering Info
  tempSwapProb = 0.1;
  invTempsVec.push_back(1); 

  // Prior
  wmThresh = 0.15;
  absRateNormal = 0.9;
  absRatePenalty = 1.38e-87;
  stdSmoothness = 14;
  kGenSmooth = s2kSmooth(stdSmoothness);
  angleCutoff = 90;
  shapeLinearityMidCl = 0.175;
  shapeLinearityWidthCl = 0.15;
  shapeUniformS = 80;
}

DTITractParamsFile* 
DTITractParamsFile::load1(const char* filename)
{
  DTITractParamsFile* params = new DTITractParamsFile();
  std::ifstream stream(filename, std::ios::in);
  std::cerr << "Loading Track Params File" << std::endl;

  if(stream) {
    unsigned int version;
    getLine(stream, version);
    if (version != 1) {
      delete params;
      std::cerr << "Need tracking parameters version 1." << std::endl;
      return NULL;
    }
    // Filenames
    std::string strImgDir;
    std::ostringstream ost;
    getLine(stream,strImgDir);
    getLine(stream,params->wmFilename);
    ost << strImgDir << params->wmFilename;
    params->wmFilename = ost.str();
    ost.str("");
    getLine(stream,params->pdfFilename);
    ost << strImgDir << params->pdfFilename;
    params->pdfFilename = ost.str();
    ost.str("");
    getLine(stream,params->voiMaskFilename);
    ost << strImgDir << params->voiMaskFilename;
    params->voiMaskFilename = ost.str();
    // Basic pathway properties
    getLine(stream,params->nDesiredSamples);
    getLine(stream,params->nMaxChainLength);
    getLine(stream,params->nMinChainLength);
    getLine(stream,params->dStepSizeMm);
    // ROI start regions
    getLine(stream,params->bStartIsSeedRegion);
    getLine(stream,params->bEndIsSeedRegion);
    // Save out settings
    getLine(stream, params->nSaveOutSpacing);
    //  Prior
    getLine(stream, params->wmThresh);
    getLine(stream, params->absRateNormal);
    getLine(stream, params->absRatePenalty);
    getLine(stream, params->stdSmoothness);
    params->kGenSmooth = s2kSmooth(params->stdSmoothness);
    // Assume that stdSmoothness was given in degrees
    params->stdSmoothness = 1 / ((sin(params->stdSmoothness*M_PI/180))*(sin(params->stdSmoothness*M_PI/180)));
    getLine(stream, params->angleCutoff);
    params->angleCutoff = params->angleCutoff*M_PI/180;  // Text file degrees, software radians
    std::vector<double> shapeParams;
    getLine(stream, shapeParams);
    if( shapeParams.size() != 3 ) {
      std::cerr << "Error reading shape function parameters from file." << std::endl;
      exit(0);
    }
    params->shapeLinearityMidCl = shapeParams[0];
    params->shapeLinearityWidthCl =  shapeParams[1];
    params->shapeUniformS =  shapeParams[2];
    // Close the file stream
    stream.close();
  }
  return params;
}

/***********************************************************************
 *  Method: DTITractParamsFile::load0
 *  Params: const char *filename
 * Returns: DTITractParamsFile *
 * Effects: 
 ***********************************************************************/
DTITractParamsFile *
DTITractParamsFile::load0(const char *filename)
{
  DTITractParamsFile* params = new DTITractParamsFile();
  std::ifstream stream(filename, std::ios::in);

  std::cerr << "Loading Tract Params File" << std::endl;

  if(stream) {
    
    getLine(stream,params->tensorsFilename);
    getLine(stream,params->wmFilename);
    getLine(stream,params->pdfFilename);
    getLine(stream,params->voiMaskFilename);
    getLine(stream,params->exMaskFilename);
    getLine(stream,params->bUseWayMask);
    getLine(stream,params->nDesiredSamples);
    getLine(stream,params->nMaxChainLength);
    getLine(stream,params->nMinChainLength);
    getLine(stream,params->dStepSizeMm);

    params->startROIPos = DTIVector(3);
    getLine(stream,params->startROIPos);

    params->startROISize = DTIVector(3);
    getLine(stream,params->startROISize);

    getLine(stream,params->bStartIsSeedRegion);

    params->endROIPos = DTIVector(3);
    getLine(stream,params->endROIPos);

    params->endROISize = DTIVector(3);
    getLine(stream,params->endROISize);

    getLine(stream,params->bEndIsSeedRegion);

    getLine(stream, params->nSaveOutSpacing);

    //  Prior
    getLine(stream, params->wmThresh);
    getLine(stream, params->absRateNormal);
    getLine(stream, params->absRatePenalty);
    getLine(stream, params->stdSmoothness);
    params->kGenSmooth = s2kSmooth(params->stdSmoothness);
    // Assume that stdSmoothness was given in degrees
    params->stdSmoothness = 1 / ((sin(params->stdSmoothness*M_PI/180))*(sin(params->stdSmoothness*M_PI/180)));
    getLine(stream, params->angleCutoff);
    params->angleCutoff = params->angleCutoff*M_PI/180;  // Text file degrees, software radians
    std::vector<double> shapeParams;
    getLine(stream, shapeParams);
    if( shapeParams.size() != 3 ) {
      std::cerr << "Error reading shape function parameters from file." << std::endl;
      exit(0);
    }
    params->shapeLinearityMidCl = shapeParams[0];
    params->shapeLinearityWidthCl =  shapeParams[1];
    params->shapeUniformS =  shapeParams[2];
    stream.close();
  }
  return params;
}



void 
DTITractParamsFile::save1(const char* filename)
{
  std::ofstream stream(filename, std::ios::out);
  std::cerr << "Saving Track Params File" << std::endl;

  if(stream) {
    stream << "Parameters Version: 1" << std::endl;
    stream << "Image Directory: " << " " << std::endl;
    stream << "WM Mask Filename: " << this->wmFilename << std::endl;
    stream << "PDF Filename: " << this->pdfFilename << std::endl;
    stream << "ROI Mask Filename: " << this->voiMaskFilename << std::endl;
    stream << "Desired Samples: " << this->nDesiredSamples << std::endl;
    stream << "Max Pathway Nodes: " << this->nMaxChainLength << std::endl;
    stream << "Min Pathway Nodes: " << this->nMinChainLength << std::endl;
    stream << "Step Size (mm): " << this->dStepSizeMm << std::endl;
    setLine(stream,"ROI1 Starts Tracks: ", this->bStartIsSeedRegion);
    setLine(stream,"ROI2 Starts Tracks: ", this->bEndIsSeedRegion);
    stream << "Save Out Spacing: " << this->nSaveOutSpacing << std::endl;
    stream << "WM Mask Threshold: " << this->wmThresh << std::endl;
    stream << "Absorption Rate WM: " << this->absRateNormal << std::endl;
    stream << "Absorption Rate Penalty for NonWM: " << this->absRatePenalty << std::endl;
    stream << "Local Path Segment Smoothness Standard Deviation: " << this->stdSmoothness << std::endl;
    stream << "Local Path Segment Angle Cutoff (degrees): " << this->angleCutoff << std::endl;
    std::vector<double> shapeParams;
    shapeParams.push_back(this->shapeLinearityMidCl);
    shapeParams.push_back(this->shapeLinearityWidthCl);
    shapeParams.push_back(this->shapeUniformS);
    setLine(stream,"ShapeFunc Params (LinMidCl,LinWidthCl,UniformS): ", shapeParams);
    stream.close();
  }
}


/***********************************************************************
 *  Method: DTITractParamsFile::save060822
 *  Params: const char *filename
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTITractParamsFile::save0(const char *filename)
{
  std::ofstream stream(filename, std::ios::out);
  std::cerr << "Saving Tract Params File" << std::endl;

  if(stream) {

    stream << "Tensor Filename: " << this->tensorsFilename << std::endl;
    stream << "WM Mask Filename: " << this->wmFilename << std::endl;
    stream << "PDF Filename: " << this->pdfFilename << std::endl;
    stream << "ROI Mask Filename: " << this->voiMaskFilename << std::endl;
    stream << "Exclusion Mask Filename: " << this->exMaskFilename << std::endl;
    setLine(stream,"Using Way Mask: ",this->bUseWayMask);
    stream << "Desired Samples: " << this->nDesiredSamples << std::endl;
    stream << "Max Pathway Nodes: " << this->nMaxChainLength << std::endl;
    stream << "Min Pathway Nodes: " << this->nMinChainLength << std::endl;
    stream << "Step Size (mm): " << this->dStepSizeMm << std::endl;
    setLine(stream,"Start ROI Pos (ijk): ", this->startROIPos);
    setLine(stream,"Start ROI Size (ijk): ", this->startROISize);
    setLine(stream,"Start Is Seed ROI: ", this->bStartIsSeedRegion);
    setLine(stream,"End ROI Pos (ijk): ", this->endROIPos);
    setLine(stream,"End ROI Size (ijk): ", this->endROISize);
    setLine(stream,"End Is Seed ROI: ", this->bEndIsSeedRegion);
    stream << "Save Out Spacing: " << this->nSaveOutSpacing << std::endl;
    stream << "Threshold for WM/GM specification: " << this->wmThresh << std::endl;
    stream << "Absorption Rate WM: " << this->absRateNormal << std::endl;
    stream << "Absorption Rate Penalty for NonWM: " << this->absRatePenalty << std::endl;
    stream << "Local Path Segment Smoothness Standard Deviation: " << this->stdSmoothness << std::endl;
    stream << "Local Path Segment Angle Cutoff (degrees): " << this->angleCutoff << std::endl;
    std::vector<double> shapeParams;
    shapeParams.push_back(this->shapeLinearityMidCl);
    shapeParams.push_back(this->shapeLinearityWidthCl);
    shapeParams.push_back(this->shapeUniformS);
    setLine(stream,"ShapeFunc Params (LinMidCl,LinWidthCl,UniformS): ", shapeParams);

    stream.close();
  }
}

void
DTITractParamsFile::getLine(std::istream &stream, unsigned int &i)
{
  std::string temp;
  while( temp.rfind(':') == std::string::npos)
    stream >> temp;
  stream >> i;
}

void
DTITractParamsFile::getLine(std::istream &stream, double &d)
{
  std::string temp;
  while( temp.rfind(':') == std::string::npos)
    stream >> temp;
  stream >> d;
}

// void
// DTITractParamsFile::getLine(std::istream &stream, std::vector<double> &v)
// {
// //   std::string temp;
// //   while( temp.rfind(':') == std::string::npos)
// //     stream >> temp;
// //   stream >> d;
// //   v.push_back(d);
// //   while( 
// }

void
DTITractParamsFile::getLine(std::istream &stream, DTIVector &v)
{
  std::string temp;
  char c;
  //double d[3];
  while( temp.rfind(':') == std::string::npos)
    stream >> temp;

  stream >> v[0] >> c >> v[1] >> c >> v[2];
}

void
DTITractParamsFile::getLine(std::istream &stream, std::vector<double> &v)
{
  std::string temp;
  char c;
  
  // Lets clear out the vector first
  v.erase( v.begin(),v.end() );

  while( temp.rfind(':') == std::string::npos)
    stream >> temp;

  // Get the opening vector character [, (
  stream >> temp;
  double d;
  while( temp.rfind(']') == std::string::npos && temp.rfind(')') == std::string::npos ) {
    stream >> d;
    v.push_back(d);
    stream >> temp;
  }
}

void
DTITractParamsFile::getLine(std::istream &stream, std::string &str)
{
  std::string temp;
  while( temp.rfind(':') == std::string::npos)
    stream >> temp;
  stream >> str;
}

void
DTITractParamsFile::getLine(std::istream &stream, bool &b)
{
  std::string temp;
  std::string str_value;
  while( temp.rfind(':') == std::string::npos)
    stream >> temp;
  stream >> str_value;
  // Convert to bool
  if( !str_value.compare("true") || !str_value.compare("TRUE") )
    b = true;
  else
    b = false;
}

/***********************************************************************
 *  Method: DTITractParamsFile::getLine
 *  Params: std::istream &stream, DTIMCMethod &m
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTITractParamsFile::getLine(std::istream &stream, DTIMCMethod &m)
{
  std::string temp;
  getLine(stream,temp);
  const char* chr = temp.c_str();
  if(chr[0] == 'M') {
    m = MCMC;
  }
  else if(chr[0] == 'F') {
    m = FORWARD_STEP;
  }
//   else if(chr[0] == 'C') {
//     m = CCB;
//   }
//   else if(chr[0] == 'E') {
//     m = ECCB;
//   }
  else if(chr[0] == 'O') {
    m = OLA;
  }
  else if(chr[0] == 'K') {
    m = KEEP;
  }
  else if(chr[0] == 'S') {
    m = SISR;
  }
  else {
    std::cerr << "Invalid Monte Carlo Method from parameter file." << std::endl;
    exit(1);
  }
}

/***********************************************************************
 *  Method: DTITractParamsFile::getLine
 *  Params: std::istream &stream, DTIPathWeight &w
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTITractParamsFile::getLine(std::istream &stream, DTIPathWeight &w)
{
  std::string temp;
  getLine(stream,temp);
  if( !temp.compare("UNIF") ) {
    w = UNIF;
  }
  else if(!temp.compare("PRIOR") ) {
    w = PRIOR;
  }
  else if( !temp.compare("LIKE") ) {
    w = LIKE;
  }
  else if( !temp.compare("POST") ) {
    w = POST;
  }
  else {
    std::cerr << "Invalid path weight." << std::endl;
    exit(1);
  }
}




/***********************************************************************
 *  Method: DTITractParamsFile::setLine
 *  Params: std::ostream &stream, std::vector<double> &v
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTITractParamsFile::setLine(std::ostream &stream, const char* string, std::vector<double> &v)
{
  if( v.size() < 1)
    return;

  stream << string;
  // Get the opening vector character [, (
  stream << "[ ";
  stream << v[0];
  for (int i=1; i < v.size(); i++)
    stream << ", " << v[i];

  stream << " ]" << std::endl;
}


/***********************************************************************
 *  Method: DTITractParamsFile::setLine
 *  Params: std::ostream &stream, DTIMCMethod &m
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTITractParamsFile::setLine(std::ostream &stream, const char* string,  DTIMCMethod &m)
{
  stream << string;

  switch(m) {
  case MCMC:
    stream << "MCMC" << std::endl;
    break;
  case FORWARD_STEP:
    stream << "FORWARD_STEP" << std::endl;
    break;
  case OLA:
    stream << "OLA" << std::endl;
    break;
  case KEEP:
    stream << "KEEP_ALL" << std::endl;
    break;
  case SISR:
    stream << "SISR" << std::endl;
    break;
  default:
    stream << "MCMC" << std::endl;
    break;
  }
}


/***********************************************************************
 *  Method: DTITractParamsFile::setLine
 *  Params: std::ostream &stream, DTIPathWeight &w
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTITractParamsFile::setLine(std::ostream &stream, const char* string, DTIPathWeight &w)
{
  stream << string;

  switch(w) {
  case UNIF:
    stream << "UNIF" << std::endl;
    break;
  case PRIOR:
    stream << "PRIOR" << std::endl;
    break;
  case LIKE:
    stream << "LIKE" << std::endl;
    break;
  case POST:
    stream << "POST" << std::endl;
    break;
  default:
    stream << "POST" << std::endl;
    break;
  }
}


/***********************************************************************
 *  Method: DTITractParamsFile::setLine
 *  Params: std::ostream &stream, DTIVector &v
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTITractParamsFile::setLine(std::ostream &stream, const char* string, DTIVector &v)
{
  stream << string;
  stream << v[0] << ", " << v[1] << ", " << v[2] << std::endl;
}



/***********************************************************************
 *  Method: DTITractParamsFile::setLine
 *  Params: std::ostream &stream, bool b
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTITractParamsFile::setLine(std::ostream &stream, const char* string, bool b)
{
  stream << string;
  if(b)
    stream << "true" << std::endl;
  else
    stream << "false" << std::endl;
}

double 
DTITractParamsFile::s2kSmooth(double s)
{
  s = s-4;
  if(s<0)s=4;
  if(s>54.74) s = 54.74;
  double k = 1 / (sin(s*M_PI/180.0)*sin(s*M_PI/180.0));
  return k;
}


