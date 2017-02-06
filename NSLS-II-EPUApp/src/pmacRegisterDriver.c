#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include <cantProceed.h>
#include <epicsStdio.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsThread.h>
#include <iocsh.h>

#include <asynDriver.h>
#include <asynDrvUser.h>
#include <asynOctetSyncIO.h>
#include <asynInt32.h>
#include <asynFloat64.h>

#include <epicsExport.h>
#define NCHANNELS 16

/* driver private data */
typedef struct drvPvt {
    const char    *portName;
    const char    *pmacPort;
    int           connected;
    
    asynUser      *pasynUser;
    
    asynInterface common;
    asynInterface asynDrvUser;
    asynInterface asynInt32;
    asynInterface asynFloat64;
} drvPvt;

typedef struct drvUserData {
    char *var;
    int inv;
    epicsFloat64 scale;
    epicsFloat64 tmot;
} drvUserData;

static int pmacRegisterDriverInit(const char *registerPort,const char *pmacPort);

/* asynCommon methods */
static void report(void *drvPvt,FILE *fp,int details);
static asynStatus connect(void *drvPvt,asynUser *pasynUser);
static asynStatus disconnect(void *drvPvt,asynUser *pasynUser);
static asynCommon common = { report, connect, disconnect };

/* asynDrvUser */
static asynStatus create(void *drvPvt,asynUser *pasynUser,
    const char *drvInfo, const char **pptypeName,size_t *psize);
static asynStatus getType(void *drvPvt,asynUser *pasynUser,
    const char **pptypeName,size_t *psize);
static asynStatus destroy(void *drvPvt,asynUser *pasynUser);
static asynDrvUser drvUser = {create,getType,destroy};

/* asynInt32 methods */
static asynStatus int32Write(void *drvPvt,asynUser *pasynUser,epicsInt32 value);
static asynStatus int32Read(void *drvPvt,asynUser *pasynUser,epicsInt32 *value);
static asynStatus int32GetBounds(void *drvPvt, asynUser *pasynUser,
                                epicsInt32 *low, epicsInt32 *high);

/* asynFloat64 methods */
static asynStatus float64Write(void *drvPvt,asynUser *pasynUser,
    epicsFloat64 value);
static asynStatus float64Read(void *drvPvt,asynUser *pasynUser,
    epicsFloat64 *value);


static int pmacRegisterDriverInit(const char *registerPort,const char *pmacPort)
{
    drvPvt     *pdrvPvt;
    char       *portName;
    char       *pmacPortName;
    asynStatus status;
    int        nbytes;    
    asynInt32  *pasynInt32;
    asynFloat64 *pasynFloat64;
    
    /* allocate memory for private data*/
    nbytes = sizeof(drvPvt) + sizeof(asynInt32) + sizeof(asynFloat64);
    nbytes += strlen(registerPort) + 1;
    nbytes += strlen(pmacPort) + 1;
    pdrvPvt = callocMustSucceed(nbytes,sizeof(char),"pmacRegisterDriverInit");
    
    pasynInt32 = (asynInt32 *)(pdrvPvt + 1);
    pasynFloat64 = (asynFloat64 *)(pasynInt32 + 1);
    portName = (char *)(pasynFloat64 + 1);
    pmacPortName = portName + strlen(registerPort) + 1;
    
    strcpy(portName,registerPort);
    strcpy(pmacPortName, pmacPort);
    pdrvPvt->portName = portName;
    pdrvPvt->pmacPort = pmacPortName;
    pdrvPvt->connected = 0;
    pdrvPvt->pasynUser = NULL;
    
    /* initializing interfaces */
    pdrvPvt->common.interfaceType = asynCommonType;
    pdrvPvt->common.pinterface  = (void *)&common;
    pdrvPvt->common.drvPvt = pdrvPvt;
    
    pdrvPvt->asynDrvUser.interfaceType = asynDrvUserType;
    pdrvPvt->asynDrvUser.pinterface = (void *)&drvUser;
    pdrvPvt->asynDrvUser.drvPvt = pdrvPvt;

    pdrvPvt->asynInt32.interfaceType = asynInt32Type;
    pdrvPvt->asynInt32.pinterface  = pasynInt32;
    pdrvPvt->asynInt32.drvPvt = pdrvPvt;
    pasynInt32->write = int32Write;
    pasynInt32->read = int32Read;
    pasynInt32->getBounds = int32GetBounds;
    
    pdrvPvt->asynFloat64.interfaceType = asynFloat64Type;
    pdrvPvt->asynFloat64.pinterface  = pasynFloat64;
    pdrvPvt->asynFloat64.drvPvt = pdrvPvt;
    pasynFloat64->write = float64Write;
    pasynFloat64->read = float64Read;
    
    /* registering interfaces */
    status = pasynManager->registerPort(portName,ASYN_CANBLOCK,1,0,0);
    if(status!=asynSuccess) goto error;
    
    status = pasynManager->registerInterface(portName,&pdrvPvt->common);
    if(status!=asynSuccess) goto error;
   
    status = pasynManager->registerInterface(portName, &pdrvPvt->asynDrvUser);
    if(status!=asynSuccess) goto error;
   

    status = pasynManager->registerInterface(portName, &pdrvPvt->asynInt32);
    if(status!=asynSuccess) goto error;
   

    status = pasynManager->registerInterface(portName, &pdrvPvt->asynFloat64);
    if(status!=asynSuccess) goto error;
   
    return 0;

error:
    printf("pmacRegisterDriverInit failed\n");
    free(pdrvPvt);
    return -1;
}

/* asynCommon methods */
static void report(void *pvt,FILE *fp,int details)
{
    drvPvt *pdrvPvt = (drvPvt *)pvt;
    
    fprintf(fp,"    pmacRegisterDriver: connected:%s\n", (pdrvPvt->connected ? "Yes" : "No"));
}

static asynStatus connect(void *pvt,asynUser *pasynUser)
{
    drvPvt     *pdrvPvt = (drvPvt *)pvt;
    asynStatus  status;  

    asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s *pmacRegisterDriver:connect\n", pdrvPvt->portName);
    if(pdrvPvt->connected) {
        asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:connect port already connected\n", pdrvPvt->portName);
	epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize, "%s pmacRegisterDriver:connect port already connected", pdrvPvt->portName);
        return asynError;
    }
        
    status = pasynOctetSyncIO->connect(pdrvPvt->pmacPort, -1, &pdrvPvt->pasynUser, NULL);
    if(status!=asynSuccess){
        asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:connect connection to asynOctet failed\n", pdrvPvt->portName);	
        return status;
    }  
        
    status = pasynManager->exceptionConnect(pasynUser);
    if(status!=asynSuccess){
        asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:connect exceptionConnecti failed\n", pdrvPvt->portName);	
        return status;
    }  
    pdrvPvt->connected = 1;
    return asynSuccess;
}

static asynStatus disconnect(void *pvt,asynUser *pasynUser)
{
    drvPvt    *pdrvPvt = (drvPvt *)pvt;    
    asynStatus status;  

    asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s *pmacRegisterDriver:disconnect\n", pdrvPvt->portName);
    if(!pdrvPvt->connected) {
        asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegistryDriver:disconnect port not connected\n", pdrvPvt->portName);
	epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize, "%s pmacRegistryDriver:disconnect port not connected", pdrvPvt->portName);
        return asynError;
    }
    
    status = pasynOctetSyncIO->disconnect(pdrvPvt->pasynUser);
    if(status!=asynSuccess){
        asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:connect disconnecting from asynOctet failed\n", pdrvPvt->portName);	
        return status;
    }  
    pdrvPvt->pasynUser = NULL;
    
    status = pasynManager->exceptionDisconnect(pasynUser);
    if(status!=asynSuccess){
        asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:connect exceptionDisconnect failed\n", pdrvPvt->portName);	
        return status;
    }      
    pdrvPvt->connected = 0;
    return asynSuccess;
}

static asynStatus int32Write(void *pvt,asynUser *pasynUser,
                                 epicsInt32 value)
{
    drvPvt      *pdrvPvt  = (drvPvt *)pvt;
    drvUserData *pdrvUser = pasynUser->drvUser;
    asynStatus   status;
    char         wbuf[64],rbuf[64];
    size_t       len, nout, nin;
    int          reason;
    
    asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s *pmacRegisterDriver:int32Write %s=%d\n", pdrvPvt->portName, pdrvUser->var, value);
    
    len = snprintf(wbuf, sizeof(wbuf), "%s=%d", pdrvUser->var, pdrvUser->inv ? !value : value);
    if (len > sizeof(wbuf)) {
        asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:int32Write write buffer overflow\n", pdrvPvt->portName);
        epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,"%s pmacRegisterDriver:int32Write write buffer overflow", pdrvPvt->portName);
        return asynError;
    }

    status = pasynOctetSyncIO->writeRead(pdrvPvt->pasynUser, wbuf, len, rbuf, sizeof(rbuf),pdrvUser->tmot, &nout, &nin, &reason);
    if (nout != len) {
      epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,"%s pmacRegisterDriver:int32Write write to asyncOctet failed", pdrvPvt->portName);
      status = asynError;
    }
    if (status != asynSuccess) {
         asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:int32Write write to asyncOctet failed\n", pdrvPvt->portName);         
         return status;
    }
    return asynSuccess;
}

static asynStatus int32Read(void *pvt,asynUser *pasynUser,
                                 epicsInt32 *value)
{
    drvPvt      *pdrvPvt = (drvPvt *)pvt;
    drvUserData *pdrvUser = pasynUser->drvUser;
    asynStatus   status;
    char         rbuf[64];
    size_t       len, nout, nin;
    int          reason;
    
    asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s *pmacRegisterDriver:int32Read %s\n", pdrvPvt->portName, pdrvUser->var);
    
    len = strlen(pdrvUser->var);
    
    status = pasynOctetSyncIO->writeRead(pdrvPvt->pasynUser, pdrvUser->var, len, rbuf, sizeof(rbuf),pdrvUser->tmot, &nout, &nin, &reason);
    if (nout != len) {
      epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,"%s pmacRegisterDriver:int32Read write to asyncOctet failed", pdrvPvt->portName);
      status = asynError;
    }
    if (status != asynSuccess) {
         asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:int32Read write to asyncOctet failed\n", pdrvPvt->portName);         
         return status;
    }

    rbuf[sizeof(rbuf)-1]=0;
    if (sscanf(rbuf, "%d", value) != 1) {
      asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:int32Read parse of returning value failed (%s)\n", pdrvPvt->portName, rbuf);
      epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,"%s pmacRegisterDriver:int32Read parse of returning value failed (%s)", pdrvPvt->portName, rbuf);
      return asynError;
    }
    
    if (pdrvUser->inv) *value = !(*value);
    return asynSuccess;
}
static asynStatus int32GetBounds(void *pvt, asynUser *pasynUser,
                                epicsInt32 *low, epicsInt32 *high)
{
    drvPvt      *pdrvPvt = (drvPvt *)pvt;
    
    asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s *pmacRegisterDriver:int32GetBounds\n", pdrvPvt->portName);
    *low = 0x80000000;
    *high= 0x7fffffff;
    return asynSuccess;
}

static asynStatus float64Write(void *pvt,asynUser *pasynUser,
                              epicsFloat64 value)
{
    drvPvt      *pdrvPvt = (drvPvt *)pvt;
    drvUserData *pdrvUser = pasynUser->drvUser;
    asynStatus   status;

    char         wbuf[64],rbuf[64];
    size_t       len, nout, nin;
    int          reason;
     
    asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s *pmacRegisterDriver:float64Write %s=%lf\n", pdrvPvt->portName, pdrvUser->var, value);
    
    len = snprintf(wbuf, sizeof(wbuf), "%s=%lf", pdrvUser->var, value*pdrvUser->scale);
    if (len > sizeof(wbuf)) {
        asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:float64Write write buffer overflow\n", pdrvPvt->portName);
        epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,"%s pmacRegisterDriver:float64Write write buffer overflow", pdrvPvt->portName);
        return asynError;
    }

    status = pasynOctetSyncIO->writeRead(pdrvPvt->pasynUser, wbuf, len, rbuf, sizeof(rbuf), pdrvUser->tmot, &nout, &nin, &reason);
    if (nout != len) {
      epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,"%s pmacRegisterDriver:float64Write write to asyncOctet failed", pdrvPvt->portName);
      status = asynError;
    }
    if (status != asynSuccess) {
         asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:float64Write write to asyncOctet failed\n", pdrvPvt->portName);         
         return status;
    }
  
    return asynSuccess;
}

static asynStatus float64Read(void *pvt,asynUser *pasynUser,
                              epicsFloat64 *value)
{
    drvPvt      *pdrvPvt = (drvPvt *)pvt;
    drvUserData *pdrvUser = pasynUser->drvUser;
    asynStatus   status;
    char         rbuf[64];
    size_t       len, nout, nin;
    int          reason;
        
    asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s *pmacRegisterDriver:float64Read %s\n", pdrvPvt->portName, pdrvUser->var);
    
    len = strlen(pdrvUser->var);

    status = pasynOctetSyncIO->writeRead(pdrvPvt->pasynUser, pdrvUser->var, len, rbuf, sizeof(rbuf),pdrvUser->tmot, &nout, &nin, &reason);
    if (nout != len) {
      epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,"%s pmacRegisterDriver:float64Read write to asyncOctet failed", pdrvPvt->portName);
      status = asynError;
    }
    if (status != asynSuccess) {
         asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:float64Read write to asyncOctet failed\n", pdrvPvt->portName);         
         return status;
    }

    rbuf[sizeof(rbuf)-1]=0;
    if (sscanf(rbuf, "%lf", value) != 1) {
      asynPrint(pasynUser,ASYN_TRACE_ERROR, "%s pmacRegisterDriver:float64Read parse of returning value failed (%s)\n", pdrvPvt->portName, rbuf);
      epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,"%s pmacRegisterDriver:float64Read parse of returning value failed (%s)", pdrvPvt->portName, rbuf);
      return asynError;
    } 
 
    *value = *value / pdrvUser->scale;
    return asynSuccess;
}

static asynStatus create(void *pvt,asynUser *pasynUser,
    const char *drvInfo, const char **pptypeName,size_t *psize)
{
    drvPvt      *pdrvPvt = (drvPvt *)pvt;   
    drvUserData *pdrvUser;
    char         var[64];        
    int          n;
    char         *p;
        
    if(!drvInfo) goto error;    
    
    n = sscanf(drvInfo, " var ( %64[^) ] )", var);    
    if (n!=1) goto error;
    
    pdrvUser = callocMustSucceed(sizeof(drvUserData)+strlen(var)+1,sizeof(char),"pmacRegisterDriverInit"); 
    pasynUser->drvUser = pdrvUser;
    pdrvUser->var = (char *)(pdrvUser + 1);    
    strcpy(pdrvUser->var, var);
    
    if ((p=strstr(drvInfo, "scale"))!=NULL) {
	n = sscanf(p, " scale ( %lf )", &pdrvUser->scale);
 	if (n!=1 || (pdrvUser->scale==0.0 || pdrvUser->scale==-0.0)) goto cleanup;
    } else pdrvUser->scale = 1.0;
    
    if ((p=strstr(drvInfo, "inv"))!=NULL) {
	n = sscanf(p, " inv ( %d )", &pdrvUser->inv);
	if (n!=1) goto cleanup;
    } else pdrvUser->inv = 0;

    if ((p=strstr(drvInfo, "tmot"))!=NULL) {
	n = sscanf(p, " tmot ( %lf )", &pdrvUser->tmot);
	if (n!=1) goto cleanup;
    } else pdrvUser->tmot = 1.0;

    asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s pmacRegisterDriver:create %s %d %lf\n", pdrvPvt->portName, pdrvUser->var, pdrvUser->inv, pdrvUser->scale);            
            
    pasynUser->reason = 0;
    if(pptypeName) *pptypeName = pdrvUser->var;
    if(psize) *psize = sizeof(int);
    
    return asynSuccess;
    
cleanup:
    free(pdrvUser);
error:
    asynPrint(pasynUser,ASYN_TRACE_ERROR,"%s pmacRegisterDriver:asynDrvUser:create failed. got \"%s\" expecting var(<str>)[scale(<float>)][inv(<int>)][tmot(<float>)]\n", pdrvPvt->portName,drvInfo);
    epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,
        "%s pmacRegisterDriver:asynDrvUser:create failed. got \"%s\" expecting var(<str>)[scale(<float>)][inv(<int>)][tmot(<float>)]", pdrvPvt->portName,drvInfo);
    return asynError;
}

static asynStatus getType(void *drvPvt,asynUser *pasynUser,
    const char **pptypeName,size_t *psize)
{
    *pptypeName = ((drvUserData *)pasynUser->drvUser)->var;
    *psize = sizeof(int);
    return asynSuccess;
}

static asynStatus destroy(void *drvPvt,asynUser *pasynUser)
{    
    free(pasynUser->drvUser);      
    return asynSuccess;
}

/* register int32DriverInit*/
static const iocshArg pmacRegisterDriverInitArg0 = { "registerPort", iocshArgString };
static const iocshArg pmacRegisterDriverInitArg1 = { "pmacPort", iocshArgString };
static const iocshArg *pmacRegisterDriverInitArgs[] = {
    &pmacRegisterDriverInitArg0,&pmacRegisterDriverInitArg1};
static const iocshFuncDef pmacRegisterDriverInitFuncDef = {
    "pmacRegisterDriverInit", 2, pmacRegisterDriverInitArgs};
static void pmacRegisterDriverInitCallFunc(const iocshArgBuf *args)
{
    pmacRegisterDriverInit(args[0].sval,args[1].sval);
}

static void pmacRegisterDriverRegister(void)
{
    static int firstTime = 1;
    if (firstTime) {
        firstTime = 0;
        iocshRegister(&pmacRegisterDriverInitFuncDef, pmacRegisterDriverInitCallFunc);
    }
}
epicsExportRegistrar(pmacRegisterDriverRegister);
