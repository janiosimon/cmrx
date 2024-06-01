/** @defgroup os_isr Interrupt service routines
 * @ingroup os 
 *
 * Routines available from running interrupt service routine context.
 *
 * By default, all the interrupt service routines have higher priority than the kernel
 * service call handler. CMRX is not expecthing this to change. This configuration causes
 * that service call handler is not callable from within interrupt service handlers. 
 * Following routines are provided so that interrupt service routines can ask kernel 
 * to perform certain tasks while in servicing interrupt.
 *
 * Never perform direct calls into kernel other than methods listed in this group. These
 * methods are not reentrant and calling them from within interrupt handler may corrupt 
 * kernel internal state.
 * @{ 
 */
#include <cmrx/ipc/isr.h>
#include <conf/kernel.h>
#include <cmrx/os/runtime.h>
#include <cmrx/os/sched.h>
#include <cmrx/os/context.h>

void isr_kill(Thread_t thread_id, uint32_t signal)
{
	if (thread_id < OS_THREADS
			&& signal < 32
			&& (
				os_threads[thread_id].state == THREAD_STATE_READY
				|| os_threads[thread_id].state == THREAD_STATE_RUNNING
				|| os_threads[thread_id].state == THREAD_STATE_STOPPED
			   )
	   )
	{
		os_threads[thread_id].signals |= 1 << signal;
		if (os_threads[thread_id].state == THREAD_STATE_STOPPED)
		{
			os_threads[thread_id].state = THREAD_STATE_READY;
		}
		/* this stuff is dodgy and would deserve complete 
		 * rewrite to be more robust and more predictable */
		if (os_threads[thread_id].priority > os_threads[os_get_current_thread()].priority)
		{
			if (schedule_context_switch(os_get_current_thread(), thread_id))
			{
				os_set_current_thread(thread_id);
			}
		}
	}
}

/** @} */
