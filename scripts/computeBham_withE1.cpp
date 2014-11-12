#include <iostream>
#include <math.h>
#include <stdio.h>
#include <omp.h>
#include <sstream>

const double pi=3.1412;

//NOTE VARIABLE NAMES FOLLOW MATLAB NOTATION...
/** 
*  Args:
*         t = tangent vector
*  e{1,2,3} = eigenvalues (in decreasing order)
*         C = Bingham constant of integration (set to 1 to compute surface integral)
*  Returns:
*         The Bingham distribution score for the given tangent value
*  
*  NOTE:
*  Assumes that the D tensor is identity with diag : (e1,e2,e3). 
*  i.e. eigvecs are unit vecs along x,y,z (no loss while computing integration 
*  constants since the surface area computation is unaffected by rotations) */
double ctrBhamScore(const double *t /** sz=3*/, const double e1, const double e2, const double e3, const double C)
{
	double sigmaM = 0.2443; //==pi*14/180; User param. From paper (pg. 7 col. 2, para 1)
	double eta = .175; // User param. From paper (pg. 7 col. 2, para 2)

	// CL: Tensor linearity index = abs(eig1 - eig2) / sum(eigs);
	// Right-2nd para on page 4 of Contrack J'Vision 2008.
    double CL = fabs(e1 - e2) / (e1+e2+e3);
	
	// Compute delta = 100deg / (1+ exp( - (eta - CL) / 0.015 ) );
	// SM : Move to radians instead of degrees. Original cpp may not be
	// implemented with SI units.
	// 100 deg =  pi*100/180 rad = 1.7453 rad
	// In cpp file:
	// double linshape_ds = uniform_s / (1+exp(-(linearityMidCl-p.fCl)*10/linearityWidthCl));
	// linearityMidCl = 0.174;  linearityWidthCl = .174 (differs from paper)
	double delta = (1.7453) / ( 1.0 + exp(- (eta - CL) / 0.015) ); // 100 in degrees

	// Compute the term for eigenvector 3
	double sigma3star = e3 / ( e2 + e3 ) * delta;
	double sigma3 = sigmaM + sigma3star;
	double t3 = t[2];//Since D is diag, eigvec is unit v(:,3)'*t;
	t3 = t3 / sin(sigma3);

	// Compute the term for eigenvector 2
	double sigma2star = e2 / ( e2 + e3 ) * delta;
	double sigma2 = sigmaM + sigma2star;
	double t2 = t[1]; //Since D is diag, eigvec is unit v(:,2)'*t;
	t2 = t2 / sin(sigma2);

	double bhamScore = C* exp( -(t3*t3) - (t2*t2));

	return bhamScore;
}

/** Computes the {partition function, normalizing pdf constant, surface integral}
  * for the Bingham distribution on a sphere, provided a set of eigenvalues.
  * Assumes that the diffusion tensor is diagonal.
  * 
  * Args:
  *    e{1,2,3} = Eigenvalues of the diffusion tensor (decreasing order)
  * dtheta/dphi = The numerical integration steps on a sphere
  * 
  *  Note that this sphere surface integrator works quite well with dtheta
  *  and dphi = 0.002. 
  *  
  *  ===========================
  *  Step  ||  Area(unit-sphere)
  *  ===========================
  *  0.5   || 13.7296
  *  0.1   || 12.6523
  *  0.05  || 12.6537
  *  0.01  || 12.6014
  *  0.005 || 12.5757                                  <== ok for quick results
  *  0.002 || 12.5706 (substantially slower than .005) <== best quality/speed tradeoff
  *  0.001 || 12.5654 (substantially slower than .002)
  *  0.0005|| 12.5628 (REALLY slow)
  *  0.0001|| You don't want to use this...
  *
  *  Correct answer : 4*pi ~= 12.5664
  *  */
double ctrFindIntegConstt(const double e1, const double e2, const double e3, 
	const double dtheta = 0.002, const double dphi = 0.002)
{
	// Unit vector relative to which integration proceeds, we'll muck with it below:
	double t[3];
	double r=1;            // Integrate over unit sphere in RAS (xyz) coordinates
	double ar=0.0;

	for(double theta = -pi; theta<pi;theta+=dtheta) //From lower pole to upper pole
	{
		// NOTE : Ideally this should integrate to 2*pi. However, the distrib is symmetric
		//        along one axis. So we can just compute it for half the sphere and double
		//        the total integral value at the end (see return line).
	    for(double phi = 0; phi<pi; phi+=dphi) //Integrate over a disk sliced moving along the pole
	    {
	    	// Direction along which to compute the BHAM score
	        t[0] = r * cos(theta) * cos(phi); // x
	        t[1] = r * cos(theta) * sin(phi); // y
	        t[2] = r * sin(theta); // z
	        
	        // Patch area = height of disk * section-of-circumference
	        double dar = fabs( (r * dtheta/2.0) * (dphi * r*cos(theta)) );
	        
	        // Note, we are just computing the raw function at the position t, so
	        // we just pass 1 as the normalizing constant
	        double bhscore = ctrBhamScore(t, e1, e2, e3, 1);
	        
	        double darb = dar * bhscore; //At that point.
	        ar = ar + darb;
	    }
	}
	// Double the area to compensate for the fact that we only integrated
	// phi from 0 to pi (not 2*pi). Symmetric function on sphere. So double
	// it and return..
	return ar * 2;
}

const int nthreads=16;


/** The main function that computes the Bingham partition function for a variety
  * of eigenvector/value instances. Note that this is a modified version of the
  * commonly used Bingham function (the creators of contrack decided to reformulate it). 
  * 
  * You may set the eigenvalue spacing and integration steps.
  * 
  * NOTE : Will dump out files that look like:
  *           bhamConst-e1e2e3C-t[id]-[de]-[dtheta].txt
  *        where id = thread id, de = eigenvalue step, dtheta = integration step 
  */
int main()
{
	const double einit = 0.01, de=0.01, dtheta=0.002, dphi=dtheta;
	const double e_max = 4; // Max eigenvalue
	
	omp_set_num_threads(nthreads);
	int thread_id;
	double e1,e2,e3,dd;

	FILE * fp[nthreads];
	std::stringstream ss[nthreads];
	std::string fname[nthreads];

#pragma omp parallel private(thread_id,e1,e2,e3,dd)
	{
		thread_id = omp_get_thread_num();
		ss[thread_id]<<"bhamConst-e1e2e3C-t"<<thread_id<<"-"<<de<<"-"<<dtheta<<".txt";
		fname[thread_id] = ss[thread_id].str();
		fp[thread_id] = fopen(fname[thread_id].c_str(),"w");

		printf("\n I am thread %d. Outfile: %s", thread_id, fname[thread_id].c_str());

		// Run the actual loop over eigenvalues
		// The extra stuff in e1's iteration is to enable multi-threading..
		for(e1=einit+ static_cast<double>(thread_id)*de;
			e1<e_max;e1+=nthreads*de){
			for(e2=einit;e2<e1;e2+=de){
				for(e3=einit;e3<e2;e3+=de){
					dd = ctrFindIntegConstt(e1,e2,e3,dtheta,dphi);
					fprintf(fp[thread_id], "%lf %lf %lf %lf\n", e1, e2, e3, dd);
				}
				printf("\n t[%d], e1 : %lf, e2 : %lf",thread_id,e1,e2);
				fflush(fp[thread_id]);//Flush the file buffer...
			}
		}
		fclose(fp[thread_id]);
	}
	return 0;	
}
