/**
 * @file    perf_counters.cpp
 * @author  Zeke
 * @date   10/05/2016
 *  * @brief  An interface to the Intel PCM.2-11, used can specific the input parameters.
 * 
 */
#include "perf_counters.h"

/** Intel PCM is only available from C++ */
#if defined(__cplusplus) && defined(PERF_COUNTERS)

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "cpucounters.h" /* intel perf counters monitor */

using namespace std;

/** \ingroup PerformanceMonitoring Intel Performance Counter Monitor instance */

static PCM * pcmInstance;

#if PER_CORE==1
static CoreCounterState before_state;
static CoreCounterState after_state;
#elif PER_SOCKET==1
static SocketCounterState before_state;
static SocketCounterState after_state;
#else 
static SystemCounterState before_state;
static SystemCounterState after_state;
#endif


extern int prefetch_model_for_each_core; //0: default enable, 15: disable all....
 
/** The monitored events can be customized. Only 4 fully programmable counters
 *  are supported on microarchitecture codenamed Nehalem/Westmere.
 */

PCM::CustomCoreEventDescription MyEvents[4];
std::string MyEventNames[4];
int numEvents;

//for iMIC:
PCM::CustomCoreEventDescription MIC_Events[4];
std::string MIC_EventNames[4];
int MIC_Events_num;

//for R3QPI::::
PCM::CustomCoreEventDescription R3QPI_Events[3];
std::string R3QPI_EventNames[3];
int R3QPI_Events_num;

	
/** internal state variables */
static double eventAcc[12] = {0.0};


/** custom performance counters config file, if NULL no custom config. */
char * PCM_CONFIG;

/** the output file for performance counter results, if NULL output to stdout */
char * PCM_OUT;

static char * 
mystrdup (const char *s) 
{
    char *ss = (char*) malloc (strlen (s) + 1);

    if (ss != NULL)
        memcpy (ss, s, strlen(s) + 1);

    return ss;
}

void
PCM_initPerformanceMonitor(const struct Monitor_Event *pcmcfg, const char * pcmout)
{
    PCM * m = PCM::getInstance();
    PCM::ErrorCode status;
    pcmInstance = m;
    numEvents = 0;
	    
    if(pcmout)
        PCM_OUT = mystrdup(pcmout);
    /* else */
    /*     PCM_CONFIG = NULL; */
	
#if 1
 if (pcmcfg != NULL)
 {
	 
	 
	if (pcmcfg->core_event_model == 0) //default
	{
         MyEvents[0].event_number = 0x2e; // UOPS_ISSUED.ANY event number 
         MyEvents[0].umask_value  = 0x41;  // UOPS_ISSUED.ANY umask
         MyEvents[1].event_number = 0xd2; // DTLB_LOAD_MISSES.ANY event number
         MyEvents[1].umask_value  = 0x08;  // DTLB_LOAD_MISSES.ANY umask		
         MyEvents[2].event_number = 0xd2; // UOPS_ISSUED.ANY event number 
         MyEvents[2].umask_value  = 0x07;  // UOPS_ISSUED.ANY umask
         MyEvents[3].event_number = 0xd1; // DTLB_LOAD_MISSES.ANY event number
         MyEvents[3].umask_value  = 0x02;  // DTLB_LOAD_MISSES.ANY umask		
    }    
	else if (pcmcfg->core_event_model == 1) //custom with core_event
	{
         MyEvents[0].event_number = pcmcfg->core_event[0][0];
         MyEvents[0].umask_value  = pcmcfg->core_event[0][1];
         MyEvents[1].event_number = pcmcfg->core_event[1][0];
         MyEvents[1].umask_value  = pcmcfg->core_event[1][1];
         MyEvents[2].event_number = pcmcfg->core_event[2][0];
         MyEvents[2].umask_value  = pcmcfg->core_event[2][1];
         MyEvents[3].event_number = pcmcfg->core_event[3][0];
         MyEvents[3].umask_value  = pcmcfg->core_event[3][1];
	}    
	else if (pcmcfg->core_event_model == 2) //custom with pattern
	{
         MyEvents[0].event_number = 0x0e; // 
         MyEvents[0].umask_value  = 0x01;  // 
         MyEvents[1].event_number = 0x08; //
         MyEvents[1].umask_value  = 0x01;  // 
	}	
		
	if (pcmcfg->iMIC_event_model == 0) //default
	{
         MIC_Events[0].event_number = 0x04; //  
         MIC_Events[0].umask_value  = 0x03;  //
         MIC_Events[1].event_number = 0x04; // 
         MIC_Events[1].umask_value  = 0x0c;  // 
         MIC_Events_num             = 2;		 
	}    
	else if (pcmcfg->iMIC_event_model == 1) //custom with core_event
	{
         MIC_Events[0].event_number = pcmcfg->iMIC_event[0][0];
         MIC_Events[0].umask_value  = pcmcfg->iMIC_event[0][1];
         MIC_Events[1].event_number = pcmcfg->iMIC_event[1][0];
         MIC_Events[1].umask_value  = pcmcfg->iMIC_event[1][1];
         MIC_Events[2].event_number = pcmcfg->iMIC_event[2][0];
         MIC_Events[2].umask_value  = pcmcfg->iMIC_event[2][1];
         MIC_Events[3].event_number = pcmcfg->iMIC_event[3][0];
         MIC_Events[3].umask_value  = pcmcfg->iMIC_event[3][1];
         MIC_Events_num             = 4;		 
    }    
	else if (pcmcfg->iMIC_event_model == 2) //custom with pattern
	{
         MIC_Events[0].event_number = 0x04; //  CAS_READ
         MIC_Events[0].umask_value  = 0x03;  //
         MIC_Events[1].event_number = 0x04; //  CAS_WRITE
         MIC_Events[1].umask_value  = 0x0c;  // 	
		 
         MIC_Events[2].event_number = 0x02; //  PRE_COUNT:PAGE_MISS
         MIC_Events[2].umask_value  = 0x01;  // 
		 
         MIC_Events[3].event_number = 0x01; // ACT_COUNT:ALL
         MIC_Events[3].umask_value  = 0x0b;  // 		 
         MIC_Events_num             = 4;		 
    }

	
 }
 else //else from if (pcmcfg != NULL)
 {
         MyEvents[0].event_number = 0x2e;  // UOPS_ISSUED.ANY event number 
         MyEvents[0].umask_value  = 0x41;  // UOPS_ISSUED.ANY umask
         MyEvents[1].event_number = 0xd2;  // DTLB_LOAD_MISSES.ANY event number
         MyEvents[1].umask_value  = 0x08;  // DTLB_LOAD_MISSES.ANY umask		
         MyEvents[2].event_number = 0xd2;  // UOPS_ISSUED.ANY event number 
         MyEvents[2].umask_value  = 0x07;  // UOPS_ISSUED.ANY umask
         MyEvents[3].event_number = 0xd1;  // DTLB_LOAD_MISSES.ANY event number
         MyEvents[3].umask_value  = 0x02;  // DTLB_LOAD_MISSES.ANY umask		 
 }

		
    status = pcmInstance->program(PCM::CUSTOM_CORE_EVENTS, MyEvents);

/*		
  #ifdef CONFIG_PREFETCH_ENABLE
    pcmInstance->prefetch_model_for_each_core = pcmcfg->prefetch_model;
	if (pcmcfg->prefetch_model != 0)
	  pcmInstance->config_hardware_prefetch_disable(pcmInstance->prefetch_model_for_each_core);
  #endif
*/  
#else 
    if(pcmcfg)
        PCM_CONFIG = mystrdup(pcmcfg);

    if(PCM_CONFIG) {
        ifstream inpf(PCM_CONFIG);
        /**
         * \fn void PCM_initPerformanceMonitor(const char * perf_config)
         * \brief initializes the performance counters from given config file.
         * \param perf_config contains up to 4 counters given by name
         * event_number and umask_value on each line.
         * Example:
         * MyEvents[0].event_number = 0x0e; // UOPS_ISSUED.ANY event number 
         * MyEvents[0].umask_value  = 0x01;  // UOPS_ISSUED.ANY umask
         * MyEvents[1].event_number = 0x08; // DTLB_LOAD_MISSES.ANY event number
         * MyEvents[1].umask_value  = 0x01;  // DTLB_LOAD_MISSES.ANY umask
         * ... add your own event ids here for on-core counter 2 and 3
         */
        while (!inpf.eof() && numEvents < 4) {
            inpf >> MyEventNames[numEvents] 
                 >> hex >> MyEvents[numEvents].event_number 
                 >> hex >> MyEvents[numEvents].umask_value;
            numEvents++;
        }
        inpf.close();

        if (pcmInstance->good()) 
            status = pcmInstance->program(PCM::CUSTOM_CORE_EVENTS, MyEvents);
    }
    else {
        status = pcmInstance->program();
    }
#endif

#ifdef CONFIG_PREFETCH_ENABLE
    pcmInstance->prefetch_model_for_each_core = pcmcfg->prefetch_model;
	if (pcmcfg->prefetch_model != 0)
	  pcmInstance->config_hardware_prefetch_disable(pcmInstance->prefetch_model_for_each_core);
#endif
  
    switch (status)
    {
    case PCM::Success:
        break;
    case PCM::MSRAccessDenied:
        cout << "Access to Intel(r) Performance Counter Monitor has denied "
             << "(no MSR or PCI CFG space access)." << endl;
        break;
    case PCM::PMUBusy:
        cout << "Access to Intel(r) Performance Counter Monitor has denied "
             << "(Performance Monitoring Unit is occupied by other application)."
             << "Try to stop the application that uses PMU." << endl;
        cout << "Alternatively you can try to reset PMU configuration at your"
             << " own risk. Try to reset? (y/n)" << endl;
        char yn;
        std::cin >> yn;
        if ('y' == yn)
        {
            m->resetPMU();
            cout << "PMU configuration has been reset. Try to rerun the "
                 << "program again." << endl;
        }
        break;
    default:
        cout << "Access to Intel(r) Performance Counter Monitor has denied "
             << "(Unknown error)." << endl;
        break;
    }

}

void
PCM_start()
{
#if PER_CORE==1
  before_state = getCoreCounterState(0);
#elif PER_SOCKET==1
  before_state = getSocketCounterState(0); 
#else
  before_state = getSystemCounterState(); 
#endif
}

void
PCM_stop()
{
#if PER_CORE==1
  after_state = getCoreCounterState(0);
#elif PER_SOCKET==1
  after_state = getSocketCounterState(0); 
#else
  after_state = getSystemCounterState(); 
#endif
}

void
PCM_printResults()
{
    ostream * out;
    ofstream outf;

    if(PCM_OUT) {
      outf.open(PCM_OUT, ios::app);
        out = &outf; 
    }
    else {
        out = &cout;
    }

    (*out) << "Instructions-retired " 
           << getInstructionsRetired(before_state, after_state) << endl;
    (*out) << "Active-cycles " 
           << getCycles(before_state, after_state)  << endl;
    (*out) << "IPC " 
           << getIPC(before_state, after_state) << endl;

	if(numEvents == 0){
        (*out) << "L2Misses " 
               << getL2CacheMisses(before_state, after_state) << endl;
        (*out) << "L3Misses " 
               << getL3CacheMisses(before_state, after_state) << endl;
        (*out) << "L2HitRatio " 
               << getL2CacheHitRatio(before_state, after_state) << endl;
        (*out) << "L3HitRatio " 
               << getL3CacheHitRatio(before_state, after_state) << endl;

        (*out) << "CyclesLostDueL2CacheMisses " 
               << getCyclesLostDueL2CacheMisses(before_state, after_state) << endl;
        (*out) << "CyclesLostDueL3CacheMisses " 
               << getCyclesLostDueL3CacheMisses(before_state, after_state) << endl;
#if PER_CORE==0
     uint64 BytesReadFromMC = getBytesReadFromMC(before_state, after_state);
		(*out) << "BytesReadFromMC: " << BytesReadFromMC << endl;
	 uint64 BytesWrittenToMC = getBytesWrittenToMC(before_state, after_state);
		(*out) << "BytesWrittenToMC: " << BytesWrittenToMC << endl;
/*     uint64 PrecountPagemissFromMC = getPrecountPagemissFromMC(before_state, after_state);
		(*out) << "PrecountPagemissFromMC: " << PrecountPagemissFromMC << endl;
     uint64 ActcountFromMC = getActcountFromMC(before_state, after_state);
		(*out) << "ActcountFromMC: " << ActcountFromMC << endl;
		
	 (*out) <<	"Page Miss ratio: " << (double) PrecountPagemissFromMC/(double)((BytesReadFromMC+BytesWrittenToMC)/64) << endl;
	 (*out) <<	"Page Empty ratio: " << (double) (ActcountFromMC-PrecountPagemissFromMC)/(double)((BytesReadFromMC+BytesWrittenToMC)/64) << endl;
*/	 
#endif
	} 
    
    for(int i = 0; i < numEvents; i++) {
        (*out) << MyEventNames[i] << " "
               << getNumberOfCustomEvents(i, before_state, after_state) 
               << endl;
    }

    if(PCM_OUT)
        outf.close();
}

void
PCM_accumulate()
{
  // "Instructions-retired " 
  eventAcc[0] += getInstructionsRetired(before_state, after_state);
  //  (*out) << "Active-cycles " 
  eventAcc[1] += getCycles(before_state, after_state);
  //  (*out) << "IPC " 
  eventAcc[2] += getIPC(before_state, after_state);

  if(numEvents == 0){
    //(*out) << "L2Misses " 
    eventAcc[3] += getL2CacheMisses(before_state, after_state);
    //      (*out) << "L3Misses " 
    eventAcc[4] += getL3CacheMisses(before_state, after_state);
    //(*out) << "L2HitRatio " 
    eventAcc[5] += getL2CacheHitRatio(before_state, after_state);
    //        (*out) << "L3HitRatio " 
    eventAcc[6] += getL3CacheHitRatio(before_state, after_state);

    //(*out) << "CyclesLostDueL2CacheMisses " 
    eventAcc[7] += getCyclesLostDueL2CacheMisses(before_state, after_state);
    //(*out) << "CyclesLostDueL3CacheMisses " 
    eventAcc[8] += getCyclesLostDueL3CacheMisses(before_state, after_state);

#if PER_CORE==0
    //(*out) << "BytesFromMC " << 
    eventAcc[9] += getBytesReadFromMC(before_state, after_state);
    //(*out) << "BytesWrittenToMC " << 
    eventAcc[10] += getBytesWrittenToMC(before_state, after_state);
#endif
  } 
    
  for(int i = 0; i < numEvents; i++) {
    //(*out) << MyEventNames[i] << " "
    eventAcc[i+3] = getNumberOfCustomEvents(i, before_state, after_state);
  }
}

void
PCM_printAccumulators()
{
  ostream * out;
  ofstream outf;

  if(PCM_OUT) {
    outf.open(PCM_OUT, ios::app);
    out = &outf; 
  }
  else {
    out = &cout;
  }

  (*out) << "Instructions-retired " << eventAcc[0] << endl;
  (*out) << "Active-cycles "        << eventAcc[1]  << endl;
  (*out) << "IPC "                  << eventAcc[2] << endl;

  if(numEvents == 0){
    (*out) << "L2Misses "  << eventAcc[3] << endl;
    (*out) << "L3Misses "  << eventAcc[4] << endl;
    (*out) << "L2HitRatio "<< eventAcc[5] << endl;
    (*out) << "L3HitRatio "<< eventAcc[6] << endl;

    (*out) << "CyclesLostDueL2CacheMisses " 
	   << eventAcc[7] << endl;
    (*out) << "CyclesLostDueL3CacheMisses " 
	   << eventAcc[8] << endl;

    (*out) << "BytesFromMC " << eventAcc[9] << endl;
    (*out) << "BytesWrittenToMC " << eventAcc[10] << endl;
  } 
    
  for(int i = 0; i < numEvents; i++) {
    (*out) << MyEventNames[i] << " " << eventAcc[i+3] << endl;
  }

  if(PCM_OUT)
    outf.close();
}

void
PCM_cleanup()
{
 #ifdef CONFIG_PREFETCH_ENABLE	
	if (pcmInstance->prefetch_model_for_each_core != 0)
	  pcmInstance->config_hardware_prefetch_enable();
 #endif
  	
    pcmInstance->cleanup();

 #ifdef CONFIG_PREFETCH_ENABLE	
	if (pcmInstance->prefetch_model_for_each_core != 0)
	  pcmInstance->config_hardware_prefetch_enable();
  #endif
  
    if(PCM_CONFIG)
        free(PCM_CONFIG);

    if(PCM_OUT)
        free(PCM_OUT);
}

void
PCM_log(char * msg)
{
    ostream * out;
    ofstream outf;

    if(PCM_OUT) {
        outf.open(PCM_OUT, ios::app);
        out = &outf; 
    }
    else {
        out = &cout;
    }

    (*out) << msg << std::endl;
  
    if(PCM_OUT)
        outf.close();
}

#else

/* just placeholders */
void PCM_initPerformanceMonitor(const char * pcmcfg, const char * pcmout){}
void PCM_start(){}
void PCM_stop(){}
void PCM_printResults(){}
void PCM_cleanup(){}
void PCM_accumulate(){}
void PCM_printAccumulators(){}
void PCM_log(char * msg){}

#endif /*PERF_COUNTERS*/
