rm -rf Quench_linux*.zip
zip Quench_linux32.zip Quench32/Quench
zip Quench_linux64.zip Quench64/Quench
mv -f Quench_linux32.zip /var/www/html/software/downloads/DTI_visualization/.
mv -f Quench_linux64.zip /var/www/html/software/downloads/DTI_visualization/.
#
#Create the directory for backup
cd ../oldReleases
mkdir $(ls -l ../Quench32 | awk '{ print $6 }')$(ls -l ../Quench32 | awk '{ print $7}')_09
cd $(ls -l ../Quench32 | awk '{ print $6 }')$(ls -l ../Quench32 | awk '{ print $7}')_09
pwd
cp ../../Quench32 .
cp ../../Quench64 .
cp /var/www/html/software/downloads/DTI_visualization/Quench_win32.zip .
cd ../../tmp
pwd
#
#copy to old files
cp -f ../Quench32 ../Quench32Old
cp -f ../Quench64 ../Quench64Old
#
#copy the new files
cp -f Quench32/Quench ../Quench32
cp -f Quench64/Quench ../Quench64
cp -f ~shireesh/Quench_win32.zip /var/www/html/software/downloads/DTI_visualization/Quench_win32.zip 
