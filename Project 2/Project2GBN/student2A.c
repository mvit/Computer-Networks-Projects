#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project2.h"
 
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for Project 2, unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/

#define WINDOW_SIZE 8
#define BUFFER_SIZE 50

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

//Define node for our buffer
typedef struct _node {
	struct msg message;
	struct _node *next;
} node;

typedef node *nodeptr;

extern int TraceLevel;
extern double AveTimeBetweenMsgs;
static int nextseqnum = 0;
static int base = 0;

//our window frame
struct pkt buffer[BUFFER_SIZE];
//our head node
nodeptr head = NULL;

//Buffer code adapted from "C How to Program 7th Edition"
//Adds message to buffer
void add_to_buffer(nodeptr *n, struct msg content) {
	nodeptr newNode;
	nodeptr current;
	nodeptr previous;
	
	newNode = malloc(sizeof(node));
	
	newNode->message = content;
	newNode->next = NULL;
	
	previous = NULL;
	current = *n;
	
	while(current != NULL) {
		previous = current;
		current = current->next;
	}
	
	if (previous == NULL) {
		if (TraceLevel == 2) printf("Adding to head\n");
		newNode->next = *n;
		*n = newNode;
	}
	else {
		if (TraceLevel == 2) printf("Adding to buffer\n");
		previous->next=newNode;
	}
}
//grabs the top message in the buffer and shifts buffer accordingly
struct msg pop_buffer(nodeptr *n) {
	
	if (TraceLevel == 2) printf("Popping from buffer\n");
	struct msg messageout;
	nodeptr nextmsg;
	
	nextmsg = *n;
	messageout = (*n)->message;
	*n = (*n)->next;
	free(nextmsg);
	
	return messageout;
}
/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
	
	struct pkt package;
	int check;

	
	if (nextseqnum < base + WINDOW_SIZE && nextseqnum < BUFFER_SIZE) {
		if (TraceLevel == 2) printf("Sending a package\n");
	
		//make packet
		package.seqnum = nextseqnum;
		package.acknum = 0;
		memset(package.payload, '\0', sizeof(20));
		strncpy(package.payload, message.data, 20);

		check = get_checksum(package);
		package.checksum = check;

		//store in buffer
		
		buffer[nextseqnum] = package;
		
		//send packet
		if (TraceLevel == 2) printf("----------------------------A %s, %i, %i\n", package.payload, package.seqnum, package.checksum);
		tolayer3(AEntity, package);
		
		//start timer
		if (base == nextseqnum) {
			startTimer(AEntity, AveTimeBetweenMsgs * 5);
		}
		
		nextseqnum++;

	}
	else {
		add_to_buffer(&head, message);
	}
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
	//printf("ACK%i RECEIVED WITH seqnum %i, checksum %i\n", packet.acknum, packet.seqnum, packet.checksum);
	//printf("Base B4 num is %i\n", base);
	int check = get_checksum(packet);
	
	if (packet.checksum == check) {
		base = packet.seqnum + 1;
		if (TraceLevel == 2) printf("Base AFTER num is %i\n", base);
		
		if (base == nextseqnum) stopTimer(AEntity);
		else startTimer(AEntity, AveTimeBetweenMsgs * 5);
		
		if (head != NULL) {
			if (TraceLevel == 2) printf("Packages still in queue");
			A_output(pop_buffer(&head));
		}
	}
	else {
		if (TraceLevel == 2) printf("Wrong Package\n");
	}
}


/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
	int i;
	if (TraceLevel == 2) printf("Timer Finished \n");
	if (TraceLevel == 2) printf("Base is %i and NextSeq is %i\n", base, nextseqnum);
	startTimer(AEntity, AveTimeBetweenMsgs * 5);
	for(i = base; i <= nextseqnum; i++) {
		tolayer3(AEntity, buffer[i]);
	}
	
}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	if (TraceLevel == 2) printf("----------------------Average time between messages = %f\n", AveTimeBetweenMsgs);
}
