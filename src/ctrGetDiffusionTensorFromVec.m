function [ Dtensor valid ] = ctrGetDiffusionTensorFromVec( D )
%CTRGETDIFFUSIONTENSORFROMVEC Converts a vector into a matrix and returns it
% 
% Inputs :
% D : The vector of diffusion tensor coefficients
% 
% Outputs :
% Dtensor : A symmetric matrix 
%   valid : Whether the matrix eigenvalues are all positive
% 
% HISTORY:
% 2013.06.12 SM: wrote it.

Dtensor = [D(1), D(4), D(5);
  D(4), D(2), D(6);
  D(5), D(6), D(3)];

%Test if this is a valid diffusion tensor. All positive eigenvalues
if(min(eigs(Dtensor))<=0)
  valid = 0;
else
  valid = 1;
end

end

