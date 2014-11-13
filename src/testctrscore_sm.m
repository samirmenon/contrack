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

%% Now load the cached Bingham constants
% Note that the stored values are surface integrals.
CBcached = load('../data/bhamConst-0.002.txt')  ;
% The contract paper uses the convention where the constant is 1/surf-integral
CBcached(:,4) = 1./CBcached(:,4);

%% Plot the diffusion tensors for the path (just to see that all is well)
% NOTE TODO : Replace me with the function "t_mrdFibers"

% Get tensors for a path
fignum = figure('XVisual',...
  '0x27 (TrueColor, depth 24, RGB mask 0xff0000 0xff00 0x00ff)',...
  'Color',[1 1 1]);
hold on;

% Get the diffusion tensor eigenvalues and eigenvectors
[dt6eigVec dt6eigVal] = dtiSplitTensor(dt6);

for fib_id=1:length(fg.fibers)/4:length(fg.fibers),
  fib_id = round(fib_id)
  % Plot the fibers for a single fiber to make sure everything looks good.
  plot3(fg.fibers{fib_id}(1,1:10:end),fg.fibers{fib_id}(2,1:10:end), ...
    fg.fibers{fib_id}(3,1:10:end)'.'); hold on;

  %Extract the tensors along the path and plot them.
  [tensors valid] = ctrExtractDWITensorsAlongPath(fg.fibers{fib_id}, dt6, ...
    fib2voxXform,CBcached,dt6eigVec, dt6eigVal);
  for i=1:10:size(fg.fibers{fib_id},2),
    % now plot the rotated ellipse
    [x, y, z] = ctrPlotGetPointSamplesOnEllipsoid( ...
      [fg.fibers{fib_id}(1,i),fg.fibers{fib_id}(2,i),fg.fibers{fib_id}(3,i)], tensors{i}.DT);
    sc = surf(x,y,z);
    shading interp; alpha(0.5);
  end
end

figure(fignum); hold off;
xlabel('X (mm, ras, red)');ylabel('Y (mm, ras, green)');zlabel('Z (mm, ras, blue)');
grid on; axis square;


%% Now score the path.
tmpStructural = dwiData.vol(:,:,:,1,1);
[scores logscores unstable] = contrack_score(fg, dt6, fib2voxXform,  ...
  tmpStructural.*0 + 1, CBcached, dt6eigVec, dt6eigVal);

%% Plot your scores.. Say 1/10th of them...
ll = logscores(1:10:end);

fignum = figure('XVisual',...
  '0x27 (TrueColor, depth 24, RGB mask 0xff0000 0xff00 0x00ff)',...
  'Color',[1 1 1]);

hist(ll,floor(length(ll)/10));
xlabel('Log(Contrack Score)');
ylabel(sprintf('N-Fibers (of %d)', length(ll)));
box off;
title('ltLGN\_ltCalcFreesurfer\_9\_20110827T152126\_top5000\_edited.pdb');

figname = sprintf('../scripts/figs/Fig_RD_CtrScore');
if(2 == exist('plot2svg','file'))
  plot2svg([figname '.svg']);
end
if(2==exist('export_fig','file'))
  export_fig ([figname '.png'], '-png');
end