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

