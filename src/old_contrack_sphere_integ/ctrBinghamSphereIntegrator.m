function [ localLogLikelihood ] = ctrBinghamSphereIntegrator( D, thermalNoiseK, uniformityParam, linearityMidCl, linearityWidthCl, CL)
%CTRBINGHAMSPHEREINTEGRATOR This function computes the normalizing constant
%required for sampling a Bingham distribution over a sphere.
%  Arguments:
%                  D : The diffusion tensor for which to compute the
%                      spherical normalization constant.
%      thermalNoiseK : The temperature in Kelvin
%    uniformityParam : Needs description. 100 degrees. NOT S.I.
%     linearityMidCl : η in the paper
%   linearityWidthCl : 0.015 in the paper
%                 CL : Tensor linearity index
% 
% HISTORY:
% 2013.03 SM: wrote it.

%Hard wired constant in cpp code
if( thermalNoiseK > 205 || thermalNoiseK == 0)
  scannerNoiseK = thermalNoiseK;
end

[v d] = eig(D);

% From paper: δ = 100deg / ( 1 + exp(- (η - CL) / 0.015) )
delta = uniformityParam / (1+exp(-(linearityMidCl-CL)*10/linearityWidthCl));
eig3div23 = d(3,3) / (d(2,2)+d(3,3)); %Ratio of eig3 to 2+3

% Paper pg 4. sigma*(i) = delta * eig(i) / ( eig(2)+eig(3) )
sigmastar3 = delta*eig3div23;
sigmastar2 = delta*(1-eig3div23);

% Why is this in the cpp file?
%//  invsinsqsigma3 = 1 / ((sigmastar3*pi/180)*(sigmastar3*pi/180));
%//  invsinsqsigma2 = 1 / ((sigmastar2*pi/180)*(sigmastar2*pi/180));

% This is messy. SI units here (radian). Degrees elsewhere. Weird.
% NOTE TODO : Seems to set sigmam to zero.
% Paper formula = 1/ sin(sigmam + sigma*) ^ 2
invsinsqsigma3 = 1 / ((sin(sigmastar3*pi/180))*(sin(sigmastar3*pi/180)));
invsinsqsigma2 = 1 / ((sin(sigmastar2*pi/180))*(sin(sigmastar2*pi/180)));

% This might be playing the role of the sigmam
comp_k1 = scannerNoiseK*invsinsqsigma3 / (invsinsqsigma3 + scannerNoiseK + 2*sqrt(scannerNoiseK*invsinsqsigma3));
comp_k2 = scannerNoiseK*invsinsqsigma2 / (invsinsqsigma2 + scannerNoiseK + 2*sqrt(scannerNoiseK*invsinsqsigma2));

% The sum of these goes into the exponent function
k1 = -comp_k1;
k2 = -comp_k2;
if(k1 > -1 ), k1 = -1; end;
if(k2 > -1 ) k2 = -1; end;
if( k2 < -205 || k1 < -205 ) k2 = -205; k1 = -205; end;
if( k1 > k2 ) k1 = k2 - 0.01; end; %// This has only happened because of round off error

localLogLikelihood = computeLocalLogLikelihoodFB5(tangent,p.vE1,p.vE2,k1,k2);        
end

% ************************************************************************
% x = tangent vector. e1 = eig 1. e2 = eig 2. k1 and k2 are some weird
% scanner noise constants.
function [logL, angle, deviation] = computeLocalLogLikelihoodFB5(x, e1, e2, k1, k2)
cosTheta = x. e1';
deviation = k1;

if(cosTheta > 1)
  angle = 0;
elseif( cosTheta < -1)
  angle = pi;
else
  angle = acos(cosTheta);
end
logL= logFB5PDF(x,e1,e2,k1,k2) - 2.53; % log(1/(4*PI))
end

% ************************************************************************
% 
function [ logL ] = logFB5PDF(x, e1, e2, k1, k2)
% Find normalization
log_A = log( normFB5(k1, k2) );

logL = logFB5PDF(x, e1, e2, k1, k2, log_A);
end

% ************************************************************************
%
function [ A ] = normFB5(k1, k2)
A = 0;
if (k2 > 0.0 || k1 > k2)
  tmp_errmsg = sprintf('llegal concentration parameters {k1, k2} = { %f, %f}',k1,k2);
  error(tmp_errmsg);
  return;
end
der = zeros(2,1);
hes = zeros(2,1);
level = 8;

%// from Kent's paper
if (k2 < -8.5)
  mode = 3;
else
  mode = 1;
  if (k1 <= -10.0 && k1 / k2 >= 2.0)
    mode = 2;
  end
end
A = normFB5CleanMeUpAndDeleteMe(k1, k2, der, hes, mode, level) / (4.0 * pi);
end

% ************************************************************************
%