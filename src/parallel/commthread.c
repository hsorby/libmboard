/* $Id: commthread.c 2925 2012-07-20 14:12:17Z lsc $ */
/*!
 * \file parallel/commthread.c
 * \code
 *      Author: Lee-Shawn Chin 
 *      Date  : June 2009
 *      Copyright (c) 2009 STFC Rutherford Appleton Laboratory
 * \endcode
 * 
 * \brief Main routine run by communication thread
 * 
 * Routines within this file are expected to be run by a single thread
 * which will perform all communication. Therefore, the data structures
 * that it uses (e.g. MBI_Comm_Indices) may not be designed
 * to be thread-safe. 
 * So, if you are planning to enhance this setup such that more than 
 * one thread can be used to perform data movement and communication,
 * do be aware of thread-safety issues. This includes issues regarding
 * the thread-safety of the underlying MPI library.
 * 
 */
#include "mb_parallel.h"
#include "mb_commqueue.h"
#include "mb_syncqueue.h"
#include "mb_commroutines.h"

/* variables global to this file only */
static pthread_t ct;
static int TERMINATE = 0;

/* routines available to this file only */
static void *commthread_main(void *params);
inline static void processSyncRequests(void);
inline static void processPendingComms(void);
inline static void initiate_board_sync(MBt_Board mb);
inline static void commthread_sendTerminationSignal(void);
inline static void complain_and_terminate(int rc, int stage);

/* array of transition functions */
MBIt_commroutine MBI_trans_func[MB_COMM_STAGES_TOTAL];

/*! \brief Shortcut to check for termination flag
 * 
 * Can be used as <tt> if TermFlagSet(); doSomething();</tt>
 */
#define TermFlagSet() (0 != TERMINATE)

/*! 
 * \brief initialise and start communication thread
 * \return Return Code
 * 
 * This routine should only be called once during the initialisation
 * of the message board environment.
 * 
 * It initialises the Sync Queue and Comm Queue, then spawns the
 * communication thread. The new thread initialises with the
 * commthread_main() routine.
 * 
 */
int MBI_CommThread_Init(void) {
    
    int rc; 
    
    /* init sync queue */
    rc = MBI_SyncQueue_Init();
    assert(0 == rc);
    if (0 != rc) return rc;
    
    /* init comm queue */
    rc = MBI_CommQueue_Init();
    assert(0 == rc);
    if (0 != rc) return rc;
    
    /* initialise mapping of state to transition function */
    MBI_initialise_commroutine_mapping();
    
    /* start communication thread */
    rc = pthread_create(&ct, NULL, &commthread_main, NULL);
    assert(0 == rc);
    if (0 != rc) return MB_ERR_INTERNAL;
    
    P_INFO("Communication Thread initialised");
    return MB_SUCCESS;
}

/* 
 * \brief joins communication thread and clean up
 * \return Return code
 * 
 * Send the termination signal to the communication thread, then
 * wait for it to end.
 * 
 * Then, delete the Sync Queue and Comm Queue.
 * 
 */
int MBI_CommThread_Finalise(void) {
    
    int rc;

    /* set termination flag and send wake-up signal */
    commthread_sendTerminationSignal();
    
    /* wait for communication thread to end */
    rc = pthread_join(ct, NULL);
    assert(rc == 0);
    if (rc != 0) return MB_ERR_INTERNAL;

    /* sync queue should be empty by now */
    assert(MBI_SyncQueue_isEmpty());
    rc = MBI_SyncQueue_Delete();
    assert(0 == rc);
    if (0 != rc) return rc;
  
    /* comm queue should be empty by now */
    assert(MBI_CommQueue_isEmpty());
    rc = MBI_CommQueue_Delete();
    assert(0 == rc);
    if (0 != rc) return rc;
    
    P_INFO("Communication Thread finalised");
    return MB_SUCCESS;
}

/* 
 * \brief main function run by communication thread
 * \param[in] params Dummy argument (required by pthreads)
 * \return \c NULL
 * 
 * Continuously process the Sync Queue and Comm Queue until terminated.
 * 
 * If both queues are empty, sleep until woken up by signal (issued
 * by MBI_SyncQueue_Push() ).
 */
static void *commthread_main(void *params) {
    
    int rc;
    
    /* acknowledge+hide "unused parameter" compiler warnings */
    ACKNOWLEDGED_UNUSED(params);
    ACKNOWLEDGED_UNUSED(rc); /* used only for assertions */
    
    /* continue looping until termination flag is read */
    while (1) {
        
        /* ----- check for condition where we either sleep, or quit ----- */
        
        /* lock Sync Request Queue */
        rc = pthread_mutex_lock(&MBI_SRQLock); 
        assert(0 == rc);
        /* Comm Queue need not be locked since only used by this thread */
        
        if (MBI_SyncQueue_isEmpty() && MBI_CommQueue_isEmpty())
        {
            if (TermFlagSet())
            {
                /* unlock Sync Request Queue */
                rc = pthread_mutex_unlock(&MBI_SRQLock); 
                assert(0 == rc);
                break; /* breaking out of loop ends thread */
            }
            else
            {
                /* no work to do, sleep for a bit */
                rc = pthread_cond_wait(&MBI_SRQCond, &MBI_SRQLock);
                assert(0 == rc);
                
                /* unlock Sync Request Queue */
                rc = pthread_mutex_unlock(&MBI_SRQLock); 
                assert(0 == rc);
                
                continue; /* restart main loop */
            }
        }
        
        /* unlock Sync Request Queue */
        rc = pthread_mutex_unlock(&MBI_SRQLock); 
        assert(0 == rc);
        
        /* ----- if queue not empty, start processing ----- */
        while(!MBI_SyncQueue_isEmpty() || !MBI_CommQueue_isEmpty())
        {
            if (!MBI_SyncQueue_isEmpty()) processSyncRequests();
            else processPendingComms();
        }
    }
    
    pthread_exit(NULL); /* thread ends */
    
    return NULL;
}

/* 
 * \brief issue termination request to communication thread 
 * 
 * Sets termination flag and wakes comm thread up if sleeping 
 */
inline static void commthread_sendTerminationSignal(void) {
    
    int rc;
    ACKNOWLEDGED_UNUSED(rc); /* used only for assertions */
    
    P_INFO("Sending TERM signal to Communication Thread");
    /* set termination flag */
    TERMINATE = 1;
    
    /* wake comm thread if sleeping */
    rc = pthread_cond_signal(&MBI_SRQCond); 
    assert(0 == rc);
}

/*
 * \brief process pending sync requests
 * 
 * For each node in Sync Queue, pop from queue and process using
 * init_board_sync().
 *  
 */
inline static void processSyncRequests(void) {
    
    int rc;
    MBt_Board mb = MB_NULL_MBOARD;
    ACKNOWLEDGED_UNUSED(rc); /* used only for assertions */
    
    /* get first request */
    rc = MBI_SyncQueue_Pop(&mb);
    assert(rc == MB_SUCCESS);
    
    while (mb != MB_NULL_MBOARD) /* loop till queue empty */
    {
        initiate_board_sync(mb);
        
        /* get next request */
        rc = MBI_SyncQueue_Pop(&mb);
        assert(rc == MB_SUCCESS);
    }
}

/* 
 * \brief process pending communications
 * 
 * For each node in Comm queue, run the appropriate transition function.
 * 
 * The whole simulation should be aborted if the transition function returns
 * with an error. 
 * 
 * If transition routine returns ::MB_SUCCESS_2, it means communication has
 * completed for that node and can be removed from the queue. Once the node
 * is dequeued, a signal is sent to wake up any other threads that may be
 * waiting on the board.
 */
inline static void processPendingComms(void) {
    int rc;
    struct MBIt_commqueue *node, *next;
    MBIt_Board *board;
    
    /* loop thru comm queue */
    next = MBI_CommQueue_GetFirstNode();
    while (next)
    {
        node = next;
        next = node->next;
        
        assert(node->stage >= 0);
        assert(node->stage < MB_COMM_STAGES_TOTAL);
        
        /* run transtition function of given stage (state) */
        rc = (MBI_trans_func[node->stage])(node);
        if (rc != MB_SUCCESS)
        {
            /* Comm for this node has completed. Remove from list */
            if (rc == MB_SUCCESS_2) 
            {
                /* get reference to board */
                board = node->board;
                assert(board != NULL);
                assert(board->syncCompleted == MB_TRUE);
                /* dequeue comm node */
                MBI_CommQueue_Pop(node);

                /* send signal to wake main thread waiting on this board */
                rc = pthread_cond_signal(&(board->syncCond));
                assert(0 == rc);
            }
            else complain_and_terminate(rc, node->stage);
        }
    }
}

/* 
 * \brief initiate synchronisation of a board 
 * 
 * If the board has a filter function attached, put it in the ::MB_COMM_OLD_PRE_TAGGING
 * state, else, skip the tagging phase and go straight into the 
 * ::MB_COMM_OLD_PRE_PROPAGATION state.
 * 
 * The board is then added to the Comm Queue for processing by 
 * processPendingComms().
 * 
 */
inline static void initiate_board_sync(MBt_Board mb) { 
    
    int rc;
#ifdef _EXTRA_CHECKS
    MBIt_Board *board;
    
    /* check that mb is a valid board handle */
    board = (MBIt_Board*)MBI_getMBoardRef(mb);
    assert(board != NULL);
#endif
    
    ACKNOWLEDGED_UNUSED(rc); /* used only for assertions */

    /* push board into communication queue */
    assert(MBI_CONFIG.comm_protocol > MB_CONFIG_PROTO_START);
    assert(MBI_CONFIG.comm_protocol < MB_CONFIG_PROTO_END);
    switch (MBI_CONFIG.comm_protocol) 
    {
        case MB_CONFIG_PROTO_HANDSHAKE:
            rc = MBI_CommQueue_Push(mb, MB_COMM_HANDSHAKE_PRE_PROP);
            assert(rc == MB_SUCCESS);
            break;
        case MB_CONFIG_PROTO_OLD:
            rc = MBI_CommQueue_Push(mb, MB_COMM_OLD_PRE_TAGGING);
            assert(rc == MB_SUCCESS);
            break;
        default:
            assert(0); /* incomplete config setup ? */
    }
}

/* 
 * \brief complain then terminate simulation
 * 
 * Print out some information to stderr, then abort the whole simulation.
 * 
 */
inline static void complain_and_terminate(int rc, int stage) {
    
    char *reason = NULL;
    
    switch(rc) {
        case MB_ERR_MEMALLOC:
            reason = "Failed to allocate required memory";
            break;
        
        case MB_ERR_INTERNAL:
            reason = "Internal error (possibly a bug)";
            break;
            
        case MB_ERR_MPI:
            reason = "An MPI routine call failed";
            break;
    }
    
    fprintf(stderr, "=== ERROR (libmboard communication thread) ===\n");
    fprintf(stderr, " * Processor %d has terminated this simulation while processing\n", MBI_CommRank);
    fprintf(stderr, "   sync of boards (stage=%d).\n", stage);
    fprintf(stderr, " * Reason: %s\n\n", reason);
#ifndef _EXTRA_CHECKS
    fprintf(stderr, "For more information on where the error occured, please use the\n");
    fprintf(stderr, "debug version of libmboard\n");
#endif
    
    /* abort the simulation */
    MPI_Abort(MBI_CommWorld, 1);
}
