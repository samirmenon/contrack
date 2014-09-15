clear
i0 = [1:1:100];
i1 = [1:1:100];
i2 = [1:1:99];
fstr = '../../contrack/bham/BinghamConstantsCoarse_%d/BinghamConstantsCoarse_%d_%d/BinghamConstantsCoarse_%d_%d_%d.mat';
outfile = 'delme/bhamDataTmp%d.mat';
% bhamData will contain the three eigenvalues and the partition function of the Bham distrib
% i.e., it will contain the surface integral of the ellipsoid with the three eigenvalues
bhamData = zeros(length(i0),length(i1),length(i2),4);

%start at this point
is = 37;
ct = 3000;
load(sprintf(outfile,ct)); ct = ct+1;

% exec the loop
for i=is:1:max(i0),
  for j=i:1:max(i1),
    for k=j:1:max(i2),
      fname = sprintf(fstr,i,i,j,i,j,k);
      if exist(fname,'file'),
        d = load(fname);
        bhamData(d.i,d.j,d.k,1) = d.ar;
        bhamData(d.i,d.j,d.k,2) = d.e1;
        bhamData(d.i,d.j,d.k,3) = d.e2;
        bhamData(d.i,d.j,d.k,4) = d.e3;
      end
    end
    save(sprintf(outfile,ct),'bhamData');
    ct = ct+1;
  end
end

save('BhamDataFin.mat','bhamData');