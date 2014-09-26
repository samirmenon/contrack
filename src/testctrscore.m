% This is a sample application to test an implementation of contrack
% 
% For the algorithm's details, please refer to:
% 
% ConTrack : Finding the most likely pathways between brain regions using
% diffusion tractography. Sherbondy, Dougherty, Ben-Shachar, Napel,
% Wandell, J' Vision 2008. 8(9):15, 1-16.
%
% We test against vistadata/conTrack.  Put vistadata on your path.
%
% Author : SM <smenon@stanford.edu>
% Date   : 2013-02-08
%


% Parameters from the standard run:
% 
% Params: 1
% Image Directory: /home/samir/Code/fmri/contrack.git/data/sub100311rd/dti06trilin/bin/ 
% image_dir = '/home/samir/Code/fmri/contrack.git/data/sub100311rd/dti06trilin/bin/';
conTrack_dir = fullfile(mrvDataRootPath,'conTrack','data');
image_dir = fullfile(conTrack_dir,'sub100311rd','dti06trilin','bin');

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
fiber_file = fullfile(conTrack_dir,'sub100311rd','fibers','conTrack','opticRadiation','ltLGN_ltCalcFreesurfer_9_20110827T152126_top5000_edited.pdb');
exist(fiber_file,'file')

fg = dtiLoadFiberGroup(fiber_file);

%% Load the diffusion tensors
file_tensor = fullfile(conTrack_dir,'sub100311rd','dti06trilin','bin','tensors.nii.gz');
dwiData = niftiRead(file_tensor); % Just to get the xform

fib2voxXform = inv(dwiData.qto_xyz); % Fibers are in ras
[dt6, xformToAcpc, mmPerVoxel, fileName, desc, intentName] = dtiLoadTensorsFromNifti(file_tensor);

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
fName = fullfile(contrackRootPath,'data','bhamConst-AllEigs-01-002-Full.txt');
CBcached = load(fName);
CBcached(:,4) = CBcached(:,4) ./ 4*pi;

% In the future, shift to a Matlab file.
save BinghamC CBcached

%% Now score the path.
tmpStructural = dwiData.data(:,:,:,1,1);
nFibers = fgGet(fg,'n fibers');

lst = zeros(1,nFibers);
lst(1:1000:nFibers) = 1;
lst = logical(lst)

fg2 = fgExtract(fg,lst,'keep');

%%
tic
[scores unstable] = contrack_score(fg2, dt6, fib2voxXform,  tmpStructural.*0 + 1, CBcached);
toc

