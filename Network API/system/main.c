/*  main.c  - main */

#include <xinu.h>

char* rawServerIP;
uint32 remoteServerIP;
uint16 remotePort = 44000;
uint32 localIP;
uint32 partnerIP;
uint16 localPort;
uint16 partnerPort;

uint16 smallerPort = 55289;
uint16 greaterPort = 55290;

uint16 MESSAGE_MAX_LENGTH = 100;

uid32 chat_slot;

pid32 senderProcess, receiverProcess;

/*------------------------------------------------------------------------
 * convertToStringIP - converts IP from uint32 to to dotted string IP
 *------------------------------------------------------------------------
 */
void convertToStringIP(
	uint32 ip, 		/* uint32 format of IP address */
	char* buf		/* buffer where the dotted string format will be stored */
	)
{   
	uint32 b0 = ip & 0xFF, remain = ip >> 8;
    uint32 b1 = remain & 0xFF; remain = remain >> 8;
    uint32 b2 = remain & 0xFF; remain = remain >> 8;
    uint32 b3 = remain & 0xFF;
	sprintf(buf, "%d.%d.%d.%d", b3, b2, b1, b0);

}


/*------------------------------------------------------------------------
 * echoMessage - send message to echo server and receive a reply
 *------------------------------------------------------------------------
 */
syscall echoMessage(
)

{
	
    int32 retval;						/* For return value of udp_recv */

	/* Get the addess of the remote server */
	remoteServerIP = dnslookup("xinu14.cs.purdue.edu");

    /* Verify that we have obtained a local IP address */
    if (getlocalip() == SYSERR) {
		return SYSERR;
	}  
    localIP = (uint32) getlocalip();

    /* Choosing one of the ports for part 1 */
	uint16 echoLocalPort = greaterPort;    

	/* call udp_register and check for error */
	uid32 slot = udp_register(remoteServerIP, remotePort, echoLocalPort);
    if (slot == SYSERR) {
        kprintf("Could not register a udp port.");
        return SYSERR;
    }

    /* Send a request message to the echo server */
    char* messageToSend = "Iamwritingacompletelyrandommessagesothatitdoesnotoverlapwithanyotherstudentsbecauseoverlappingwouldcauseproblem.";
	
	char messageToReceive[strlen(messageToSend)];

    retval = udp_send(slot, messageToSend, strlen(messageToSend));
    if (retval == SYSERR) {
		fprintf(stderr,"Cannot send to the server\n");
		udp_release(slot);
		return SYSERR;
	}

    /* Read the response from the remote server */
	retval = udp_recv(slot,messageToReceive, strlen(messageToSend), 500);

	if ( (retval == SYSERR) || (retval == TIMEOUT) ) {
		/* if there is error, or timeout, server might not be running */
		kprintf("Failure. Make sure the remote server is working and try again.\n");
		udp_release(slot);
		return SYSERR;
	}
    
	/* check if the sent message and the received message are the same  */ 
    if ( strncmp(messageToSend, (char*) messageToReceive, strlen(messageToSend)) == 0 ) {
		/* If same, return success */
        kprintf("Message echoed: %s\n", messageToSend);
        kprintf("success\n");

    } else {

		/* else return a failure */
		kprintf("Failure. Make sure the remote server is working and try again.\n");
	
	}

	/* release the slot */
	udp_release(slot);

    return OK;
}

/*------------------------------------------------------------------------
 * runXinuShell - run the Xinu shell (for usage during termination)
 *------------------------------------------------------------------------
 */
syscall runXinuShell(
) 
{
	/* Run the Xinu shell */
	recvclr();
	resume(create(shell, 8192, 50, "shell", 1, CONSOLE));
	return OK;
}

/*------------------------------------------------------------------------
 * alocatePorts - allocate local and remote ports according to IP addess sizes
 *------------------------------------------------------------------------
 */
syscall allocatePorts(

)
{
	/* If local ip greater then partnerIP, set the bigger port as localport
		and vice versa */
	if (localIP > partnerIP) {
		localPort = greaterPort;
		partnerPort = smallerPort;
	} else if ( localIP < partnerIP ) {
		localPort = smallerPort;
		partnerPort = greaterPort;
	} else {  
		/* If user inputs the same address, return error */
		kprintf("Both the ports are same. This should not happen!!");
		return SYSERR;
	}

	return OK;
}

/*------------------------------------------------------------------------
 * sendChatMessage - sender process for the chat system
 *------------------------------------------------------------------------
 */
syscall sendChatMessage(

	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();

	
	int32 retval;

	kprintf("\nStart the conversation! Say something: ");
	
	uint32 count = 0;		/* counter to count successive blank lines */
	
	/* repeatedly call send */
	while (1) {

		char sentMessage[200] = {};
		/* Get input message from user */
		fgets(sentMessage, 100, stdin);
		
		/* discard the newline character */
		sentMessage[strlen(sentMessage)-1] = '\0';
		
		/* send message and check for error */
		retval = udp_send(chat_slot, sentMessage, strlen(sentMessage));
		if (retval == SYSERR) {
			kprintf("Cannot send message to partner\n");
			udp_release(chat_slot);
			return SYSERR;
		} 

		/* blank message sent, increment count. Otherwise, reset count */
		if (strlen(sentMessage) == 0) {
			count++;
		} else {
			/* if not blank, reset the count to check for consecutive */
			count = 0;
		}
		
		/* if two consecutive blank messages sent, kill receiver process,
			start xinu shell, and release slot */
		if (count >= 2) {

			kprintf("\nTwo consecutive blank messages sent. Chat ended!\n");
			kill(receiverProcess);
			udp_release(chat_slot);
			runXinuShell();
			break;

		} else if (count == 1) {
			/* give warning message to user if one blank line is sent */
			kprintf("1 blank line sent. Send one more blank line (press enter) to end chat.\n");
		}
		

	}

	restore(mask);		/* Restore interrupts */
	return OK;
}

/*------------------------------------------------------------------------
 * receiveChatMessage - receiver process for the chat system
 *------------------------------------------------------------------------
 */
syscall receiveChatMessage(
	) 

{
	intmask	mask;			/* Saved interrupt mask		*/
	
	mask = disable();		/* disable interrupts */

	uint32 recv_count = 0;	/* counter to check for two blank lines received */
	int32 retval;			/* to have the return value of udp_recv */

	/* repeatedly call udp_recv */
	while (1) {

		char receivedMessage[200] = {};		/* buffer for message received */

		/* Read the response from the chat partner */
		retval = udp_recv(chat_slot, receivedMessage, MESSAGE_MAX_LENGTH, 500);

		/* If there is error, release slot and return error */
		if ( (retval == SYSERR) ) {
			udp_release(chat_slot);
			restore(mask);
			return SYSERR;

		} else if (retval == TIMEOUT) {
			/* If TIMEOUT is returned, skip rest and repeat the loop */
			continue;

		} else {
			/* If actual message received */

			/* if message is blank, increment counter */
			if (strlen(receivedMessage) == 0) {
				recv_count++;
			} else {
				/* if message not blank, reset counter */
				recv_count = 0;
			}

			/* Print the message */
			kprintf("\nMessaged received: %s\nYou: ", receivedMessage);
		}

		/* If two consecutive blank messages received, notify the user,
			kill the sender process, releases slot, and run xinu shell */ 
		if (recv_count >= 2) {
			kprintf("\nTwo consecutive blank messages received. Chat ended!\n");
			kill(senderProcess);
			udp_release(chat_slot);
			runXinuShell();
			break;
		}

	}

	restore(mask);		/* Restore interrupts */
	return OK;
}




/*------------------------------------------------------------------------
 * startChat - gets relevant IP addresses and port and start the chat system
 *------------------------------------------------------------------------
 */
syscall startChat(

)
{
	intmask	mask;			/* Saved interrupt mask		*/
	
	mask = disable();		/* disable interrupts */

    /* Verify that we have obtained an IP address */
    if (getlocalip() == SYSERR) {
		restore(mask);		/* Restore interrupts */
		return SYSERR;
	}  

	/* get local IP address */
    localIP = (uint32) getlocalip();
	char localIPString[20] = {};
	
	/* convert to dotted string format and print local IP address */
	convertToStringIP(localIP, localIPString);
	kprintf("Local IP address: %s\n", localIPString);

	/* get IP address of partner */
	char partnerIPString[20] = {};
	kprintf("Input the IP address of the user you want to chat with: ");
	fgets(partnerIPString, 20, stdin);

	/* discard the newline character */
	partnerIPString[strlen(partnerIPString)-1] = '\0';

	/* convert dotted string format to uint32 format and check for error */
	if ( dot2ip(partnerIPString, &partnerIP) == SYSERR ) {
		restore(mask);		/* Restore interrupts */
		return SYSERR;
	}

	/* allocate ports according to IP size and check for error */
	if (allocatePorts() == SYSERR) {
		restore(mask);		/* Restore interrupts */
		return SYSERR;
	}

	/* call udp_register with partner information and check for error */
    chat_slot = udp_register(partnerIP, partnerPort, localPort);
    if (chat_slot == SYSERR) {
        kprintf("Could not register a udp port with partner information.");
		restore(mask);		/* Restore interrupts */
        return SYSERR;
    }		
	
	/* create and resume the sender and receiver process */
	receiverProcess = create(receiveChatMessage, 1024, 10, "receiveChatMessageProcess", 0);
	senderProcess = create(sendChatMessage, 1024, 10, "sendChatMessageProcess", 0);
	
	resume(receiverProcess);
	resume(senderProcess);

	restore(mask);		/* Restore interrupts */
	return OK;
}


/*------------------------------------------------------------------------
 * decideFunction - takes user input to see which option of the program 
 * is to be accessed
 *------------------------------------------------------------------------
 */
void decideFunction(
)
{
    kprintf("Press 1 to echo a message from remote server (Part 1)\n");
    kprintf("Press 2 to start the chat system (Part 2)\n");
    kprintf("Choice: ");
    
    char choice[10] = {};
    char* comp1 = "1";
    char* comp2 = "2";

	/* get choice from user */
	fgets(choice, 10, stdin);
	/* discard the newline character */
	choice[strlen(choice)-1] = '\0';
	
	/* call appropriate function according to user choice or print invalid
		choice and finish the program */
    if ( strncmp(choice, comp1, 1) == 0 ) {
        echoMessage();
    } else if ( strncmp(choice, comp2, 1) == 0 ) {
		startChat();    
    } else {
        kprintf("Invalid choice\n");
    }
    
}


/*------------------------------------------------------------------------
 * main - main process that calls the relevent function for program implementation
 *------------------------------------------------------------------------
 */
process	main(void)
{
	decideFunction();
	return OK;
    
}
