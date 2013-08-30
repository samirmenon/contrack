function [ Bconstt Wconstt ] = ctrGetBinghamIntegConstt( dt6, sampling_res, vox_range )
% CTRGENBINGHAMPATVOXELS Integrates the Bingham distribution over a sphere
% for each voxel and computes the normalizing constant at each point.
% 
%     [ Bconstt ] = ctrGenBinghamIntegConstt( tensors )
% 
% Inputs:
%          dt6 : mrVista's diffusion data structure containing the tensors.
% sampling_res : The integration step resolution. Default = 0.001.
%    vox_range : (Optional) Allows computing this for a subset of total vox
% 
% Outuputs:
%   Bconstt : The Bingham constant obtained by numerically integrating the
%             Bingham function over a sphere for each voxel.
%   Wconstt : The Watson constant obtained by numerically integrating the
%             Watson function over a sphere for each voxel.
% 
% HISTORY:
% 2013.06.12 SM: wrote it.

% Set default sampling resolution.
if ~exist('sampling_res'), 
  sampling_res=0.001; 
end

% Integrate over a unit sphere. The poles (-pi:pi) are along the RAS z axis
dtheta = sampling_res; % Sampling resolution
dphi = sampling_res;   % Sampling resolution
r=1;            % Integrate over unit sphere in RAS (xyz) coordinates

% Get the Bconstt value for each voxel
Bconstt = squish(dt6(:,:,:,1).*0,3);
data = squish(dt6,3);

% Set default sampling resolution.
if ~exist('vox_range'), 
  vox_range = [1:1:length(Bconstt)];
end

for i=vox_range,
  % Transform the diffusion vector values into a matrix and check whether
  % it is valid (has pos eigs)
  dvec = data(i,:);
  [ D valid ] = ctrGetDiffusionTensorFromVec( dvec );
  
  % Don't care about the invalid voxels
  if(valid == 0)
    Bconstt(i) = NaN;
    continue;
  end
  
  % Initialize vars.
  theta = 0;
  
  % Numerical integration (simple 1st order)
  ar = 0;        % Set the integrated area to zero at the start
  arw = 0;       % Set the integrated area to zero at the start
  t = [1 0 0];   % Unit vector along which to compute the Bham integration constt
  % theta = rotation along +y axis.
  % phi = rotation about +z axis
  % theta = 0, phi = 0 => x axis
  for theta = -pi:dtheta:pi, %From lower pole to upper pole
    for phi = 0:dphi:2*pi, %Integrate over a disk sliced moving along the pole
      % Direction along which to compute the BHAM score
      t(1) = r * cos(theta) * cos(phi); % x
      t(2) = r * cos(theta) * sin(phi); % y
      t(3) = r * sin(theta); % z
      
      % Patch area = height of disk * section-of-circumference
      dar = abs( (r*dtheta/2) * (dphi * r*cos(theta)) );
      
      % Note, we are just computing the raw function at the position t, so
      % we just pass 1 as the normalizing constant
      bhscore = ctrBinghamScore(t, D, 1);
      
      darb = dar * bhscore; %At that point.
      ar = ar + darb;
      
      % Note, we are just computing the raw function at the position t, so
      % we just pass 1 as the normalizing constant
      watscore = ctrWatsonScore(t, D, 1);
      
      darw = dar * watscore; %At that point.
      arw = arw + darw;
    end
  end
  
  % Set the integration constant
  Bconstt(i) = ar; % The integration constant.
  Wconstt(i) = arw; % The integration constant.
  i
end

%Turn the constants into a 3d data struct again to match dt6
Bconstt = reshape(Bconstt,[size(dt6,1) size(dt6,2) size(dt6,3)]);
Wconstt = reshape(Wconstt,[size(dt6,1) size(dt6,2) size(dt6,3)]);
end

