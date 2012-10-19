
// This file is for common cdf calculation routines

//  DCDFLIB is a library of C++ routines, using double precision
//  arithmetic, for evaluating cumulative probability density
//  functions.

// DCDFLIB includes routines for evaluating the cumulative density
// functions of a variety of standard probability distributions. An
// unusual feature of this library is its ability to easily compute
// any one parameter of the CDF given the others. This means that a
// single routine can evaluate the CDF given the usual parameters, or
// determine the value of a parameter that produced a given CDF value.

//  Author:
//     Barry Brown, James Lovato, and Kathy Russell,
//     Department of Biomathematics,
//     University of Texas,
//     Houston, Texas.

double dpmpar ( int *i );

void cumnor ( double *arg, double *result, double *ccum );

double dinvnr ( double *p, double *q );

int ipmpar ( int *i );

double fifdint ( double a );

double stvaln ( double *p );

double eval_pol ( double a[], int *n, double *x );

void cdfnor ( int *which, double *p, double *q, double *x, double *mean,
  double *sd, int *status, double *bound );

