function [ tensors valid ] = ctrExtractDWITensorsAlongPath(xyzTract, dt6, fib2voxXform)
%CTREXTRACTDWITENSORSALONGPATH Extracts the tensors along a set of fiber
%paths
%   Extracts diffusion tensors along a set of pathways
% 
% Inputs :
% 
% xyzTract : The tract matrix: [[x0 y0 z0]' [x1 y1 z1]'... [xn yn zn]']
%      dt6 : The diffusion tensors across the brain.
% fib2voxXform : The xfrom from tract xyz space to fiber voxel space.
% 
% Outputs :
% 
% tensors : A struct array of tensors wrt each fiber point == tensor{i}.D
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
for j=1:size(fibXYZVoxNN,2),
  D = dt6(fibXYZVoxNN(1,j),fibXYZVoxNN(2,j),fibXYZVoxNN(3,j),:);
  D = squeeze(D);
  [ Dtensor valid ] = ctrGetDiffusionTensorFromVec( D );
  tensors{j}.D = squeeze(Dtensor);
  valid(j) = valid;
end

end
