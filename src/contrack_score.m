function [ scores, logscores, algo_unstable ] = contrack_score(fg, dt6, fib2voxXform, dwiROI, CBcached, dt6eigVec, dt6eigVal)
% Runs the Contrack score algorithm to rate a set of fiber tracts
%     [ scores, algo_unstable ] = contrack_score( fg, dt6, dwiSeg, dwiROI )
% 
% Inputs:
%   fg : A fiber group, which essentially summarizes a pdb file.
%        The pdb file is imported as `fgGet(fg,'fibers')` (struct with 
%        3xn-paths).
%        NOTE : This is typically loaded from a pdb file
%        >> fg = dtiLoadFiberGroup('fname.pdb')
% 
%   dt6 : The diffusion weighted data
% 
%   dwiROI : An ROI to avoid (passing this will give the fiber a zero
%            score). Size of full dwi data, where 0 indicates region to
%            avoid and 1 indicates acceptable region.
%
%  CBcached : The cached Bingham constants.
% 
% dt6eigVec : The dt6 eigenvectors. Could be passed in. Else these are 
%            obtained using [dt6eigVec dt6eigVal] = dtiSplitTensor(dt6);
% 
% dt6eigVal : The dt6 eigenvalues. Could be passed in. Else these are 
%            obtained using [dt6eigVec dt6eigVal] = dtiSplitTensor(dt6);
% 
% Outuputs:
%   score : A vector of n * 1, with scores for each fiber, ordered using
%           the same numerical index as the `fgGet(fg,'fibers')` input.
%logscore : A vector of n * 1, with log-scores for each fiber, ordered using
%           the same numerical index as the `fgGet(fg,'fibers')` input.
%           If the algo is unstable, this diverges from the score...
% algo_unstable : Whether this iteration of the algorithm was numerically
%                 unstable (log and normal scores diverge).
% 
% HISTORY:
% 2012.12.05 SM: wrote it.

% Count the fibers. Each will be scored independently
fgfibers = fgGet(fg,'fibers');
n_fibers = length(fgfibers);
% Size the scores
scores = zeros(n_fibers,1);
logscores = zeros(n_fibers,1);
algo_unstable = zeros(n_fibers,1);

% Compute the Bingham distribution constants
% sigmaM = pi*14/180; % User param. From paper (pg. 7 col. 2, para 1)
% eta = .175;         % User param. From paper (pg. 7 col. 2, para 2)

% Compute the Watson distribution constants
% NOTE TODO : Remove these; seem useless...
% CW = 1;     % Normalizing constant. NOTE TODO : Get this from the user.
% sigmaC = 1; % Angular dispersion 
lambda = exp(-2); % User length scoring param. From paper (pg. 7 col. 2, para 3)
loglambda = -2;  % User length scoring param. From paper (pg. 7 col. 2, para 3)
angleCutoff = 2.26; % radians = 129.488462 degrees

% Get the diffusion tensor eigenvalues and eigenvectors
if ~exist('dt6eigVec','var') || ~exist('dt6eigVal','var')
  [dt6eigVec dt6eigVal] = dtiSplitTensor(dt6);
end

for f_ctr=1:n_fibers,
  % Algorithm to compute the score :
  % Q(s) = p(D|s) p(s)
  %  s = Estimated pathway
  %  D = Raw diffusion data
  
  % Get the raw fiber data (xyz tangent vectors along a trajectory)
  fgf = fgfibers{f_ctr}; % size = 3xfiber-len
  fgftan = diff(fgfibers{f_ctr}')'; % size = 3xfiber-len - 1
  
  % Get the diffusion tensors along the fiber path
  tensors = ctrExtractDWITensorsAlongPath(fgf,dt6,fib2voxXform,CBcached, ...
    dt6eigVec,dt6eigVal);
  
  % Stage 1: Compute p(D|s) = Π_{i=1:n} [ p(Di | ti) ]
  % Di are diffusion tensors at each point along the pathway
  % ti are tangent vectors at each point along the pathway
  pds = 1;
  logpds = 0;
  %For each tangent vector along the length of the fiber
  for j=1:size(fgftan,2),
    % Compute the score for the tangent
    % NOTE : We use a j+1 addressing because the tangents are diff(points)
    pdt = ctrBinghamScore(fgftan(:,j), tensors{j+1}.D, tensors{j+1}.C);
    logpdt = ctrLogBinghamScore(fgftan(:,j), tensors{j+1}.D, tensors{j+1}.C);
    % Multiply the estimates for this point
    pds = pds * pdt;
    logpds = logpds + logpdt;
    % No need to loop if the score goes to zero somewhere in the middle
    % Check: We only abort if "both" normal and log scores go to zero.
    if((pds == 0) && (logpds == -Inf)), 
      scores(f_ctr) = 0; logscores(f_ctr)=-Inf; 
      break; 
    end;
  end
  if((pds == 0) && (logpds == -Inf)), 
    scores(f_ctr) = 0; logscores(f_ctr)=-Inf; 
    continue; 
  end;
  
  % Stage 2: Compute p(s) = pend(s1)*pend(sn) Π_{i=1:n} [ p(Di | ti) ]
  % Get the roi values along the fiber path (used in computing p).
  roi = ctrExtractROIAlongPath(fgf, dwiROI, fib2voxXform);
  % Make sure the fiber starts and ends in the ROI.
  pends1 = roi(1);
  pendsn = roi(end);
  
  % Initialize ps
  ps = pends1 * pendsn;
  logps = log(pends1 * pendsn); %Either 0 or -Inf. Bcoz ROI gives {0, 1}.
  
  % No need to loop if the score goes to zero somewhere in the middle
  % Check: We only abort if "both" normal and log scores go to zero.
  if((ps == 0) && (logps == -Inf)), 
    scores(f_ctr) = 0; logscores(f_ctr)=-Inf; 
    continue; 
  end;
  
  %For each tangent vector along the length of the fiber
  for j=2:size(fgf,2)-1,
    % Local angle between two edges of the tract
    segPre = (fgf(:,j) - fgf(:,j-1));
    segPre = segPre./norm(segPre);
    segPost = (fgf(:,j+1) - fgf(:,j));
    segPost = segPost./norm(segPost);
    % Inv cos of the dot product
    thetaSeg = acos(segPost'*segPre);
    % Apply a cut-off angle
    if( thetaSeg > angleCutoff ), ps = 0; break; end;
    
    % Compute the watson score 
    % NOTE : We use a j-1 addressing because the tangents are diff(points)
    pcurve = ctrWatsonScore(fgftan(:,j-1), tensors{j}.D, tensors{j+1}.C, thetaSeg);
    logpcurve = ctrLogWatsonScore(fgftan(:,j-1), tensors{j}.D, tensors{j+1}.C, thetaSeg);
    % Compute the manual knob and wm mask
    plen = lambda * roi(j); % If the ROI is zero, this will exit.
    logplen = loglambda + log(roi(j)); % If the ROI is zero, this will exit.
    % Multiply the estimates for this point
    ps = ps * pcurve * plen;
    logps = logps + logpcurve + logplen;
    % No need to loop if the score goes to zero somewhere in the middle
    % Check: We only abort if "both" normal and log scores go to zero.
    if((ps == 0) && (logps == -Inf)), break; end;
  end
  
  % Stage 3: Compute the score Q(s)
  cscore = pds * ps;
  clogscore = logpds + logps;
  %Error check. 
  if(abs(exp(clogscore) - cscore) > 0.00001 || cscore == 0),
    algo_unstable(f_ctr) = 1;
  else
    algo_unstable(f_ctr) = 0;
  end;
  scores(f_ctr) = cscore;
  logscores(f_ctr) = clogscore;
end

end

