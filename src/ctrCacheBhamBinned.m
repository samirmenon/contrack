function [ ] = ctrCacheBhamBinned( filename, sample_res_sph, sample_res_eig, min_eig, max_eig, sge, sge_path)
%CTRCACHEBHAMBINNED Caches the Bingham and Watson distribution integration
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
%        min_eig : (Optional) The min eigenvalue. Default = sample_res_eig.
%        max_eig : (Optional) The max eigenvalue. Default = 3.
%
% Outputs:
%     A file with the constants for the specified range.
%
% HISTORY:
% 2013.09.22 SM: wrote it.

% Set default sampling resolution.
if notDefined('filename'),
    filename='BinghamConstants.mat';
end

if notDefined('sample_res_sph'),
    sample_res_sph=0.001;
end

if notDefined('sample_res_eig'),
    sample_res_eig=0.01;
end

if notDefined('min_eig'),
    min_eig=sample_res_eig;
end

if notDefined('max_eig'),
    max_eig=3; % Value provided by Ariel Rokem. Covers most dwi data at 3T.
end

if notDefined('sge_path'),
    sge_path='/hsgs/u/arokem/tmp/bham/';
end


% Integrate over a unit sphere. The poles (-pi:pi) are along the RAS z axis
dtheta = sample_res_sph; % Sampling resolution
dphi = sample_res_sph;   % Sampling resolution
r=1;            % Integrate over unit sphere in RAS (xyz) coordinates

eigval = min_eig:sample_res_eig:max_eig;
n = length(eigval);
BEigs = zeros(n,n,n,3);
BConstt = zeros(n,n,n);

for i=1:n,
    for j=1:n,
        for k=1:n,
            tic
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
            % Call out to helper function
            cmd_str = '[t, ar] = ctrBinghamGrid(t, D, ar, eigval, r, dtheta, dphi, i, j, k)';
            if sge
                name = sprintf('job_bingham_%s_%s_%s', num2str(i), num2str(j), num2str(k));
                filename = sprintf('%s/BinghamConstants_%s_%s_%s.mat', sge_path, num2str(i), num2str(j), num2str(k));
                cmd_str = [cmd_str ' ' filename];
                % We want to pass the namespace in with this
                sgerun2(cmd_str, name, true);
            else
                eval(cmd_str);
            end
            
            % If we are not sending this to the grid, we will overwrite the
            % file we have in every iteration:
            if ~sge
                % Set the Bingham constant to the integrated area.
                BConstt(i,j,k) = ar;
                disp(['Saving Bham for ' num2str([i j k])]);
                save(filename, 'BConstt','BEigs');
            end
            
        end
    end
end

save(filename, 'BConstt','BEigs');

end
