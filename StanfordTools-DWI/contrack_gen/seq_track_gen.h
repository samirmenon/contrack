#ifndef SEQ_TRACK_GEN_H
#define SEQ_TRACK_GEN_H

class DTIPathwayInterface;
class DTIPathwayFactoryInterface;
class DTIPathway;
class DTISeedGenerator;
#include <typedefs.h>
#include <DTIVolume.h>

class SequentialTrackGenerator {

 public:
 // Constructor / Destructor
  SequentialTrackGenerator (DTIPathwayFactoryInterface *pathwayFactory, DTIScalarVolume *voiMaskVol, DTIScalarVolume *faVolume, DTIVectorVolume *pdfVolume, double stepSizeMm, double kGenSmooth);
  ~SequentialTrackGenerator();
  
  // Generate a single pathway
  DTIPathwayInterface* generatePathway();
  
  // Setting configuration of tracking
  void setStartSeedGenerator (DTISeedGenerator *seedGen) { _start_seed_generator = seedGen; }
  void setEndSeedGenerator   (DTISeedGenerator *seedGen) { _end_seed_generator = seedGen; }
  void setWMVolume(DTIScalarVolume* wm){_wm_volume = wm;}
  void setMaxNumNodes(unsigned int n) {_max_num_nodes = n;}
  void setMinNumNodes(unsigned int n) {_min_num_nodes = n;}
  void setWMThresh(double d){ _wm_thresh = d; }
  void setSTDSmoothness(double d){ _std_smooth = d; }
  void setAngleCutoff(double d){ _angle_cutoff = d; }
  void setShapeParams(double sp1, double sp2, double sp3){ _shapeLinearityMidCl=sp1; _shapeLinearityWidthCl=sp2; _shapeUniformS=sp3; }
  void setNumWayVOI(int n) { _num_way_voi = n; }
  void setAxialPathways(bool b) { _bAxialPathways = b; }
  bool isOnlyFailedTerm() { return _bOnlyFailedTerm; }
  void setStartVOIIndex(int n){_start_voi = n;}
  void setStopVOIIndex(int n){_stop_voi = n;}

  void setupPDFCachedGrid(const char* pointsFilename);
  void setupPDFCache();
  static DTIVector randTangentCache(const DTIVector &curDir, std::vector<DTIVector> &vVecs);
  

  static void drawTangent( DTIVectorVolume *pdfVolume,
                           const DTIVector& curP,
			   const DTIVector& curDir,
			   DTIVector& nextDir /*out*/,
			   double &log_prob_chose, 
			   //			   uint &nCountDrawTangent,
 			   std::vector<DTIVector> &vVecs, 
// 			   std::vector<double> &vCDF,
			   double prior_k = 18,
			   double linearityMidCl = 0.175,
			   double linearityWidthCl = 0.15,
			   double uniform_s = 80,
			   double kGenSmooth = 32);

  private:
  static double absorptionRate( DTIScalarVolume *wm_volume, 
				DTIScalarVolume *voi_volume, 
				const DTIVector &curPos, 
				double wmThresh, 
				int stop_voi);
  static int reachedWayMaskFlag( DTIScalarVolume *wm_volume, 
				 DTIScalarVolume *voi_volume, 
				 const DTIVector &curPos, 
				 double wmThresh,
				 int way_voi,
				 int start_voi );
  static DTIVector initFirstDir( bool bWhichEnd, 
                                 const DTIPathwayInterface *prevPath, 
                                 unsigned int startI, 
                                 int endI, 
                                 const DTIVector &curPos, 
                                 const DTIVector &vox_size);	
  void getStartpoint(DTIVector &seedPoint);	
  static bool satisfiedTerm( DTIScalarVolume *voi_volume, 
			     const DTIVector &curPos, 
			     int stop_voi );

private:
  DTISeedGenerator *_start_seed_generator;
  DTISeedGenerator *_end_seed_generator;
  DTIScalarVolume *_wm_volume;
  DTIVectorVolume *_pdf_volume;
  DTIScalarVolume *_voi_volume;
  bool _bEndValidCortex;
  bool _bStartValidCortex;
  DTIVector _step_size_voxel;
  DTIVector _vox_size;
  int _num_way_voi;
  unsigned int _max_num_nodes;
  unsigned int _min_num_nodes;
  DTIPathwayFactoryInterface *_path_factory;
  bool   _bKeep_all;
  double _wm_thresh;
  double _std_smooth;
  double _shapeLinearityMidCl; 
  double _shapeLinearityWidthCl;
  double _shapeUniformS;
  double _angle_cutoff;
  double _kGenSmooth;
  bool _bOnlyFailedTerm;
  bool _bAxialPathways;
  int _stop_voi;
  int _start_voi;

  int setupGrow(DTIVector &seedPoint);
  int goGrow(DTIPathwayInterface* path, DTIVector &seedPoint);


public:
//    float _tSetup;
//    float _tGrow;
//    float _tTangent;
//    uint _nCountDrawTangent;
//   // For cached tangent drawing computations
  bool _bCachedPDF;
  std::vector<DTIVector> _vCachedPDFSmooth;
//   std::vector<double> _vCachedCDF;
};

#endif
