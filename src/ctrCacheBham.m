function [ ] = ctrCacheBham( filename, sample_res_sph, sample_res_eig )
%CTRCACHEBHAM Caches the Bingham and Watson distribution integration
% constants for a variety of tensors.
% 
% The Watson score is the Bingham score when the second and third
% eigenvalues are equal, so it is subsumed in this computation.
% 
% The tensor values are based on: 
% https://vistalab.stanford.edu/newlm/index.php/ConTrack_Score
% (see the bottom of the wiki page).
% Essentially, we will span a range of eigenvalues from 0.01 to 3.
% 
% Users may compute this at arbitrary numerical precision by specifying the
% inputs.
% 
% Inputs:
%       filename : (Optional) File for saving Bingham integration constnts
% sample_res_sph : (Optional) The integration step resolution. Default = 0.001.
% sample_res_eig : (Optional) The eigenvalue resolution. Default = 0.01.
% 
% Outuputs: 
%     A file with the constants for the specified range.
% 
% HISTORY:
% 2013.09.22 SM: wrote it.

% Set default sampling resolution.
if ~exist('filename'), 
  filename='BinghamConstants.mat'; 
end

if ~exist('sample_res_sph'),
  sample_res_sph=0.001; 
end

if ~exist('sample_res_eig'), 
  sample_res_eig=0.01; 
end

% Integrate over a unit sphere. The poles (-pi:pi) are along the RAS z axis
dtheta = sample_res_sph; % Sampling resolution
dphi = sample_res_sph;   % Sampling resolution
r=1;            % Integrate over unit sphere in RAS (xyz) coordinates

eigval = sample_res_eig:sample_res_eig:3;
n = length(eigval);
BEigs = zeros(n,n,n,3);
BConstt = zeros(n,n,n);

for i=1:n,
  for j=1:n,
    for k=1:n,
      % Three eigenvalues to be used
      BEigs(i,j,k,1) = eigval(i);
      BEigs(i,j,k,2) = eigval(j);
      BEigs(i,j,k,3) = eigval(k);
      
      D = eye(3);
      D(1,1) = eigval(i);
      D(2,2) = eigval(j);
      D(3,3) = eigval(k);
      
      % Numerical integration (simple 1st order)
      % Pre-computes the integration constants for Bingham
      theta = 0;
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
        end
      end
      
      % Set the Bingham constant to the integrated area.
      BConstt(i,j,k) = ar;
    end
  end
end

save(filename, 'BConstt','BEigs');

end
