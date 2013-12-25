function [ bhamScore ] = ctrBinghamScore(t, D, C, trace_d)
%CTBINGHAMSCORE Computes the Bingham score at a point on a fiber tract
%  Arguments:
%   t: The tangent to the path at this point.
%   D: The diffusion tensor at the point along a tract
%   C: C(σ3, σ2). The normalizing constant that ensures the Bingham
%      distribution integrates to 1 over the unit sphere at this point.
%   trace_d : (optional), the trace of the eigenvector matrix of D
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
%              (For CL, see right-2nd para on page 4 of Contrack J'Vision 2008.)
%
% HISTORY:
% 2012.12.05 SM: wrote it.

% If t is a row vector resize it to be a col vector
if min(size(t) == [1 3]),
    t = t';
end

sigmaM = pi*14/180; % User param. From paper (pg. 7 col. 2, para 1)
eta = .175; % User param. From paper (pg. 7 col. 2, para 2)

%If the input D was a cell array, assume that it is {eigvals, eigvecs}:
if length(size(D))==3
    v = D(1);
    d = D(2);
else
    % Otherwise, if it's a matrix, compute the eigenvectors and eigenvalues of the diffusion tensor
    [v d] = eigs(D);
end

% CL: Tensor linearity index = abs(eig1 - eig2) / sum(eigs);
% Right-2nd para on page 4 of Contrack J'Vision 2008.

try
    % We use 'try/catch', because testing for the existence of a variable
    % takes too much time:
    CL = abs(d(1,1) - d(2,2)) / trace_d;
catch
    trace_d = trace(d);
    CL = abs(d(1,1) - d(2,2)) / trace_d;
end
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