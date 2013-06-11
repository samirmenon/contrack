function X=RandSampleSphere(N,spl)
% Generate a random or stratified sampling a unit sphere.
%
% INPUT ARGUMENTS:
%   - N   : desired number of point samples. N=200 is default.
%   - spl : can be 'stratified' or 'random'. The former setting is the
%           default.
%
% OUTPUT:
%   - X  : N-by-3 array of sample point coordinates.
%
% AUTHOR: Anton Semechko (a.semechko@gmail.com)
% DATE: June.2012
%

if nargin<1 || isempty(N), N=200; end
if nargin<2 || isempty(spl), spl='uniform'; end

switch spl
    
    case 'uniform'
        
        % Generate uniform (although random) sampling of the sphere
        z=2*rand(N,1)-1;
        t=2*pi*rand(N,1);
        r=sqrt(1-z.^2);
        x=r.*cos(t);
        y=r.*sin(t);
        
    case 'stratified'
        
        % Uniformly sample the unfolded right cylinder
        lon=2*pi*rand(N,1);
        z=2*rand(N,1)-1;
        
        % Convert z to latitude
        lat=acos(z);
        
        % Convert spherical to rectangular co-ords
        x=cos(lon).*sin(lat);
        y=sin(lon).*sin(lat);
    
    otherwise
        
        error('Invalid option')

end

X=[x,y,z];
