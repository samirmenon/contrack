function ar = ctrBinghamGrid(ar, eigval, dtheta, dphi, i, j, k, filename)

% Compute the Bingham score for a

disp(['Integrating Bham for ' num2str([eigval(i) eigval(j) eigval(k)]) ':']);
iter = 1;
iter10pc = round(round(2*pi/dtheta)/10);
% We provide ctrBinghamScore with the solution to the eigenvalue decomp
% problem:
D = [diag([eigval(i) eigval(j) eigval(k)]; eye(3)'];
% Unit vector relative to which integration proceeds, we'll muck with it
% below:
t = [1, 0, 0];
r=1;            % Integrate over unit sphere in RAS (xyz) coordinates

for theta = -pi:dtheta:pi, %From lower pole to upper pole
    for phi = 0:dphi:2*pi, %Integrate over a disk sliced moving along the pole
        % Direction along which to compute the BHAM score
        t(1) = r * cos(theta) * cos(phi); % x
        t(2) = r * cos(theta) * sin(phi); % y
        t(3) = r * sin(theta); % z
        
        % Patch area = height of disk * section-of-circumference
        dar = abs( (r * dtheta/2) * (dphi * r*cos(theta)) );
        
        % Note, we are just computing the raw function at the position t, so
        % we just pass 1 as the normalizing constant
        bhscore = ctrBinghamScore(t, D, 1, 3);
        
        darb = dar * bhscore; %At that point.
        ar = ar + darb;
    end
    iter = iter+1;
    if(rem(iter,iter10pc)==0)
        fprintf(' +10pc');
    end
end

if exist('filename')
    save(filename, 'ar', 'eigval', 'i', 'j', 'k');
end
