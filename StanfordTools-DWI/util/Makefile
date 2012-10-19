# Makefile for util (utility library used by contrack_gen)
#
include ../Makefile.definitions

OUTPUT_DIR = .
CFLAGS += -O5 $(RAPID_INCLUDE_PATH) -I../nifti -I. $(SYS_CXXFLAGS)
SRCS =  DTITractParamsFile.cpp  DTIVolumeIO.cpp DTIFilterPathwayScalar.cpp DTIFilterAlgorithm.cpp\
	DTIRandomSeedGenerator.cpp DTIRandomSeedGeneratorMask.cpp \
	DTIPathwayInterface.cpp DTIPathway.cpp DTIPathwayFactory.cpp \
	DTIPathwayDatabase.cpp DTIPathwayIO.cpp DTIStats.cpp DTIMath.cpp dcdf.cpp \
	DTITrackingData.cpp DTIFilterTree.cpp FunctionNodes.cpp FunctionParser.cpp \
	DTIFilterROI.cpp DTIFilter.cpp DTIPathwaySelection.cpp ROIManager.cpp SubjectData.cpp\
	DTIPathwayIntersectionVolume.cpp ui_utils.cpp PathwayGroup.cpp PDBHelper.cpp \
	DTIPathDistanceMatrix.cpp DTIQueryProcessor.cpp Wm4/Wm4BSplineFitBasis.cpp Wm4/Wm4BSplineCurveFit.cpp

OBJS = $(SRCS:%.cpp=%.o) 
TARGET = $(OUTPUT_DIR)/libutil.a

default: $(TARGET)

$(TARGET) : ${OBJS}
	/bin/rm -f $@
	${AR} $@ ${OBJS}

.cpp.o: $*.cpp Makefile
	$(CPP) $(CFLAGS) -c $*.cpp -o $*.o

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJS)

depend:
	makedepend -f- -- $(CFLAGS) -- $(SRCS) > Makefile.dependencies

ifeq ($(wildcard Makefile.dependencies),Makefile.dependencies)
include Makefile.dependencies
  else
# do nothing
  endif

