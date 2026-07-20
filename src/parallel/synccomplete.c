/* $Id: synccomplete.c 1930 2009-07-02 13:47:53Z lsc $ */
/*!
 * \file parallel/synccomplete.c
 * \code
 *      Author: Lee-Shawn Chin 
 *      Date  : April 2008 
 *      Copyright (c) 2008 STFC Rutherford Appleton Laboratory
 * \endcode
 * 
 * \brief Parallel implementation of MB_SyncComplete()
 * 
 */
#include "mb_parallel.h"

/*!
 * \brief Completes the synchronisation of a board.
 * \ingroup MB_API
 * \param[in] mb MessageBoard Handle
 * 
 * (more ...)
 * 
 * Synchronisation of a null board (::MB_NULL_MBOARD) is valid, and will 
 * return immediately with ::MB_SUCCESS
 * 
 * Possible return codes:
 *  - ::MB_SUCCESS
 *  - ::MB_ERR_INVALID  (Invalid board. E.g. if board is not being synchronised.)
 * 
 */
int MB_SyncComplete(MBt_Board mb) {
    
    int rc;
    MBIt_Board *board;
    #ifdef _EXTRA_INFO
    double sleep_time;
    #endif
    
    /* Check for NULL message board */
    if (mb == MB_NULL_MBOARD) return MB_SUCCESS;

    /* get object mapped to mb handle */
    board = (MBIt_Board *)MBI_getMBoardRef(mb);
    if (board == NULL) return MB_ERR_INVALID;

    /* check if board is locked */
    if (board->locked == MB_FALSE) return MB_ERR_INVALID;
    
    /* capture lock */
    rc = pthread_mutex_lock(&(board->syncLock));
    assert(0 == rc);
    if (rc != 0) return MB_ERR_INTERNAL;

    /* if sync not complete, wait till it is */
    while(board->syncCompleted != MB_TRUE)
    {
        #ifdef _EXTRA_INFO
        sleep_time = MPI_Wtime();
        #endif
        P_INFO("Board (%d) sync incomplete. Sleeping ... ", (int)mb);
        rc = pthread_cond_wait(&(board->syncCond), &(board->syncLock));
        assert(rc == 0);
        
        #ifdef _EXTRA_INFO
        P_INFO("Board (%d) sync completed. Asleep for %.3f seconds", 
                (int)mb, MPI_Wtime() - sleep_time);
        #endif
    }
    
    /* release lock */
    rc = pthread_mutex_unlock(&(board->syncLock));
    assert(0 == rc);
    if (rc != 0) return MB_ERR_INTERNAL;
    
    /* unlock board */
    board->locked = MB_FALSE;
    P_INFO("Board (%d) unlocked", (int)mb);
    
    /* return */
    return MB_SUCCESS;
}
