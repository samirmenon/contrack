%% Test file..
% This file generates some representative figures for the Bingham
% distribution. Could potentially help you understand the math..

% This plots the cpp data..
clear;

addpath([pwd '/../src']);

if ~exist('B','var'),  
  B = load('../data/bhamConst-0.002.txt');
  % Invert bham integral to convert it into a normalizing constt...
  B(:,4) = 1./B(:,4);
end

[d idx] = sort(B(:,1),'ascend');
Bs = B(idx,:);

%% Plot the data to be used by contrack
tlbl = {'Eigenvalue 1','Eigenvalue 2','Eigenvalue 3','Bham Integral'};

close all;
figure(1); hold on;
ii = 1:500:size(Bs,1); 
scatter3(Bs(ii,1),Bs(ii,2),Bs(ii,3),Bs(ii,4)*5,'k');

xlabel(tlbl{1});
ylabel(tlbl{2});
zlabel(tlbl{3});
title('Diffusion Eigenvalues vs. Surface Integral');
axis tight;
axis square;
grid on;

figname = ['./figs/Fig_BhamPartFunc'];

if(2 == exist('plot2svg','file'))
  plot2svg([figname '.svg']);
end
if(2==exist('export_fig','file'))
  export_fig ([figname '.png'], '-png');
end



%% Perfect contrack scenario. Simulated data. No distortion...
close all;
eigset = {[1 1 .001], [1 .1 .1], [1 .5 .1], [1 .5 .5], [1 1 .1], [1 1 .5], [1 1 1]};
for iii = 1:1:length(eigset),
  j=1;
  tensors = []; valid = [];
  
  D = eye(3);
  % The code sort of treats the max eigval to be 1
  D(1,1) = eigset{iii}(1); D(2,2) = eigset{iii}(2); D(3,3) = eigset{iii}(3);
  tensors{j}.DT = D;
  
  [v d] = eigs(D);
  d = [d(1,1) d(2,2) d(3,3)];
  tensors{j}.D{1} = v;
  tensors{j}.D{2} = d;
  tensors{j}.D{3} = sum(d);
  
  valid(j) = 1;
  
  % Extract C from the cached values.
  % The format is [e1 e2 e3 C]
  CBtmp = B(:,[1 2 3]);
  CBtmp(:,1) = CBtmp(:,1) - d(1);
  CBtmp(:,2) = CBtmp(:,2) - d(2);
  CBtmp(:,3) = CBtmp(:,3) - d(3);
  twoNorm = sqrt(sum(abs(CBtmp).^2,2)); %# The two-norm of each column
  [delme, tnidx] = min(twoNorm);
  tensors{j}.C = B(tnidx,4);
  
  % Now call the scoring algo. This should return a high score..
  samples = [0:0.03:1];
  sco = samples.*0;
  
  for i=1:1:length(samples),
    tau=samples(i);
    t = [(1-tau) tau 0];
    t = t./norm(t);
    
    D = tensors{j}.D;
    C = tensors{j}.C;
    
    % ******************* COMPUTE THE BINGHAM SCORE **********************
    sco(i) = ctrBinghamScore(t, D, C);
    % ****************** COMPUTED THE BINGHAM SCORE **********************
  end
  
  fignum = figure('XVisual',...
    '0x27 (TrueColor, depth 24, RGB mask 0xff0000 0xff00 0x00ff)',...
    'Color',[1 1 1]);
  hold off;
  plot(samples,sco,'k','LineWidth',2);
  box off;
  xlabel('Fiber direction.');
  set(gca, ...
  'XTick', [0 0.5 1], ...
  'XTickLabels', {'[1 0 0]', '[0.7071 0.7071 0]', '[0 1 0]'});
  ylabel('Bingham Score');
  title(sprintf('Bingham Score for Tensor Eigs (along x,y,z): %.3f, %.3f, %.3f',d(1), d(2), d(3)));
  
  figname = sprintf('./figs/Fig_BhamFuncVal_%.3f_%.3f_%.3f',d(1), d(2), d(3));
  if(2 == exist('plot2svg','file'))
    plot2svg([figname '.svg']);
  end
  if(2==exist('export_fig','file'))
    export_fig ([figname '.png'], '-png');
  end
end

%% Plot some interesting aspects of the algorithm.
% Compare the dispersion factor spec in the Contrack Paper and C++ code..
fignum = figure('XVisual',...
  '0x27 (TrueColor, depth 24, RGB mask 0xff0000 0xff00 0x00ff)',...
  'Color',[1 1 1]);

i=1; sco = []; 
sigmaM = 0.2443; % == 14 deg..pi*14/180 rad; User param. From paper (pg. 7 col. 2, para 1)
eta = .175; % User param. From paper (pg. 7 col. 2, para 2)
for CL = 0:0.01:1, 
  sco(i) = (pi*100/180) / ( 1 + exp(- (eta - CL) / 0.015) );i=i+1; 
end;  
subplot(2,1,1);
plot([0:0.01:1],sco);
xlabel('CL [ Tensor linearity index = eig(1)-eig(2) / sum(eigs) ]');
ylabel('delta [Dispersion factor]');
title('Delta computed as per Contrack paper spec');

i=1; sco = []; 
linearityMidCl = 0.174;  linearityWidthCl = .174;
uniform_s=pi*100/180;
for CL = 0:0.01:1, 
  sco(i) = uniform_s / (1+exp(-(linearityMidCl- CL)*10/linearityWidthCl));
  i=i+1; 
end;  
subplot(2,1,2);
plot([0:0.01:1],sco);
xlabel('CL [ Tensor linearity index = eig(1)-eig(2) / sum(eigs) ]');
ylabel('delta [Dispersion factor]');
title('Delta computed as per C++ code');

figname = sprintf('./figs/Fig_TensorLin');
if(2 == exist('plot2svg','file'))
  plot2svg([figname '.svg']);
end
if(2==exist('export_fig','file'))
  export_fig ([figname '.png'], '-png');
end