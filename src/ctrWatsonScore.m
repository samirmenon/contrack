function [ watScore ] = ctrWatsonScore(CW, sigmaC, thetaSeg)
%CTWatsonSCORE Computes the Watson score at a point on a fiber tract
%  NOTE TODO : Replace cryptic argument names with descriptive ones.
%  Arguments:
%   CW: C(σ). The normalizing constant that ensures the Bingman
%      distribution integrates to 1 over the unit sphere at this point.
%   sigmaC: Angular Dispersion parameter for the dataset
%   thetaSeg: Angle between two segment chunks
%  
%  Returns:
%   watScore: C(σ) exp(cos(thetaSeg)^2 / sin(σ)^2)

% Requires a curve of 90 degrees or less.
if abs(thetaSeg) > pi/2,
  watScore = 0;
else
  watScore = CW* exp( cos(thetaSeg)^2 / sin(sigmaC)^2 );
end

end

