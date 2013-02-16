function [ bhamScore ] = ctrBinghamScore(t, D, C, CL, sigmaM, eta )
%CTBINGHAMSCORE Computes the Bingham score at a point on a fiber tract
%  NOTE TODO : Replace cryptic argument names with descriptive ones.
%  Arguments:
%   t: The tangent to the path at this point.
%   D: The diffusion tensor at the point along a tract
%   C: C(σ3, σ2). The normalizing constant that ensures the Bingman
%      distribution integrates to 1 over the unit sphere at this point.
%   CL: Tensor linearity index
%   sigmaM: Dispersion parameter for the dataset
%   eta: User parameter that modulates the dispersion factor.
%  
%  Returns:
%   bhamScore: C(σ3, σ2) exp(-(v3'*t / sin(σ3))^2 -(v2'*t / sin(σ2))^2)
%              σi = σm + σi* 
%              σi* = λi' / (λi + λi') * δ
%              δ = 100deg / ( 1 + exp(- (η - CL) / 0.015) )

% Compute the eigenvectors and eigenvalues of the diffusion tensor
[v d] = eigs(D);

% Compute delta = 100deg / (1+ exp( - (eta - CL) / 0.015 ) );
% SM : Update. We move to radians instead of degrees.
delta = (100 * pi/180) / ( 1 + exp(- (eta - CL) / 0.015) );

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

