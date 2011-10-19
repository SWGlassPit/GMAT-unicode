# $Id: Makefile 9154 2011-03-04 23:07:17Z djcinsb $
# makefile for GMAT's CInterface Plugin
# This is the src level makefile 
#
# This file builds the plug-in base code

include ./CInterfaceEnv.mk


TARGET = ../lib/libCInterface$(SHARED_EXTENSION)


all: prep $(TARGET)

prep:
	rm -rf $(TARGET)

HEADERS = -Iinclude \
          -Iplugin \
          -Ifactory \
          -Ipropagator \
          -Icommand \
          -I$(GMAT_CODE_LOCATION)/base/include \
          -I$(GMAT_CODE_LOCATION)/base/attitude \
          -I$(GMAT_CODE_LOCATION)/base/burn \
          -I$(GMAT_CODE_LOCATION)/base/command \
          -I$(GMAT_CODE_LOCATION)/base/configs \
          -I$(GMAT_CODE_LOCATION)/base/coordsystem \
          -I$(GMAT_CODE_LOCATION)/base/executive \
          -I$(GMAT_CODE_LOCATION)/base/factory \
          -I$(GMAT_CODE_LOCATION)/base/forcemodel \
          -I$(GMAT_CODE_LOCATION)/base/foundation \
          -I$(GMAT_CODE_LOCATION)/base/function \
          -I$(GMAT_CODE_LOCATION)/base/hardware \
          -I$(GMAT_CODE_LOCATION)/base/interface \
          -I$(GMAT_CODE_LOCATION)/base/interpolator \
          -I$(GMAT_CODE_LOCATION)/base/interpreter \
          -I$(GMAT_CODE_LOCATION)/base/math \
          -I$(GMAT_CODE_LOCATION)/base/parameter \
          -I$(GMAT_CODE_LOCATION)/base/plugin \
          -I$(GMAT_CODE_LOCATION)/base/propagator \
          -I$(GMAT_CODE_LOCATION)/base/solarsys \
          -I$(GMAT_CODE_LOCATION)/base/solver \
          -I$(GMAT_CODE_LOCATION)/base/stopcond \
          -I$(GMAT_CODE_LOCATION)/base/subscriber \
          -I$(GMAT_CODE_LOCATION)/base/spacecraft \
          -I$(GMAT_CODE_LOCATION)/base/util 


clean : archclean

archclean :
	rm -rf */*.o
	rm -rf $(MEX_OBJECTS)
	rm -rf $(TARGET)
	rm -rf */*~
	rm -rf core
	rm -rf */core

OBJECTS = \
    command/PrepareMissionSequence.o \
    factory/CCommandFactory.o \
    plugin/CInterfacePluginFunctions.o
    
    
ifeq ($(LINUX_MAC), 1)
ifeq ($(MAC_SPECIFIC), 1)
$(TARGET) : $(OBJECTS)
	mkdir -p ../lib
	$(CPP) $(OBJECTS) -o $(TARGET) $(SHARED_LIB_FLAGS) -L$(GMAT_BIN_LOCATION) -l$(BASE_LIBRARY)
	install_name_tool -change lib64/lib$(BASE_LIBRARY).dylib $(BASE_LIB_LOCATION)lib$(BASE_LIBRARY).dylib $(TARGET)
	cp $(TARGET) $(GMAT_BIN_LOCATION)
else
$(TARGET) : $(OBJECTS)
	mkdir -p ../lib
	$(CPP) $(OBJECTS) -o $(TARGET) $(SHARED_LIB_FLAGS) -L$(GMAT_BIN_LOCATION) -l$(BASE_LIBRARY) $(PLATFORM_LINK_FLAGS)
	cp $(TARGET) $(GMAT_BIN_LOCATION)
endif	
else
$(TARGET) : $(OBJECTS)
	mkdir -p ../lib
	$(CPP) $(OBJECTS) -o $(TARGET) $(SHARED_LIB_FLAGS)
	cp $(TARGET) $(GMAT_BIN_LOCATION)
endif


$(OBJECTS): %.o: %.cpp %.hpp
	$(CPP) $(CPPFLAGS) $(HEADERS) -c -o $@ $<
    