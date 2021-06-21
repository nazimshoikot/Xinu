/* pc.c -  produce, consume, produce_consume_init, create_producer, create_consumer, produce_consume_destory*/

#include <xinu.h>

//standard variables used by testcases

char buffer[PCBUFFER];
uint8 producer_index = 0;
uint8 consumer_index = 0;
sid32 producer_sem = NULL;
sid32 consumer_sem = NULL;
uint32 produced = 0;
uint32 consumed = 0;
uint32 leftovers = 0; // leftovers
uint8 nproducers = 0;
uint8 nconsumers = 0;
pid32 producers[MAXPRODUCERS];
pid32 consumers[MAXCONSUMERS];

/*------------------------------------------------------------------------
 * produce - produces quantity of characters for the consumer to print
 *------------------------------------------------------------------------
 */
void produce(
		char c,			// character to be added to buffer
		uint8 quantity	//  how many times to be added
	)
{
	//TODO: safely produce a character and place it in the buffer, track number of characters produced and index into the buffer.
		int i; 
		for (i = 0; i < quantity; i++) {
			leftovers = produced - consumed;

			if ((leftovers < PCBUFFER)) {
							
				wait(producer_sem); // wait for the producer_sem

				buffer[producer_index] = c;
				
				produced++;
				

				producer_index++;	
				
				// wrapping
				if (producer_index >= PCBUFFER) {
					producer_index = 0;
				}
				
				signal(consumer_sem); // signal the consumer sem
			}
		}
}

/*------------------------------------------------------------------------
 * consume - consumes characters from the producer
 *------------------------------------------------------------------------
 */
void consume(
		uint8 quantity
	)
{
		int i;
		for (i = 0; i < quantity; i ++) {
			// uprintf("INSIDE CONSUME LOOP\n");
			leftovers = produced - consumed;
			if (leftovers > 0) {
				wait(consumer_sem);
				//TODO: safely consume a character and remove it from the buffer, track number of characters consumed and index into the buffer
				char c = buffer[consumer_index];
				kprintf("%c", c);	//feel free to rename 'c', but print in this format
				buffer[consumer_index] = 'd';
				consumed++;

				consumer_index++;
				// wrapping
				if (consumer_index >= PCBUFFER) {
					consumer_index = 0;
				}

				signal(producer_sem);
			}
		}
				
}

/*------------------------------------------------------------------------
 * produce_consume_init - initializes producer and consumer variables
 *------------------------------------------------------------------------
 */
void produce_consume_init(
		void
	)
{
	producer_sem = semcreate(PCBUFFER);
	consumer_sem = semcreate(0);
}

/*------------------------------------------------------------------------
 * create_producer - creates a producer thread
 *------------------------------------------------------------------------
 */
void create_producer(
		char c,
		uint8 quantity
	)
{
	if (nproducers == MAXPRODUCERS) {
		return;
	}
	producers[nproducers] = create(produce, 512, 10, "Producer", 2, c, quantity);
	resume(producers[nproducers]);
	nproducers++;
}

/*------------------------------------------------------------------------
 * create_consumer - creates a consumer thread
 *------------------------------------------------------------------------
 */
void create_consumer(
		uint8 quantity
	)
{
	if (nconsumers == MAXCONSUMERS) {
		return;
	}
	consumers[nconsumers] = create(consume, 512, 10, "Consumer", 1, quantity);
	resume(consumers[nconsumers]);
	nconsumers++;
}

/*------------------------------------------------------------------------
 * produc_consume_destroy - gracefully destroys producers and consumers
 *------------------------------------------------------------------------
 */
void produce_consume_destroy(
		void
	)
{
	wait(producer_sem);
	uint8 i;
	for (i = 0; i < nproducers; i++) {
		kill(producers[i]);
	}
	for (i = 0; i < nconsumers; i++) {
		kill(consumers[i]);
	}
	semdelete(producer_sem);
	semdelete(consumer_sem);
	kprintf("Produced %d characters, consumed %d characters, %d leftover.\n",
		produced, consumed, produced - consumed);
	kprintf("Leftovers:\n");
	for (i = 0; i < PCBUFFER; i++) {
		kprintf("%c", buffer[i]);
	}
}
