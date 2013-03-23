function [ output_args ] = ctrBinghamSphereIntegrator( D, thermalNoiseK, uniformityParam, linearityMidCl, linearityWidthCl, CL)
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

localLogLikelihood = computeLocalLogLikelihoodFB5(tangent,p.vE1,p.vE2,k1,k2,angle,deviation);
        
        
end

