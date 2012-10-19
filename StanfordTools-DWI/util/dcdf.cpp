
#include <cmath>
using namespace std;
#include "dcdf.h"

double dpmpar ( int *i )

//****************************************************************************
//
//  Purpose:
//
//    DPMPAR provides machine constants for double precision arithmetic.
//
//  Discussion:
//
//     DPMPAR PROVIDES THE double PRECISION MACHINE CONSTANTS FOR
//     THE COMPUTER BEING USED. IT IS ASSUMED THAT THE ARGUMENT
//     I IS AN INTEGER HAVING ONE OF THE VALUES 1, 2, OR 3. IF THE
//     double PRECISION ARITHMETIC BEING USED HAS M BASE B DIGITS AND
//     ITS SMALLEST AND LARGEST EXPONENTS ARE EMIN AND EMAX, THEN
// 
//        DPMPAR(1) = B**(1 - M), THE MACHINE PRECISION,
// 
//        DPMPAR(2) = B**(EMIN - 1), THE SMALLEST MAGNITUDE,
// 
//        DPMPAR(3) = B**EMAX*(1 - B**(-M)), THE LARGEST MAGNITUDE.
// 
//     WRITTEN BY
//        ALFRED H. MORRIS, JR.
//        NAVAL SURFACE WARFARE CENTER
//        DAHLGREN VIRGINIA
//
//     MODIFIED BY BARRY W. BROWN TO RETURN DOUBLE PRECISION MACHINE
//     CONSTANTS FOR THE COMPUTER BEING USED.  THIS MODIFICATION WAS
//     MADE AS PART OF CONVERTING BRATIO TO DOUBLE PRECISION
//
{
  static int K1 = 4;
  static int K2 = 8;
  static int K3 = 9;
  static int K4 = 10;
  static double value,b,binv,bm1,one,w,z;
  static int emax,emin,ibeta,m;

    if(*i > 1) goto S10;
    b = ipmpar(&K1);
    m = ipmpar(&K2);
    value = pow(b,(double)(1-m));
    return value;
S10:
    if(*i > 2) goto S20;
    b = ipmpar(&K1);
    emin = ipmpar(&K3);
    one = 1.0;
    binv = one/b;
    w = pow(b,(double)(emin+2));
    value = w*binv*binv*binv;
    return value;
S20:
    ibeta = ipmpar(&K1);
    m = ipmpar(&K2);
    emax = ipmpar(&K4);
    b = ibeta;
    bm1 = ibeta-1;
    one = 1.0;
    z = pow(b,(double)(m-1));
    w = ((z-one)*b+bm1)/(b*z);
    z = pow(b,(double)(emax-2));
    value = w*z*b*b;
    return value;
}
//****************************************************************************

void cumnor ( double *arg, double *result, double *ccum )

//****************************************************************************
// 
//  Purpose:
// 
//    CUMNOR computes the cumulative normal distribution.
//
//  Discussion:
//
//    This function evaluates the normal distribution function:
//
//                              / x
//                     1       |       -t*t/2
//          P(x) = ----------- |      e       dt
//                 sqrt(2 pi)  |
//                             /-oo
//
//    This transportable program uses rational functions that
//    theoretically approximate the normal distribution function to
//    at least 18 significant decimal digits.  The accuracy achieved
//    depends on the arithmetic system, the compiler, the intrinsic
//    functions, and proper selection of the machine-dependent
//    constants.
//
//  Author: 
//
//    W J Cody
//    Mathematics and Computer Science Division
//    Argonne National Laboratory
//    Argonne, IL 60439
//
//  Reference:
//
//    W J Cody,
//    Rational Chebyshev approximations for the error function,
//    Mathematics of Computation, 
//    1969, pages 631-637.
//
//    W J Cody, 
//    Algorithm 715: 
//    SPECFUN - A Portable FORTRAN Package of Special Function Routines 
//      and Test Drivers,
//    ACM Transactions on Mathematical Software,
//    Volume 19, 1993, pages 22-32.
//
//  Parameters:
//
//    Input, double *ARG, the upper limit of integration.
//
//    Output, double *CUM, *CCUM, the Normal density CDF and
//    complementary CDF.
//
//  Local Parameters:
//
//    Local, double EPS, the argument below which anorm(x) 
//    may be represented by 0.5D+00 and above which  x*x  will not underflow.
//    A conservative value is the largest machine number X
//    such that   1.0D+00 + X = 1.0D+00   to machine precision.
//
{
  static double a[5] = {
    2.2352520354606839287e00,1.6102823106855587881e02,1.0676894854603709582e03,
    1.8154981253343561249e04,6.5682337918207449113e-2
  };
  static double b[4] = {
    4.7202581904688241870e01,9.7609855173777669322e02,1.0260932208618978205e04,
    4.5507789335026729956e04
  };
  static double c[9] = {
    3.9894151208813466764e-1,8.8831497943883759412e00,9.3506656132177855979e01,
    5.9727027639480026226e02,2.4945375852903726711e03,6.8481904505362823326e03,
    1.1602651437647350124e04,9.8427148383839780218e03,1.0765576773720192317e-8
  };
  static double d[8] = {
    2.2266688044328115691e01,2.3538790178262499861e02,1.5193775994075548050e03,
    6.4855582982667607550e03,1.8615571640885098091e04,3.4900952721145977266e04,
    3.8912003286093271411e04,1.9685429676859990727e04
  };
  static double half = 0.5e0;
  static double p[6] = {
    2.1589853405795699e-1,1.274011611602473639e-1,2.2235277870649807e-2,
    1.421619193227893466e-3,2.9112874951168792e-5,2.307344176494017303e-2
  };
  static double one = 1.0e0;
  static double q[5] = {
    1.28426009614491121e00,4.68238212480865118e-1,6.59881378689285515e-2,
    3.78239633202758244e-3,7.29751555083966205e-5
  };
  static double sixten = 1.60e0;
  static double sqrpi = 3.9894228040143267794e-1;
  static double thrsh = 0.66291e0;
  static double root32 = 5.656854248e0;
  static double zero = 0.0e0;
  static int K1 = 1;
  static int K2 = 2;
  static int i;
  static double del,eps,temp,x,xden,xnum,y,xsq,min;
//
//  Machine dependent constants
//
    eps = dpmpar(&K1)*0.5e0;
    min = dpmpar(&K2);
    x = *arg;
    y = fabs(x);
    if(y <= thrsh) {
//
//  Evaluate  anorm  for  |X| <= 0.66291
//
        xsq = zero;
        if(y > eps) xsq = x*x;
        xnum = a[4]*xsq;
        xden = xsq;
        for ( i = 0; i < 3; i++ )
        {
            xnum = (xnum+a[i])*xsq;
            xden = (xden+b[i])*xsq;
        }
        *result = x*(xnum+a[3])/(xden+b[3]);
        temp = *result;
        *result = half+temp;
        *ccum = half-temp;
    }
//
//  Evaluate  anorm  for 0.66291 <= |X| <= sqrt(32)
//
    else if(y <= root32) {
        xnum = c[8]*y;
        xden = y;
        for ( i = 0; i < 7; i++ )
        {
            xnum = (xnum+c[i])*y;
            xden = (xden+d[i])*y;
        }
        *result = (xnum+c[7])/(xden+d[7]);
        xsq = fifdint(y*sixten)/sixten;
        del = (y-xsq)*(y+xsq);
        *result = exp(-(xsq*xsq*half))*exp(-(del*half))**result;
        *ccum = one-*result;
        if(x > zero) {
            temp = *result;
            *result = *ccum;
            *ccum = temp;
        }
    }
//
//  Evaluate  anorm  for |X| > sqrt(32)
//
    else  {
        *result = zero;
        xsq = one/(x*x);
        xnum = p[5]*xsq;
        xden = xsq;
        for ( i = 0; i < 4; i++ )
        {
            xnum = (xnum+p[i])*xsq;
            xden = (xden+q[i])*xsq;
        }
        *result = xsq*(xnum+p[4])/(xden+q[4]);
        *result = (sqrpi-*result)/y;
        xsq = fifdint(x*sixten)/sixten;
        del = (x-xsq)*(x+xsq);
        *result = exp(-(xsq*xsq*half))*exp(-(del*half))**result;
        *ccum = one-*result;
        if(x > zero) {
            temp = *result;
            *result = *ccum;
            *ccum = temp;
        }
    }
    if(*result < min) *result = 0.0e0;
//
//  Fix up for negative argument, erf, etc.
//
    if(*ccum < min) *ccum = 0.0e0;
}
//****************************************************************************


double dinvnr ( double *p, double *q )

//****************************************************************************
// 
//  Purpose:
// 
//    DINVNR computes the inverse of the normal distribution.
//
//  Discussion:
//
//    Returns X such that CUMNOR(X)  =   P,  i.e., the  integral from -
//    infinity to X of (1/SQRT(2*PI)) EXP(-U*U/2) dU is P
//
//    The rational function on page 95 of Kennedy and Gentle is used as a start
//    value for the Newton method of finding roots.
//
//  Reference:
//
//    Kennedy and Gentle, 
//    Statistical Computing,
//    Marcel Dekker, NY, 1980,
//    QA276.4  K46
//
//  Parameters:
//
//    Input, double *P, *Q, the probability, and the complementary
//    probability.
//
//    Output, double DINVNR, the argument X for which the
//    Normal CDF has the value P.
//
{
#define maxit 100
#define eps (1.0e-13)
#define r2pi 0.3989422804014326e0
#define nhalf (-0.5e0)
#define dennor(x) (r2pi*exp(nhalf*(x)*(x)))
  static double dinvnr,strtx,xcur,cum,ccum,pp,dx;
  static int i;
  static unsigned long qporq;

//
//     FIND MINIMUM OF P AND Q
//
    qporq = *p <= *q;
    if(!qporq) goto S10;
    pp = *p;
    goto S20;
S10:
    pp = *q;
S20:
//
//     INITIALIZATION STEP
//
    strtx = stvaln(&pp);
    xcur = strtx;
//
//     NEWTON INTERATIONS
//
    for ( i = 1; i <= maxit; i++ )
    {
        cumnor(&xcur,&cum,&ccum);
        dx = (cum-pp)/dennor(xcur);
        xcur -= dx;
        if(fabs(dx/xcur) < eps) goto S40;
    }
    dinvnr = strtx;
//
//     IF WE GET HERE, NEWTON HAS FAILED
//
    if(!qporq) dinvnr = -dinvnr;
    return dinvnr;
S40:
//
//     IF WE GET HERE, NEWTON HAS SUCCEDED
//
    dinvnr = xcur;
    if(!qporq) dinvnr = -dinvnr;
    return dinvnr;
#undef maxit
#undef eps
#undef r2pi
#undef nhalf
#undef dennor
}
//****************************************************************************


int ipmpar ( int *i )

//****************************************************************************
//
//  Purpose:
//  
//    IPMPAR returns integer machine constants. 
//
//  Discussion:
//
//    Input arguments 1 through 3 are queries about integer arithmetic.
//    We assume integers are represented in the N-digit, base-A form
//
//      sign * ( X(N-1)*A**(N-1) + ... + X(1)*A + X(0) )
//
//    where 0 <= X(0:N-1) < A.
//
//    Then:
//
//      IPMPAR(1) = A, the base of integer arithmetic;
//      IPMPAR(2) = N, the number of base A digits;
//      IPMPAR(3) = A**N - 1, the largest magnitude.
//
//    It is assumed that the single and double precision floating
//    point arithmetics have the same base, say B, and that the
//    nonzero numbers are represented in the form
//
//      sign * (B**E) * (X(1)/B + ... + X(M)/B**M)
//
//    where X(1:M) is one of { 0, 1,..., B-1 }, and 1 <= X(1) and
//    EMIN <= E <= EMAX.
//
//    Input argument 4 is a query about the base of real arithmetic:
//
//      IPMPAR(4) = B, the base of single and double precision arithmetic.
//
//    Input arguments 5 through 7 are queries about single precision
//    floating point arithmetic:
//
//     IPMPAR(5) = M, the number of base B digits for single precision.
//     IPMPAR(6) = EMIN, the smallest exponent E for single precision.
//     IPMPAR(7) = EMAX, the largest exponent E for single precision.
//
//    Input arguments 8 through 10 are queries about double precision
//    floating point arithmetic:
//
//     IPMPAR(8) = M, the number of base B digits for double precision.
//     IPMPAR(9) = EMIN, the smallest exponent E for double precision.
//     IPMPAR(10) = EMAX, the largest exponent E for double precision.
//
//  Reference:
//
//    Fox, Hall, and Schryer,
//    Algorithm 528,
//    Framework for a Portable FORTRAN Subroutine Library,
//    ACM Transactions on Mathematical Software,
//    Volume 4, 1978, pages 176-188.
//
//  Parameters:
//
//    Input, int *I, the index of the desired constant.
//
//    Output, int IPMPAR, the value of the desired constant.
//
{
  static int imach[11];
  static int ipmpar;
//     MACHINE CONSTANTS FOR AMDAHL MACHINES. 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 16;
//   imach[5] = 6;
//   imach[6] = -64;
//   imach[7] = 63;
//   imach[8] = 14;
//   imach[9] = -64;
//   imach[10] = 63;
//
//     MACHINE CONSTANTS FOR THE AT&T 3B SERIES, AT&T
//       PC 7300, AND AT&T 6300. 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 2;
//   imach[5] = 24;
//   imach[6] = -125;
//   imach[7] = 128;
//   imach[8] = 53;
//   imach[9] = -1021;
//   imach[10] = 1024;
//
//     MACHINE CONSTANTS FOR THE BURROUGHS 1700 SYSTEM. 
//
//   imach[1] = 2;
//   imach[2] = 33;
//   imach[3] = 8589934591;
//   imach[4] = 2;
//   imach[5] = 24;
//   imach[6] = -256;
//   imach[7] = 255;
//   imach[8] = 60;
//   imach[9] = -256;
//   imach[10] = 255;
//
//     MACHINE CONSTANTS FOR THE BURROUGHS 5700 SYSTEM. 
//
//   imach[1] = 2;
//   imach[2] = 39;
//   imach[3] = 549755813887;
//   imach[4] = 8;
//   imach[5] = 13;
//   imach[6] = -50;
//   imach[7] = 76;
//   imach[8] = 26;
//   imach[9] = -50;
//   imach[10] = 76;
//
//     MACHINE CONSTANTS FOR THE BURROUGHS 6700/7700 SYSTEMS. 
//
//   imach[1] = 2;
//   imach[2] = 39;
//   imach[3] = 549755813887;
//   imach[4] = 8;
//   imach[5] = 13;
//   imach[6] = -50;
//   imach[7] = 76;
//   imach[8] = 26;
//   imach[9] = -32754;
//   imach[10] = 32780;
//
//     MACHINE CONSTANTS FOR THE CDC 6000/7000 SERIES
//       60 BIT ARITHMETIC, AND THE CDC CYBER 995 64 BIT
//       ARITHMETIC (NOS OPERATING SYSTEM). 
//
//   imach[1] = 2;
//   imach[2] = 48;
//   imach[3] = 281474976710655;
//   imach[4] = 2;
//   imach[5] = 48;
//   imach[6] = -974;
//   imach[7] = 1070;
//   imach[8] = 95;
//   imach[9] = -926;
//   imach[10] = 1070;
//
//     MACHINE CONSTANTS FOR THE CDC CYBER 995 64 BIT
//       ARITHMETIC (NOS/VE OPERATING SYSTEM). 
//
//   imach[1] = 2;
//   imach[2] = 63;
//   imach[3] = 9223372036854775807;
//   imach[4] = 2;
//   imach[5] = 48;
//   imach[6] = -4096;
//   imach[7] = 4095;
//   imach[8] = 96;
//   imach[9] = -4096;
//   imach[10] = 4095;
//
//     MACHINE CONSTANTS FOR THE CRAY 1, XMP, 2, AND 3. 
//
//   imach[1] = 2;
//   imach[2] = 63;
//   imach[3] = 9223372036854775807;
//   imach[4] = 2;
//   imach[5] = 47;
//   imach[6] = -8189;
//   imach[7] = 8190;
//   imach[8] = 94;
//   imach[9] = -8099;
//   imach[10] = 8190;
//
//     MACHINE CONSTANTS FOR THE DATA GENERAL ECLIPSE S/200. 
//
//   imach[1] = 2;
//   imach[2] = 15;
//   imach[3] = 32767;
//   imach[4] = 16;
//   imach[5] = 6;
//   imach[6] = -64;
//   imach[7] = 63;
//   imach[8] = 14;
//   imach[9] = -64;
//   imach[10] = 63;
//
//     MACHINE CONSTANTS FOR THE HARRIS 220. 
//
//   imach[1] = 2;
//   imach[2] = 23;
//   imach[3] = 8388607;
//   imach[4] = 2;
//   imach[5] = 23;
//   imach[6] = -127;
//   imach[7] = 127;
//   imach[8] = 38;
//   imach[9] = -127;
//   imach[10] = 127;
//
//     MACHINE CONSTANTS FOR THE HONEYWELL 600/6000
//       AND DPS 8/70 SERIES. 
//
//   imach[1] = 2;
//   imach[2] = 35;
//   imach[3] = 34359738367;
//   imach[4] = 2;
//   imach[5] = 27;
//   imach[6] = -127;
//   imach[7] = 127;
//   imach[8] = 63;
//   imach[9] = -127;
//   imach[10] = 127;
//
//     MACHINE CONSTANTS FOR THE HP 2100
//       3 WORD DOUBLE PRECISION OPTION WITH FTN4 
//
//   imach[1] = 2;
//   imach[2] = 15;
//   imach[3] = 32767;
//   imach[4] = 2;
//   imach[5] = 23;
//   imach[6] = -128;
//   imach[7] = 127;
//   imach[8] = 39;
//   imach[9] = -128;
//   imach[10] = 127;
//
//     MACHINE CONSTANTS FOR THE HP 2100
//       4 WORD DOUBLE PRECISION OPTION WITH FTN4 
//
//   imach[1] = 2;
//   imach[2] = 15;
//   imach[3] = 32767;
//   imach[4] = 2;
//   imach[5] = 23;
//   imach[6] = -128;
//   imach[7] = 127;
//   imach[8] = 55;
//   imach[9] = -128;
//   imach[10] = 127;
//
//     MACHINE CONSTANTS FOR THE HP 9000. 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 2;
//   imach[5] = 24;
//   imach[6] = -126;
//   imach[7] = 128;
//   imach[8] = 53;
//   imach[9] = -1021;
//   imach[10] = 1024;
//
//     MACHINE CONSTANTS FOR THE IBM 360/370 SERIES,
//       THE ICL 2900, THE ITEL AS/6, THE XEROX SIGMA
//       5/7/9 AND THE SEL SYSTEMS 85/86. 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 16;
//   imach[5] = 6;
//   imach[6] = -64;
//   imach[7] = 63;
//   imach[8] = 14;
//   imach[9] = -64;
//   imach[10] = 63;
//
//     MACHINE CONSTANTS FOR THE IBM PC. 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 2;
//   imach[5] = 24;
//   imach[6] = -125;
//   imach[7] = 128;
//   imach[8] = 53;
//   imach[9] = -1021;
//   imach[10] = 1024;
//
//     MACHINE CONSTANTS FOR THE MACINTOSH II - ABSOFT
//       MACFORTRAN II. 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 2;
//   imach[5] = 24;
//   imach[6] = -125;
//   imach[7] = 128;
//   imach[8] = 53;
//   imach[9] = -1021;
//   imach[10] = 1024;
//
//     MACHINE CONSTANTS FOR THE MICROVAX - VMS FORTRAN. 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 2;
//   imach[5] = 24;
//   imach[6] = -127;
//   imach[7] = 127;
//   imach[8] = 56;
//   imach[9] = -127;
//   imach[10] = 127;
//
//     MACHINE CONSTANTS FOR THE PDP-10 (KA PROCESSOR). 
//
//   imach[1] = 2;
//   imach[2] = 35;
//   imach[3] = 34359738367;
//   imach[4] = 2;
//   imach[5] = 27;
//   imach[6] = -128;
//   imach[7] = 127;
//   imach[8] = 54;
//   imach[9] = -101;
//   imach[10] = 127;
//
//     MACHINE CONSTANTS FOR THE PDP-10 (KI PROCESSOR). 
//
//   imach[1] = 2;
//   imach[2] = 35;
//   imach[3] = 34359738367;
//   imach[4] = 2;
//   imach[5] = 27;
//   imach[6] = -128;
//   imach[7] = 127;
//   imach[8] = 62;
//   imach[9] = -128;
//   imach[10] = 127;
//
//     MACHINE CONSTANTS FOR THE PDP-11 FORTRAN SUPPORTING
//       32-BIT INTEGER ARITHMETIC. 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 2;
//   imach[5] = 24;
//   imach[6] = -127;
//   imach[7] = 127;
//   imach[8] = 56;
//   imach[9] = -127;
//   imach[10] = 127;
//
//     MACHINE CONSTANTS FOR THE SEQUENT BALANCE 8000. 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 2;
//   imach[5] = 24;
//   imach[6] = -125;
//   imach[7] = 128;
//   imach[8] = 53;
//   imach[9] = -1021;
//   imach[10] = 1024;
//
//     MACHINE CONSTANTS FOR THE SILICON GRAPHICS IRIS-4D
//       SERIES (MIPS R3000 PROCESSOR). 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 2;
//   imach[5] = 24;
//   imach[6] = -125;
//   imach[7] = 128;
//   imach[8] = 53;
//   imach[9] = -1021;
//   imach[10] = 1024;
//
//     MACHINE CONSTANTS FOR IEEE ARITHMETIC MACHINES, SUCH AS THE AT&T
//       3B SERIES, MOTOROLA 68000 BASED MACHINES (E.G. SUN 3 AND AT&T
//       PC 7300), AND 8087 BASED MICROS (E.G. IBM PC AND AT&T 6300). 

   imach[1] = 2;
   imach[2] = 31;
   imach[3] = 2147483647;
   imach[4] = 2;
   imach[5] = 24;
   imach[6] = -125;
   imach[7] = 128;
   imach[8] = 53;
   imach[9] = -1021;
   imach[10] = 1024;

//     MACHINE CONSTANTS FOR THE UNIVAC 1100 SERIES. 
//
//   imach[1] = 2;
//   imach[2] = 35;
//   imach[3] = 34359738367;
//   imach[4] = 2;
//   imach[5] = 27;
//   imach[6] = -128;
//   imach[7] = 127;
//   imach[8] = 60;
//   imach[9] = -1024;
//   imach[10] = 1023;
//
//     MACHINE CONSTANTS FOR THE VAX 11/780. 
//
//   imach[1] = 2;
//   imach[2] = 31;
//   imach[3] = 2147483647;
//   imach[4] = 2;
//   imach[5] = 24;
//   imach[6] = -127;
//   imach[7] = 127;
//   imach[8] = 56;
//   imach[9] = -127;
//   imach[10] = 127;
//
    ipmpar = imach[*i];
    return ipmpar;
}
//******************************************************************************


double fifdint ( double a )

//****************************************************************************
//
//  Purpose:
// 
//    FIFDINT truncates a double number to an integer.
//
//  Parameters:
//
// a     -     number to be truncated 
{
  return (double) ((int) a);
}


double stvaln ( double *p )

//****************************************************************************
//
//  Purpose:
// 
//    STVALN provides starting values for the inverse of the normal distribution.
//
//  Discussion:
//
//    The routine returns X such that 
//      P = CUMNOR(X),  
//    that is, 
//      P = Integral from -infinity to X of (1/SQRT(2*PI)) EXP(-U*U/2) dU.
//
//  Reference:
//
//    Kennedy and Gentle,
//    Statistical Computing, 
//    Marcel Dekker, NY, 1980, page 95,
//    QA276.4  K46
//
//  Parameters:
//
//    Input, double *P, the probability whose normal deviate 
//    is sought.
//
//    Output, double STVALN, the normal deviate whose probability
//    is P.
//
{
  static double xden[5] = {
    0.993484626060e-1,0.588581570495e0,0.531103462366e0,0.103537752850e0,
    0.38560700634e-2
  };
  static double xnum[5] = {
    -0.322232431088e0,-1.000000000000e0,-0.342242088547e0,-0.204231210245e-1,
    -0.453642210148e-4
  };
  static int K1 = 5;
  static double stvaln,sign,y,z;

    if(!(*p <= 0.5e0)) goto S10;
    sign = -1.0e0;
    z = *p;
    goto S20;
S10:
    sign = 1.0e0;
    z = 1.0e0-*p;
S20:
    y = sqrt(-(2.0e0*log(z)));
    stvaln = y+ eval_pol ( xnum, &K1, &y ) / eval_pol ( xden, &K1, &y );
    stvaln = sign*stvaln;
    return stvaln;
}
//**********************************************************************

double eval_pol ( double a[], int *n, double *x )

//****************************************************************************
// 
//  Purpose:
// 
//    EVAL_POL evaluates a polynomial at X.
//
//  Discussion:
//
//    EVAL_POL = A(0) + A(1)*X + ... + A(N)*X**N
//
//  Modified:
//
//    15 December 1999
//
//  Parameters:
//
//    Input, double precision A(0:N), coefficients of the polynomial.
//
//    Input, int *N, length of A.
//
//    Input, double *X, the point at which the polynomial 
//    is to be evaluated.
//
//    Output, double EVAL_POL, the value of the polynomial at X.
//
{
  static double devlpl,term;
  static int i;

  term = a[*n-1];
  for ( i = *n-1-1; i >= 0; i-- )
  {
    term = a[i]+term**x;
  }

  devlpl = term;
  return devlpl;
}
//****************************************************************************


//****************************************************************************

void cdfnor ( int *which, double *p, double *q, double *x, double *mean,
  double *sd, int *status, double *bound )

//****************************************************************************
//
//  Purpose:
// 
//    CDFNOR evaluates the CDF of the Normal distribution.
//
//  Discussion:
//
//    A slightly modified version of ANORM from SPECFUN
//    is used to calculate the cumulative standard normal distribution.
//
//    The rational functions from pages 90-95 of Kennedy and Gentle
//    are used as starting values to Newton's Iterations which 
//    compute the inverse standard normal.  Therefore no searches are
//    necessary for any parameter.
//
//    For X < -15, the asymptotic expansion for the normal is used  as
//    the starting value in finding the inverse standard normal.
//
//    The normal density is proportional to
//    exp( - 0.5D+00 * (( X - MEAN)/SD)**2)
//
//  Reference:
//
//    Abramowitz and Stegun,  
//    Handbook of Mathematical Functions 
//    1966, Formula 26.2.12.
//
//    W J Cody,
//    Algorithm 715: SPECFUN - A Portable FORTRAN Package of
//      Special Function Routines and Test Drivers,
//    ACM Transactions on Mathematical Software,
//    Volume 19, pages 22-32, 1993.
//
//    Kennedy and Gentle,
//    Statistical Computing,
//    Marcel Dekker, NY, 1980,
//    QA276.4  K46
//
//  Parameters:
//
//    Input, int *WHICH, indicates which argument is to be calculated
//    from the others.
//    1: Calculate P and Q from X, MEAN and SD;
//    2: Calculate X from P, Q, MEAN and SD;
//    3: Calculate MEAN from P, Q, X and SD;
//    4: Calculate SD from P, Q, X and MEAN.
//
//    Input/output, double *P, the integral from -infinity to X 
//    of the Normal density.  If this is an input or output value, it will
//    lie in the range [0,1].
//
//    Input/output, double *Q, equal to 1-P.  If Q is an input
//    value, it should lie in the range [0,1].  If Q is an output value,
//    it will lie in the range [0,1].
//
//    Input/output, double *X, the upper limit of integration of 
//    the Normal density.
//
//    Input/output, double *MEAN, the mean of the Normal density.
//
//    Input/output, double *SD, the standard deviation of the 
//    Normal density.  If this is an input value, it should lie in the
//    range (0,+infinity).
//
//    Output, int *STATUS, the status of the calculation.
//    0, if calculation completed correctly;
//    -I, if input parameter number I is out of range;
//    1, if answer appears to be lower than lowest search bound;
//    2, if answer appears to be higher than greatest search bound;
//    3, if P + Q /= 1.
//
//    Output, double *BOUND, is only defined if STATUS is nonzero.
//    If STATUS is negative, then this is the value exceeded by parameter I.
//    if STATUS is 1 or 2, this is the search bound that was exceeded.
//
{
  static int K1 = 1;
  static double z,pq;

//
//     Check arguments
//
    *status = 0;
    if(!(*which < 1 || *which > 4)) goto S30;
    if(!(*which < 1)) goto S10;
    *bound = 1.0e0;
    goto S20;
S10:
    *bound = 4.0e0;
S20:
    *status = -1;
    return;
S30:
    if(*which == 1) goto S70;
//
//     P
//
    if(!(*p <= 0.0e0 || *p > 1.0e0)) goto S60;
    if(!(*p <= 0.0e0)) goto S40;
    *bound = 0.0e0;
    goto S50;
S40:
    *bound = 1.0e0;
S50:
    *status = -2;
    return;
S70:
S60:
    if(*which == 1) goto S110;
//
//     Q
//
    if(!(*q <= 0.0e0 || *q > 1.0e0)) goto S100;
    if(!(*q <= 0.0e0)) goto S80;
    *bound = 0.0e0;
    goto S90;
S80:
    *bound = 1.0e0;
S90:
    *status = -3;
    return;
S110:
S100:
    if(*which == 1) goto S150;
//
//     P + Q
//
    pq = *p+*q;
    if(!(fabs(pq-0.5e0-0.5e0) > 3.0e0*dpmpar(&K1))) goto S140;
    if(!(pq < 0.0e0)) goto S120;
    *bound = 0.0e0;
    goto S130;
S120:
    *bound = 1.0e0;
S130:
    *status = 3;
    return;
S150:
S140:
    if(*which == 4) goto S170;
//
//     SD
//
    if(!(*sd <= 0.0e0)) goto S160;
    *bound = 0.0e0;
    *status = -6;
    return;
S170:
S160:
//
//     Calculate ANSWERS
//
    if(1 == *which) {
//
//     Computing P
//
        z = (*x-*mean)/ *sd;
        cumnor(&z,p,q);
    }
    else if(2 == *which) {
//
//     Computing X
//
        z = dinvnr(p,q);
        *x = *sd*z+*mean;
    }
    else if(3 == *which) {
//
//     Computing the MEAN
//
        z = dinvnr(p,q);
        *mean = *x-*sd*z;
    }
    else if(4 == *which) {
//
//     Computing SD
//
        z = dinvnr(p,q);
        *sd = (*x-*mean)/z;
    }
    return;
}
//****************************************************************************
