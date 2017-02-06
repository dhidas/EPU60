####################################################
## save_restore setup
save_restoreSet_Debug(0)

# status-PV prefix, so save_restore can find its status PV's.
save_restoreSet_status_prefix("SR:C07-ID:G1A{EPU:1}")

# Ok to save/restore save sets with missing values (no CA connection to PV)?  
save_restoreSet_IncompleteSetsOk(1)

# Save dated backup files?
save_restoreSet_DatedBackupFiles(1)

# Number of sequenced backup files to write
save_restoreSet_NumSeqFiles(1)
save_restoreSet_SeqPeriodInSeconds(300)

set_savefile_path("${TOP}", "autosaveSaves")

set_requestfile_path("$(TOP)/iocBoot/iocEPU1/")
## End of autosave set-up
####################################################
