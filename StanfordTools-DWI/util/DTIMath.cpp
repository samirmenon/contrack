/***********************************************************************
 * AUTHOR: David Akers <dakers>
 *   FILE: .//DTIMath.cpp
 *   DATE: Fri Sep 23 13:06:57 2005
 *  DESCR: 
 ***********************************************************************/
#include "DTIMath.h"
#include <math.h>
#include "Box-Muller.h"
#include "typedefs.h"
#include <tnt/tnt_array1d_utils.h>
#include <tnt/tnt_array2d_utils.h>
#include <tnt/tnt_vec.h>
#include <jama/jama_eig.h>
#include "dcdf.h" 		// For the normal PDF stuff
#ifdef _WIN32
#include <time.h>
#endif

static double *FAC_LOOKUP = NULL;
static unsigned int count_randzeroone = 0;

double factorial_compute (int n) 
{
  int ans = 1;
  for (int i = 2; i <= n; i++) {
    ans *= i;
  }
  return ans;
}



/***********************************************************************
 *  Method: DTIMath::randn
 *  Params: 
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::randn()
{
  return randn_generator::get(0,1);
}


/***********************************************************************
 *  Method: DTIMath::randn
 *  Params: double mean, double std
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::randn(double mean, double std)
{
  return randn_generator::get(mean,std);
}

/***********************************************************************
 *  Method: DTIMath::randRice
 *  Params: double mean, double std
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::randRice(double mean, double std)
{
  double x = randn(mean,std);
  double y = randn(0,std);
  double r = sqrt(x*x + y*y);
  return r;
}

/***********************************************************************
 *  Method: DTIMath::randzeroone
 *  Params: 
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::randzeroone()
{
  // Uniform distribution for range [0,1], i.e. including 0 and 1
  int randomnum = rand();
  //int randomnum = 2147480467;
  return (double) randomnum/RAND_MAX;
}

/***********************************************************************
 *  Method: DTIMath::factorial
 *  Params: int n
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::factorial(int n)
{
  if (!FAC_LOOKUP) {
    FAC_LOOKUP = new double[100];
    for (int i = 0; i < 100; i++) {
      FAC_LOOKUP[i] = (double) factorial_compute(i);
    }
  }
  // XXX No assert in -mno-cygwin
  //assert (n < 100 && n >= 0);
  return (double) FAC_LOOKUP[n];
}


/***********************************************************************
 *  Method: DTIMath::nchoosek
 *  Params: int n, int k
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::nchoosek(int n, int k)
{
  return factorial (n) / (factorial (n-k) * factorial (k));
}


/***********************************************************************
 *  Method: DTIMath::randn3
 *  Params: double r[3]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIMath::randn3(double r[3])
{
  // double r1 = DTIMath::randn();
//   double theta = r1*M_PI/5;
//   double r2 = DTIMath::randzeroone();
//   double phi = r2*2*M_PI;
//   double dist = DTIMath::randn();
//   dist *= dist;

//   r[0] = dist*cos(theta)*sin(phi);
//   r[1] = dist*sin(theta)*sin(phi);
//   r[2] = dist*cos(phi);

// For now don't worry about different cov matrices
  r[0] = randn();
  r[1] = randn();
  r[2] = randn();
}


/***********************************************************************
 *  Method: DTIMath::pdf_randn3
 *  Params: double r[3]
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::pdf_randn3(double r[3])
{
//   DTIVector vec = DTIVector(3,r);
//   double dist = abs(vec);
//   double phi = acos(r[2]/dist);
//   double theta = acos(r[0]/(dist*sin(phi)));
//   double p_phi = 1./(2*M_PI);
//   double p_theta = 1./sqrt(2*M_PI)*exp(-(theta*5/M_PI)*(theta*5/M_PI)/2);
//   double p_dist = 2*(1./sqrt(2*M_PI)*exp(-dist/2));
//   return p_phi*p_theta*p_dist;

  return (1./(2*M_PI)*1./sqrt(2*M_PI))*exp(-(r[0]*r[0]+r[1]*r[1]+r[2]*r[2])/2);
}

void 
DTIMath::getPerpVectors(const DTIVector &vec, DTIVector &perp1, DTIVector &perp2)
{
  int i;
  for (i = 0; i < 3; i++)
    if (vec[i] != 0)
      break;

  // This is a zero vector
  if(i==3)
    return;

  int second = i;
  int first = (second + 1) % 3;
  perp1[0] = perp1[1] = perp1[2] = 0;
  perp1[first] = sqrt(1/(1+sqr(vec[first])/sqr(vec[second])));
  perp1[second] = -vec[first]*perp1[first]/vec[second];
 
  // Get cross-product of these
  perp2 = cross(vec,perp1);
}

/***********************************************************************
 *  Method: DTIMath::perturbVector
 *  Params: const DTIVector &orig, double theta, double phi
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::perturbVector(const DTIVector &orig, double theta, double phi) 
{
  // Generate a vector that is rotated by the amount theta (radians)
  // in a direction orthogonal to the original vector.  Then rotate by
  // an amount phi (radians) about the original vector as an axis of
  // rotation.  (This function is used mostly to provide a random
  // offset of direction vectors about their original orientations.)

  if(theta == 0)
    return orig.copy();

  DTIVector perp1;
  DTIVector perp2;
  getPerpVectors(orig, perp1, perp2);

  //This rotation matrix is about the perpendicular vector
  //Rperp = [perp; orig; perp2];
  
  // This rotation matrix is about the original vector
  //  RorigInverse = [orig; perp1; perp2];
  TNT::Array2D<double> RorigTranspose = make_matrix (orig[0], perp1[0], perp2[0],
						     orig[1], perp1[1], perp2[1],
						     orig[2], perp1[2], perp2[2]);

  // Give biased rotation about z axis  
  TNT::Array2D<double> Rz = make_matrix ( cos(theta), sin(theta), 0,
					 -sin(theta), cos(theta), 0,
					  0,          0,          1);
  // Uniform rotation about x axis 
  TNT::Array2D<double> Rx = make_matrix (1, 0,        0, 
					 0, cos(phi), sin(phi),
					 0, -sin(phi), cos(phi));

  TNT::Array2D<double> inputVec(3,1);
  inputVec[0][0] = 1;
  inputVec[1][0] = 0;
  inputVec[2][0] = 0;
  DTIVector trial;
  TNT::Array2D<double> trialVec(3,1);
  trialVec = TNT::matmult(TNT::matmult (TNT::matmult(RorigTranspose, Rx), Rz), inputVec);

  trial[0] = trialVec[0][0];
  trial[1] = trialVec[1][0];
  trial[2] = trialVec[2][0];

  return trial.copy();

}


/***********************************************************************
 *  Method: DTIMath::cross
 *  Params: const DTIVector &a, const DTIVector &b
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::cross(const DTIVector &a, const DTIVector &b)
{
  DTIVector result;
  result[0] = a[1]*b[2]-a[2]*b[1];
  result[1] = a[2]*b[0]-a[0]*b[2];
  result[2] = a[0]*b[1]-a[1]*b[0];
  return result.copy();
}

/***********************************************************************
 *  Method: DTIMath::tangent2axis
 *  Params: const DTIVector &a
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::tangent2axis(const DTIVector &a)
{
  DTIVector result;
  DTIVector x(1,0,0);
  DTIVector y(0,1,0);
  DTIVector z(0,0,1);
  result[0] = fabs(dproduct(x,a));
  result[1] = fabs(dproduct(y,a));
  result[2] = fabs(dproduct(z,a));
  return result.copy();
}


/***********************************************************************
 *  Method: DTIMath::make_matrix
 *  Params: double x1, double x2, double x3, double y1, double y2, double y3, double z1, double z2, double z3
 * Returns: TNT::Array2D<double>
 * Effects: 
 ***********************************************************************/
TNT::Array2D<double>
DTIMath::make_matrix(double x1, double x2, double x3, double y1, double y2, double y3, double z1, double z2, double z3)
{
  TNT::Array2D<double> retval (3,3);
  retval[0][0] = x1;
  retval[0][1] = x2;
  retval[0][2] = x3;
  retval[1][0] = y1;
  retval[1][1] = y2;
  retval[1][2] = y3;
  retval[2][0] = z1;
  retval[2][1] = z2;
  retval[2][2] = z3;
  return retval;
}


/***********************************************************************
 *  Method: DTIMath::getStdParker
 *  Params: double fa
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::getStdParker(double fa)
{
  // used Matlab's nlintool for this to find best fit for a,b,c

  //  1.0978   -1.9567   -0.1437
  //  XXX hack must fix tensors
  if( fa > 0.88)
    fa = 0.88;
  if( fa < 0 )
    fa = 0;

  double a = 1.0978;
  double b = -1.9567;
  double c = -0.1437;
  // XXX playing around with std values
  return (a*exp(b*fa)+c);
  //return 0.17;
}


/*************************************************************************
 * Function Name: DTIMath::getMajorEigenvector
 * Parameters: const DTITensor &tensor
 * Returns: DTIVector
 * Effects: 
 *************************************************************************/
DTIVector
DTIMath::getMajorEigenvector(const DTITensor &tensor)
{
  // use jama to compute eigenvectors, eigenvalues of tensor.
  // pick the largest magnitude eigenvalue and return corresponding 
  // eigenvector.

   TNT::Array1D<float> eigenvalues;
   JAMA::Eigenvalue<float> eig(tensor);
   eig.getRealEigenvalues (eigenvalues);
   int index = 0;
   double largest = fabs (eigenvalues[0]);
   for (int i = 1; i < 3; i++) {
     if (fabs(eigenvalues[i]) > largest) {
       largest = fabs(eigenvalues[i]);
       index = i;
     }
   } 
  
   TNT::Array2D<float> eigenvectors;
   eig.getV (eigenvectors);
   DTIVector result;
   result[0] = eigenvectors[0][index];
   result[1] = eigenvectors[1][index];
   result[2] = eigenvectors[2][index];

  return result;
}
/***********************************************************************
 *  Method: DTIMath::rotateVectorAboutAxis
 *  Params: const DTIVector &orig, const DTIVector &axis, double theta
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::rotateVectorAboutAxis(const DTIVector &orig, const DTIVector &axis, double theta)
{
  // Rotate vector about arbitrary unit vector axis

  double x = axis[0];
  double y = axis[1];
  double z = axis[2];
  // Create matrix for rotation code from
  // http://www.euclideanspace.com/maths/algebra/matrix/orthogonal/rotation/
  TNT::Array2D<double> R = make_matrix  (1 + (1-cos(theta))*(x*x-1),       -z*sin(theta)+(1-cos(theta))*x*y, y*sin(theta)+(1-cos(theta))*x*z, 
					 z*sin(theta)+(1-cos(theta))*x*y,  1 + (1-cos(theta))*(y*y-1),      -x*sin(theta)+(1-cos(theta))*y*z,
					 -y*sin(theta)+(1-cos(theta))*x*z, x*sin(theta)+(1-cos(theta))*y*z,  1 + (1-cos(theta))*(z*z-1)      );

  TNT::Array2D<double> inputVec(3,1);
  inputVec[0][0] = orig[0];
  inputVec[1][0] = orig[1];
  inputVec[2][0] = orig[2];
  
  TNT::Array2D<double> trialVec(3,1);
  trialVec = TNT::matmult(R, inputVec);

  DTIVector out;
  out[0] = trialVec[0][0];
  out[1] = trialVec[1][0];
  out[2] = trialVec[2][0];
  
  return out.copy();
}


/***********************************************************************
 *  Method: DTIMath::rand_point_sphere
 *  Params: 
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::rand_point_sphere()
{

  // First generate latitude
  double z = 1;
  while(z>=1 || z<=-1)
    z = randzeroone()*2 - 1;
  double theta = asin(z);

  // Now generate longitude
  double phi = 2*M_PI;
  while(phi>=2*M_PI)
    phi = randzeroone()*2*M_PI;

  // Now create euclidean values from latitude and longitude
  DTIVector vec;
  vec[0] = cos(theta)*cos(phi);
  vec[1] = cos(theta)*sin(phi);
  vec[2] = z;

  return vec.copy();
}


/***********************************************************************
 *  Method: DTIMath::rand_point_sphere_strat
 *  Params: std::vector<DTIVector*> vec_list, unsigned int num_samples
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
void
DTIMath::rand_point_sphere_strat(std::vector<DTIVector> &vec_list, unsigned int num_samples)
{
  unsigned int axis_res = (unsigned int)floor(sqrt((double)num_samples));
  float res_fac = 1./axis_res;
  double z_res_fac = res_fac*2;
  double phi_res_fac = res_fac*2*M_PI;
  double z;
  double theta;
  double phi;
  unsigned int samples = 0;

  for( int zi=0; zi < axis_res; zi++) {
    z = randzeroone()*z_res_fac - 1 + zi*z_res_fac;
    theta = asin(z);
    for( int pi=0; pi < axis_res; pi++) {
      phi = randzeroone()*phi_res_fac + pi*phi_res_fac;
      DTIVector vec;
      vec[0] = cos(theta)*cos(phi);
      vec[1] = cos(theta)*sin(phi);
      vec[2] = z;
      vec_list.push_back(vec);
      samples++;
    }
  }
  while(samples < num_samples) {
    z = randzeroone()*2 - 1;
    theta = asin(z);
    phi = randzeroone()*2*M_PI;
    DTIVector vec;
    vec[0] = cos(theta)*cos(phi);
    vec[1] = cos(theta)*sin(phi);
    vec[2] = z;
    vec_list.push_back(vec);
    samples++;
  }
}


/***********************************************************************
 *  Method: DTIMath::rand_point_cone
 *  Params: const DTIVector &dir, const double theta_min, const double theta_max
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::rand_point_cone(const DTIVector &dir, const double theta_min, const double theta_max)
{

  DTIVector vec;
  if(theta_min < 0.00001 && theta_max > M_PI - 0.00001)
    vec = rand_point_sphere();
  else if(theta_max < 0.00001)
    vec = dir.copy();
  else {
    double cos_theta = -2;
    double cos_theta_max = cos(theta_max);
    //while ( cos_theta <= cos_theta_max)
    //  cos_theta = DTIMath::randzeroone()*2 - 1;
    double cos_theta_min = cos(theta_min);
    double t = 1;
    while ( t >= 1 || t <= 0)
      t = DTIMath::randzeroone();
    cos_theta = t*cos_theta_max + (1-t)*cos_theta_min;
    double theta = acos(cos_theta);
    double phi = DTIMath::randzeroone()*2*M_PI;
    // Create new vector
    vec = (perturbVector(dir,theta,phi)).copy();
  }

    return vec.copy();

// Randomly select theta and phi
//     double t = 1;
//     while ( t >= 1 || t <= 0)
//       t = DTIMath::randzeroone();
//     double theta = t*theta_max + (1-t)*theta_min;
}


/***********************************************************************
 *  Method: DTIMath::rand_point_cone_strat
 *  Params: std::vector<DTIVector> &vec_list, unsigned int num_samples, const DTIVector &dir, const double theta_min, const double theta_max
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIMath::rand_point_cone_strat(std::vector<DTIVector> &vec_list, unsigned int num_samples, const DTIVector &dir, const double theta_min, const double theta_max)
{
  if(theta_min < 0.00001 && theta_max > M_PI - 0.00001)
    rand_point_sphere_strat(vec_list,num_samples);
  else if(theta_max < 0.00001)
    for(int i=0; i<num_samples; i++) vec_list.push_back(dir.copy());
  else {

    unsigned int axis_res = (unsigned int)floor(sqrt((double)num_samples));
    float res_fac = 1./axis_res;
    double phi_res_fac = res_fac*2*M_PI;
    double theta;
    double phi;
    unsigned int samples = 0;

    double cos_theta = -2;
    double cos_theta_max = cos(theta_max);
    double cos_theta_min = cos(theta_min);
    double t;
    while ( t >= 1 || t <= 0)
      t = DTIMath::randzeroone();
    for( int ti=0; ti < axis_res; ti++) {
      t = 1;
      while ( t >= 1 || t <= 0)
	t = randzeroone()*res_fac + ti*res_fac;
      cos_theta = t*cos_theta_max + (1-t)*cos_theta_min;
      theta = acos(cos_theta);
      for( int pi=0; pi < axis_res; pi++) {
	phi = randzeroone()*phi_res_fac + pi*phi_res_fac;
	DTIVector vec;
	vec_list.push_back( (perturbVector(dir,theta,phi)).copy() );
	samples++;
      }
    }
    while(samples < num_samples) {
      t=1;
      while ( t >= 1 || t <= 0)
	t = DTIMath::randzeroone();
      cos_theta = t*cos_theta_max + (1-t)*cos_theta_min;
      theta = acos(cos_theta);
      phi = DTIMath::randzeroone()*2*M_PI;
      vec_list.push_back( (perturbVector(dir,theta,phi)).copy() );
      samples++;
    }
  }
}

/***********************************************************************
 *  Method: DTIMath::COS
 *  Params: const double theta
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::COS(const double theta)
{
  // This only does the hemisphere cos
  double cos_theta;

  if( theta < 0.00001 )
    cos_theta = 1;
  else if( theta > M_PI - 0.00001 )
    cos_theta = -1;
  else
    cos_theta = cos(theta);

  return cos_theta;
}

/***********************************************************************
 *  Method: DTIMath::logFB5PDF
 *  Params: const DTIVector &x, const DTIVector &e1, const DTIVector &e2, double k1, double k2
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::logFB5PDF(const DTIVector &x, const DTIVector &e1, const DTIVector &e2, double k1, double k2)
{
  // Find normalization
  double log_A = log( DTIMath::normFB5(k1, k2) );

  return logFB5PDF(x, e1, e2, k1, k2, log_A);
}

/***********************************************************************
 *  Method: DTIMath::logFB5PDF
 *  Params: const DTIVector &x, const DTIVector &e1, const DTIVector &e2, double k1, double k2, double log_A
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::logFB5PDF(const DTIVector &x, const DTIVector &e1, const DTIVector &e2, double k1, double k2, double log_A)
{
  // Fisher-Bingham or Kent distribution on sphere
  // x is a unit vector on sphere
  // e1 is principal unit length axis
  // e2 is secondary unit length axis
  // k1 and k2 concentration parameters where k1<0 and k2<=k1/2
  // log_A is the log of the total area normalization constant 
  //   will make this function integrate to 4*PI over the sphere
  if( k1 > 0 || k2 > 0) {
    std::cerr << "FB5 distribution calculation error k1= " << k1 << " k2= " << k2 << std::endl;
    return 0;
  }

  double xDotE1 = dproduct(x,e1);
  double xDotE2 = dproduct(x,e2);
  
  return k1 * xDotE1 * xDotE1 + k2 * xDotE2 * xDotE2 - log_A;
}


/***********************************************************************
 *  Method: DTIMath::randFB5Vector
 *  Params: const DTIVector &e1, const DTIVector &e2, const DTIVector &e3, double k1, double k2
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::randFB5Vector(const DTIVector &e1, const DTIVector &e2, const DTIVector &e3, double k1, double k2)
{
  // Find normalization
  //cerr << "k1: " << k1 << " k2: " << k2 << endl;
  double log_A = log( DTIMath::normFB5(k1, k2) );

  return randFB5Vector(e1, e2, e3, k1, k2, log_A);
}

/***********************************************************************
 *  Method: DTIMath::randFB5Vector
 *  Params: const DTIVector &e1, const DTIVector &e2, const DTIVector &e3, double k1, double k2, double log_A
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::randFB5Vector(const DTIVector &e1, const DTIVector &e2, const DTIVector &e3, double k1, double k2, double log_A)
{
  // From Camino originally written by Dan Alexander
  int rejectionCounter = 0;

  if (k1 == 0.0) {
    // uniform distribution
    double phi = M_PI * 2.0 * DTIMath::randzeroone();
    double theta = acos(2.0 * DTIMath::randzeroone() - 1.0);
    DTIVector trial;
    DTIMath::convertSphToEucCamino(trial, theta, phi, 1.0);
    trial = norm( trial.copy() );
    return trial.copy();
  } 

  double kW = 0;
  double logtempNormC;
  if ( abs(k2) > 0.02 * abs(k1) + 5) {
    kW = -1.0 * k2;
    logtempNormC = -1.0 * k2 + log_A;
  }
  else {
    kW = k1;
    logtempNormC = log_A;
  }
  double watsonNormC = exp(log(DTIMath::hyper1F1(0.5, 1.5, kW, 1e-9)) - logtempNormC);

  while (true) {    

    DTIVector trial = DTIMath::randWatsonVector(e3, kW);
    double x = DTIMath::randzeroone();
    // C++ handles log( inf ) differently than java numerics
    // here we will special handle the case
    if( watsonNormC == DTIMath::double_inf() )
      return trial.copy();
    
    // Must be a numerical accuracy problem when we are using the logs for small k
    if (kW < 700.0) {      
      double watsonPDF = exp(DTIMath::logWatsonPDF(dproduct(e3,trial), kW) + log(watsonNormC));
      double fb5PDF = exp(DTIMath::logFB5PDF( trial, e1, e2, k1, k2, log_A));  
      if ( x <=  fb5PDF / watsonPDF ) {
	return trial.copy();
      }
      else {
	rejectionCounter++;
	if (rejectionCounter % 10000 == 0) {
	  std::cerr << "fb5PDF: " << fb5PDF << "watsonPDF: " << watsonPDF << std::endl;    
 	  std::cerr << "k1: " << k1 << " k2: " << k2 << " log_A: " << log_A << std::endl;
 	  std::cerr << "e1: " << e1[0] << " , " << e1[1] << " , " << e1[2] << std::endl;
 	  std::cerr << "e2: " << e2[0] << " , " << e2[1] << " , " << e2[2] << std::endl;
	  std::cerr << "Still rejecting samples after " << rejectionCounter << " attempts" << std::endl;
	}
      }
    }
    else {
      double logWatsonPDF = DTIMath::logWatsonPDF(dproduct(e3,trial), kW) + log(watsonNormC);   
      double logFB5PDF = DTIMath::logFB5PDF( trial, e1, e2, k1, k2, log_A);
      if ( log(x) <=  logFB5PDF - logWatsonPDF ) {
	return trial.copy();
      }   
      else {
	rejectionCounter++;
	if (rejectionCounter % 10000 == 0) {
	  std::cerr << "Still rejecting samples after " << rejectionCounter << " attempts" << std::endl;
	}
      }
    }    
  }
}
/***********************************************************************
 *  Method: DTIMath::logWatsonPDF
 *  Params: double xDotMean, double k
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::logUPSpherePDF(double xDotMean, double k)
{
  // Made up uni-polar distribution.  
  // Created by taking a Watson distribution on the hemisphere 
  // and extending the pdf value at 90 degrees to the hemisphere 
  // opposite the mean vector
  // The exp of this function will integrate to 4*PI over the sphere
  
  // Get value at edge
  //double dEdge = logWatsonPDF(0,k);
  if(xDotMean>0) {
    return logWatsonPDF(xDotMean,k);
    // XXX BELOW SEEMS ODD
    // Adjust other hemisphere values based on edge contribution
    //double dAdd = 1 - exp(dEdge);
    //return log(dAdd+exp(logWatsonPDF(xDotMean,k)));
  } else {
    // Just extend the Watson edge value to this location
    double dEdge = logWatsonPDF(0,k);
    return dEdge;
  }
}


/***********************************************************************
 *  Method: DTIMath::randWatsonVector
 *  Params: const DTIVector &mean, double k
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::randUPSphereVector(const DTIVector &mean, double k)
{
  double dProbNeg = log(0.5) + logWatsonPDF(0,k);//2*M_PI/(4*M_PI)*exp(logWatsonPDF(0,k));
  DTIVector v;
  if(log(randzeroone())<dProbNeg) {
    // Draw from the hemisphere not containing the mean
    v = rand_point_sphere();
    if(dproduct(v,mean)>0)
      v = -v;
  }else {
    // Draw from the Watson distribution on the side of the mean
    v = randWatsonVector(mean,k);
    if(dproduct(v,mean)<0)
      v=-v;
  }
  return v.copy();
}

/***********************************************************************
 *  Method: DTIMath::logWatsonPDF
 *  Params: double xDotMean, double k
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::logWatsonPDF(double xDotMean, double k)
{
  // Watson distribution
  // The exp of this function will integrate to 4*PI over the sphere
  return k * xDotMean * xDotMean - log(DTIMath::hyper1F1(0.5, 1.5, k, 1.0e-9));
}


/***********************************************************************
 *  Method: DTIMath::randWatsonVector
 *  Params: const DTIVector &mean, double k
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
DTIMath::randWatsonVector(const DTIVector &mean, double k)
{
  // From Armin Schwartzmann code -- doesn't handle large k very well
  DTIVector trial;
  
//   double c = k/(exp(k)-1);
//   double z;
//   while (true) {
//     double u = DTIMath::randzeroone();
//     double v = DTIMath::randzeroone();
//     z = log(u*k/c + 1)/k;
//     if ( v < exp(k*(z*z-z)) )
//       break;
//   }
//   // z = cos(theta) and r = sin(theta)
//   double r = sqrt(1-z*z);
//   double phi = 2*M_PI*DTIMath::randzeroone();
//   trial[0] = r*cos(phi);
//   trial[1] = r*sin(phi);
//   trial[2] = z;
//   // Rotation to mean using Rodrigues' formula
//   r = sqrt(mean[0]*mean[0] + mean[1]*mean[1]);
//   if (r > 0) {
//     TNT::Array2D<double> W = make_matrix (0, 0, mean[0]/r,
// 					  0, 0, mean[1]/r,
// 					  -mean[0]/r, -mean[1]/r, 0);
//     TNT::Array2D<double> E = make_matrix (1, 0, 0,
// 					  0, 1, 0,
// 					  0, 0, 1);
//     W = E.copy() + sqrt(1-mean[2]*mean[2])*W.copy() + (1-mean[2])*TNT::matmult(W,W).copy();
//     TNT::Array2D<double> inputVec(3,1);
//     inputVec[0][0] = trial[0];
//     inputVec[1][0] = trial[1];
//     inputVec[2][0] = trial[2];
    
//     TNT::Array2D<double> trialVec(3,1);
//     trialVec = TNT::matmult(W, inputVec);
    
//     trial[0] = trialVec[0][0];
//     trial[1] = trialVec[1][0];
//     trial[2] = trialVec[2][0];

//   }


  // Taken from Camino software originally written by Dan Alexander
  // Convert cartesian vector to spherical coordinates
  double meanTheta;
  double meanPhi;
  double r;
  DTIMath::convertEucToSphCamino(mean, meanTheta, meanPhi, r);
  
   double s;
   if (k < 0.0) {
     // girdle distribution
     // 0
     // (-1 * k) is +ve
     double c1 = sqrt(-1.0 * k);
     double c2 = atan(c1);
     s = 0.0;
     while (true) {      
       double u = DTIMath::randzeroone();
       double v = DTIMath::randzeroone();	    
       s = (1.0 / c1) * tan(c2 * u);
       if ( v <= (1.0 - k * s * s) * exp(k * s * s) ) {
 	break;
       }
     }    
   }
   else if (k == 0.0) {
     // uniform distribution 
     double phi = M_PI * 2.0 * DTIMath::randzeroone();
     double theta = acos(2.0 * DTIMath::randzeroone() - 1.0);
     DTIMath::convertSphToEucCamino(trial, theta, phi, 1.0);
     trial = norm( trial.copy() );
     return trial.copy();
   }
   else if (k > 700.0) {
     // was	double c = 1.0 / (Math.exp(k) - 1.0); 	
     // (\exp{\kappa} - 1.0} ---> \exp{\kappa} for large k
     double lnc = -1.0 * k;
     s = 0.0;
     while (true) {
       // 1
       double u = DTIMath::randzeroone(); 
       double v = DTIMath::randzeroone(); 	    
       // 2
       // was    s = (1.0 / k) * Math.log(1.0 + u / c);      
       // log (1 + u / c) -----> log(u / c) for (u / c) very large
       s = (1.0 / k) * (log(u) - lnc);	    
       // 3
       if ( v <= exp(k * s * (s - 1.0)) ) {
 	break;
       }      
     }
   }
   else {
     // bipolar distribution    
     // 0
     double c = 1.0 / (exp(k) - 1.0); 	    
     s = 0.0;	    
     while (true) {		
       // 1
       double u = DTIMath::randzeroone(); 
       double v = DTIMath::randzeroone(); 
       // 2
       s = (1.0 / k) * log(1.0 + u / c);		
       // 3
       if ( v <= exp(k * s * (s - 1.0)) ) {
 	break;
       }	    
     }
   }
	    
   double theta = meanTheta + acos(s);	    
   double phi = meanPhi;
  
   // theta wraps at PI
   if (theta < 0.0 || theta > M_PI) {
     phi = M_PI + phi;
   }
   DTIMath::convertSphToEucCamino(trial, theta, phi, 1.0);
   trial = norm( trial.copy() );
   if (DTIMath::randzeroone() > 0.5) {
     trial = -trial;
   }
   
   // Now rotate sample about the mean axis by random angle gamma, between 0 and 2 * PI
   // this part makes the distribution cylindrically symmetric
   double gamma = M_PI * 2.0 * DTIMath::randzeroone();
   double cg = cos(gamma);
   double sg = sin(gamma);
  
   double sampX = 
     (mean[0]*mean[0]*(1.0 - cg) + cg) * trial[0] + 
     (mean[0]*mean[1]*(1.0 - cg) - mean[2]*sg) * trial[1] + 
     (mean[0]*mean[2]*(1.0 - cg) + mean[1]*sg) * trial[2];
  
   double sampY = 
     (mean[1]*mean[0]*(1.0 - cg) + mean[2]*sg) * trial[0] + 
     (mean[1]*mean[1]*(1.0 - cg) + cg) * trial[1] + 
     (mean[1]*mean[2]*(1.0 - cg) - mean[0]*sg) * trial[2];  
  
   double sampZ = 
     (mean[2]*mean[0]*(1.0 - cg) - mean[1]*sg) * trial[0] + 
     (mean[2]*mean[1]*(1.0 - cg) + mean[0]*sg) * trial[1] + 
     (mean[2]*mean[2]*(1.0 - cg) + cg) * trial[2];

   trial[0] = sampX;
   trial[1] = sampY;
   trial[2] = sampZ;

  trial = norm( trial.copy() );
  return trial.copy();
}

/***********************************************************************
 *  Method: DTIMath::lognormPDF
 *  Params: double x, double m, double s
 * Returns: double
 * Effects: 
 ***********************************************************************/
// XXX This function is now specialized to only be a windowed gaussian
// that specifically is taking absolute values between 0 and max_value,
// therefore we still must consider the negative domain of the pdf
// when returning the pdf value
double
DTIMath::lognormPDF(double x, double m, double s, double max_value)
{
  double  a = x-m;
  double pcdf_m;
  double qcdf;
  int status_cdf;
  double bound;
  int which = 1;
  
  // Get maximum value for cdf
  cdfnor( &which, &pcdf_m, &qcdf, &max_value, &m, &s, &status_cdf, &bound);
  double value = log(INVSQRT2PI) -log(s) - a*a/(2*s*s) - log((pcdf_m-0.5));// + log(quanta);

  return value;
}


 /***********************************************************************
  *  Method: DTIMath::normPDF
  *  Params: double x, double m, double s
  * Returns: double
  * Effects: 
  ***********************************************************************/
 // XXX This function is now specialized to only be a windowed gaussian
 // that specifically is taking absolute values between 0 and max_value,
 // therefore we still must consider the negative domain of the pdf
 // when returning the pdf value
 double
 DTIMath::normPDF(double x, double m, double s, double max_value)
 {
   double  a = x-m;
   double pcdf;
   double qcdf;
   int status_cdf;
   double bound;
   int which = 1;

   cdfnor( &which, &pcdf, &qcdf, &max_value, &m, &s, &status_cdf, &bound);

   return INVSQRT2PI * 1/s * exp(-a*a/(2*s*s)) * 1/((pcdf-0.5));
 }



/***********************************************************************
 *  Method: DTIMath::f4matrix
 *  Params: int th, int zh, int yh, int xh
 * Returns: float ****
 * Effects: 
 ***********************************************************************/
float ****
DTIMath::f4matrix(int th, int zh, int yh, int xh)
{

  // allocate a float 4matrix with range t[0..th][0..zh][0..yh][0..xh] 
  // adaptation of Numerical Recipes in C nrutil.c allocation routines 
  
  int j;
  int nvol = th+1;
  int nslice = zh+1;
  int nrow = yh+1;
  int ncol = xh+1;
  float ****t;
  
  
  /** allocate pointers to vols */
  //t=(float ****) malloc((size_t)((nvol)*sizeof(float***)));
  t = new float***[nvol];
  if (!t) std::cerr<<"f4matrix: allocation failure"<<std::endl;
  
  /** allocate pointers to slices */
  //t[0]=(float ***) malloc((size_t)((nvol*nslice)*sizeof(float**)));
  t[0]= new float**[nvol*nslice];
  if (!t[0]) std::cerr<<"f4matrix: allocation failure"<<std::endl;
  
  /** allocate pointers for ydim */
  //t[0][0]=(float **) malloc((size_t)((nvol*nslice*nrow)*sizeof(float*)));
  t[0][0]= new float*[nvol*nslice*nrow];
  if (!t[0][0]) std::cerr<<"f4matrix: allocation failure"<<std::endl;
  
  
  /** allocate the data blob */
  //t[0][0][0]=(float *) malloc((size_t)((nvol*nslice*nrow*ncol)*sizeof(float)));
  t[0][0][0]= new float[nvol*nslice*nrow*ncol];
  if (!t[0][0][0]) std::cerr<<"f4matrix: allocation failure"<<std::endl;
  
  
  /** point everything to the data blob */
  for(j=1;j<nrow*nslice*nvol;j++) t[0][0][j]=t[0][0][j-1]+ncol;
  for(j=1;j<nslice*nvol;j++) t[0][j]=t[0][j-1]+nrow;
  for(j=1;j<nvol;j++) t[j]=t[j-1]+nslice;
  
  return t;
}


/***********************************************************************
 *  Method: DTIMath::f2matrix
 *  Params: int yh, int xh
 * Returns: float **
 * Effects: 
 ***********************************************************************/
float **
DTIMath::f2matrix(int yh, int xh)
{
  // allocate a float 4matrix with range t[0..th][0..zh][0..yh][0..xh] 
  // adaptation of Numerical Recipes in C nrutil.c allocation routines 
  
  int j;
  int nrow = yh+1;
  int ncol = xh+1;
  float **t;
  
  /** allocate pointers for ydim */
  //t[0][0]=(float **) malloc((size_t)((nvol*nslice*nrow)*sizeof(float*)));
  t= new float*[nrow];
  if (!t) std::cerr<<"f4matrix: allocation failure"<<std::endl;
  
  
  /** allocate the data blob */
  //t[0][0][0]=(float *) malloc((size_t)((nvol*nslice*nrow*ncol)*sizeof(float)));
  t[0]= new float[nrow*ncol];
  if (!t[0]) std::cerr<<"f4matrix: allocation failure"<<std::endl;
  
  /** point everything to the data blob */
  for(j=1;j<nrow;j++) t[j]=t[j-1]+ncol;
  
  return t;
}



/***********************************************************************
 *  Method: DTIMath::convertEucToSph
 *  Params: const DTIVector &vec, double &theta, double &phi, double &r
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIMath::convertEucToSph(const DTIVector &vec, double &theta, double &phi, double &r)
{
  // These spherical coordinates are specified so that if a vector
  // goes from the origin to the point in euclidean space then theta
  // is the angle (radians) between this vector and XY plane.  Phi is
  // then the angle (radians) that the projection of the vector on the
  // XY plane makes with the X axis and r is the distance away from
  // the origin along the vector.

  r = abs(vec);
  DTIVector z(0.0f); z[2]=1;
  DTIVector y(0.0f); y[1]=1;
  if(fabs(dproduct(vec,z)-r*r)<1e-6) {
    theta = M_PI/2.0;
    phi = 0;
  } else if(fabs(dproduct(vec,y)-r*r)<1e-6) {
    theta = 0;
    phi = M_PI/2;
  } else {
    theta = asin( vec[2] / r );
    phi = acos( vec[0] / (r*cos(theta)) );
  }
}

/***********************************************************************
 *  Method: DTIMath::convertEucToSphCamino
 *  Params: const DTIVector &vec, double &theta, double &phi, double &r
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIMath::convertEucToSphCamino(const DTIVector &vec, double &theta, double &phi, double &r)
{
  r = abs(vec);
  theta= acos(vec[2]/r);
  
  // phi goes from 0.0 (+x axis) and wraps at 2 * PI
  // theta goes from 0.0 (+z axis) and wraps at PI
  // if x and y are 0.0
  if (vec[0] == 0.0 && vec[1] == 0.0) {
    phi = 0.0; 
  }
  else {    
    // ie, if ( x == 0 && y == 0 ) == false
    if (vec[1] == 0.0) {
      if (vec[0] > 0.0) {
	phi = 0.0;
      }
      else {
	phi = M_PI;
      }
    }
    else if (vec[0] == 0.0) {      
      // avoid div by zero
      if (vec[1] > 0) {
	phi = M_PI / 2.0;
      }
      else {
	phi = 1.5 * M_PI;
      }
    }
    else if (vec[0] > 0.0 && vec[1] > 0.0) { // first quadrant
      phi = atan(vec[1] / vec[0]);
    }
    else if (vec[0] < 0.0 && vec[1] > 0.0) { // second quadrant
      phi = M_PI + atan(vec[1] / vec[0]);
    }
    else if (vec[0] < 0.0 && vec[1] < 0.0) { // third quadrant
      phi =  M_PI + atan(vec[1] / vec[0]);
    }
    else { // fourth quadrant
      phi = 2.0 * M_PI + atan(vec[1] / vec[0]); 
    }
    
  }
}



/***********************************************************************
 *  Method: DTIMath::convertSphToEuc
 *  Params: DTIVector &vec, double theta, double phi, double r
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIMath::convertSphToEuc(DTIVector &vec, double theta, double phi, double r)
{
  // There spherical coordinates are specified so that if a vector
  // goes from the origin to the point in euclidean space then theta
  // is the angle (radians) between this vector and XY plane.  Phi is
  // then the angle (radians) that the projection of the vector on the
  // XY plane makes with the X axis and r is the distance away from
  // the origin along the vector.

  vec[0] = r*cos(theta)*cos(phi);
  vec[1] = r*cos(theta)*sin(phi);
  vec[2] = r*sin(theta);
}

/***********************************************************************
 *  Method: DTIMath::convertSphToEucCamino
 *  Params: DTIVector &vec, double theta, double phi, double r
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIMath::convertSphToEucCamino(DTIVector &vec, double theta, double phi, double r)
{
  double rst = r * sin(theta);
  vec[0] = rst*cos(phi);
  vec[1] = rst*sin(phi);
  vec[2] = r*cos(theta);
}

/***********************************************************************
 *  Method: DTIMath::angle
 *  Params: DTIVector &a, DTIVector &b
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::angle(const DTIVector &a, const DTIVector &b)
{
  double cosTheta = fabs(dproduct (a, b))/(abs(a)*abs(b));
  double angle;

  if(cosTheta > 1)
    angle = 0;
  else if( cosTheta < -1)
    angle = M_PI;
  else
    angle = acos(cosTheta);

  return angle;
}


/***********************************************************************
 *  Method: DTIMath::normCWatson
 *  Params: double k
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::normCWatson(double k)
{
  // XXX Not a useful approximation for the size of k that we use
  return k / (M_PI*exp(k));
}

/***********************************************************************
 *  Method: DTIMath::hyper1F1
 *  Params: double a, double b, double x, double eps
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::hyper1F1(double a, double b, double x, double eps)
{
  // Code originally from Camino written by Dan Alexander
  double sum, ratio, aks, bks, kfact, tk, xpow, eps10, term, fk, bMaPkM1, oneMPkM1;
  double termold, termnew, aPkM1, aMbPk, mx;
  int k;
  double asympThresh = 28.0; // Hard value set by Dan Alexander
  
  if (a == b) {
    return exp(x);
  }  
  if (b <= 0.0) {
    std::cerr << "hyper1F1: b <= 0.0, 0 returned" << std::endl;
    return 0.0;
  }
  eps10 = 0.1 * eps;
  
  if (abs(x) < asympThresh) {
    
    sum = 1.0;
    ratio = 10.0;
    aks = a;
    bks = b;
    kfact = 1.0;
    tk = 1.0;
    xpow = 1.0;
    
    while (ratio > eps10) {
      
      tk *= aks / (bks * kfact);
      xpow *= x;      
      term = tk * xpow;      
      sum += term;
      aks += 1.0;
      bks += 1.0;
      kfact += 1.0;
      
      ratio = abs(term / sum);
    }
    return sum;

  } // end if |x| < asympThresh


  // use asymptotic series
  if (x >= asympThresh) {
    if (a <= 0.0) {
      std::cerr << "a <= 0.0 and x >= " <<  asympThresh << ". 0 returned" << std::endl;
      return 0.0;
    }
    k = 0; 
    fk = 1.0;
    bMaPkM1 = b - a;
    oneMPkM1 = 1.0 - a;
    term = 1.0;
    termold = 100.0;
    sum = 1.0;
    ratio = 10.0;

    while (ratio > eps10) {
      k++;
      term *= bMaPkM1 * oneMPkM1 / (fk * x);
      sum += term;
      bMaPkM1 += 1.0;
      oneMPkM1 += 1.0;
      fk += 1.0;
      ratio = abs(term / sum);      
      termnew = abs(term);

      if (termnew > termold) {
	std::cerr << "Series diverging after " << k << " terms" << std::endl;
	ratio = 0.0; // returns
      }
      else {
	termold = termnew;
      }
      
    } // end while
    
    return exp(DTIMath::gammln(b) + x - DTIMath::gammln(a)) * pow(x, a - b) * sum;

    
  } // end if 

  
  // x <= -asympThresh  
  if (b <= a) {
    std::cerr << "b <= a, x <= -" << asympThresh << ". 0 returned" << std::endl;
    return 0.0;
  }

  mx = -1.0 * x;
  k = 0;
  fk = 1.0;
  aMbPk = a - b + 1.0;
  aPkM1 = a;
  term = 1.0;
  termold = 100.0;
  sum = 1.0;
  ratio = 10.0;

  while (ratio < eps10) {
    
    k++;
    term *= aPkM1 * aMbPk / (fk * mx);
    sum += term;
    aPkM1 += 1.0;
    aMbPk += 1.0;
    fk += 1.0;
    
    ratio = abs(term / sum);
    
    termnew = abs(term);
    
    
    if (termnew > termold) {
      std::cerr << "Series diverging after " << k << " terms" << std::endl;
      ratio = 0.0; // returns
    }
    else {
      termold = termnew;
    }
    
  } // end while
  
  return exp( DTIMath::gammln(b) - DTIMath::gammln(b - a) ) * sum / pow(mx, a);
}


/***********************************************************************
 *  Method: DTIMath::gammln
 *  Params: double xx
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::gammln(double xx)
{
  // Code originally written in Camino by Dan Alexander
  // The gamma function. See NRC p. 214.
  
  // Euler's constant. (NRC p. 223)
  double EULERGAMMA = 0.5772156649;
  double x, y, tmp, ser;
  double cof[6] = { 76.18009172947146, -86.50532032941677, 24.01409824083091,
		   -1.231739572450155, 0.1208650973866179e-2, -0.5395239384953e-5 };
  int j;
  y = x = xx;
  tmp = x + 5.5;
  tmp -= (x + 0.5) * log(tmp);
  ser = 1.000000000190015;
  for (j = 0; j <= 5; j++)
    ser += cof[j] / ++y;
  return -tmp + log(2.5066282746310005 * ser / x);
}



/***********************************************************************
 *  Method: DTIMath::normFB5
 *  Params: double k1, double k2
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::normFB5(double k1, double k2)
{
  if (k2 > 0.0 || k1 > k2) {
    cerr << "Illegal concentration parameters {k1, k2} = {" << k1 << ", " << k2 << "}" << endl;
    return 0;
  }
  double der[2];
  double hes[4];
  int level[1];
  level[0] = 8;
  int mode = 0;

  // from Kent's paper
  if (k2 < -8.5) {
    mode = 3;
  }
  else {
    mode = 1;    
    if (k1 <= -10.0 && k1 / k2 >= 2.0) {
      mode = 2;
    }
  }	
  return normFB5(k1, k2, der, hes, mode, level) / (4.0 * M_PI);
}


/***********************************************************************
 *  Method: DTIMath::normFB5
 *  Params: double k1, double k2, double der[], double hes[], int mode, int level[]
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::normFB5(double k1, double k2, double der[], double hes[], int mode, int level[])
{
  double c1 = 15.74960995;
  double pi2 = 6.283185308;
  int c__9 = 9;
  int c__1 = 1;
  int c__3 = 3;
  int c__5 = 5;
  double c_b28 = 8.;
  int init = 0;
  
  /* System generated locals */
  int i__1, i__2, i__3;
  double ret_val = 0.0;
  
  /* Local variables */
  double dera, derb, fact, derd, derl, trma, trmb, term, fact0, term0;
  
  double a[10];
  double b[8];
  double d[8];
  double e[9];
  
  int i, j, k, n;
  double hesaa, hesab;
  int ifail;
  double hesbb;
  double cfold[9];
  double y, termb, termd, terml, b2;
  double bb[9]; 
  double ea,be;
  double dd[9];
  double cf[9];
  double eb, de, al;

  double add, eps;
  int imx = 0;
  int jmx = 0;
  double sum, add0, bes0;
  
  
  /*    THIS FUNCTION CALCULATES THE NORMALIZATION CONSTANT FOR THE       BI
	N00050*/
  /*      BINGHAM DISTRIBUTION AND ITS DERIVATIVES WITH RESPECT TO        BI
	  N00060*/
  /*       THE PARAMETERS K1 AND K2.                                    BI
	   N00070*/
  /*                                                                      BI
									  N00080*/
  /*    INPUT:  K1 <= K2 <= 0   CONCENTRATION PARAMETERS                BI
	N00090*/
  /*            MODE = 1  USE TAYLOR SERIES EXPANSION                     BI
		N00100*/
  /*                   2  NORMAL-VON MISES ASYMPTOTIC SERIES              BI
		       N00110*/
  /*                   3  BIVARIATE NORMAL ASYMPTOTIC SERIES              BI
		       N00120*/
  /*             LEVEL >= 0  NUMBER OF TERMS USED IN ASYMPTOTIC SERIES    BI
		       N00130*/
  /*                         (MODES 2 AND 3)                              BI
			     N00140*/
  /*     OUTPUT: BINGC   NORMALIZATION CONSTANT                           BI
	 N00150*/
  /*             DER(2)  DERIVATIVES OF BINGC WRT  K1 AND K2            BI
		 N00160*/
  /*             HES(2,2) SECOND DERIVATIVES                              BI
		 N00170*/
  /*                      BASED ON POWER SERIES(MODE 1) OR THE DOMINANT   BI
			  N00180*/
  /*                      TERM IN THE ASYMPTOTIC EXPANSION(MODE = 2 OR 3) BI
			  N00190*/
  /*             LEVEL    NUMBER OF TERMS USED IN EACH SUM (MODE 1)       BI
		 N00200*/
  /*             IND = IFAIL = 1 IF SERIES DO NOT CONVERGE                BI
		 N00210*/
  /* Parameter adjustments */
  //	hes -= 3;
  // 	--der;

  /* Function Body */
  //    *ind = 0;
  eps = 1e-6;
  /*    LEVUP = MAXIMUM NO OF LEVELS                                      BI
	      N00310*/
  if (k1 > k2 || k2 > 0.0) {
    ifail = 1;
    return ret_val;
  }
  //     if (init == 1) {
  // 	goto L6;
  //     }
  if (init != 1) {
    
    init = 1;
    e[0] = 1.0;
    for (i = 1; i <= 8; ++i) {
      /* L5: */
      e[i] = e[i - 1] * (i * 2.0 - 1.0);
    }
    /*    THE E(I) REPRESENT THE 2*I'TH CENTRAL MOMENT OF THE STANDARD      BI
	  N00430*/
    /*    NORMAL DISTRIBUTION                                               BI
	  N00440*/
    
  }

  
  // L6:
  /* L100: */
  //     if (*mode != 1) {
  // 	goto L200;
  //     }
    
  if (mode == 1) {
    
    /*                                                                      BI
									    N00480*/
    /*    NOW WE FIND BOUNDS ON THE NUMBER OF TERMS NEEDED FOR GIVEN        BI
	  N00490*/
    /*     ACCURACY IN TERMS OF THE EXP FUNCTION                            BI
	   N00500*/
    /*                                                                      BI
									    N00510*/
    //	*ind = 0;
    al = -(k1);
    be = -(k1) + k2;
    ea = exp(al);
    trma = al;
    for (i = 2; i <= 100; ++i) {
      trma = trma * al / i;
      if ((double) i > al && trma / ea < eps) {
	imx = i;
	break; // goto L62;
      }
	    }
    if (i == 101) {
      cerr << "k1: " << k1 << " k2: " << k2 << endl;
      cerr << "L61: Normalization constant couldn't converge" << endl;
      exit(0);
    }
    /* L61: */
    
    // 	    *ind = 1;
    //	L62:
    eb = exp(be);
    trmb = be;
    for (i = 2; i <= 100; ++i) {
      trmb = trmb * be / i;
      if ((double) i > be && trmb / eb < eps) {
	jmx = i;
	break; // goto L64;
      }
		/* L63: */
    }
    if (i == 101) {
      cerr << "L63: Normalization constant couldn't converge" << endl;
      exit(0);
    }
    //	*ind = 1;
    //	L64:
    term0 = pi2 * 2;
    ret_val = 0.0;
    dera = 0.0;
    derb = 0.0;
    hesaa = 0.0;
    hesab = 0.0;
    hesbb = 0.0;
    if (abs(al) < 1e-15) {
      al = 1e-15;
    }
    if (abs(be) < 1e-15) {
      be = 1e-15;
    }
    /*    BINGC = EXP(K1)*SUM( TERM(I,J) )  OVER I,J >=0                   BI
	  N00860*/
    /*     TERM0 = TERM(I,0)                                                BI
	   N00870*/
    /*                                                                      BI
									    N00880*/
    i__1 = imx;
    for (i = 0; i <= i__1; ++i) {
      if (i >= 1) {
	term0 = term0 * (i - 0.5) / (i + 0.5) / i * al;
      }
      term = term0;
      sum = term;
      dera += term * i / al;
      hesaa += term * i * (i - 1) / al / al;
      i__2 = jmx;
      for (j = 1; j <= i__2; ++j) {
	term = term * (j - .5) / ((i + j + .5) * j) * be;
	dera += term * i / al;
	derb += term * j / be;
	hesaa += term * i * (i - 1) / al / al;
	hesab += term * i * j / al / be;
	hesbb += term * j * (j - 1) / be / be;
	sum += term;
	if (term < 1e-16 && j >= 2) {
	  break; //    goto L34;
	}
	/* L32: */
      }
      // L34:
      ret_val += sum;
      if (sum < 1e-16 && i >= 2) {
	break; // goto L33;
      }
      /* L31: */
    }
    //	L33:
    level[0] = (imx>jmx)?imx:jmx;
    der[0] = 1 - (dera + derb) / ret_val;
    der[1] = derb / ret_val;
    b2 = ret_val * ret_val;
    hesaa = hesaa / ret_val - dera * dera / b2;
    hesab = hesab / ret_val - dera * derb / b2;
    hesbb = hesbb / ret_val - derb * derb / b2;
    hes[0] = hesaa + hesab * 2 + hesbb;
    hes[2] = -hesab - hesbb;
    hes[1] = hes[2];
    hes[3] = hesbb;
    ret_val *= exp(k1);
    
    // exception already thrown if needed
    // 	if (*ind == 1) {
    // 	    cout << " NO CONVERGENCE IN BINGC; MODE 1\n";
    // 	    exit(0);
    // 	}
    return ret_val;
    
  } // end if mode == 1
  
  // L200:
  //    if (*mode != 2) {
  //        goto L300;
  //    }
  else if (mode == 2) {
    
    /*    A(K)=  BESRAT   (BE) = I (BE)/I   (BE)                            BI
	  N01260*/
    /*                 K-1        K      K-1                                BI
		       N01270*/
    /*    THEN SET                                                          BI
	  N01280*/
    /*     A(K) = K'TH MOMENT FOR VON MISES DSN                             BI
	   N01290*/
    /*          = I (BE)/I (BE) = OLD  A(1)...A(K)                          BI
		N01300*/
    /*             K      0                                                 BI
		   N01310*/
    /*                                                                      BI
									    N01320*/
    al = k2 - k1 * 2.0;
    be = -(k2) / 2.0;
    if (al <= 0.001) {
      ret_val = c1;
      der[0] = 3.0;
      der[1] = 1.0;
      hes[0] = 1.0;
      hes[2] = 0.0;
      hes[1] = 0.0;
      hes[3] = 1.0;
      return ret_val;
    }
    y = be * be / 4.0;
    bes0 = bstrs0(y);
    /*           = I (BE) * 1 * EXP(-BE)                                    BI
		 N01470*/
    /*              0                                                       BI
		    N01480*/
    /*                                                                      BI
									    N01490*/
    a[9] = besrat(c_b28, be, eps);
    for (i = 8; i >= 1; --i) {
      /* L1: */
      a[i] = be / (be * a[i + 1] + (i << 1));
    }
    for (i = 2; i <= 9; ++i) {
		if (a[i - 1] < 1e-15) {
		    a[i - 1] = 0.;
		}
		/* L2: */
		a[i] = a[i - 1] * a[i];
	    }
	    a[0] = 1.;
	    bb[0] = a[1];
	    for (i = 0; i <= 8; ++i) {
		cfold[i] = 0.0;
		/* L50: */
		cf[i] = 0.0;
	    }
	    cf[0] = 1.0;
	    /* L56: */
	    term = 1.0;
	    fact0 = 1.0;
	    termb = bb[0];
	    terml = 0.0;
	    // 	if (*level == 0) {
	    // 	    goto L8;
	    // 	}
	    if (level[0] != 0) {

		i__1 = level[0];
		for (k = 1; k <= i__1; ++k) {
		    n = k;
		    i__2 = n - 1;
		    for (i = 0; i <= i__2; ++i) {
			/* L52: */
			cfold[i] = cf[i];
		    }
		    if (n == 1) {
			cf[0] = 0.0;
			cf[1] = 1.0;
		    }
		    if (n == 2) {
			cf[0] = 0.5;
			cf[1] = 0.0;
			cf[2] = 0.5;
		    }
		    if (n > 2) {
			cf[0] = cfold[1] / 2.0;
			cf[1] = cfold[0] + cfold[2] / 2.0;
			cf[n - 1] = cfold[n - 2] / 2.0;
			cf[n] = cfold[n - 1] / 2.0;
		    }
		    // 		if (n <= 3) {
		    // 		    goto L54;
		    // 		}
		    if (n > 3) {
		    
			i__2 = n - 2;
			for (i = 2; i <= i__2; ++i) {
			    /* L53: */
			    cf[i] = (cfold[i - 1] + cfold[i + 1]) / 2.0;
			}
		    }
		    //		L54:
		    b[n - 1] = 0.0;
		    bb[n] = 0.0;
		    i__2 = n;
		    for (i = 0; i <= i__2; ++i) {
			if (i == 0) {
			    bb[n] += cf[0] * a[1];
			} else {
			    bb[n] += cf[i] * (a[i - 1] + a[i + 1]) / 2.0;
			}
			/* L55: */
			b[n - 1] += cf[i] * a[i];
		    }
		    if (k == 1) {
			fact0 = fact0 * (-1 / al) / k;
		    } else {
			fact0 = fact0 * (-be / al) / k;
		    }
		    fact = fact0 * be;
		    termb += e[k] * (b[k - 1] * fact0 * k + bb[k] * fact);
		    terml -= k * e[k] * b[k - 1] * fact / al;
		    add = e[k] * b[k - 1] * fact;
		    term += add;
		    if (abs(add) / term < 1e-15) {
			break; // goto L8;
		    }
		    /* L3: */
		}
	    } // if level != 0

 	
	    // 	L8:
	    /* L4: */
	    ret_val = c1 / sqrt(al) * bes0 * term;
	    derl = -0.5 / al + terml / term;
	    derb = termb / term - 1;
	    der[0] = derl * -2;
	    der[1] = derl - derb * 0.5;
	    hes[0] = 1.0 / (k1 * 2.0 * k1);
	    hes[2] = 0.0;
	    hes[1] = hes[2];
	    hes[3] = (a[2] * 0.5 + 0.5 - a[1] * a[1]) / 4.0;
	    return ret_val;

	

	} // end if mode == 2
	else if (mode == 3) {
	    // 	 L300:
	    // 	if (*mode != 3) {
	    // 	goto L400;
	    //     }

	    al = k2 * -2.0;
	    if (al == 0.0) {
		ret_val = 1.0;
		der[0] = 1.0;
		der[1] = 1.0;
		hes[0] = 1.0;
		hes[2] = 0.0;
		hes[1] = 0.0;
		hes[3] = 1.0;
		return ret_val;
	    }
	    de = k2 / k1;
	    for (i = 0; i <= 8; ++i) {
		cf[i] = 0.0;
		/* L41: */
		cfold[i] = 0.0;
	    }
	    cf[0] = 1.0;
	    /* L47: */
	    term = 1.0;
	    fact = 1.0;
	    add = 1.0;
	    termd = 0.0;
	    terml = 0.0;
	    // 	if (*level == 0) {
	    // 	    goto L9;
	    // 	}
	    if (level[0] != 0) {
 	    
		i__1 = level[0];
		for (k = 1; k <= i__1; ++k) {
		    n = k;
		    i__2 = n - 1;
		    for (i = 0; i <= i__2; ++i) {
			/* L43: */
			cfold[i] = cf[i];
		    }
		    cf[0] = cfold[0];
		    cf[n] = cfold[n - 1];
		    // 		if (n < 2) {
		    // 		    goto L44;
		    // 		}
		    if (n >= 2) {
			i__2 = n - 1;
			for (i = 1; i <= i__2; ++i) {
			    /* L45: */
			    cf[i] = cfold[i - 1] + cfold[i];
			}
			/*    THE CF REPRESENT THE BINOMIAL COEFFICIENTS                      
			      BIN02560*/
		    }

		    //		L44:
		    d[n - 1] = 0.0;
		    dd[n] = 0.0;
		    i__2 = n;
		    for (i = 0; i <= i__2; ++i) {
			if (i > 0) {
			    i__3 = i - 1;
			    dd[n] += i * cf[i] * e[i] * e[n - i] * pow(de, i__3);
			}
			/* L46: */
			d[n - 1] += cf[i] * e[i] * e[n - i] * pow(de, i);
		    }
		    fact = -fact * (-0.5 - k + 1) / k / al;
		    termd += fact * dd[k];
		    terml -= fact * d[k - 1] * k / al;
		    add0 = add;
		    add = fact * d[k - 1];
		    term += add;
		    if (abs(add) / term < 1e-10) {
			break; // goto L9;
		    }
		    /* L12: */
		}
	    
	    } // if level != 0
	
	    //	L9:
	    /* L11: */
	    ret_val = 2 / al * sqrt(de) * pi2 * term;
	    derl = -1 / al + terml / term;
	    derd = 0.5 / de + termd / term;
	    der[0] = derd * (-(k2)) / (k1 * k1);
	    der[1] = derl * -2 + derd / k1;
	    hes[0] = 1.0 / (k1 * 2.0 * k1);
	    hes[2] = 0.0;
	    hes[1] = 0.0;
	    hes[3] = 1.0 / (k2 * 2.0 * k2);
	    /*    DER(1)=DERL                                                       BI
		  N02810*/
	    /*    DER(2)=DERD                                                       BI
		  N02820*/
	    /*    THIS GIVES DERIVATIVES WRT LAMBDA AND SPIKE                       BI
		  N02830*/
	    /*                                                                      BI
										    N02840*/
	    return ret_val;

	} // end if mode == 3

	else { 
	    //	L400:
	  cerr << "INVALID MODE IN BINGC" << endl;
	    return ret_val;
	}

    } /* normFB5_ */     


/***********************************************************************
 *  Method: DTIMath::bstrs0
 *  Params: double y
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::bstrs0(double y)
{
  /* Format strings */
  //	String fmt_2 = "(\002 BSTRS0 NO CONVERGENCE, BSTRS0=\002,e12.5\ ,\002 COEF=\002,e12.5,\002 Y=\002,e12.5);
  //	String fmt_8 = "(\002 BSTRS0  ASYMPTOTICS NOT CONVERGED\002)";
  
  /* System generated locals */
  double ret_val;
  
  /* Local variables */
  double coef;
  int i, k;
  double t, u, x, z, b1, f1;
  
  double pi2 = atan(1.0) * 8.0;
	
  /*                                                                      BI
									  N03350*/
  /*   INPUT:                                                             BI
       N03360*/
  /*        Y = X*X/4   WHERE X IS THE ARGUMENT OF THE BESSEL FUNCTION    BI
	    N03370*/
  /*                                                                      BI
									  N03380*/
  /*   OUTPUT:                                                            BI
       N03390*/
  /*                                                                      BI
									  N03400*/
  /*        BSTRS0 = I (X) * EXP(-X)                                      BI
	    N03410*/
  /*                  0                                                   BI
		      N03420*/
  /*                                                                      BI
									  N03430*/
  /*                WHERE  X = SQRT(4*Y)                                  BI
		    N03440*/
  /*                                                                      BI
									  N03450*/
  /*          SO THAT   0   LT  BSTRS0  LE  1                             BI
	      N03460*/
  /*                                                                      BI
									  N03470*/
  /*                                                                      BI
									  N03480*/
  x = sqrt(y * 4.0);
  //     if (*y >= 100.) {
  // 	goto L3;
  //     }
  if (y < 100.0) {
    ret_val = exp(-x);
    coef = ret_val;
    for (k = 1; k <= 200; ++k) {
      coef = coef * y / (k * k);
      b1 = ret_val;
      ret_val += coef;
      if (b1 == ret_val) {
	return ret_val;
      }
      /* L1: */
    }
    return ret_val;
  }
  
  //L3:
  
  ret_val = 1.0 / sqrt(pi2 * x);
  coef = ret_val;
  u = 0.0;
  z = x * 8;
  for (i = 1; i <= 40; ++i) {
    t = i * 2.0 - 1.0;
    coef = -coef * (u - t * t) / i / z;
    f1 = ret_val;
    ret_val += coef;
    if (f1 == ret_val) {
      return ret_val;
    }
    /* L7: */
  }
  return ret_val;
}


/***********************************************************************
 *  Method: DTIMath::besrat
 *  Params: double v, double x, double eps
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIMath::besrat(double v, double x, double eps)
{
  /* Format strings */
  //	String fmt_3 = "(\002 FAULTY RATIO\002,4(2x,f12.5))";
  
  /* System generated locals */
  int i__1;
  
  /* Local variables */
  int m, n;
  double r, a1, u0, u1, y0, x2, y1, er, cx;
  double d1[20];
  double rat = 0.0;
	
  /*    THIS ROUTINE CALCULATES THE BESSEL FUNCTION RATIO                 BI
	N02920*/
  /*                                                                      BI
									  N02930*/
  /*           I   (X)/I (X)  = RAT, SAY                                  BI
	       N02940*/
  /*            V+1     V                                                 BI
		N02950*/
  /*                                                                      BI
									  N02960*/
  /*      WHERE I(.) IS THE MODIFIED BESSEL FUNCTION,                     BI
	  N02970*/
  /*      BY THE METHOD OF AMOS(1974)                                     BI
	  N02980*/
  /*    INPUT:  V  ORDER OF THE BESSEL FUNCTION                           BI
	N02990*/
  /*            X  ARGUMENT OF THE BESSEL FUNCTION                        BI
		N03000*/
  /*            EPS  ACCURACY DESIRED                                     BI
		N03010*/
  /*    OUTPUT: RAT   BESSEL FUNCTION RATIO                               BI
	N03020*/
  /*                                                                      BI
									  N03030*/
  x2 = x * x;
  cx = (v + 1.0) * 2.0;
  u0 = v + 1.5;
  y0 = u0 * u0;
  d1[0] = x / (u0 - 1. + sqrt(y0 + x2));
  for (n = 1; n <= 19; ++n) {
    u0 = v + (double) n + 1.5;
    y0 = u0 * u0;
    a1 = x / (u0 - 1.0 + sqrt(y0 + x2));
    i__1 = n;
    for (m = 1; m <= i__1; ++m) {
      r = a1 / d1[m - 1];
      d1[m - 1] = a1;
      u1 = v + (double) n - (double) m + 1.0;
      y1 = u1 * u1;
      /* L2: */
      a1 = x / (u1 + sqrt(y1 + x2 * r));
    }
    d1[n] = a1;
    rat = a1;
    er = x / (cx + x * d1[n - 1]);
    er -= a1;
    er = abs(er) / a1;
    /* L1: */
    if (er < eps) {
      return rat;
    }
  }
  return rat;
}




/***********************************************************************
 *  Method: DTIMath::eye
 *  Params: int size
 * Returns: TNT::Array2D<double>
 * Effects: 
 ***********************************************************************/
TNT::Array2D<double>
DTIMath::eye(int size)
 {
   TNT::Array2D<double> A(size,size);
   for(int jj=0; jj<size; jj++)
     for(int ii=0; ii<size; ii++)
       if (ii == jj)
	 A[ii][jj] = 1;
       else
	 A[ii][jj] = 0;
   return A;  
 }



/***********************************************************************
 *  Method: DTIMath::expm
 *  Params: const TNT::Array2D<double> A
 * Returns: TNT::Array2D<double>
 * Effects: 
 ***********************************************************************/
TNT::Array2D<double>
DTIMath::expm(const TNT::Array2D<double> A)
{
  // use jama to compute eigenvectors

  if( A.dim1() != A.dim2() || A.dim1() != 2){
    std::cerr << "Expm can only handle 2x2 square matrices!" << std::endl;
    exit(-1);
  }

   TNT::Array1D<double> eigenvalues;
   JAMA::Eigenvalue<double> eig(A);
   eig.getRealEigenvalues (eigenvalues);
   TNT::Array2D<double> S = eye(eigenvalues.dim1());
   
   for (int ii = 0; ii < eigenvalues.dim1(); ii++) {
     S[ii][ii] = exp(eigenvalues[ii]);
   } 
  
   TNT::Array2D<double> V;
   eig.getV (V);
   TNT::Array2D<double> retval = V * S * inv2(V);

  return retval;
}


/***********************************************************************
 *  Method: DTIMath::inv2
 *  Params: const TNT::Array2D<double> A
 * Returns: TNT::Array2D<double>
 * Effects: 
 ***********************************************************************/
TNT::Array2D<double>
DTIMath::inv2(const TNT::Array2D<double> A)
{
  TNT::Array2D<double> R (2,2);
  double detA = A[0][0]*A[1][1] - A[0][1]*A[1][0];
  R[0][0] = A[1][1]/detA;
  R[1][1] = A[0][0]/detA;
  R[0][1] = -A[0][1]/detA;
  R[1][0] = -A[1][0]/detA;
  return R.copy();
}

// /***********************************************************************
//  *  Method: DTIMath::unif_hemisphere_pts
//  *  Params: int nTheta, int nPhi
//  * Returns: std::vector<DTIVector>
//  * Effects: 
//  ***********************************************************************/
// std::vector<DTIVector>
// DTIMath::unif_hemisphere_pts(int nTheta, int nPhi)
// {
//   vector<DTIVector> vecs;

//   // Setup thetas and phis
//   vector<float> vTheta;
//   vector<float> vPhi;
//   for(int ii=0; ii<nTheta; ii++)
//     vTheta.push_back(M_PI/2*(float)ii/(nTheta-1));
//   for(int ii=0; ii<nPhi; ii++)
//     vPhi.push_back(2*M_PI*(float)ii/(nPhi-1));

//   // Create tangent for each hemisphere point
//   for(int pp=0;pp<nPhi;pp++) {
//     for(int tt=0;tt<nTheta;tt++) {
//       DTIVector v;
//       DTIMath::convertSphToEuc(v, vTheta[tt], vPhi[pp], 1);
//       vecs.push_back(v);
//     }
//   }
//   return vecs;
// }

/***********************************************************************
 *  Method: DTIMath::initRNG
 *  Params: int offset
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIMath::initRNG(int offset)
{
  // Offset is useful for parallel programs that might actually get identical time()
  // Initialize RNG
  uint seed = (unsigned int)time((time_t *)NULL)+offset;
  srand(seed);
}

/***********************************************************************
 *  Method: DTIMath::initRNG
 *  Params: int offset
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIMath::initRNG(int offset, uint seed)
{
  // Offset is useful for parallel programs that might actually get identical time()
  // Initialize RNG
  srand(seed+offset);
}

double 
DTIMath::computeLinePlaneIntersection (const double p0[3], const double p1[3], const double n0[3], const double v0[3]) 
{
  Vector3<double> num1;
  Vector3<double> denom1;
  Vector3<double> vN0;
  for(int ii=0; ii<3; ii++) {
    num1[ii] = v0[ii]-p0[ii];
    denom1[ii] = p1[ii]-p0[ii];
    vN0[ii] = n0[ii];
  }
  double num = dproduct(vN0, num1);
  double denom = dproduct(vN0, denom1);
  if (denom != 0) 
    return num/denom;
  else 
    return 1000.0;
}

bool 
DTIMath::linesIntersect (double pt1[3], double pt2[3], double pt3[3], double pt4[3]) 
{
  double denom = (pt4[1] - pt3[1])*(pt2[0] - pt1[0]) - (pt4[0] - pt3[0])*(pt2[1]-pt1[1]);
  if (denom == 0) return false;
  else {
    double ua = ((pt4[0] - pt3[0])*(pt1[1]-pt3[1]) - (pt4[1]-pt3[1])*(pt1[0]-pt3[0])) / denom;
    double ub = ((pt2[0] - pt1[0])*(pt1[1]-pt3[1]) - (pt2[1]-pt1[1])*(pt1[0]-pt3[0])) / denom;
    if (ua >= 0 && ub >= 0 && ua <= 1 && ub <= 1) {
      return true;
    }
  }
  return false;
}

