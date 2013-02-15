function [ tensors ] = ctrExtractDWITensorsAlongPath(xyzTractRAS, dwiData )
%CTREXTRACTDWITENSORSALONGPATH Extracts the tensors along a set of fiber
%paths
%   Extracts diffusion tensors along a set of pathways

tensors = [];

% Get vox positions for tensor x,y,z,1,6
% Remove the 1
dwiTensors = squeeze(dwiData.vol); 

%The fibers are in xyz. So convert the DWI data from voxels to xyz.
ras2vox = inv(dwiData.vox2ras); 

% Convert the tract's xyz coordinates (ras?) into voxel coordinates
fibXYZRAS = xyzTractRAS;
fibXYZAff = [fibXYZRAS;ones(1,size(fibXYZRAS,2))];
fibXYZVox = ras2vox * fibXYZAff;
fibXYZVoxNN = round(fibXYZVox);

% Find the tensors along the tract's voxels
for j=1:size(fibXYZVoxNN,2),
  D = dwiData.vol(fibXYZVoxNN(1,j),fibXYZVoxNN(2,j),fibXYZVoxNN(3,j),1,:);
  D = squeeze(D);
  D = [D(1), D(4), D(5);
       D(4), D(2), D(6);
       D(5), D(6), D(3)];
  tensors{j}.D = squeeze(D);
end

end
