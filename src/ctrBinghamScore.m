function [ bhamScore ] = ctrBinghamScore(t, D, C)
%CTBINGHAMSCORE Computes the Bingham score at a point on a fiber tract
%  NOTE TODO : Replace cryptic argument names with descriptive ones.
%  Arguments:
%   t: The tangent to the path at this point.
%   D: The diffusion tensor at the point along a tract
%   C: C(σ3, σ2). The normalizing constant that ensures the Bingham
%      distribution integrates to 1 over the unit sphere at this point.
%  
%  Uses constants:
%              σm = pi*14/180; % User param. From paper (pg. 7 col. 2, para 1)
%              η = .175; % User param. From paper (pg. 7 col. 2, para 2)
%
%  Returns:
%   bhamScore: C(σ3, σ2) exp(-(v3'*t / sin(σ3))^2 -(v2'*t / sin(σ2))^2)
%              σi = σm + σi*
%              σi* = λi' / (λi + λi') * δ
%              δ = 100deg / ( 1 + exp(- (η - CL) / 0.015) )
%              CL = abs(eigval_1 - eigval_2) / sum(eigval_i)
% 
% HISTORY:
% 2012.12.05 SM: wrote it.

sigmaM = pi*14/180; % User param. From paper (pg. 7 col. 2, para 1)
eta = = .175; % User param. From paper (pg. 7 col. 2, para 2)

% Compute the eigenvectors and eigenvalues of the diffusion tensor
[v d] = eigs(D);

% CL: Tensor linearity index = abs(eig1 - eig2) / sum(eigs);
% Right-2nd para on page 4 of Contrack J'Vision 2008.
CL = abs(d(1,1) - d(2,2)) / trace(d);

% Compute delta = 100deg / (1+ exp( - (eta - CL) / 0.015 ) );
% SM : Do not move to radians instead of degrees. Original cpp is not
% implemented with SI units.
% In cpp file:
% double linshape_ds = uniform_s / (1+exp(-(linearityMidCl-p.fCl)*10/linearityWidthCl));
% linearityMidCl = 0.174;  linearityWidthCl = .174 (differs from paper)
delta = (100) / ( 1 + exp(- (eta - CL) / 0.015) ); % 100 is in degrees

% Compute the term for eigenvector 3
sigma3star = d(3,3) / ( d(2,2) + d(3,3) ) * delta;
sigma3 = sigmaM + sigma3star;
t3 = v(:,3)'*t;
t3 = t3 / sin(sigma3);

% Compute the term for eigenvector 2
sigma2star = d(2,2) / ( d(2,2) + d(3,3) ) * delta;
sigma2 = sigmaM + sigma2star;
t2 = v(:,2)'*t;
t2 = t2 / sin(sigma2);

bhamScore = C* exp( -(t3*t3) - (t2*t2));

end