% This will fit the Bingham distribution surface integral with a parametric
% function...
clear;
load('BhamDataFin.mat');

D = squish(bhamData,3);
% Unf, there are gaps in the dataset. Remove them.
Dnz = D(find(D(:,1) ~= 0),:);

%% Plot a few surfaces...
ii = 0; idx = 4;
% Selected indices for xyz axes (idx will be color)
idxsel = setdiff(1:1:4,idx);
tlbl = {'Eigenvalue 1','Eigenvalue 2','Eigenvalue 3','Bham Integral'};


close all;
figure(1); hold off;

scatter3(Dnz(:,idxsel(1)),Dnz(:,idxsel(2)),Dnz(:,idxsel(3)),...
  50,Dnz(:,idx), 'filled');
colorbar;
caxis([0 max(Dnz(:,idx))]);
xlabel(tlbl{idxsel(1)});
ylabel(tlbl{idxsel(2)});
zlabel(tlbl{idxsel(3)});
title( sprintf('Bham Eigenvalues vs. Surface Integral; %s on colorbar', tlbl{idx}) );
colormap copper