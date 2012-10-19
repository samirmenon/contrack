#ifndef DTI_MATH_H
#define DTI_MATH_H

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include "typedefs.h"
#include <vector>
#include <limits>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef INVSQRT2PI
#define INVSQRT2PI 1/sqrt(2*M_PI)
#endif

class DTIMath {

 public:

  static inline float  float_eps(){return std::numeric_limits<float>::epsilon();}
  static inline double double_eps(){return std::numeric_limits<double>::epsilon();}
  static inline float  float_inf(){return std::numeric_limits<float>::infinity();}
  static inline double double_inf(){return std::numeric_limits<double>::infinity();}
  static inline float  float_neg_inf(){return -float_inf();}
  static inline double double_neg_inf(){return -double_inf();}

  static inline unsigned int interpNN(float f){return (unsigned int)floor(f);}

  static float round(float x)  { return x<0 ? ceil((x)-0.5f) : floor((x)+0.5f); }
  static double round(double x) { return x<0 ? ceil((x)-0.5) : floor((x)+0.5); }

  // grid computation
  //static std::vector<DTIVector> unif_hemisphere_pts(int nTheta/*max: pi/2*/, int nPhi/*max: 2*pi*/);

  static void convertEucToSph(const DTIVector& vec, double& theta, double& phi, double &r);
  static void convertSphToEuc(DTIVector& vec, double theta, double phi, double r);
  static void convertEucToSphCamino(const DTIVector& vec, double& theta, double& phi, double &r);
  static void convertSphToEucCamino(DTIVector& vec, double theta, double phi, double r);
  static double angle(const DTIVector &a, const DTIVector &b);

  static void initRNG(int offset=0);
  static void initRNG(int offset, uint seed);
  static double randn(); // random number from normal distribution (mean 0: std: 1)
  static double randn(double mean, double std);
  static double randRice(double mean, double std);
  static double randzeroone (); // random number uniform [0,1]
  static void randn3(double r[3]); // random number from normal for 3D
  static double pdf_randn3(double r[3]); // random number from normal for 3D
  static DTIVector rand_point_sphere();
  static void rand_point_sphere_strat(std::vector<DTIVector> &vec_list, unsigned int num_samples);
  static DTIVector rand_point_cone(const DTIVector &dir, const double theta_min, const double theta_max);
  static void rand_point_cone_strat(std::vector<DTIVector> &vec_list, unsigned int num_samples, const DTIVector &dir, const double theta_min, const double theta_max);

  static double factorial (int n); 
  static double nchoosek (int n, int k);

  static double sqr (double x) { return x*x; }
  static float  sqr (float x ) { return x*x; }

  static void getPerpVectors(const DTIVector &vec, DTIVector &perp1, DTIVector &perp2);
  
  static DTIVector perturbVector (const DTIVector &orig, double theta, double phi);
  static DTIVector rotateVectorAboutAxis (const DTIVector &orig, const DTIVector &axis, double theta);
  static DTIVector cross (const DTIVector &a, const DTIVector &b);
  static DTIVector tangent2axis(const DTIVector &a);

  static TNT::Array2D<double> make_matrix (double x1, double x2, double x3,
					   double y1, double y2, double y3,
					   double z1, double z2, double z3);

  static TNT::Array2D<double> expm(const TNT::Array2D<double> A);
  static TNT::Array2D<double> eye(int size);
  // Inverse for 2x2 well-conditioned matrices
  static TNT::Array2D<double> inv2(const TNT::Array2D<double> A);


  static double getStdParker (double fa);

  static DTIVector getMajorEigenvector (const DTITensor &tensor);
  static double COS(const double theta);

  static double logUPSpherePDF(double xDotMean, double k);
  static DTIVector randUPSphereVector(const DTIVector &mean, double k);

  static double logFB5PDF(const DTIVector &x, const DTIVector &e1, const DTIVector &e2, double k1, double k2, double log_A);
  static double logFB5PDF(const DTIVector &x, const DTIVector &e1, const DTIVector &e2, double k1, double k2);
  static double logWatsonPDF( double xDotMean, double k);
  static DTIVector randWatsonVector(const DTIVector &mean, double k);
  static DTIVector randFB5Vector(const DTIVector &e1, const DTIVector &e2, const DTIVector &e3, double k1, double k2, double log_A);
  static DTIVector randFB5Vector(const DTIVector &e1, const DTIVector &e2, const DTIVector &e3, double k1, double k2);
  static double normCWatson(double k);
  static double hyper1F1(double a, double b, double x, double eps);
  static double gammln(double xx);

  static double normFB5(double k1, double k2);
  static double normFB5(double k1, double k2, double der[], double hes[], int mode, int level[]);
  static double bstrs0(double y);
  static double besrat(double v, double x, double eps);

  static double lognormPDF(double x, double m, double s, double max_value);
  static double normPDF(double x, double m, double s, double max_value);

  static float ****f4matrix(int th, int zh,  int yh, int xh);
  static float **f2matrix(int yh, int xh);

  static double computeLinePlaneIntersection (const double p0[3], const double p1[3], const double n0[3], const double v0[3]);
  static bool linesIntersect (double pt1[3], double pt2[3], double pt3[3], double pt4[3]);
};



#endif
