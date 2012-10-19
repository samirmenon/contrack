#!/bin/bash
rm -f contrack.tar.gz contrack.tar
tar -cf contrack.tar Makefile* DTIQuery contrack_score contrack_gen util nifti
gzip contrack.tar 
