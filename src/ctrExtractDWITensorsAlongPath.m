function [ tensors valid ] = ctrExtractDWITensorsAlongPath(xyzTract,dt6, fib2voxXform, CBcached, dt6eigVec, dt6eigVal)
%CTREXTRACTDWITENSORSALONGPATH Extracts the tensors along a set of fiber
%paths
%   Extracts diffusion tensors along a set of pathways
% 
% Inputs :
% 
%     xyzTract : The tract matrix: [[x0 y0 z0]' [x1 y1 z1]'... [xn yn zn]']
%          dt6 : The diffusion tensors across the brain.
% fib2voxXform : The xfrom from tract xyz space to fiber voxel space.
%     CBcached : The cached Bingham constants.
%    dt6eigVec : The dt6 eigenvectors. Could be passed in. Else these are 
%            obtained using [dt6eigVec dt6eigVal] = dtiSplitTensor(dt6);
%    dt6eigVal : The dt6 eigenvalues. Could be passed in. Else these are 
%            obtained using [dt6eigVec dt6eigVal] = dtiSplitTensor(dt6);
% 
% Outputs :
% 
% tensors : A struct array of tensors & other info at each fiber point
% 
% NOTE : It may be efficient to get the diffusion eigenvectors and values:
%        using: [dt6eigVec dt6eigVal] = dtiSplitTensor(dt6) in advance..
% 
% HISTORY:
% 2012.12.05 SM: wrote it.

tensors = [];
valid = [];

% Get the diffusion tensor eigenvalues and eigenvectors
if ~exist('dt6eigVec','var') || ~exist('dt6eigVal','var')
  dt6eigVec =[];
  dt6eigVal =[];
  [dt6eigVec dt6eigVal] = dtiSplitTensor(dt6);
end

% Convert the tract's xyz coordinates (ras?) into voxel coordinates
fibXYZ = xyzTract;
fibXYZAff = [fibXYZ;ones(1,size(fibXYZ,2))];
fibXYZVox = fib2voxXform * fibXYZAff;
fibXYZVoxNN = round(fibXYZVox);

% Find the tensors along the tract's voxels
for j=1:size(fibXYZVoxNN,2),
  D = dt6(fibXYZVoxNN(1,j),fibXYZVoxNN(2,j),fibXYZVoxNN(3,j),:);
  D = squeeze(D);
  [Dtensor valid_tmp] = ctrGetDiffusionTensorFromVec( D );
  tensors{j}.DT = squeeze(Dtensor);
  
  % Compute the eigenvectors and eigenvalues of the diffusion tensor
  v = squeeze(dt6eigVec(fibXYZVoxNN(1,j),fibXYZVoxNN(2,j),fibXYZVoxNN(3,j),:,:));
  d = squeeze(dt6eigVal(fibXYZVoxNN(1,j),fibXYZVoxNN(2,j),fibXYZVoxNN(3,j),:,:));
  tensors{j}.D{1} = v;
  tensors{j}.D{2} = d;
  tensors{j}.D{3} = sum(d);

  valid(j) = valid_tmp;
  
  % Extract C from the cached values.
  % The format is [e1 e2 e3 C]
  CBtmp = CBcached(:,[1 2 3]);
  CBtmp(:,1) = CBtmp(:,1) - d(1);
  CBtmp(:,2) = CBtmp(:,2) - d(2);
  CBtmp(:,3) = CBtmp(:,3) - d(3);
  twoNorm = sqrt(sum(abs(CBtmp).^2,2)); %# The two-norm of each column
  [delme, tnidx] = min(twoNorm);
  tensors{j}.C = CBcached(tnidx,4);
  
  % Extract C-Watson from the cached values.
  % The format is [e1 e2 e3 C]
  CBtmp(:,1) = CBtmp(:,1) - d(1);
  CBtmp(:,2) = CBtmp(:,2) - d(2);
  CBtmp(:,3) = CBtmp(:,3) - d(2);
  twoNorm = sqrt(sum(abs(CBtmp).^2,2)); %# The two-norm of each column
  [delme, tnidx] = min(twoNorm);
  tensors{j}.CWat = CBcached(tnidx,4);
end

end
