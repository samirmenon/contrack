function [ watScore ] = ctrWatsonScore(t, D, C, thetaSeg, pAngle)
%CTRWATSONSCORE Computes the Watson score at a point on a fiber tract
%  Arguments:
%   t: The tangent to the path at this point.
%   D: The diffusion tensor at the point along a tract
%   C: C(σ3, σ2). The normalizing constant that ensures the Bingham
%      distribution integrates to 1 over the unit sphere at this point.
%      Cached...
%             *** NOTE : C == 1/(bham-surface-integral) ***
% thetaSeg: (Optional). Returns zero of local segment angle is greater than
%           90 degs. This is only relevant while scoring (not for computing
%           the integration constant).
% pAngle: The angle (in rad) around the tangent direction to consider while
%         computing the probability score. 
%         NOTE : Default = 0.0873 rad (== 5deg)..
%         To make this clear, the probability that a tangent points along a
%         specific direction tends to zero. The probability that it points
%         along a finite zone is more reasonable.
%
%  Uses constants:
%   σm = pi*14/180; % User param. From paper (pg. 7 col. 2, para 1)
%   η = .175; % User param. From paper (pg. 7 col. 2, para 2)
%  
%  Returns:
%   watScore: C(σ) exp(cos(thetaSeg)^2 / sin(σ)^2)
% 
% NOTE : sigmaC is computed using the formulation for the Bingham score,
% with the additional constraint that both tensor eigenvalues in the
% formulation are assumed to be equal. See paper (pg. 4 col. 2, para 1).
% 
%  Uses constants:
%              σm = pi*14/180; % User param. From paper (pg. 7 col. 2, para 1)
%              η = .175; % User param. From paper (pg. 7 col. 2, para 2)
% 
%   watScore: C(σ3, σ2) exp(-(v3'*t / sin(σ3))^2 -(v2'*t / sin(σ2))^2)
%              with σ3 == σ2, we have:
%           = C(σ2, σ2) exp(-(v2'*t / sin(σ2))^2 -(v2'*t / sin(σ2))^2)
%           = C(σ2, σ2) exp(-(2*(v2'*t)^2 / sin(σ2))^2)
%           = C(σ2, σ2) exp(-(2*cos(tangent to eigvec angle)^2 / sin(σ2))^2)
% 
%              σi = σm + σi*
%              σi* = λi' / (λi + λi') * δ
%              δ = 100deg / ( 1 + exp(- (η - CL) / 0.015) )
%              CL = abs(eigval_1 - eigval_2) / sum(eigval_i)
%              (For CL def, see right-2nd para on page 4 of Contrack J'Vision 2008.)


% Set default sampling resolution.
if ~exist('thetaSeg'), 
  thetaSeg=0; 
end
 
% If t is a row vector resize it to be a col vector
if min(size(t) == [1 3]),
    t = t';
end

if ~exist('pAngle','var')
  pAngle = 5 * pi/180;
end

%Area over which to compute the tangent direction's probability.
%Area of the curved region = 2*pi*r*h
h = 1 - cos(pAngle); r = 1;
dar = 2*pi*r*h *2; %The final x2 is to include diametrically opposite sphere area..

% Requires a curve of 90 degrees or less.
if abs(thetaSeg) > pi/2,
  watScore = 0;
else 
  sigmaM = pi*14/180; % User param. From paper (pg. 7 col. 2, para 1)
  eta = .175; % User param. From paper (pg. 7 col. 2, para 2)
  
  %If the input D was a cell array, assume that it is {eigvals, eigvecs}:
  v = D{1};
  d = D{2};
  
  % CL: Tensor linearity index = abs(eig1 - eig2) / sum(eigs);
  % Right-2nd para on page 4 of Contrack J'Vision 2008.
  % "The linearity index is a measure of anisotropy, and is the positive
  % difference between the largest two eigenvalues of the diffusion tensor
  % divided by the sum of its eigenvalues"
  %    Value range : [0, 1)
  CL = abs(d(1) - d(2)) / (d(1)+2*d(2));
  
  % Compute delta = 100deg / (1+ exp( - (eta - CL) / 0.015 ) );
  % SM : We will move to radians instead of degrees. Original cpp is not
  % implemented with SI units (???).
  % 100 deg =  pi*100/180 rad = 1.7453 rad
  %
  %  NOTE : Discrepancy wrt. cpp file code:
  % double linshape_ds = uniform_s / (1+exp(-(linearityMidCl-p.fCl)*10/linearityWidthCl));
  % linearityMidCl = 0.174;  linearityWidthCl = .174 (differs from paper)
  % However, both codes give similar delta values in the end...
  %
  % Value range : CL=0,delta=100; CL=.1.8,delta=42; CL=0.3,delta=0ish...
  delta = (1.7453) / ( 1 + exp(- (eta - CL) / 0.015) ); % 100 is in degrees
  
  % Compute the term for eigenvector 2
  sigma2star = d(2) / ( d(2) + d(3) ) * delta;
  sigma2 = sigmaM + sigma2star;
  t2 = v(:,2)'*t; % == cos(tangent to eigvec angle)
  t2 = t2 / sin(sigma2);
  
  %Convert the score to a probability
  watScore = exp( -2*(t2*t2));  
  watScore = C * watScore * dar;
end

end

