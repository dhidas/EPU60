TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += singleMotors

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install singleMotors.dbd into <top>/dbd
DBD += singleMotors.dbd

# specify all source files to be compiled and added to the library
#singleMotors_SRCS += xxx

singleMotors_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

