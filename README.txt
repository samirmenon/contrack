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
2. How best to load/store the diffusion tensors?
3. Compute this in xyz space or voxel space?
4. Is the xyz ras or acpc? 
5. Ask for xform as argument or read from data struct?
6. When best to compute the C normalizing constant for the bingham distrib?
7. When best to compute sigma_m? How exactly?
8. When best to compute sigmaC? How exactly?
9. When best to compute CW? How exactly?
10. PDS is for node. PS is for edges except boundary nodes
11. 
