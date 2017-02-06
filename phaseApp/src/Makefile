TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += phase

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install phase.dbd into <top>/dbd
DBD += phase.dbd

# specify all source files to be compiled and added to the library
#phase_SRCS += xxx

phase_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

