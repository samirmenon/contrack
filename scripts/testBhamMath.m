%% Test file..
% This file generates some representative figures for the Bingham
% distribution. Could potentially help you understand the math..

% This plots the cpp data..
clear;

addpath([pwd '/../src']);

if ~exist('B','var'),  
  B = load('../data/bhamConst-MixEigs-0.002.txt');
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

figname = ['./figs/Fig_BhamPartFunc.svg'];
plot2svg(figname);

%% Perfect contrack scenario. Simulated data. No distortion...

eigset = {[1 .5 .5], [1 .5 .1], [3 1.2 0.1]};
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
    t = [(1-tau) tau 0.001];
    t = t./norm(t);
    
    D = tensors{j}.D;
    C = tensors{j}.C;
    
    % **************** Actual Bham Computation Code **********************
    % If t is a row vector resize it to be a col vector
    if min(size(t) == [1 3]),
      t = t';
    end
    
    sigmaM = 0.2443; % == 14 deg..pi*14/180 rad; User param. From paper (pg. 7 col. 2, para 1)
    eta = .175; % User param. From paper (pg. 7 col. 2, para 2)
    
    % D is a cell array, assume that it is {eigvals, eigvecs, trace}:
    v = D{1};
    d = D{2};
    
    % CL: Tensor linearity index = abs(eig1 - eig2) / sum(eigs);
    % Right-2nd para on page 4 of Contrack J'Vision 2008.
    % "The linearity index is a measure of anisotropy, and is the positive
    % difference between the largest two eigenvalues of the diffusion tensor
    % divided by the sum of its eigenvalues"
    %    Value range : [0, 1)
    CL = abs(d(1) - d(2)) / D{3};
    
    % Compute delta = 100deg / (1+ exp( - (eta - CL) / 0.015 ) );
    % SM : We will move to radians instead of degrees. Original cpp is not
    % implemented with SI units (???).
    % 100 deg =  pi*100/180 rad = 1.7453 rad
    %
    %  NOTE : Discrepancy wrt. cpp file code:
    % double linshape_ds = uniform_s / (1+exp(-(linearityMidCl-p.fCl)*10/linearityWidthCl));
    % linearityMidCl = 0.174;  linearityWidthCl = .174 (differs from paper)
    % However, both codes give similar delta values in the end...
    %
    % Value range : CL=0,delta=100; CL=.1.8,delta=42; CL=0.3,delta=0ish...
    delta = 1.7453 / ( 1 + exp(- (eta - CL) / 0.015) ); % 100 in degrees
    
    % Compute the term for eigenvector 3
    sigma3star = d(3) / ( d(2) + d(3) ) * delta;
    sigma3 = sigmaM + sigma3star;
    t3 = v(:,3)'*t;
    t3 = t3 / sin(sigma3);
    
    % Compute the term for eigenvector 2
    sigma2star = d(2) / ( d(2) + d(3) ) * delta;
    sigma2 = sigmaM + sigma2star;
    t2 = v(:,2)'*t;
    t2 = t2 / sin(sigma2);
    
    % To understand the score, consider some cases:
    %
    % Case 1: Perfectly aligned with top eigenvector.
    %  t = e0, bhamscore = C * e(0) = C
    %
    % Case 2: Perfectly aligned with second eigvec, 2nd,3rd eigval are .5.
    %  t = e1, bhamscore = C * 1/sin(14deg + [0,100)deg)
    %
    bhamScore = C * exp( -(t3*t3) - (t2*t2));
    % **************** END : Actual Bham Computation Code **********************
    
    % Convert Bham score to probability...
    % Polar coords... In case you want the exact probability within a certain
    % nearby region (say +-5 degrees or something; area along both dirs)
    r = 1; %t must be a unit vector..
    angle_region = 5 * pi/180; %for +-5 deg
    h = 1 - cos(angle_region);
    dar = 2*pi*r*h;
    
    sco(i) = bhamScore * dar;
  end
  
  figure; hold off;
  plot(samples,sco,'k','LineWidth',2);
  box off;
  xlabel('tau: fiber dir = (1-tau, tau, 0.001)./norm');
  ylabel('Bingham Score');
  title(sprintf('Score changes for tensor eigs: %.3f, %.3f, %.3f',d(1), d(2), d(3)));
  
  figname = sprintf('./figs/Fig_BhamFuncVal_%.3f_%.3f_%.3f.svg',d(1), d(2), d(3));
  plot2svg(figname);
end

%% Plot some interesting aspects of the algorithm.
% Compare the dispersion factor spec in the Contrack Paper and C++ code..
figure;
i=1; sco = []; 
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

figname = sprintf('./figs/Fig_TensorLin.svg');
plot2svg(figname);