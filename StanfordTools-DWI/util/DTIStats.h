#ifndef DTI_STATS_H
#define DTI_STATS_H

class DTIPathwayDatabase;
class DTIPathwayInterface;
class DTITrackingData;

#include "typedefs.h"
#include <vector>
#include "DTIVolume.h"

class DTIStats {

  // NON-SCORING functions
 public:
  static double computeLength (DTIPathwayInterface *path, PathwayProperty id, double step_size_mm);
  static double computeCurvature (DTIPathwayInterface *path, PathwayProperty id);
  static double computeEPDistance (DTIPathwayInterface *path, 
				   PathwayProperty distID, 
				   PathwayProperty xposID, 
				   PathwayProperty yposID, 
				   PathwayProperty zposID);
  static double computeFA (DTIPathwayInterface *path, PathwayProperty id, DTIScalarVolume *faVol);
  static double computeLinearity(DTIPathwayInterface *path, PathwayProperty id, DTIVectorVolume *pdfVol);
  static int    computeSampleOrder(DTIPathwayInterface *path, PathwayProperty id, int order);
  static double computeDistance (const DTIVector &p1, const DTIVector &p2);  
  static bool computeAllPathPointsInBounds(const DTIPathwayInterface *path, const DTIScalarVolume *vol);

 private:
  static double computeLocalCurvature (const DTIVector &pt1, const DTIVector &pt2, const DTIVector &pt3);
 
  // SCORING functions
public:
  static float scorePathway(DTIPathwayInterface* pathway, 
			    float lengthParam, 
			    float smoothParam, 
			    float tensorShapeParam, 
			    DTITrackingData *tData,
			    bool bFreeEnds, 
			    bool bReportInf=false);
  static float scorePathway(DTIPathwayInterface* pathway, 
			    float lengthParam, 
			    float smoothParam, 
			    float tensorShapeParam, 
			    std::vector<double> &vecLike,
			    std::vector<double> &vecPrior,
			    std::vector<double> &vecAbs,
			    DTITrackingData *tData,
			    bool bFreeEnds,
			    bool bReportInf=false);

  static double computeLogScoreLikelihood( DTIPathwayInterface *path, 
					   PathwayProperty likelihoodID, 
					   std::vector<double> &vecLikelihood,
					   DTIVectorVolume *pdfParamsImage, 
					   double linearityMidCl = 0.175,
					   double linearityWidthCl = 0.15,
					   double uniform_s = 80,
					   bool bReportInf = false);

  static double computeLogScorePrior( DTIPathwayInterface *path, 
				      PathwayProperty priorID,
				      std::vector<double> &vecPrior,   				 
				      double stdSmoothness = 3.14/5,   				 
				      double angleCutoff = 1.571,
				      bool bReportInf = false);

  static double computeLogScoreAbsorption( DTIPathwayInterface *path, 
					   PathwayProperty id, 
					   std::vector<double> &vecAbs,
					   DTIScalarVolume *wmImage,
					   DTIScalarVolume *voiImage,					 
					   double wmThresh,
					   int start_voi,
					   int stop_voi,
					   bool bReportInf=false);

  // LOCAL SCORING functions
  static double computeLocalScoreAbsorptionRate( DTIScalarVolume *wm_volume, 
						 DTIScalarVolume *voi_volume, 
						 const DTIVector &curPos, 
						 double wmThresh, 
						 int stop_voi );
   
  static double computeLocalLogLikelihoodFB5 (const DTIVector &x, 
					      const DTIVector &e1, 
					      const DTIVector &e2, 
					      double k1,
					      double k2,
					      double &angle, 
					      double &deviation);
  
  static double computeLocalLogPriorWatson (const DTIVector &v1, 
					    const DTIVector &v2, 
					    double &angle, 
					    double k = 18, 
					    double angleCutoff = 1.571);
private:
  static void computePathTangent ( DTIVector &tangent, 
				   const DTIVector &p1, 
				   const DTIVector &p2, 
				   const DTIVector &p3 );
  static void computePathTangent ( DTIVector &tangent, 
				   const DTIVector &p1, 
				   const DTIVector &p2 );
};


#endif
