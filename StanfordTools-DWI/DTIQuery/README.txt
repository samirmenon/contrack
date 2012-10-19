**** DTIQuery 1.10 README ****

This document describes mouse and keyboard shortcuts for using DTI-Query, 
and a short revision history. To learn how to use DTI-Query, please see  
our Visualization 2004 paper (and the related video illustrating its use):

http://graphics.stanford.edu/papers/dti-query/

Mouse operations:

	Left mouse - rotate
	Right mouse - zoom
	Shift + Left mouse - pan
	Ctrl + Left mouse - drag current VOI on current tomogram

Keyboard commands/shortcuts (note that all menu options also have keyboard
shortcuts -- these are not listed here):

	's', 'c', and 'a': select sagittal, coronal, or axial tomogram
	',' and '.': slide the current tomogram back and forth
	'b': cycle background color
	'k': toggle anti-aliasing
	'[': cycle between camera positions
	'w': toggle line widths
	't': toggle all VOI labels on/off

Revision History
----------------

DTI-Query 1.0 (August 17, 2005): 

* Initial version released.


DTI-Query 1.01 (September 20, 2005): 

* Fixed a bug in tensor interpolation code (DTIUtil/DTIVolume.cpp, lines
95-102). New sample data, binaries and code released. New data uses an FA
termination threshold of 0.15, as in our two published papers. (The
original release accidentally used a threshold of 0.1.)



DTI-Query 1.1 Beta (March 17, 2007): 

* Improved support for tensor data formats. (Now supports 5D tensor files, 
  and arbitrary assignment of tensor indices.) Has been tested with tensor 
  image files output by the AFNI tools.

* Support for MacOS X (Carbon)

* Binaries released for Windows, Linux (32- and 64-bit), and MacOS X (PPC 
  and Intel)

* Added choice of integration method (Euler's method, Runge Kutta 2nd 
  order, Runge Kutta 4th order) for STT tracking.

* Improved support for reading NIFTI background images (correctly handles 
  q-form/s-form matrices).

* Can now save visible pathways (as a binary format, or as a NIFTI mask image).

* Many minor GUI enhancements and bug-fixes.



DTI-Query 1.1 (November 3, 2007): 

* Pathways are now stored in voxel space (untransformed). On loading, they
are transformed by the matrix stored in the header. On saving, they are
transformed by the inverse of this matrix. 

* Fixed inconsistencies in data storage across multiple platforms, which
was making it impossible to transfer datasets computed on one platform to
another. NOTE that DTI-Query 1.1 beta cannot read the new 1.1 format, nor
can DTI-Query 1.1 read the old format. Apologies for the transition.

* In precomputation, added the ability to specify an "output label,"  
avoiding hard-coding the names of the output files (fa.nii.gz and
precomputed.pdb). This should make it easier to experiment with different 
algorithm settings.

* Fixed several bugs in lookup tables (would cause some background images
to appear completely black or white).

* Added the ability to cycle backwards through images (Ctrl-Shift-B).

* When loading new background images, can now append to the current set of 
images, or replace the existing set. (Previously, loading always 
appended.)

* Added a display of the name of the current background image (in the 
lower left hand corner of the window).

* Added the ability to specify an arbitrary "output prefix" (allows for 
outputting multiple .pdb files in the same directory with different 
algorithm settings). 
