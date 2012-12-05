Samir's contrack mod.

Vista lab software:

white : File server + web server
* To get the data:  sftp://white/biac3/wandell4/users/smenon

azure : Matlab
peach : Matlab

================
To set up contrack with Eclipse:
0. Get StanfordTools-DWI (a subdir in mrVista)
1. Go to the Makefiles for contrack gen and score
2. Replace -O5 with -O0 -g -ggdb -pg
3. Recompile both
4. Run them with :
<base_dir>/contrack.git/data/sub100311rd/fibers/conTrack/opticRadiation$ cat ctrScript_20110827T152126.sh 
$ contrack_gen -i ctrSampler_20110827T152126.txt -p ltLGN_ltCalcFreesurfer_9_20110827T152126.pdb


TODO:

1. While comparing the tensor data, use nearest neighbour?
> Interpolate the diffusion tensors : Ask Bob/Jason for the code.

2. How best to load/store the diffusion tensors?
> dt6
> fgCompute (or something) : Computes dw tensors along points on each of the sample points along the the tract
> fgGets :

3. Compute this in xyz space or voxel space?
> Just copy contrack. (Add an option).

4. Is the xyz ras or acpc? 
> Most likely ras. Double check.

5. Ask for xform as argument or read from data struct?
> Explicitly ask for the xform along with its type.

> Just go with the matlab functions for 6-9
6. When best to compute the C normalizing constant for the bingham distrib?
7. When best to compute sigma_m? How exactly?
8. When best to compute sigmaC? How exactly?
9. When best to compute CW? How exactly?

10. PDS is for node. PS is for edges except boundary nodes

===================================

Code sharing : Set up the repository on Github.

===================================
