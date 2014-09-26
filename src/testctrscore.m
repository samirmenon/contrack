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
image_dir = '/home/samir/Code/fmri/contrack.git/data/sub100311rd/dti06trilin/bin/';
% WM/GM Mask Filename: wmProb.nii.gz
wmgm_mask = 'wmProb.nii.gz';
% PDF Filename: pdf.nii.gz
pdf_file = 'pdf.nii.gz';
% ROI MASK Filename: ltLGN_ltCalcFreesurfer_9_20110827T152126.nii.gz
roi_file = 'ltLGN_ltCalcFreesurfer_9_20110827T152126.nii.gz';
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
fiber_file = '/home/samir/Code/fmri/contrack.git/data/sub100311rd/fibers/conTrack/opticRadiation/ltLGN_ltCalcFreesurfer_9_20110827T152126_top5000_edited.pdb';
fg = dtiLoadFiberGroup(fiber_file);

%% Load the diffusion tensors
file_tensor = '/home/samir/Code/fmri/contrack.git/data/sub100311rd/dti06trilin/bin/tensors.nii.gz';
dwiData = load_nifti(file_tensor); % Just to get the xform
fib2voxXform = inv(dwiData.vox2ras); % Fibers are in ras
[dt6, xformToAcpc, mmPerVoxel, fileName, desc, intentName] = dtiLoadTensorsFromNifti(file_tensor);

%% Plot 3 slices so we know the rough alignment of the data planes
dtiAxSz = size(dwiData.vol);
dtiAxSz = dtiAxSz([1 2 3]);
figure(1); hold on;
% Plot the x-y plane
x = [[0 dtiAxSz(1)];[0 0]; [dtiAxSz(3) dtiAxSz(3)]./2; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'r', 'LineWidth',2);

x = [[0 dtiAxSz(1)]; [dtiAxSz(2) dtiAxSz(2)]; [dtiAxSz(3) dtiAxSz(3)]./2; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'r', 'LineWidth',2);

x = [[0 0]; [0 dtiAxSz(2)]; [dtiAxSz(3) dtiAxSz(3)]./2; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'g', 'LineWidth',2);

x = [[dtiAxSz(1) dtiAxSz(1)]; [0 dtiAxSz(2)]; [dtiAxSz(3) dtiAxSz(3)]./2; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'g', 'LineWidth',2);

% Plot the y-z plane
x = [[dtiAxSz(1) dtiAxSz(1)]./2;[0 0]; [0 dtiAxSz(3)]; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'b', 'LineWidth',2);

x = [[dtiAxSz(1) dtiAxSz(1)]./2; [dtiAxSz(2) dtiAxSz(2)]; [0 dtiAxSz(3)]; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'b', 'LineWidth',2);

x = [[dtiAxSz(1) dtiAxSz(1)]./2; [0 dtiAxSz(2)]; [0 0]; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'g', 'LineWidth',2);

x = [[dtiAxSz(1) dtiAxSz(1)]./2; [0 dtiAxSz(2)]; [dtiAxSz(3) dtiAxSz(3)]; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'g', 'LineWidth',2);

% Plot the x-z plane
x = [[0 0]; [dtiAxSz(2) dtiAxSz(2)]./2; [0 dtiAxSz(3)]; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'b', 'LineWidth',2);

x = [[dtiAxSz(1) dtiAxSz(1)]; [dtiAxSz(2) dtiAxSz(2)]./2; [0 dtiAxSz(3)]; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'b', 'LineWidth',2);

x = [[0 dtiAxSz(1)]; [dtiAxSz(2) dtiAxSz(2)]./2; [0 0]; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'r', 'LineWidth',2);

x = [[0 dtiAxSz(1)]; [dtiAxSz(2) dtiAxSz(2)]./2; [dtiAxSz(3) dtiAxSz(3)]; [1 1]];
x = dwiData.vox2ras * x;
plot3(x(1,:),x(2,:),x(3,:),'r', 'LineWidth',2);

%% Plot the diffusion tensors for the path (just to see that all is well)
% Get tensors for a path
fignum = figure('XVisual',...
  '0x27 (TrueColor, depth 24, RGB mask 0xff0000 0xff00 0x00ff)',...
  'Color',[1 1 1]);
hold on;
for fib_id=1:length(fg.fibers)/40:length(fg.fibers),
  fib_id = round(fib_id)
  % Plot the fibers for a single fiber to make sure everything looks good.
  plot3(fg.fibers{fib_id}(1,1:10:end),fg.fibers{fib_id}(2,1:10:end), fg.fibers{fib_id}(3,1:10:end)'.'); hold on;

  %Extract the tensors along the path and plot them.
  [tensors valid] = ctrExtractDWITensorsAlongPath(fg.fibers{fib_id}, dt6, fib2voxXform);
  for i=1:10:size(fg.fibers{fib_id},2),
    % now plot the rotated ellipse
    [x, y, z] = ctrPlotGetPointSamplesOnEllipsoid( ...
      [fg.fibers{fib_id}(1,i),fg.fibers{fib_id}(2,i),fg.fibers{fib_id}(3,i)], tensors{i}.D);
    sc = surf(x,y,z);
    shading interp; alpha(0.5);
  end
end

figure(fignum); hold off;
xlabel('X (mm, ras, red)');ylabel('Y (mm, ras, green)');zlabel('Z (mm, ras, blue)');
grid on; axis square;

%% Now load the cached Bingham constants
% Note that the stored values are surface integrals. Div by 4*pi
CBcached = load('../data/bhamConst-AllEigs-01-002-Full.txt')./4*pi;

%% Now score the path.
tmpStructural = dwiData.vol(:,:,:,1,1);
[scores unstable] = contrack_score(fg, dt6, fib2voxXform,  tmpStructural.*0 + 1, CBcached);

