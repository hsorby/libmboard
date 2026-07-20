int rc;
int flag;
MBt_Board myboard;
myMessageType staticMsg;
/* .... more code .... */

rc = MB_Create(&myboard, sizeof(myMessageType));
/* .... more code .... */

/* create messages to add to board */
staticMsg.value = 200;


if ( MB_AddMessage(myboard, (void *)&staticMsg) != MB_SUCCESS )
{
    fprintf(stderr, "Error adding message to board\n");
    
    /* check valur of rc to determine reason of failure. Handle error */
    /* don't continue if error can't be handled */
    exit(1);
}

if ( MB_SyncStart(myboard) != MB_SUCCESS )
{
    fprintf(stderr, "Unable to begin synchronisation\n");
    
    /* check valur of rc to determine reason of failure. Handle error */
    /* don't continue if error can't be handled */
    exit(1);
}

/* check if synchronisation has completed */
MB_SyncTest(myboard, &flag);
if (flag == MB_TRUE) 
{
    
    /* a successful call to MB_SyncTest would already complete
     * the communication and unlock the board. MB_SyncComplete()
     * is not needed.
     */
    process_message_board();
}
else
{
    
    do_something_else_first();
    
    if ( MB_SyncComplete(myboard) != MB_SUCCESS ) /* wait till sync done */
    {
        fprintf(stderr, "Unable to begin synchronisation\n");
        
        /* check valur of rc to determine reason of failure. Handle error */
        /* don't continue if error can't be handled */
        exit(1);
    }
    
    process_message_board();
}

/* .... rest of program .... */
