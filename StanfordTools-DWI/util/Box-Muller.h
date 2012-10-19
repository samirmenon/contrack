/*
 *  Box-Muller.h
 *  anneal
 *
 *  Created by J. Johnson on Thu May 30 2002.
 *  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
 *
 */

/***********************************************************************************
This is the header file for the random class, which is for generating different distributions of random numbers.
************************************************************************************
class randn (Box-Muller method) J. Johnson
    Function(s) for randn
        get(double mean, double sigma, int seed);	This funciton takes a flat
                                    distribution of random numbers and generates
                                    a gaussian distribution of random numbers.
                                    mean is simply the mean of the mean of the 
                                    distribution, and sigma is the standard 
                                    deviation of the distribution.  seed is the int
                                    used to seed (srand(seed)) the random number
                                    generator.  It should never be the same number
                                    two times in a row.
***********************************************************************************/
#include <iostream>
#include <cmath>
#include <cstdlib>
#include "random.h"
using namespace std;

#ifndef BoxMuller_h
#define BoxMuller_h

class randn_generator
{
private:

public:
    randn_generator(){};
    static double get(double mean, double sigma, int seed);
    static double get(double mean, double sigma);
    static double pf_get(double mean, double sigma);
};

// XXX This doesn't look like the method I am familiar with we also
// should be able to get two gaussians from each draw
double randn_generator::get(double mean, double sigma, int seed)//Box-Muller method
{
    srand(seed);
    
    //double x1, x2, w=2;
    double pars[3];
    pars[2]=2;  //ensures that loop is carried out atleast once. (If pars[2] is 
                //greater than one, it is bad and produces a NaN resutl).
    
    while(pars[2]>=1.0)
    {
        pars[0] = 2.0 * double(rand())/RAND_MAX - 1.0;
        pars[1] = 2.0 * double(rand())/RAND_MAX - 1.0;
        pars[2] = pars[0] * pars[0] + pars[1] * pars[1];
    }
    
    pars[2] = sqrt( (-2.0 * log( pars[2] ) ) / pars[2] );
    pars[2]*= pars[0];
       
    return mean+pars[2]*sigma;
}


double randn_generator::get(double mean, double sigma)//Box-Muller method
{
    
    //double x1, x2, w=2;
    double pars[3];
    pars[2]=2;  //ensures that loop is carried out atleast once. (If pars[2] is 
                //greater than one, it is bad and produces a NaN resutl).
    
    while(pars[2]>=1.0)
    {
        pars[0] = 2.0 * double(rand())/RAND_MAX - 1.0;
        pars[1] = 2.0 * double(rand())/RAND_MAX - 1.0;
        pars[2] = pars[0] * pars[0] + pars[1] * pars[1];
    }
    
    pars[2] = sqrt( (-2.0 * log( pars[2] ) ) / pars[2] );
    pars[2]*= pars[0];
       
    return mean+pars[2]*sigma;
}

double randn_generator::pf_get(double mean, double sigma)//Box-Muller method
{
    //double x1, x2, w=2;
    double pars[3];
    pars[2]=2;  //ensures that loop is carried out atleast once. (If pars[2] is 
                //greater than one, it is bad and produces a NaN resutl).
    
    while(pars[2]>=1.0)
    {
        pars[0] = 2.0 * double(pf_rand())/RAND_MAX - 1.0;
        pars[1] = 2.0 * double(pf_rand())/RAND_MAX - 1.0;
        pars[2] = pars[0] * pars[0] + pars[1] * pars[1];
    }
    
    pars[2] = sqrt( (-2.0 * log( pars[2] ) ) / pars[2] );
    pars[2]*= pars[0];
       
    return mean+pars[2]*sigma;
}

#endif
