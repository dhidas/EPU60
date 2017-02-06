TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = NSLS-II-EPU
# NSLS-II-EPU.dbd will be created and installed
DBD += NSLS-II-EPU.dbd

# NSLS-II-EPU.dbd will be made up from these files:
NSLS-II-EPU_DBD += base.dbd

# Include dbd files from all support applications:
#NSLS-II-EPU_DBD += xxx.dbd

#BNL standards:                                                                             
NSLS-II-EPU_DBD += system.dbd                                                                  
NSLS-II-EPU_DBD += devIocStats.dbd                                                             
NSLS-II-EPU_DBD += asSupport.dbd                                                               
NSLS-II-EPU_DBD += caPutLog.dbd                                                                

# Add all the support libraries needed by this IOC
#NSLS-II-EPU_LIBS += xxx

#BNL standards:                                                                             
NSLS-II-EPU_LIBS += devIocStats                                                                
NSLS-II-EPU_LIBS += autosave                                                                   
NSLS-II-EPU_LIBS += caPutLog                                                                   

# NSLS-II-EPU_registerRecordDeviceDriver.cpp derives from NSLS-II-EPU.dbd
NSLS-II-EPU_SRCS += NSLS-II-EPU_registerRecordDeviceDriver.cpp

# Build pmacRegister driver
NSLS-II-EPU_SRCS += pmacRegisterDriver.c

# Build the main IOC entry point on workstation OSs.
NSLS-II-EPU_SRCS_DEFAULT += NSLS-II-EPUMain.cpp
NSLS-II-EPU_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#NSLS-II-EPU_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
NSLS-II-EPU_LIBS += $(EPICS_BASE_IOC_LIBS)
NSLS-II-EPU_LIBS += asyn
NSLS-II-EPU_LIBS += busy
NSLS-II-EPU_LIBS += calc
NSLS-II-EPU_LIBS += motor
NSLS-II-EPU_LIBS += pmacAsynIPPort
NSLS-II-EPU_LIBS += pmacAsynMotor
NSLS-II-EPU_LIBS += pmacAsynCoord
NSLS-II-EPU_LIBS += autosave
NSLS-II-EPU_LIBS += devIocStats


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

