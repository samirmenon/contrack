function [ Bconstt ] = ctrGetBinghamIntegConstt( dt6 )
% CTRGENBINGHAMPATVOXELS Integrates the Bingham distribution over a sphere
% for each voxel and computes the normalizing constant at each point.
% 
%     [ Bconstt ] = ctrGenBinghamIntegConstt( tensors )
% 
% Inputs:
%   dt6 : mrVista's diffusion data structure containing the tensors.
% 
% Outuputs:
%   Bconstt : The Bingham constant obtained by numerically integrating the
%             Bingham function over a sphere for each voxel.
% 
% HISTORY:
% 2013.06.12 SM: wrote it.

% Integrate over a unit sphere. The poles (-pi:pi) are along the RAS z axis
dtheta = 0.001; % Sampling resolution
dphi = 0.001;   % Sampling resolution
r=1;            % Integrate over unit sphere in RAS (xyz) coordinates

% Get the Bconstt value for each voxel
Bconstt = squish(dt6(:,:,:,1).*0,3);
data = squish(dt6,3);

for i=1:1:length(Bconstt),
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
  theta = 0; ar = 0;
  
  % Numerical integration (simple 1st order)
  ar = 0;        % Set the integrated area to zero at the start
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
      
      dar = dar * bhscore; %At that point.
      ar = ar + dar;
    end
  end
  
  % Set the integration constant
  Bconstt(i) = ar; % The integration constant.
  i
end
 
end

