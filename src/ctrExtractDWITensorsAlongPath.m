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
  D = [D(1), D(4), D(5);
       D(4), D(2), D(6);
       D(5), D(6), D(3)];
  tensors{j}.D = squeeze(D);
  %Test if this is a valid tensor
  if(min(eigs(D))<0)
    valid(j) = 0;
  else
    valid(j) = 1;
  end
end

end
