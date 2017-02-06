TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += interlock

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install interlock.dbd into <top>/dbd
DBD += interlock.dbd

# specify all source files to be compiled and added to the library
#interlock_SRCS += xxx

interlock_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

