function [ rat ] = besratCleanMeUpAndDeleteMe(v, x, eps)
%BESRATCLEANMEUPANDDELETEME Some Bessel function computation
%   Copied from old Contrack.
%
% HISTORY:
% 2012.04.08 SM: ported it over from old contrack c++.

%   /* Format strings */
%   //	String fmt_3 = "(\002 FAULTY RATIO\002,4(2x,f12.5))";

%   /* System generated locals */
%   int i__1;

%   /* Local variables */
%   int m, n;
%   double r, a1, u0, u1, y0, x2, y1, er, cx;
d1 = zeros(20,1);
rat = 0.0;

%   /*    THIS ROUTINE CALCULATES THE BESSEL FUNCTION RATIO                 BI
% 	N02920*/
%   /*                                                                      BI
% 									  N02930*/
%   /*           I   (X)/I (X)  = RAT, SAY                                  BI
% 	       N02940*/
%   /*            V+1     V                                                 BI
% 		N02950*/
%   /*                                                                      BI
% 									  N02960*/
%   /*      WHERE I(.) IS THE MODIFIED BESSEL FUNCTION,                     BI
% 	  N02970*/
%   /*      BY THE METHOD OF AMOS(1974)                                     BI
% 	  N02980*/
%   /*    INPUT:  V  ORDER OF THE BESSEL FUNCTION                           BI
% 	N02990*/
%   /*            X  ARGUMENT OF THE BESSEL FUNCTION                        BI
% 		N03000*/
%   /*            EPS  ACCURACY DESIRED                                     BI
% 		N03010*/
%   /*    OUTPUT: RAT   BESSEL FUNCTION RATIO                               BI
% 	N03020*/
%   /*                                                                      BI
% 									  N03030*/
x2 = x * x;
cx = (v + 1.0) * 2.0;
u0 = v + 1.5;
y0 = u0 * u0;
d1(0+1) = x / (u0 - 1. + sqrt(y0 + x2));
for n=1:1:19, %(n = 1; n <= 19; ++n)
  u0 = v + n + 1.5;
  y0 = u0 * u0;
  a1 = x / (u0 - 1.0 + sqrt(y0 + x2));
  i__1 = n;
  for m=1:1:i__1,% (m = 1; m <= i__1; ++m) {
    r = a1 / d1(m - 1+1);
    d1(m - 1+1) = a1;
    u1 = v + n - m + 1.0;
    y1 = u1 * u1;
    %/* L2: */
    a1 = x / (u1 + sqrt(y1 + x2 * r));
  end
  d1(n+1) = a1;
  rat = a1;
  er = x / (cx + x * d1(n - 1+1));
  er = er - a1;
  er = abs(er) / a1;
  %     /* L1: */
  if (er < eps)
    return;
  end
end
return;

end

