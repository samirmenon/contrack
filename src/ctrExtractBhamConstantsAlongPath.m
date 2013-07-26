function [ bham_constt ] = ctrExtractBhamConstantsAlongPath(xyzTract, dt6bham, fib2voxXform)
%CTREXTRACTBHAMCONSTANTSALONGPATH Extracts the Bingham integration constants 
% along a set of fiber paths.
% 
%   Extracts Binghamn integration constants along a set of pathways
% 
% Inputs :
% 
%     xyzTract : The tract matrix: [[x0 y0 z0]' [x1 y1 z1]'... [xn yn zn]']
%      dt6bham : The integrated bingham constant at a voxel.
% fib2voxXform : The xfrom from tract xyz space to fiber voxel space.
% 
% Outputs :
% 
%  bham_constt : An array of bingham constants at each fiber point
% 
% HISTORY:
% 2012.12.05 SM: wrote it.

tensors = [];
valid = [];

% Convert the tract's xyz coordinates (ras?) into voxel coordinates
fibXYZ = xyzTract;
fibXYZAff = [fibXYZ;ones(1,size(fibXYZ,2))];
fibXYZVox = fib2voxXform * fibXYZAff;
fibXYZVoxNN = round(fibXYZVox);

% Find the tensors along the tract's voxels
bham_constt = dt6bham(fibXYZVoxNN(1,:),fibXYZVoxNN(2,:),fibXYZVoxNN(3,:));

end
