function [ tensors ] = ctrExtractDWITensorsAlongPath(xyzTract, dt6, fib2voxXform )
%CTREXTRACTDWITENSORSALONGPATH Extracts the tensors along a set of fiber
%paths
%   Extracts diffusion tensors along a set of pathways

tensors = [];

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
end

end
