function [ scores ] = contrack_score( fg )
%CONTRACK_SCORE Summary of this function goes here
% Inputs:
%   fg : A fiber group, which essentially summarizes a pdb file.
%        The pdb file is imported as fg.fibers (struct with 3xn-paths)
%        NOTE : This is typically loaded from a pdb file
%        >> fg = dtiLoadFiberGroup('fname.pdb')
% 
% Outuputs:
%   score : A vector of n * 1, with scores for each fiber, ordered using
%           the same numerical index as the fg.fibers input.

% Count the fibers. Each will be scored independently
n_fibers = length(fg.fibers);
% Size the scores
scores = zeros(n_fibers,1);

for f_ctr=1:n_fibers,
  % Algorithm to compute the score :
  % Q(s) = p(D|s) p(s)
  %  s = Estimated pathway
  %  D = Raw diffusion data
  
  % Get the raw fiber data (xyz tangent vectors along a trajectory)
  % size = 3xfiber-len
  fgf = fg.fibers{f_ctr};

  % Stage 1: Compute p(D|s) = Π_{i=1:n} [ p(Di | ti) ]
  % Di are diffusion tensors at each point along the pathway
  % ti are tangent vectors at each point along the pathway
  pds = 1;
  %For each tangent vector along the length of the fiber
  for j=1:size(fgf,2),
    % Compute the score for the tangent
    pdt
    % Multiply the estimates for this point
    pds = pds * pdt;
  end
  
  % Stage 2: Compute p(s)
  
  % Stage 3: Compute the score Q(s)
  scores(f_ctr) = pds * ps;
end

end

