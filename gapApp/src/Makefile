TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += gap

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install gap.dbd into <top>/dbd
DBD += gap.dbd

# specify all source files to be compiled and added to the library
#gap_SRCS += xxx

gap_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

