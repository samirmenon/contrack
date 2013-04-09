function [ returned_value ] = normFB5CleanMeUpAndDeleteMe(k1, k2, der, hes, mode, level)
%NORMFB5CLEANMEUPANDDELETEME Contrack C++ Bingham code ported to Matlab
%   This was some C++ code to integrate over a Bingham distribution on a
%   sphere. Converted to matlab.
%   NOTE : This code needs to be cleaned up once it is verified to work!
%
% HISTORY:
% 2012.04.08 SM: ported it over from old contrack c++.

% Constants.
c1 = 15.74960995;
pi2 = 6.283185308;
c__9 = 9;
c__1 = 1;
c__3 = 3;
c__5 = 5;
c_b28 = 8.;
init = 0;

%/* System generated locals */
% i__1, i__2, i__3;
ret_val = 0.0;

%/* Local variables */
% dera, derb, fact, derd, derl, trma, trmb, term, fact0, term0;

a = zeros(10,1);
b = zeros(8,1);
d = zeros(8,1);
e = zeros(9,1);

% i, j, k, n;
% hesaa, hesab;
% ifail;
% hesbb;
cfold = zeros(9,1);
% y, termb, termd, terml, b2;
bb = zeros(9,1);
% ea,be;
dd = zeros(9,1);
cf = zeros(9,1);
% eb, de, al;

% add, eps;
imx = 0;
jmx = 0;
% sum, add0, bes0;


%   %/*    THIS FUNCTION CALCULATES THE NORMALIZATION CONSTANT FOR THE       BI
% 	N00050*/
%   %/*      BINGHAM DISTRIBUTION AND ITS DERIVATIVES WITH RESPECT TO        BI
% 	  N00060*/
%   %/*       THE PARAMETERS K1 AND K2.                                    BI
% 	   N00070*/
%   %/*                                                                      BI
% 									  N00080*/
%   %/*    INPUT:  K1 <= K2 <= 0   CONCENTRATION PARAMETERS                BI
% 	N00090*/
%   %/*            MODE = 1  USE TAYLOR SERIES EXPANSION                     BI
% 		N00100*/
%   %/*                   2  NORMAL-VON MISES ASYMPTOTIC SERIES              BI
% 		       N00110*/
%   %/*                   3  BIVARIATE NORMAL ASYMPTOTIC SERIES              BI
% 		       N00120*/
%   %/*             LEVEL >= 0  NUMBER OF TERMS USED IN ASYMPTOTIC SERIES    BI
% 		       N00130*/
%   %/*                         (MODES 2 AND 3)                              BI
% 			     N00140*/
%   %/*     OUTPUT: BINGC   NORMALIZATION CONSTANT                           BI
% 	 N00150*/
%   %/*             DER(2)  DERIVATIVES OF BINGC WRT  K1 AND K2            BI
% 		 N00160*/
%   %/*             HES(2,2) SECOND DERIVATIVES                              BI
% 		 N00170*/
%   %/*                      BASED ON POWER SERIES(MODE 1) OR THE DOMINANT   BI
% 			  N00180*/
%   %/*                      TERM IN THE ASYMPTOTIC EXPANSION(MODE = 2 OR 3) BI
% 			  N00190*/
%   %/*             LEVEL    NUMBER OF TERMS USED IN EACH SUM (MODE 1)       BI
% 		 N00200*/
%   %/*             IND = IFAIL = 1 IF SERIES DO NOT CONVERGE                BI
% 		 N00210*/
%/* Parameter adjustments */
%//	hes -= 3;
%// 	--der;

%/* Function Body */
%//    *ind = 0;
eps = 1e-6;
%/*    LEVUP = MAXIMUM NO OF LEVELS                                      BI
% 	      N00310*/
if (k1 > k2 || k2 > 0.0)
  ifail = 1;
  returned_value = ret_val;
  return;
end
%//     if (init == 1) {
%// 	goto L6;
%//     }
if (init ~= 1)
  init = 1;
  e(0+1) = 1.0;
  for i=1:1:8, %(i = 1; i <= 8; ++i)
    %/* L5: */
    e(i+1) = e(i - 1+1) * (i * 2.0 - 1.0);
  end
  %/*    THE E(I) REPRESENT THE 2*I'TH CENTRAL MOMENT OF THE STANDARD      BI
  % 	  N00430*/
  %/*    NORMAL DISTRIBUTION                                               BI
  % 	  N00440*/
end


%// L6:
%/* L100: */
%//     if (*mode != 1) {
%// 	goto L200;
%//     }

if (mode == 1)
  %/*                                                                      BI
  % 									    N00480*/
  %/*    NOW WE FIND BOUNDS ON THE NUMBER OF TERMS NEEDED FOR GIVEN        BI
  % 	  N00490*/
  %/*     ACCURACY IN TERMS OF THE EXP FUNCTION                            BI
  % 	   N00500*/
  %/*                                                                      BI
  % 									    N00510*/
  %//	*ind = 0;
  al = -(k1);
  be = -(k1) + k2;
  ea = exp(al);
  trma = al;
  for i=2:1:100, %(i = 2; i <= 100; ++i)
    trma = trma * al / i;
    if (i > al && trma / ea < eps)
      imx = i;
      break; %// goto L62;
    end
  end
  if (i == 101)
    error('I = 101. FIX ME!');
    % NOTE TODO : Implement these error messages
    %       cerr << "k1: " << k1 << " k2: " << k2 << endl;
    %       cerr << "L61: Normalization constant couldn't converge" << endl;
    keyboard;
    % exit(0);
  end
  %/* L61: */
  
  %// 	    *ind = 1;
  %//	L62:
  eb = exp(be);
  trmb = be;
  for i = 2:1:100,
    trmb = trmb * be / i;
    if (i > be && trmb / eb < eps)
      jmx = i;
      break; %// goto L64;
    end;
    %/* L63: */
  end;
  if (i == 101)
    error('L63: Normalization constant couldn''t converge');
    keyboard;
    %exit(0);
  end
  
  %//	*ind = 1;
  %//	L64:
  term0 = pi2 * 2;
  ret_val = 0.0;
  dera = 0.0;
  derb = 0.0;
  hesaa = 0.0;
  hesab = 0.0;
  hesbb = 0.0;
  if (abs(al) < 1e-15)
    al = 1e-15;
  end;
  if (abs(be) < 1e-15)
    be = 1e-15;
  end;
  %/*    BINGC = EXP(K1)*SUM( TERM(I,J) )  OVER I,J >=0                   BI
  % N00860*/
  %/*     TERM0 = TERM(I,0)                                                BI
  % N00870*/
  %/*                                                                      BI
  % 									    N00880*/
  i__1 = imx;
  for i=0:1:i__1, %(i = 0; i <= i__1; ++i)
    if (i >= 1)
      term0 = term0 * (i - 0.5) / (i + 0.5) / i * al;
    end
    term = term0;
    sum = term;
    dera = dera + term * i / al;
    hesaa = hesaa + term * i * (i - 1) / al / al;
    i__2 = jmx;
    for j=1:1:i__2, %(j = 1; j <= i__2; ++j)
      term = term * (j - .5) / ((i + j + .5) * j) * be;
      dera = dera + term * i / al;
      derb = derb + term * j / be;
      hesaa = hesaa + term * i * (i - 1) / al / al;
      hesab = hesab + term * i * j / al / be;
      hesbb = hesbb + term * j * (j - 1) / be / be;
      sum = sum + term;
      if (term < 1e-16 && j >= 2)
        break; %//    goto L34;
      end;
      %/* L32: */
    end
    %// L34:
    ret_val = ret_val + sum;
    if (sum < 1e-16 && i >= 2)
      break; %// goto L33;
    end;
    %/* L31: */
  end
  %//	L33:
  if(imx>jmx)
    level(0+1) = imx;
  else
    level(0+1) = jmx;
  end
  der(0+1) = 1 - (dera + derb) / ret_val;
  der(1+1) = derb / ret_val;
  b2 = ret_val * ret_val;
  hesaa = hesaa / ret_val - dera * dera / b2;
  hesab = hesab / ret_val - dera * derb / b2;
  hesbb = hesbb / ret_val - derb * derb / b2;
  hes(0+1) = hesaa + hesab * 2 + hesbb;
  hes(2+1) = -hesab - hesbb;
  hes(1+1) = hes(2+1);
  hes(3+1) = hesbb;
  ret_val = ret_val * exp(k1);
  
  %// exception already thrown if needed
  %// 	if (*ind == 1) {
  %// 	    cout << " NO CONVERGENCE IN BINGC; MODE 1\n";
  %// 	    exit(0);
  %// 	}
  returned_value = ret_val;
  return;
  %// end if mode == 1
  
  
  %// L200:
  %//    if (*mode != 2) {
  %//        goto L300;
  %//    }
elseif (mode == 2)
  
  %/*    A(K)=  BESRAT   (BE) = I (BE)/I   (BE)                            BI
  % 	  N01260*/
  %/*                 K-1        K      K-1                                BI
  % 		       N01270*/
  %/*    THEN SET                                                          BI
  % 	  N01280*/
  %/*     A(K) = K'TH MOMENT FOR VON MISES DSN                             BI
  % 	   N01290*/
  %/*          = I (BE)/I (BE) = OLD  A(1)...A(K)                          BI
  % 		N01300*/
  %/*             K      0                                                 BI
  % 		   N01310*/
  %/*                                                                      BI
  % 									    N01320*/
  al = k2 - k1 * 2.0;
  be = -(k2) / 2.0;
  if (al <= 0.001)
    ret_val = c1;
    der(0+1) = 3.0;
    der(1+1) = 1.0;
    hes(0+1) = 1.0;
    hes(2+1) = 0.0;
    hes(1+1) = 0.0;
    hes(3+1) = 1.0;
    returned_value = ret_val;
    return;
  end
  y = be * be / 4.0;
  bes0 = bstrs0(y);
  %/*           = I (BE) * 1 * EXP(-BE)                                    BI
  % 		 N01470*/
  %/*              0                                                       BI
  % 		    N01480*/
  %/*                                                                      BI
  % 			N01490*/
  a(9+1) = besrat(c_b28, be, eps);
  nosuchvarexists_idx = [8:-1:1];
  for i=nosuchvarexists_idx, %(i = 8; i >= 1; --i)
    %/* L1: */
    % "Extra smart" cpp code:
    %    a(i+1) = be / (be * a(i + 1+1) + (i << 1));
    %
    % Short C++ snippet to test it's behavior.
    %
    % #include "stdio.h"
    %
    % int main()
    % {
    %         double be = 1.0;
    %         double a[100];
    %         for(int i=0;i<100;++i) a[i] = 1.0;
    %
    %         for (int i=8; i>=1; --i){
    %         a[i] = be / (be * a[i + 1] + (i << 1));
    %         printf("\n%d %d %lf",i,i<<1, a[i]);
    %         }
    % }
    
    % Equivalent matlab code:
    a(i+1) = be / (be * a(i + 1+1) + bitshift(i,1));
  end
  for i = 2:1:9, %(i = 2; i <= 9; ++i)
    if (a(i - 1+1) < 1e-15)
      a(i - 1+1) = 0.;
    end
    %/* L2: */
    a(i+1) = a(i - 1+1) * a(i+1);
  end
  a(0+1) = 1.;
  bb(0+1) = a(1+1);
  for i=0:1:8, %(i = 0; i <= 8; ++i) {
    cfold(i+1) = 0.0;
    %/* L50: */
    cf(i+1) = 0.0;
  end
  cf(0+1) = 1.0;
  %/* L56: */
  term = 1.0;
  fact0 = 1.0;
  termb = bb(0+1);
  terml = 0.0;
  %// 	if (*level == 0) {
  %// 	    goto L8;
  %// 	}
  if (level(0+1) ~= 0)
    
    i__1 = level(0+1);
    for k = 1:1:i__1, %(k = 1; k <= i__1; ++k) {
      n = k;
      i__2 = n - 1;
      for i=0:1:i__2, %(i = 0; i <= i__2; ++i)
        %/* L52: */
        cfold(i+1) = cf(i+1);
      end
      if (n == 1)
        cf(0+1) = 0.0;
        cf(1+1) = 1.0;
      end
      if (n == 2)
        cf(0+1) = 0.5;
        cf(1+1) = 0.0;
        cf(2+1) = 0.5;
      end
      if (n > 2)
        cf(0+1) = cfold(1+1) / 2.0;
        cf(1+1) = cfold(0+1) + cfold(2+1) / 2.0;
        cf(n - 1+1) = cfold(n - 2+1) / 2.0;
        cf(n+1) = cfold(n - 1+1) / 2.0;
      end
      %// 		if (n <= 3) {
      %// 		    goto L54;
      %// 		}
      if (n > 3)
        i__2 = n - 2;
        for i=2:1:i__2, %(i = 2; i <= i__2; ++i)
          %/* L53: */
          cf(i+1) = (cfold(i - 1+1) + cfold(i + 1+1)) / 2.0;
        end
      end
      %//		L54:
      b(n - 1+1) = 0.0;
      bb(n+1) = 0.0;
      i__2 = n;
      for i=0:1:i__2, %(i = 0; i <= i__2; ++i) {
        if (i == 0)
          bb(n+1) = bb(n+1) + cf(0+1) * a(1+1);
        else
          bb(n+1) = bb(n+1) + cf(i+1) * (a(i - 1+1) + a(i + 1+1)) / 2.0;
        end
        %/* L55: */
        b(n - 1+1) = b(n - 1+1) + cf(i+1) * a(i+1);
      end
      if (k == 1)
        fact0 = fact0 * (-1 / al) / k;
      else
        fact0 = fact0 * (-be / al) / k;
      end
      fact = fact0 * be;
      termb = termb + e(k+1) * (b(k - 1+1) * fact0 * k + bb(k+1) * fact);
      terml = terml - k * e(k+1) * b(k - 1+1) * fact / al;
      add = e(k+1) * b(k - 1+1) * fact;
      term = term + add;
      if (abs(add) / term < 1e-15)
        break; %// goto L8;
      end
      %/* L3: */
    end
  end %// if level != 0
  
  %// 	L8:
  %/* L4: */
  ret_val = c1 / sqrt(al) * bes0 * term;
  derl = -0.5 / al + terml / term;
  derb = termb / term - 1;
  der(0+1) = derl * -2;
  der(1+1) = derl - derb * 0.5;
  hes(0+1) = 1.0 / (k1 * 2.0 * k1);
  hes(2+1) = 0.0;
  hes(1+1) = hes(2+1);
  hes(3+1) = (a(2+1) * 0.5 + 0.5 - a(1+1) * a(1+1)) / 4.0;
  returned_value = ret_val;
  return;
  
  %// end if mode == 2
else if (mode == 3)
    %// 	 L300:
    %// 	if (*mode != 3) {
    %// 	goto L400;
    %//     }
    al = k2 * -2.0;
    if (al == 0.0)
      ret_val = 1.0;
      der(0+1) = 1.0;
      der(1+1) = 1.0;
      hes(0+1) = 1.0;
      hes(2+1) = 0.0;
      hes(1+1) = 0.0;
      hes(3+1) = 1.0;
      returned_value = ret_val;
      return;
    end
    de = k2 / k1;
    for i=0:1:8, %(i = 0; i <= 8; ++i) {
      cf(i+1) = 0.0;
      %/* L41: */
      cfold(i+1) = 0.0;
    end
    cf(0+1) = 1.0;
    %/* L47: */
    term = 1.0;
    fact = 1.0;
    add = 1.0;
    termd = 0.0;
    terml = 0.0;
    %// 	if (*level == 0) {
    %// 	    goto L9;
    %// 	}
    if (level(0+1) ~= 0) %{
      i__1 = level(0+1);
      for k=1:1:i__1, %(k = 1; k <= i__1; ++k) {
        n = k;
        i__2 = n - 1;
        for i=0:1:i__2, %(i = 0; i <= i__2; ++i) {
          %/* L43: */
          cfold(i+1) = cf(i+1);
        end
        cf(0+1) = cfold(0+1);
        cf(n+1) = cfold(n - 1+1);
        %// 		if (n < 2) {
        %// 		    goto L44;
        %// 		}
        if (n >= 2)
          i__2 = n - 1;
          for i=1:1:i__2, %(i = 1; i <= i__2; ++i) {
            %/* L45: */
            cf(i+1) = cfold(i - 1+1) + cfold(i+1);
          end
          %/*    THE CF REPRESENT THE BINOMIAL COEFFICIENTS
          % 			      BIN02560*/
        end
        
        %//		L44:
        d(n - 1+1) = 0.0;
        dd(n+1) = 0.0;
        i__2 = n;
        for i=0:1:i__2, %(i = 0; i <= i__2; ++i)
          if (i > 0)
            i__3 = i - 1;
            dd(n+1) = dd(n+1) + i * cf(i+1) * e(i+1) * e(n - i+1) * pow(de, i__3);
          end
          %/* L46: */
          d(n - 1+1) = d(n - 1+1) + cf(i+1) * e(i+1) * e(n - i+1) * pow(de, i);
        end
        fact = -fact * (-0.5 - k + 1) / k / al;
        termd = termd + fact * dd(k+1);
        terml = terml - fact * d(k - 1+1) * k / al;
        add0 = add;
        add = fact * d(k - 1+1);
        term = term + add;
        if (abs(add) / term < 1e-10)
          break; %// goto L9;
        end
        %/* L12: */
      end
    end %// if level != 0
    
    %//	L9:
    %/* L11: */
    ret_val = 2 / al * sqrt(de) * pi2 * term;
    derl = -1 / al + terml / term;
    derd = 0.5 / de + termd / term;
    der(0+1) = derd * (-(k2)) / (k1 * k1);
    der(1+1) = derl * -2 + derd / k1;
    hes(0+1) = 1.0 / (k1 * 2.0 * k1);
    hes(2+1) = 0.0;
    hes(1+1) = 0.0;
    hes(3+1) = 1.0 / (k2 * 2.0 * k2);
    %/*    DER(1)=DERL                                                       BI
    % 		  N02810*/
    %/*    DER(2)=DERD                                                       BI
    % 		  N02820*/
    %/*    THIS GIVES DERIVATIVES WRT LAMBDA AND SPIKE                       BI
    % 		  N02830*/
    %/*                                                                      BI
    % 										    N02840*/
    returned_value = ret_val;
    return;
    
    %// end if mode == 3
    
  else
    %//	L400:
    error('INVALID MODE IN BINGC');
    returned_value = ret_val;
    return;
  end
end

