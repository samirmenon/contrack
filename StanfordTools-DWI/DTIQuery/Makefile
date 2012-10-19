# Makefile for DTIQuery
#

include ../Makefile.definitions

CXXFLAGS = $(SYS_CXXFLAGS) $(SYS_INCLUDE_PATHS) -Wno-deprecated -O  \
        $(VTK_INCLUDE_PATHS) $(WX_INCLUDE_PATHS) -I../ -I../util $(RAPID_INCLUDE_PATH)

LDFLAGS =  -L../util -L../nifti $(ZLIB_LIB_PATH) $(RAPID_LIB_PATH) $(SYS_LDFLAGS) $(VTK_LIB_PATHS) $(RAPID_LIB_PATH)

VTK_LIBS = -lvtkIO \
        -lvtkRendering -lvtkGraphics -lvtkImaging \
        -lvtkFiltering -lvtkCommon -lvtkHybrid -lvtkpng -lvtkzlib \
        -lvtkftgl -lvtkfreetype -lvtkjpeg -lvtkDICOMParser \

# for FreeBSD, need -lpng -ljpeg -lz instead of -lvtkpng -lvtkzlib -lvtkjpeg

LIBRARIES = ../util/libutil.a

LIBS = -lutil -lnifti $(VTK_LIBS) \
        -lRAPID \
        $(WX_LIB_PATHS) \
        -lz $(SYS_LIBS)

SRCS = DTIController.cpp VOIPanel.cpp ViewCone.cpp \
	vtkFilledCaptionActor2D.cpp DTIFilterApp.cpp \
	SliderPanel.cpp scene.cpp FilterPanel.cpp \
	VOI.cpp ButtonChoiceDialog.cpp \
	wxVTKRenderWindowInteractor.cpp VOICube.cpp \
	VOISphere.cpp VOILoaded.cpp CameraControls.cpp \
	SystemInfoDialog.cpp vtkInteractorStyleDTI.cpp \
	../util/DTIFilterSphere.cpp ../util/DTIFilterComplex.cpp \
	../util/DTIFilterPathwayScalar.cpp ../util/DTIFilterAlgorithm.cpp \
	../util/DTIQueryProcessor.cpp ../util/DTIFilterCube.cpp 

OBJS = $(SRCS:%.cpp=%.o) 

TARGET=./dtiquery

ifdef mac
default: $(TARGET).app/Contents/PkgInfo
else
default: $(TARGET)
endif

$(TARGET).app/Contents/PkgInfo: Info.plist dtiquery.icns $(TARGET)
	mkdir -p $(TARGET).app/Contents
	mkdir -p $(TARGET).app/Contents/MacOS
	mkdir -p $(TARGET).app/Contents/Resources
	cp Info.plist $(TARGET).app/Contents/
	cp dtiquery.icns $(TARGET).app/Contents/Resources/
	echo -n 'APPL????' > $(TARGET).app/Contents/PkgInfo
	ln -f $(TARGET) $(TARGET).app/Contents/MacOS/$(TARGET)

$(TARGET): $(OBJS) $(LIBRARIES)
	g++ $(SYS_LDFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

.cpp.o: $*.cpp Makefile
	$(CPP) $(CXXFLAGS) -c $*.cpp -o $*.o

depend:
	makedepend -f- -- $(CXXFLAGS) -- $(SRCS) > Makefile.dependencies
clean:
	rm -rf $(OBJS) $(TARGET) $(TARGET).exe $(TARGET).app

ifeq ($(wildcard Makefile.dependencies),Makefile.dependencies)
include Makefile.dependencies
  else
# do nothing
  endif

