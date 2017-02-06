#!../../bin/linux-x86/NSLS-II-EPU

< envPaths

## Register all support components
dbLoadDatabase "${TOP}/dbd/NSLS-II-EPU.dbd"
NSLS_II_EPU_registerRecordDeviceDriver pdbbase

## Autosave setup ##
#< autosave.cmd
#set_pass1_restoreFile("as_EPU1.sav")

epicsThreadSleep 1

# Configure ports
pmacAsynIPConfigure("PMAC_ETH_PORT", "192.168.1.103:1025"

# asynOctetSetInputEos("PMAC_ETH_PORT", 0, "\006")
#asynSetTraceMask("PMAC_ETH_PORT", -1, 0x1f)

pmacRegisterDriverInit("PMAC_REG_PORT", "PMAC_ETH_PORT")
#asynSetTraceMask("PMAC_REG_PORT", -1, 0x1f)

pmacAsynMotorCreate("PMAC_ETH_PORT", 0, 0, 8)
drvAsynMotorConfigure("PMAC0", "pmacAsynMotor",0, 9)

# Create CS (ControllerPort, Addr, CSNumber, CSRef, Prog)
# Gap: Coordinate System 1 | PROG 10
pmacAsynCoordCreate("PMAC_ETH_PORT", 0, 1, 0, 10)
# Phase: Coordinate System 2 | PROG 12
pmacAsynCoordCreate("PMAC_ETH_PORT", 0, 2, 1, 12)

# Configure CS (PortName, DriverName, CSRef, NAxes)
drvAsynMotorConfigure("PMAC-GAP", "pmacAsynCoord", 0, 9)
drvAsynMotorConfigure("PMAC-PHASE", "pmacAsynCoord", 1, 9)

# Set scale factor (CSRef, axis, stepsPerUnit)
pmacSetCoordStepsPerUnit(0, 6, 100)
pmacSetCoordStepsPerUnit(1, 6, 100)

# Set Idle and Moving poll periods (CSRef, PeriodsMilliSeconds)
pmacSetCoordIdlePollPeriod(0, 500)
pmacSetCoordMovingPollPeriod(0, 100)
pmacSetCoordIdlePollPeriod(1, 500)
pmacSetCoordMovingPollPeriod(1, 100)

#asynReport(5)

## Load record instances
cd ${TOP}/db
dbLoadTemplate "interlock_epu1.substitutions"
dbLoadTemplate "motor_epu1.substitutions"
dbLoadTemplate "motorStatus_epu1.substitutions"
dbLoadTemplate "girderAxis_epu1.substitutions"
dbLoadTemplate "gap_epu1.substitutions"
dbLoadTemplate "phase_epu1.substitutions"
dbLoadTemplate "calibration_epu1.substitutions"
dbLoadTemplate "globalStatus_epu1.substitutions"
dbLoadRecords("iocAdminSoft.db", "IOC=SR:C07-ID:G1A{EPU:1}")
dbLoadRecords("iocAdminScanMon.db", "IOC=SR:C07-ID:G1A{EPU:1}")
dbLoadRecords("iocGeneralTime.db", "IOCNAME=SR:C07-ID:G1A{EPU:1}")
dbLoadRecords("save_restoreStatus.db", "P=SR:C07-ID:G1A{EPU:1}")

cd ${TOP}

# dhidas - I comment out the following two for lab testing
#dbLoadRecords("db/iocAdminSoft.db", "IOC=SR:CT{IOC:EPU49:C07:1}")
#dbLoadRecords("db/save_restoreStatus.db", "P=SR:CT{IOC:EPU49:C07:1}")

# dhidas - I comment out for lab testing
#Just to get put logging to work - should be changed to "default.acf" for operations
#asSetFilename("/cf-update/acf/default.acf")

## iocInit
cd ${TOP}/iocBoot/${IOC}
iocInit

## Autosave after iocInit ##
#create_monitor_set("as_EPU1.req", 30)

# dhidas - I comment out caput log for lab testing
#caPutLogInit("ioclog.cs.nsls2.local:7004", 1)

#Support channel-finder
dbl > records.dbl

# dhidas - I comment out the cp for lab testing
#system "cp records.dbl /cf-update/idioc01.epu_c23_01.dbl"


