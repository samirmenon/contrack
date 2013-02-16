function [ fiberROI ] = ctrExtractROIAlongPath( xyzTract, dwiROI, fib2voxXform )
%CTREXTRACTROIALONGPATH Extracts the roi values along a set of fiber
%paths
%   Extracts diffusion tensors along a set of pathways
% Inputs:
%     xyzTract : The fiber tract in xyz (usually RAS) coordinates.
% 
%       dwiROI : The ROI for this fiber set
% 
% fib2voxXform : The affine transformation matrix to convert fibers from 
%                xyz to ROI voxel space
% 
% Outuputs:
% fiberROI : A vector of n * 1, with 0 if fiber is in ROI. 1 if not.
% 
% HISTORY:
% 2012.02.15 SM: wrote it.

fiberROI = [];

% Convert the tract's xyz coordinates (ras?) into voxel coordinates
fibXYZ = xyzTract;
fibXYZAff = [fibXYZ;ones(1,size(fibXYZ,2))];
fibXYZVox = fib2voxXform * fibXYZAff;
fibXYZVoxNN = round(fibXYZVox);

% Find the roi along the tract's voxels
for j=1:size(fibXYZVoxNN,2),
  fiberROI(j) = dwiROI(fibXYZVoxNN(1,j),fibXYZVoxNN(2,j),fibXYZVoxNN(3,j));
end

end
