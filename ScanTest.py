#!/usr/bin/env python

from cothread.catools import caget, caput

import time
from datetime import datetime

import numpy as np



def wait_for(pv):
    time.sleep(5)
    while caget(pv) == 1:
        time.sleep(1)
    time.sleep(3)
    return





DTNow = datetime.now().strftime('%Y%m%d.%H%M%S')
OutFileName = 'ScanTest_EPU60_' + DTNow + '.dat'
fout = open(OutFileName, 'w')

# Gap and position readback PVs
PV_GAP_RB           = 'SR:C07-ID:G1A{EPU:1-Ax:Gap}-Mtr.RBV'
PV_GAP_SP           = 'SR:C07-ID:G1A{EPU:1-Ax:Gap}-Mtr-SP'
PV_GAP_MOVN         = 'SR:C07-ID:G1A{EPU:1-Ax:Gap}-Mtr.MOVN'

PV_PHASE_MODE = 'SR:C07-ID:G1A{EPU:1-Ax:Phase}Phs:Mode-SP'
PV_PHASE_SP   = 'SR:C07-ID:G1A{EPU:1-Ax:Phase}-Mtr-SP'
PV_PHASE_RB   = 'SR:C07-ID:G1A{EPU:1-Ax:Phase}-Mtr.RBV'
PV_PHASE_MOVN = 'SR:C07-ID:G1A{EPU:1-Ax:Phase}-Mtr.MOVN'

PV_TU = 'SR:C07-ID:G1A{EPU:1-Ax:TU}-Mtr.RBV'
PV_BU = 'SR:C07-ID:G1A{EPU:1-Ax:BU}-Mtr.RBV'

PV_TO = 'SR:C07-ID:G1A{EPU:1-Ax:TO}-Mtr.RBV'
PV_TI = 'SR:C07-ID:G1A{EPU:1-Ax:TI}-Mtr.RBV'
PV_BO = 'SR:C07-ID:G1A{EPU:1-Ax:BO}-Mtr.RBV'
PV_BI = 'SR:C07-ID:G1A{EPU:1-Ax:BI}-Mtr.RBV'

MONITOR = [PV_GAP_SP, PV_GAP_RB, PV_PHASE_MODE, PV_PHASE_SP, PV_PHASE_RB, PV_TU, PV_BU, PV_TO, PV_TI, PV_BO, PV_BI]

# Record field names on first line of file
fout.write( '# Time(s) ' + ' '.join( map(str, MONITOR) ) + '\n')


caput(PV_PHASE_SP, 0, wait=True)
wait_for(PV_PHASE_MOVN)

while True:
    for phase_mode in range(0, 4):

        caput(PV_GAP_SP, 15000, wait=True)
        print 'waiting for gap to move'
        wait_for(PV_GAP_MOVN)
        print 'gap done moving'
       
        MyOut =  str(str(datetime.now()) + ' ' + ' '.join( map(str, caget(MONITOR))))
        fout.write(MyOut + '\n')

        caput(PV_PHASE_MODE, phase_mode, wait=True)
        time.sleep(1)

        caput(PV_PHASE_SP, 28000, wait=True)
        wait_for(PV_PHASE_MOVN)

        MyOut =  str(str(datetime.now()) + ' ' + ' '.join( map(str, caget(MONITOR))))
        print MyOut
        fout.write(MyOut + '\n')

        caput(PV_PHASE_SP, -28000, wait=True)
        wait_for(PV_PHASE_MOVN)

        MyOut =  str(str(datetime.now()) + ' ' + ' '.join( map(str, caget(MONITOR))))
        print MyOut
        fout.write(MyOut + '\n')

        caput(PV_PHASE_SP, 0, wait=True)
        wait_for(PV_PHASE_MOVN)

        MyOut =  str(str(datetime.now()) + ' ' + ' '.join( map(str, caget(MONITOR))))
        print MyOut
        fout.write(MyOut + '\n')

        for gap in np.linspace(15000, 85000, 11):
            caput(PV_GAP_SP, gap, wait=True)
            wait_for(PV_GAP_MOVN)
            MyOut =  str(str(datetime.now()) + ' ' + ' '.join( map(str, caget(MONITOR))))
            fout.write(MyOut + '\n')


    fout.flush()

