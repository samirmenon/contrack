# Makefile for Quench
#

include ../Makefile.definitions

CC=g++

ifdef CYGWIN
WIN32SOCKFLAG = -D__USE_W32_SOCKETS
else
WIN32SOCKFLAG =
endif

#CXXFLAGS = $(SYS_CXXFLAGS) $(SYS_INCLUDE_PATHS) -DHAVE_STD_WSTRING -Wno-deprecated -O5 \
#	$(VTK_INCLUDE_PATHS) $(WX_INCLUDE_PATHS) -I./ -I./qvtk -I./qwx -I../util $(RAPID_INCLUDE_PATH) $(WIN32SOCKFLAG)

CXXFLAGS = $(SYS_CXXFLAGS) $(SYS_INCLUDE_PATHS) -DHAVE_STD_WSTRING -Wno-deprecated -g \
	$(VTK_INCLUDE_PATHS) $(WX_INCLUDE_PATHS) -I./ -I./qvtk -I./qwx -I../util $(RAPID_INCLUDE_PATH) $(WIN32SOCKFLAG)

LDFLAGS = -L../util -L../nifti $(SYS_LDFLAGS) $(VTK_LIB_PATHS) $(RAPID_LIB_PATH)

# Note: when statically linking VTK, the order of the libs matters, since they 
# are seacrched only once and in the order in which you specify them. The 
# following order works for VTK 5.2. If you get vtk link errors, try playing
# with the order of the vtk libs.
VTK_LIBS =  -lvtkRendering -lvtkIO -lvtkexpat \
         -lvtkImaging -lvtkGraphics -lvtkFiltering -lvtkCommon  -lvtksys \
         -lvtktiff -lvtkpng -lvtkzlib \
         -lvtkftgl -lvtkfreetype -lvtkjpeg -lvtkDICOMParser

LIBRARIES = ../util/libutil.a

LIBS = ${UTIL_LIB} -lnifti $(VTK_LIBS) \
        -lRAPID \
        $(WX_LIB_PATHS) \
        -lz $(SYS_LIBS)

SRCS =  qvtk/qROIEditor.cpp qwx/QuenchApp.cpp qwx/QuenchFrame.cpp qwx/QuenchController.cpp qvtk/RAPIDHelper.cpp \
		qvtk/Button.cpp qvtk/ModeSelector.cpp qvtk/Overlay.cpp qvtk/qGestureInteractor.cpp \
		qvtk/qMoveTool.cpp qvtk/qPathwayViz.cpp  qvtk/qROIViz.cpp  qvtk/qVolumeViz.cpp  \
		qvtk/VisibilityPanel.cpp qvtk/vtkFilledCaptionActor2D.cpp \
		qvtk/vtkInteractorStyleQuench.cpp qvtk/vtkROI.cpp qvtk/MeshBuilder.cpp qwx/ColorMapPanel.cpp \
		qwx/OverlayItem.cpp qwx/OverlayPanel.cpp qwx/RangeSlider.cpp qwx/Network.cpp \
		qwx/StatsCheckboxArray.cpp qwx/StatsPanel.cpp qwx/ROIPanel.cpp qwx/History.cpp \
		qwx/ImagePreview.cpp qwx/wxVTKRenderWindowInteractor.cpp \
		qwx/SceneWindow.cpp qwx/PerPointColor.cpp qvtk/Mesh.cpp  qvtk/vtkOpenGLImageOverlayActor.cpp \
		qwx/CFloatArray.cpp qwx/CParametersMap.cpp qwx/ROIPropertiesDialog.cpp \
		qvtk/vtkROIFactory.cpp qwx/SystemInfoDialog.cpp

OBJS = $(SRCS:%.cpp=%.o) 

TARGET=./Quench

ifdef mac
default: $(TARGET).app/Contents/PkgInfo
else
default: $(TARGET)
endif

#$(TARGET).app/Contents/PkgInfo: Info.plist cinch.icns $(TARGET)
#	mkdir -p $(TARGET).app/Contents
#	mkdir -p $(TARGET).app/Contents/MacOS
#	mkdir -p $(TARGET).app/Contents/Resources
#	cp Info.plist $(TARGET).app/Contents/
#	cp cinch.icns $(TARGET).app/Contents/Resources/
#	echo -n 'APPL????' > $(TARGET).app/Contents/PkgInfo
#	ln -f $(TARGET) $(TARGET).app/Contents/MacOS/$(TARGET)

$(TARGET).app/Contents/PkgInfo: osx/Info.plist $(TARGET)
	mkdir -p $(TARGET).app/Contents
	mkdir -p $(TARGET).app/Contents/MacOS
	mkdir -p $(TARGET).app/Contents/Resources
	cp osx/Info.plist $(TARGET).app/Contents/
	echo -n 'APPL????' > $(TARGET).app/Contents/PkgInfo
	cp osx/wxmac.icns $(TARGET).app/Contents/Resources
	mv $(TARGET) $(TARGET).app/Contents/MacOS/
#ln -f $(TARGET) $(TARGET).app/Contents/MacOS/$(TARGET)

$(TARGET): $(OBJS) $(LIBRARIES)
	g++ $(SYS_LDFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

.cpp.o: $*.cpp Makefile
	$(CC) $(CXXFLAGS) -c $*.cpp -o $*.o

depend:
	makedepend -f- -- $(CXXFLAGS) -- $(SRCS) > Makefile.dependencies
clean:
	rm -rf $(OBJS) $(TARGET) $(TARGET).exe $(TARGET).app

ifeq ($(wildcard Makefile.dependencies),Makefile.dependencies)
include Makefile.dependencies
  else
# do nothing
  endif

