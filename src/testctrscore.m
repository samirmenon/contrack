% This is a sample application to test an implementation of contrack
% 
% For the algorithm's details, please refer to:
% 
% ConTrack : Finding the most likely pathways between brain regions using
% diffusion tractography. Sherbondy, Dougherty, Ben-Shachar, Napel,
% Wandell, J' Vision 2008. 8(9):15, 1-16.
% 
% Author : SM <smenon@stanford.edu>
% Date   : 2013-02-08

clear;

%% Load fibers : Note that these are typically in ras xyz real-world coordinates.
fiber_file = '/home/samir/Code/fmri/contrack.git/data/sub100311rd/fibers/conTrack/opticRadiation/ltLGN_ltCalcFreesurfer_9_20110827T152126_top5000_edited.pdb';
fg = dtiLoadFiberGroup(fiber_file);

%% Plot the fibers for a single fiber to make sure everything looks good.
fib_id = 2;
figure(1);
plot3(fg.fibers{fib_id}(1,:),fg.fibers{fib_id}(2,:), fg.fibers{fib_id}(3,:)'.'); hold on;

%% Load the diffusion tensors
file_tensor = '/home/samir/Code/fmri/contrack.git/data/sub100311rd/dti06trilin/bin/tensors.nii.gz';
dwiData = load_nifti(file_tensor);

% Get tensors for a path
tensors = ctrExtractDWITensorsAlongPath(fg.fibers{fib_id},dwiData);

%% Plot the diffusion tensors for the path (just to see that all is well)
for i=1:size(fg.fibers{fib_id},2),
  % now plot the rotated ellipse
  [x, y, z] = ctrPlotGetPointSamplesOnEllipsoid( ...
    [fg.fibers{fib_id}(1,i),fg.fibers{fib_id}(2,i),fg.fibers{fib_id}(3,i)], tensors{i}.D);
  sc = surf(x,y,z);
  shading interp; alpha(0.5);
end

figure(1); hold off; grid on; axis equal; axis tight;