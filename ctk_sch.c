/*
 * License / Copyright (c) 2014
 * You can use the code at will.
 * :)
 */

/**
 * \file
 *         Impletation of scheduling mechasim for contiki
 * \author
 *         Sen yu.Huang <476814099@qq.com>
 */

#include "ctk_sch.h"

/*---------------------------------------------------------------------------*/
static struct ctimer ct_sch_tick;
struct process *process_sch_crt = NULL;
//struct sch_process *sch_process_list = NULL;
struct sch_process sch_process_current_value;
struct sch_process *sch_process_current = NULL;

struct run_queue run_queue_init;
struct run_queue *run_queues = NULL;
/*---------------------------------------------------------------------------*/
void
init_runqueues() {
	//run_queue_init stored the data, and run_queues points to it.
	uint8_t i;
	
	/*
	struct sch_add_process 	add = {11, 11, 0, 0, 0, 0};
	struct process 					old = {NULL, "HI", NULL};
	struct sch_process	pvalue = {NULL, NULL, NULL};
	struct sch_process	processvalue = 
		{(struct process *)&old, (struct sch_add_process *)&add};

	struct sch_process	*p = &pvalue;
	struct sch_process	*process = &processvalue;
*/
	run_queue_init.active = &run_queue_init.arrays[0];
	run_queue_init.expired= &run_queue_init.arrays[1];
	run_queue_init.arrays[0].nr_active = 0;
	run_queue_init.arrays[0].map 			 = 0;
	run_queue_init.arrays[1].nr_active = 0;
	run_queue_init.arrays[1].map 			 = 0;
	for (i = 0; i <= 15; ++i) {
		LIST_STRUCT_INIT(&(run_queue_init.arrays[0].queue[i]), list);
		LIST_STRUCT_INIT(&(run_queue_init.arrays[1].queue[i]), list);
	}

	run_queues = &run_queue_init;
	/*
printf("pushing...\n");
	list_push(run_queues->arrays[0].queue[0].list, process);

	p = list_head(run_queues->arrays[0].queue[0].list);
printf("head -- prio:%d correct:%d\n", p->add->prio, process->add->prio);
	p = list_pop(run_queues->arrays[0].queue[0].list);
printf("pop  -- prio:%d correct:%d\n", p->add->prio, process->add->prio);
*/
}

/*---------------------------------------------------------------------------*/
void
set_need_resched (struct sch_process *p)
{
	p->add->flag = NEED_RESCHED;
}

uint8_t
is_set_need_resched (struct sch_process *p)
{
	if (p->add->flag == NEED_RESCHED) {
		return 1;	
	} else {
		return 0;
	}
}

uint8_t
get_base_timeslice (struct sch_process *p) 
{
	uint8_t base  = 100;
	uint8_t sprio = p->add->static_prio;

	if (sprio < 10) {
		//sprio = [ 0, 9]
		base = (15-sprio) * 20;
	} else {
		//sprio = [10,15]
		base = (15-sprio) * 5;
	}
	
	return base;
}

void
update_timeslice(struct sch_process *p)
{
	//TO DO:
	//Round-Robin
	//RR seems simply, so I impletate RR first.
	if (p->add->policy == SCHED_RR &&
			!--p->add->timeslice) {
		p->add->timeslice = get_base_timeslice(p);
		p->add->first_timeslice = 0;
		set_need_resched(p);		
	}

	//conventional process
	/*
	-- (p->add->timeslice);
	if (p->add->timeslice <=0 ) {
		//quantum is exhaousted
		dequeue_task();
		set_need_reshed();
		update_prio();
	}
	*/
}

/*---------------------------------------------------------------------------*/
uint8_t check_map(uint16_t map)
{
	uint16_t mask = 0x01;
	uint8_t  offset = 0;	//prio
	for (offset = 0; offset <= 15; ++offset)	{
			if ((mask & map) != 0)	{	break;	}
			mask = mask << 1;
	}

	if (offset > 15) { offset = 100; }	//more easy to judge if there is no task.
	return offset;
}

// to change parameter, we have to use pointer
void setbit_map(uint16_t *pmap, uint8_t offset) 
{
	//be sure offset = [0,15], not judge here.
	*pmap = *pmap | (0x01 << offset);
}

void clrbit_map(uint16_t *pmap, uint8_t offset)
{
	*pmap = *pmap & (~(0x01 << offset));
}

void set_map(uint16_t *pmap)
{
	*pmap = *pmap | ((~(0x01)) | 0x01);
}

void clr_map(uint16_t *pmap)
{
	*pmap = *pmap & 0x00;
}

/*---------------------------------------------------------------------------*/
/*
 * 		It's invoked in the callback fuction of the ctimer(ct_sch_tick), which is
 * set in the scheduler_process.
 * 		In this function, I need to konw the "current process as in Linux", but 
 * when I invoke the schedule_tick(), the process_current is always the
 * scheduler_process, so I define another global varible called process_sch_crt
 * to record the last process. Detailes in macro PROCESS_YIELD_SCH().
 */
void
schedule_tick(struct sch_process *p)
{
	//time_stamp_last_tick = clock_time();
	/*
	 *	sch_process_crt is "struct sch_process", differ from 
	 *  process_sch_crt which is "struct process"
	 *	It's easy to mix, so I deliver a para to this function
	 */
	//TO DO: init the sch_process_crt
	//struct sch_process *p = sch_process_crt;

	// step 3
	// note that realtime process are actice.
	//if (p->add->flag == NEED_RESCHED) { goto end;}

	update_timeslice(p);

	//if (p->add->timeslice <= 0) { p->add->flag = NEED_RESCHED; }

}

/*---------------------------------------------------------------------------*/
void
schedule()
{
	struct sch_process *p = NULL;
	uint16_t map = run_queues->arrays->map;
	uint16_t *pmap = &(run_queues->arrays->map);
	uint8_t  ret = 100;
	struct sch_process *tmp_sch_process = NULL;

#if(0)
// testing the map operation function
uint8_t  i = 15;	
map = 0x01<<i;
ret = check_map(map);		// set the map, attend to get i here
printf("check_map [%d] - offset [%d]\n", ret, i);

setbit_map(&map, i-1);	// set a higher prio bit, attend to get i-1 here
ret = check_map(map);
printf("check_map [%d] - offset [%d]\n", ret, i-1);

clrbit_map(&map, i-1);	// clr the higher prio bit, attend to get i here
ret = check_map(map);
printf("check_map [%d] - offset [%d]\n", ret, i-1+1);

#else
// scheduling
	ret = check_map(map);
	// printf("<-map  -> queue [%d]\n", ret);
	printf("\t\t\t\t\t\tqueue [%d]\n", ret);
	// if !(0<=ret<=15), there is no task in the queue, end.
	if (ret < 0 || ret > 15) {	goto label_after_check_list;	}

//printf("pop\n");
	p = list_pop(run_queues->arrays->queue[ret].list);
	//the "STATE" need to change
	//save context and post_synch enven
	// to make clear, I use a "p" here, which should be sch_process_current
	sch_process_current = p;
	tmp_sch_process = sch_process_current;
//printf("posting the [%s] ...\n", p->old->name);
	process_post_synch(p->old, PROCESS_EVENT_POLL, NULL);
//printf("posted state[%d]\n", p->add->state);
	sch_process_current = tmp_sch_process;

	/*
	 * update map
	 * TODO:
	 * pushing back means the only the highest prio process can be called
	 * so it needs the timeslice.
   */
	if (p->add->state == SCH_PROCESS_STATE_RUNNING) {
		// if the posted process(stroed in p) is still running
		// we put it back to the list
		list_push(run_queues->arrays->queue[ret].list, p);
	} else {
		p = list_head(run_queues->arrays->queue[ret].list);
		if (p == NULL) {				
			// if not running, and the queue[ret] is empty
			// we clear the "ret"-th bit in map
			printf("\t\t\t\t\t\tqueue[%2d] is empty now!\n", ret); 
			clrbit_map(pmap, ret);	
		}
	}

label_after_check_list:
	while (0)	{ //do nothing here
	}
	process_poll(process_current);
#endif
}
/*---------------------------------------------------------------------------*/
void
ct_callback_sch_tick(void *ptr)
{
	printf("===========in intterupt=======\n");
	//printf("\t\t\t[schedule tick]\n");
	schedule_tick(sch_process_current);
	//printf("<-\"crt\"-> sch[%s]---\n", sch_process_current->old->name);
	if (is_set_need_resched(sch_process_current)) {
		//printf("\t\t\t[schedule]\n");
		schedule();
	}

	ctimer_reset(&ct_sch_tick);	
}

/*---------------------------------------------------------------------------*/
/*
 *	IMPORTANT: Spends a lot time with the "list lib"
 *	To use the list lib function in contiki, you have to define a *next pointer 
 *in the first place of a struct. MUST at the first place!
 *	For example: struct structname{struct structname * next; int otherfield;}
 *	reason: look up the list_push function, there is a forced translation
 */
#define PROCESS_SCHED(name, strname, prio)				\
	PROCESS(name, strname);		\
	struct sch_add_process add_##name = {prio, prio, 0, 1, DEFAULT_FLAG, DEFAULT_POLICY, SCH_PROCESS_STATE_RUNNING};	\
	struct sch_process sch_##name = {NULL, (struct process*)(&name), (struct sch_add_process*)(&(add_##name))};

/*---------------------------------------------------------------------------*/
/*
 * 	This macro must invoked in a function, because it assign the init value 
 * Maybe it's not a good choice to do so, but it do work.
 * 	In this macro, I save the "current sch process", push it to the prio_map_list
 * and set the corresponding bit in map.
 */
#define sch_process_init_all(...)	//not yet
/*confilict here, so use proname in the macro*/
//run_queue_init.arrays[0].queue[i]
#define sch_process_init(proname)	\
	do{	\
		sch_process_current = &sch_##proname;	\
		setbit_map( &(run_queues->arrays[0].map), (add_##proname).prio );	\
		list_push(run_queues->arrays[0].queue[(add_##proname).prio].list, sch_process_current);	\
		} while(0)
//		printf("<-test -> name - [%s] \n      sch name - [%s]\n", proname.name, (sch_##proname).old->name);
//		list_push(run_queues->arrays[0].queue[(add_##proname).prio].list, sch_process_current);
//	}while(0)

/*---------------------------------------------------------------------------*/
#define PROCESS_YIELD_SCH()	do {\
														PROCESS_YIELD();\
														process_sch_crt = process_current;\
														if(0){sch_process_current->old = process_current;}\
														;\
														} while(0)

/*---------------------------------------------------------------------------*/
#define SCH_PROCESS_EXIT() do {	\
	sch_process_current->add->state = SCH_PROCESS_STATE_NONE;	\
	} while(0);
/*---------------------------------------------------------------------------*/
void
sch_setprio(struct sch_process *p, uint8_t i) {
	list_remove(run_queues->arrays[0].queue[p->add->prio].list, p);
	list_push(run_queues->arrays[0].queue[i].list, p);
	setbit_map(&(run_queues->arrays[0].map), i);
}
#define SCH_SETPRIO_PROCESS(proname, i) do {\
	sch_setprio(&(sch_##proname), i);	\
	} while(0)
/*---------------------------------------------------------------------------*/
/*
 *		I don't know why
 * when calling the last process defined in AUTOSTART_PROCESSES
 * the schedule() calls the first process (scheduler_process), the others go well
 * Maybe it needs a watcher here, So I add a end_process process.
 */
PROCESS_SCHED(scheduler_process,  "scheduler process", 0);
PROCESS_SCHED(example_process1,   "example process1", 1);
PROCESS_SCHED(example_process2,   "example process2", 10);
PROCESS_SCHED(example_process3,   "example process3", 10);
PROCESS_SCHED(example_process4,   "example process4", 10);
PROCESS_SCHED(example_process5,   "example process5", 6);
PROCESS_SCHED(example_process6,   "example process6", 6);
PROCESS_SCHED(end_process,   			"end process", 15);
AUTOSTART_PROCESSES(&scheduler_process, 
										&example_process1, &example_process2, &example_process3, 
										&example_process4, &example_process5, &example_process6,
										&end_process);
//declare
//#define TESTLIST
void test_list();

	struct sch_process	pvalue = {NULL, NULL, NULL};
	struct sch_process	*p = &pvalue;
	struct sch_process	*process;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(scheduler_process, ev, data)
{
  PROCESS_BEGIN();	

#ifdef TESTLIST
	test_list();
	//PROCESS_YIELD_SCH();
#else

printf("<-init -> runqueues\n");	
	init_runqueues();
printf("<-begin-> scheduler process - [%s]\n", process_current->name);
printf("<-init -> sch process\n")	;
	sch_process_init(example_process1);
	sch_process_init(example_process2);
	sch_process_init(example_process3);
	sch_process_init(example_process4);
	sch_process_init(example_process5);
	sch_process_init(example_process6);
	sch_process_init(end_process);
//	sch_process_current = NULL;
#if (0)
//e test to see if it really push into the list!
printf("after push\n");
p = (struct sch_process	*)list_head(run_queues->arrays[0].queue[10].list);
printf("after get\n");
printf("example_process3 fuc[%s] [%s]\n", example_process3.name, sch_example_process3.old->name);
printf("<-test -> name - [%d][%d]\n", p->add->prio, p->old->thread);
#endif

printf("<-timer-> set\n");
	ctimer_set(&ct_sch_tick, QUANTUM_DURATION, ct_callback_sch_tick, NULL);

	while (1) {
		//printf("<-in   -> scheduler_process before yield\n");
		PROCESS_YIELD_SCH();
		//printf("<-in   -> scheduler_process after  yield\n");
	}

	printf("<-end  -> scheduler process - [%s] - should not reach here\n", 
					process_current->name);

#endif
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_process1, ev, data)
{
  PROCESS_BEGIN();	

	static uint8_t i1 = 1;
	while (1) {
		PROCESS_YIELD_SCH();
		printf("\t\t\t\t\t\t[in] - %s\n", sch_process_current->old->name);
		SCH_SETPRIO_PROCESS(example_process2, 3);
		i1--;
		if (!i1) {SCH_PROCESS_EXIT();};
	}
	
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_process2, ev, data)
{

  PROCESS_BEGIN();

	static uint8_t i2 = 1;
	while (1) {
		PROCESS_YIELD_SCH();
		printf("\t\t\t\t\t\t[in] - %s\n", sch_process_current->old->name);
		i2--;
		if (!i2) {SCH_PROCESS_EXIT();};
	}
	
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_process3, ev, data)
{
  PROCESS_BEGIN();

	while (1) {
		PROCESS_YIELD_SCH();
		printf("\t\t\t\t\t\t[in] - %s\n", sch_process_current->old->name);
	}

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_process4, ev, data)
{
  PROCESS_BEGIN();

	while (1) {
		PROCESS_YIELD_SCH();
		printf("\t\t\t\t\t\t[in] - %s\n", sch_process_current->old->name);
	}

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_process5, ev, data)
{
  PROCESS_BEGIN();

	static uint8_t i5 = 2;
	while (1) {
		PROCESS_YIELD_SCH();
		printf("\t\t\t\t\t\t[in] - %s\n", sch_process_current->old->name);
		i5--;
		if (!i5) {SCH_PROCESS_EXIT();};
	}

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_process6, ev, data)
{
  PROCESS_BEGIN();

	static uint8_t i6 = 2;
	while (1) {
		PROCESS_YIELD_SCH();
		printf("\t\t\t\t\t\t[in] - %s\n", sch_process_current->old->name);
		i6--;
		if (!i6) {SCH_PROCESS_EXIT();};
	}

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(end_process, ev, data)
{
  PROCESS_BEGIN();

	while (1) {
		PROCESS_YIELD_SCH();
	}

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

void
test_list()
{
	struct prio_map_list s_list_test[2];
	struct sch_add_process 	add = {0, 0, 0, 0, 0, 0};
	struct process 					old = {NULL, "HI", NULL};
	struct sch_process	pvalue = {NULL, NULL};
	struct sch_process	processvalue = 
		{NULL, (struct process *)&old, (struct sch_add_process *)&add};

	struct sch_process	*p = &pvalue;
	struct sch_process	*process = &processvalue;
	
	process->add->prio = 10;

printf("initing...\n");
	LIST_STRUCT_INIT(&s_list_test[0], list);
printf("pushing...\n");
	list_push(s_list_test[0].list, process);

	p = list_head(s_list_test[0].list);
printf("head -- prio:%d correct:%d\n", p->add->prio, process->add->prio);
	p = list_pop(s_list_test[0].list);
printf("pop  -- prio:%d correct:%d\n", p->add->prio, process->add->prio);
}
