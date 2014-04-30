#ifndef _CTK_SCH_H_
#define _CTK_SCH_H_

#include "contiki.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "./ctk_sch.h"
#include "lib/list.h"

#include <stdio.h> /* For printf() */

/*---------------------------------------------------------------------------*/
#define QUANTUM_DURATION 	(CLOCK_SECOND)
//#define TESTLIST					0
#define DEFAULT_POLICY 	SCHED_RR
#define DEFAULT_FLAG		NEED_RESCHED

// in process.c
#define SCH_PROCESS_STATE_NONE        0
#define SCH_PROCESS_STATE_RUNNING     1
#define SCH_PROCESS_STATE_CALLED      2
/*---------------------------------------------------------------------------*/
enum {
	ACTIVE = 1,
	EXPIRED,
	NEED_RESCHED,
	
	FLAG_IN_SCH_ADD_PROCESS
};

enum {
	SCHED_RR = 1, // Round-Robin realtime process
	SCHED_CV,			// convention process
	SCHED_NR,			// Non-Round-Robin realtime process
	
	POLICY_IN_SCH_ADD_PROCESS
};

struct sch_add_process {
	uint16_t 	prio;			  	/*[0,15]*/
	uint16_t 	static_prio;  /*[0,15]*/
	uint8_t  	timeslice; 	  /*[0,2^8]*/
	uint8_t		first_timeslice; /*0 or 1*/
	uint8_t  	flag;					/*NEED_RESCHED and so on*/
 	uint8_t		policy;				/*how to schedule the process*/
	uint8_t		state;
};
/* 
 * The list lib provided by contiki CAN NOT be declared as array form: list lname[100]
 * So I wraped it in a struct. Moreover, LIST_STRUCT() is used to declare a list in
 * the declaration of a struct. More content about list lib, look up lib/list.h
 */

struct prio_map_list {
	LIST_STRUCT(list);
};
struct prio_array_t {
	uint8_t  nr_active;				/*number of active process in the list*/
	uint16_t map;
	struct prio_map_list queue[15];	/*process to a specific prio*/
};
struct run_queue{
//	struct process *contiki_process;	/*the process define in contiki kernel*/
	struct prio_array_t *active;
	struct prio_array_t *expired;
	struct prio_array_t arrays[2];	/*only conside active array here*/
};
/*
 * 		old points to the field that define in the contiki kernel, old means the
 * "old process structure". new points to the field that define in struct 
 * sch_add_process, add means "the adding field in new process structure"
 */
struct sch_process {
	struct shc_process *next;
	struct process *old;
	struct sch_add_process *add;
};
#endif
