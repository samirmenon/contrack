#!/bin/bash

#Quench install script adapted from getgray.sh. 
##
#Written by Shireesh Agrawal

# -------------------------------------------------------------
# Options that the user can supply
# -------------------------------------------------------------

# The directory where the build will take placec
basedir="`pwd`"

# The installation directory, defaults to $basedir/install; 
# executable goes here
#instdir=$basedir/install

# The location of the build log file
logfile=$basedir/buildlog

# Release versions for the supporting software. Change to HEAD to get
# the most current version of the code
CMAKE_REL="HEAD"
VTK_REL="HEAD"
GRAY_REL="HEAD"

# -------------------------------------------------------------
# Check for necessary software
# -------------------------------------------------------------
for prog in "svn" "g++" "autoconf" "make" "git"
do
  $prog --version > /dev/null
  if [ $? -ne 0 ]
  then
    echo "Can not execute $prog on your system"
    echo "Please ask the administrator to install $prog and to "
    echo "make sure that $prog is in your path"
    exit
  fi
done

# -------------------------------------------------------------
# Initialization
# -------------------------------------------------------------
echo "PLEASE READ THE COMMENTS AT THE TOP OF THIS SCRIPT!!!"
echo "Preparing to build quench. This will take 1-2 hours and 500MB!"
echo "Log messages are placed into $logfile"

# Create the directory tree for the build
for dir in "vtk/bingcc" "cmake/install" "wxWidgets" 
do
  mkdir -p $basedir/$dir
  if [ $? -ne 0 ]; then exit; fi
done

# Create a file for CMAKE password entries
echo "/1 :pserver:anonymous@www.cmake.org:2401/cvsroot/CMake Ah%y]d" > $basedir/.cvspass
echo "/1 :pserver:anonymous@public.kitware.com:2401/cvsroot/VTK A<,]" >> $basedir/.cvspass
export CVS_PASSFILE=$basedir/.cvspass

# Set some variables
cmakebin=$basedir/cmake/install/bin/cmake

# -------------------------------------------------------------
# Check out and build cmake
# -------------------------------------------------------------
function get_cmake {
  cd $basedir/cmake

  # Use git to check out the right version
  echo "Checking out CMake (Release ${CMAKE_REL}) from git"
  git clone git://cmake.org/cmake.git CMake

  # Build cmake
  echo "Building CMake"
  cd $basedir/cmake/CMake
  ./configure --prefix=$basedir/cmake/install >> $logfile
  make -j4 >> $logfile
  make install >> $logfile

  # Test to see if everything is ok
  if [ ! -x $cmakebin ]
  then
    echo "Failed to create CMake executable"
    exit -1
  fi

 #remove other files
 cd ../../
 mkdir tP3m
 cd cmake
 mv install ../tP3m/
 rm -rf *
 mv ../tP3m/install .
 rm -rf ../tP3m
}

# -------------------------------------------------------------
# Check out and build wxWidgets
# -------------------------------------------------------------
function get_wxwidgets {
  cd $basedir/wxWidgets

  # Use SVN to check out fltk
  echo "Checking out wxWidgets from SVN"
  svn checkout http://svn.wxwidgets.org/svn/wx/wxWidgets/branches/WX_2_8_BRANCH wxWidgets-2.8>> $logfile

  echo "Building wxWidgets"
  cd $basedir/wxWidgets/wxWidgets-2.8

  mkdir build_wxWidgets
  ./configure --prefix=$basedir/wxWidgets/wxWidgets-2.8/build_wxWidgets --enable-monolithic --with-gtk --with-opengl --disable-shared --disable-unicode
  
  make -j4 >> $logfile
  make install >> $logfile

  #remove everything and just keep the build_wxWidgets
  mv build_wxWidgets ../
  rm -rf *
  mv ../build_wxWidgets .
  rm -rf .deps
  rm -rf .pch
} 

# -------------------------------------------------------------
# Check out and build VTK
# -------------------------------------------------------------
function get_vtk {
  cd $basedir/vtk

  # Use GIT to check out VTK
  echo "Checking out VTK (Release ${VTK_REL}) from GIT"
  git clone git://vtk.org/VTK.git VTK	

  # Configure VTK using CMake
  echo "Building VTK"
  cd $basedir/vtk/bingcc
  $cmakebin \
    -DBUILD_SHARED_LIBS:BOOL=OFF \
    -DBUILD_EXAMPLES:BOOL=OFF \
    -DBUILD_TESTING:BOOL=OFF \
    -DCMAKE_BUILD_TYPE:STRING=Release \
    -DVTK_USE_HYBRID:BOOL=ON \
    -DVTK_USE_ANSI_STDLIB:BOOL=ON \
    -DVTK_USE_PARALLEL:BOOL=ON \
    -DVTK_USE_RENDERING:BOOL=ON \
    -DVTK_USE_PATENTED:BOOL=ON \
    -DCMAKE_CXX_FLAGS_RELEASE:STRING="-O3 -DNDEBUG" \
    $basedir/vtk/VTK >> $logfile
  make  >> $logfile

  mkdir include 
  cp -f ../VTK/*/*.h include/
  cp *.h include/

  # Check whether the necessary libraries built
  for lib in "Rendering"
  do
    if [ ! -e $basedir/vtk/bingcc/bin/libvtk${lib}.a ]
    then
      echo "VTK library $basedir/vtk/bingcc/bin/libvtk${lib}.a failed to build!"
      exit -1
    fi
  done
  
  #remove other files
  rm -rf ../VTK
  cd ..
  mkdir temp
  mv bingcc/* temp
  cd bingcc/
  mv ../temp/include .
  mv ../temp/vtkstd .
  mv ../temp/bin .
  rm -rf ../temp
}

# -------------------------------------------------------------
# Perform the actual build tasks
# -------------------------------------------------------------
get_cmake
get_wxwidgets
get_vtk
cd $basedir
svn checkout https://simtk.org/svn/contrack StanfordTools-DWI >>$logfile
cd "`pwd`/StanfordTools-DWI/trunk/"
#tar -xvf ../../zlib*
svn checkout https://white.stanford.edu/repos/vistasrc/quench >>$logfile

#tar -xvf ../../RAPID*  >>$logfile
#HACK1 RAPID has 2 bugs
    cp -rf ../../rapid-2.01 RAPID201
    cd RAPID201
#CC is not defined
    sed  '/CC = CC -O/ c CC = g++ -O' Makefile > _12_3; rm -rf Makefile; mv -f _12_3 Makefile
#default int not supported
    sed  '/friend RAPID_Collide(double R1\[3\]\[3\], double T1\[3\], / c friend int RAPID_Collide(double R1[3][3], double T1[3], ' RAPID_private.H > _12_3; rm -rf RAPID_private.H; mv -f _12_3 RAPID_private.H
    cd ..
#end




