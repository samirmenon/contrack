function [ bham ] = ctrGetBConstFromData(eigvec, BConsts)
%GETBCONSTTFROMDATA This function extracts the bingham constant for a
%specific eigenvalue and set of pre-computed bingham constants
%     [ bham ] = ctrGetBConstFromData(eigvec, BConsts)
% 
% Inputs:
%   eigvec : The eigenvectors of the ellipsoid for the Bingham distribution
% 
%  BConsts : The data structure with pre-computed constants. Contains
%            BConsts.const(i,j,k) (actual values), and BConsts.eigs(i)
%            (eigenvalues sampled).
% 
% Outuputs:
%     bham : The Bingham integration constant
% 
% HISTORY:
% 2014.03.03 SM: wrote it.
%Extract the indices into the pre-computed matrix
idx = zeros(3,1);
for i=1:3, 
  [val idx(i)] = min(BConsts.eigs-eigvec(i));
end

bham = BConsts.const(idx(1),idx(2),idx(3));

end

