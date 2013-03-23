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

% Parameters from the standard run:
% 
% Params: 1
% Image Directory: /home/samir/Code/fmri/contrack.git/data/sub100311rd/dti06trilin/bin/ 
image_dir = '/home/samir/Code/fmri/contrack.git/data/fpContrackTestDataSet/dti150dirs_b2000/bin/';
% WM/GM Mask Filename: wmProb.nii.gz
wmgm_mask = 'wmProb.nii.gz';
% PDF Filename: pdf.nii.gz
pdf_file = 'pdf.nii.gz';
% ROI MASK Filename: ltLGN_ltCalcFreesurfer_9_20110827T152126.nii.gz
roi_file = 'anteriro_sudo_lgn_posterior_sudo_optic_radiation_2013-03-12_11.26.07.nii.gz';
% Desired Samples: 10000
% Max Pathway Nodes: 240
% Min Pathway Nodes: 3
% Step Size (mm): 1
% Start Is Seed VOI: true
% End Is Seed VOI: true
% Save Out Spacing: 50
% Threshold for WM/GM specification: 0.01
% Absorption Rate WM: 0
% Absorption Rate NotWM: 0
% Local Path Segment Smoothness Standard Deviation: 14
% Local Path Segment Angle Cutoff: 130
% ShapeFunc Params (LinMidCl,LinWidthCl,UniformS): [ 0.175, 0.15, 100 ]

% clear;

%% Load fibers : Note that these are typically in ras xyz real-world coordinates.
fiber_file = '/home/samir/Code/fmri/contrack.git/data/fpContrackTestDataSet/dti150dirs_b2000/fibers/conTrack/opticRadiation/contrack_output/ctrSamplerOutput.pdb';
fg = dtiLoadFiberGroup(fiber_file);

%% Plot the fibers for a single fiber to make sure everything looks good.
fib_id = 200;
figure(1);
plot3(fg.fibers{fib_id}(1,:),fg.fibers{fib_id}(2,:), fg.fibers{fib_id}(3,:)'.'); hold on;

%% Load the diffusion tensors
file_tensor = '/home/samir/Code/fmri/contrack.git/data/fpContrackTestDataSet/dti150dirs_b2000/bin/tensors.nii.gz';
dwiData = load_nifti(file_tensor); % Just to get the xform

%%
fib2voxXform = inv(dwiData.vox2ras);
[dt6, xformToAcpc, mmPerVoxel, fileName, desc, intentName] = dtiLoadTensorsFromNifti(file_tensor);

%% Plot the diffusion tensors for the path (just to see that all is well)
% Get tensors for a path
tensors = ctrExtractDWITensorsAlongPath(fg.fibers{fib_id}, dt6, fib2voxXform);

for i=1:size(fg.fibers{fib_id},2),
  % now plot the rotated ellipse
  [x, y, z] = ctrPlotGetPointSamplesOnEllipsoid( ...
    [fg.fibers{fib_id}(1,i),fg.fibers{fib_id}(2,i),fg.fibers{fib_id}(3,i)], tensors{i}.D);
  sc = surf(x,y,z);
  shading interp; alpha(0.5);
end

figure(1); hold off; grid on; axis equal; axis tight;

%% Now score the path.
tmpStructural = dwiData.vol(:,:,:,1,1);
[scores unstable] = contrack_score(fg, dt6, fib2voxXform, tmpStructural,  tmpStructural.*0 + 1);

