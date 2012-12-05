function [ bhamScore ] = ctrBinghamScore( D, C, t )
%CTBINGHAMSCORE Computes the Bingham score at a point on a fiber tract
%  Arguments:
%   D: The diffusion tensor at the point along a tract
%   C: C(σ3, σ2). The normalizing constant that ensures the Bingman
%      distribution integrates to 1 over the unit sphere at this point.
%   t: The tangent to the path at this point.
%  
%  Returns:
%   bhamScore: C(σ3, σ2) exp(-(v3'*t / sin(σ3))^2 -(v2'*t / sin(σ2))^2)

% Compute the eigenvectors and eigenvalues of the diffusion tensor
[v d] = eigs(D);

t1 = v(:,3)'*t;
t1 = t1 / sin(d(3,3));

t2 = v(:,2)'*t;
t2 = t2 / sin(d(2,2));

bhamScore = C* exp( -(t1*t1) - (t2*t2));

end

