function [ x, y, z ] = ctrPlotGetPointSamplesOnEllipsoid(center, Cov, n)
%CTRPLOTGETPOINTSAMPLESONELLIPSOID Samples points on an ellipsoid
%   Given a mean and covariance matrix, this function generates a set of
%   points on the corresponding spatial ellipsoid .
% Arguments:
%  center : The center (xyz vector) of the ellipsoid
%     Cov : The covariance matrix of the ellipsoid
%       n : The number of points to generate (uniformly distributed)

if ~exist('n'),
  n = 20;
end

% If X is N×N real symmetric matrix with non-negative eigenvalues, 
% then eigenvalues and singular values coincide.
% Also, since Cov is symmetric, U = V.
[U S V] = svd(Cov); 
% [U,L] = eig(Cov);

% For N standard deviations spread of data, the radii of the eliipsoid will
% be given by N*SQRT(eigenvalues).

N = 1; % choose your own N
radii = N*sqrt(diag(S));
% radii = N*sqrt(diag(L));

% Generate data for "unrotated" ellipsoid. Ie. Only scale the axes.
% [xc(i,j), yc(i,j), zc(i,j)] is a point on the ellipsoid.
[xc,yc,zc] = ellipsoid(0,0,0,radii(1),radii(2),radii(3), n);

% rotate data with orientation matrix U and center
a = kron(U(:,1),xc); 
b = kron(U(:,2),yc); 
c = kron(U(:,3),zc);

data = a+b+c; 
n = size(data,2);

x = data(1:n,:)+center(1); 
y = data(n+1:2*n,:)+center(2); 
z = data(2*n+1:end,:)+center(3);

end

