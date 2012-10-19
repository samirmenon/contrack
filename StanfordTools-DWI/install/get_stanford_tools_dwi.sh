#!/bin/bash

# Quench install script adapted from getgray.sh. 
#
# Written by Anthony Sherbondy, Reno Bowen, Shireesh Agrawal
# See rfbowen@stanford.edu.

# -------------------------------------------------------------
# Options that the user can supply
# -------------------------------------------------------------

# The directory where get_stanford_tools_dwi.sh has been downloaded
basedir="`pwd`"

# The build log file will be important for looking at compilation errors and status
logfile=$basedir/buildlog

# Place to install VTK and wxWidgets
WX_DIR=$basedir/wxWidgets
WX_BUILD=$WX_DIR/build
VTK_DIR=$basedir/vtk
VTK_BUILD=$VTK_DIR/build
CMAKE_DIR=$basedir/cmake
CMAKE_BUILD=$CMAKE_DIR/build
CMAKE_BIN=$CMAKE_BUILD/bin/cmake

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
# Directory creation
# -------------------------------------------------------------
echo "Preparing to build quench. This will take 1-2 hours and 500MB!"
echo "Log messages are placed into $logfile"
# Create the directory tree for the build
for dir in $CMAKE_DIR $VTK_DIR $WX_DIR
do
  mkdir -p $dir
  if [ $? -ne 0 ]; then exit; fi
done

# -------------------------------------------------------------
# Check out and build cmake
# -------------------------------------------------------------
function get_cmake {
  cd $CMAKE_DIR

  # Use git to check out the right version
  echo "Checking out CMake from git"
  git clone git://cmake.org/cmake.git CMake
  cd CMake
  git checkout release
  cd ..

  # Build cmake
  echo "Building CMake"
  cd $CMAKE_DIR/CMake
  ./configure --prefix=$CMAKE_BUILD >> $logfile
  make -j 4 >> $logfile
  make install >> $logfile

  # Test to see if everything is ok
  if [ ! -x $CMAKE_BIN ]
  then
    echo "Failed to create CMake executable"
    exit -1
  fi
}

# -------------------------------------------------------------
# Check out and build wxWidgets
# -------------------------------------------------------------
function get_wxwidgets {
  cd $WX_DIR

  # Use SVN to check out wxWidgets
  echo "Checking out wxWidgets from SVN"
  svn checkout http://svn.wxwidgets.org/svn/wx/wxWidgets/branches/WX_2_8_BRANCH wxWidgets-2.8>> $logfile

  echo "Building wxWidgets"
  mkdir $WX_BUILD
  cd $WX_DIR/wxWidgets-2.8
  ./configure --prefix=$WX_BUILD --enable-monolithic --with-gtk --with-opengl --disable-shared --disable-unicode LIBS=-lX11 
  make -j 4 >> $logfile
  make install >> $logfile
} 

# -------------------------------------------------------------
# Check out and build VTK
# -------------------------------------------------------------
function get_vtk {
  cd $VTK_DIR

  # Use GIT to check out VTK
  echo "Checking out VTK from GIT"
  git clone git://vtk.org/VTK.git VTK	
  cd VTK
  git checkout release
  cd ..

  # Configure VTK using CMake
  echo "Building VTK"
  mkdir $VTK_BUILD
  cd $VTK_DIR/VTK
  $CMAKE_BIN \
    -DBUILD_SHARED_LIBS:BOOL=OFF \
    -DBUILD_EXAMPLES:BOOL=OFF \
    -DBUILD_TESTING:BOOL=OFF \
    -DCMAKE_BUILD_TYPE:STRING=Release \
    -DVTK_USE_HYBRID:BOOL=ON \
    -DVTK_USE_ANSI_STDLIB:BOOL=ON \
    -DVTK_USE_PARALLEL:BOOL=ON \
    -DVTK_USE_RENDERING:BOOL=ON \
    -DVTK_USE_PATENTED:BOOL=ON \
    -DCMAKE_INSTALL_PREFIX:STRING="$VTK_BUILD" \
    -DCMAKE_CXX_FLAGS_RELEASE:STRING="-O3 -DNDEBUG" >> $logfile

  make -j 4 >> $logfile
  make install >> $logfile

  # Some relevant files are misplaced - help Quench find them 
  ln -s $VTK_BUILD/include/vtk-* $VTK_BUILD/include/vtk
  cp $VTK_BUILD/lib/vtk-*/lib*.a $VTK_BUILD/bin

  # Check whether the necessary libraries are in the right place 
  for lib in "Rendering"
  do
    if [ ! -e $VTK_BUILD/bin/libvtk${lib}.a ]
    then
      echo "Could not find VTK library $VTK_BUILD/bin/libvtk${lib}.a "
      echo " either because there were build errors or it is placed in "
      echo " the wrong directory.  Check the $buildlog for information."
      exit -1
    fi
  done
}

# -------------------------------------------------------------
# Check out and build Stanford DWI tools
# -------------------------------------------------------------
function get_dwi_tools {
    cd $basedir
    svn co https://white.stanford.edu/repos/StanfordTools-DWI >>$logfile
    
    echo "Building Stanford DWI Tools"
    cd StanfordTools-DWI
    make >> $logfile
}

# -------------------------------------------------------------
# Perform the actual build tasks
# -------------------------------------------------------------
get_cmake
get_wxwidgets
get_vtk
get_dwi_tools

#end




