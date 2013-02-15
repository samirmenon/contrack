function [ scores ] = contrack_score(fg, dwiData, dwiSeg, dwiROI )
% Runs the Contrack score algorithm to rate a set of fiber tracts
%     [ scores ] = contrack_score( fg, dwiData, dwiSeg, dwiROI )
% 
% Inputs:
%   fg : A fiber group, which essentially summarizes a pdb file.
%        The pdb file is imported as fg.fibers (struct with 3xn-paths)
%        NOTE : This is typically loaded from a pdb file
%        >> fg = dtiLoadFiberGroup('fname.pdb')
% 
%   dwiData : The diffusion weighted data
% 
%   dwiSeg : A co-registered segmented volume. Could be higher res than the
%            diffusion data. Contains the ROI.
% 
%   dwiROI : An ROI to avoid (passing this will give the fiber a zero
%            score). A matrix of size dwiSeg, where 0 indicates region to
%            avoid and 1 indicates acceptable region.
% 
% Outuputs:
%   score : A vector of n * 1, with scores for each fiber, ordered using
%           the same numerical index as the fg.fibers input.
% HISTORY:
% 2012.12.05 SM: wrote it.

% Count the fibers. Each will be scored independently
n_fibers = length(fg.fibers);
% Size the scores
scores = zeros(n_fibers,1);

% Compute the Bingham distribution constants
% NOTE TODO : Get these from somewhere.
C =1;
CL = 1;
sigmaM = 1;
eta = 1;    % User specified parameter

% Compute the Watson distribution constants
sigmaC = 1; % Angular dispersion
CW = 1;     % Normalizing constant
lambda = 1; % User specified length scoring parameter

for f_ctr=1:n_fibers,
  % Algorithm to compute the score :
  % Q(s) = p(D|s) p(s)
  %  s = Estimated pathway
  %  D = Raw diffusion data
  
  % Get the raw fiber data (xyz tangent vectors along a trajectory)
  % size = 3xfiber-len
  fgf = fg.fibers{f_ctr};
  
  % Get the diffusion tensors along the fiber path
  tensors = ctrExtractDWITensorsAlongPath(fgf,dwiData);

  % Stage 1: Compute p(D|s) = Π_{i=1:n} [ p(Di | ti) ]
  % Di are diffusion tensors at each point along the pathway
  % ti are tangent vectors at each point along the pathway
  pds = 1;
  %For each tangent vector along the length of the fiber
  for j=1:size(fgf,2),
    % Compute the score for the tangent
    pdt = ctrBinghamScore(fgf(:,j), tensors{j}.D, C, CL, sigmaM, eta );
    % Multiply the estimates for this point
    pds = pds * pdt;
    % No need to loop if the score goes to zero somewhere in the middle
    if(pds == 0) scores(f_ctr) = 0; break; end;
  end
  if(pds == 0) scores(f_ctr) = 0; continue; end;
  
  % Stage 2: Compute p(s) = pend(s1)*pend(sn) Π_{i=1:n} [ p(Di | ti) ]
  % NOTE TODO : Computing these requires idx into ROI. Where to get that?
  pends1 = 1; %Fix this later
  pendsn = 1; %Fix this later
  ps = pends1 * pendsn;
  % No need to loop if the score goes to zero somewhere in the middle
  if(ps == 0) 
    scores(f_ctr) = 0; 
    continue;
  end
  
  %For each edge of the fiber
  for j=2:size(fgf,2)-1,
    % Local angule between two edges of the tract
    segPre = (fgf(:,j) - fgf(:,j-1));
    segPre = segPre./norm(segPre);
    segPost = (fgf(:,j+1) - fgf(:,j));
    segPost = segPost./norm(segPost);
    % Inv cos of the dot product
    thetaSeg = acos(segPost'*segPre);
    % Compute the watson score 
    pcurve = ctrWatsonScore(CW, sigmaC, thetaSeg);
    % NOTE TODO : Compute the manual knob and wm mask
    plen = 1; % Fix this later.
    % Multiply the estimates for this point
    ps = ps * pcurve * plen;
    % No need to loop if the score goes to zero somewhere in the middle
    if(ps == 0) scores(f_ctr) = 0; break; end;
  end
  if(ps == 0) scores(f_ctr) = 0; continue; end;
  
  % Stage 3: Compute the score Q(s)
  scores(f_ctr) = pds * ps;
end

end

