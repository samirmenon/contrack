function rootPath=contrackRootPath()
%Determine path to root of the mrVista data directory.  
%
%        rootPath = contrackRootPath;
%
% This file and similarly named ones are used to identify the root
% directory of important repositories or files.
%
% This one is used as part of the tutorials in the mrScripts directory and
% for development of different tools.
%
% This function MUST reside in the directory at the base of the contrack
% directory structure.
%
% You should add contrack and all its subdirectories to your path.
%
% mrVista Team

rootPath=which('contrackRootPath');

rootPath=fileparts(rootPath);

return
