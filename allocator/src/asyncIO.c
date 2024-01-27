#include <asm-generic/errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <aio.h>
#include <unistd.h>
#include "../include/asyncIO.h"

struct ioRequest request[MAX_REQS];

// Initialize the array of I/O requests for the asynchronous I/O
void req_init() {
	int i;
	
	for (i = 0; i < MAX_REQS; i++) {
		request[i].state = 0;
#if MALLOC_ON
		request[i].buffer = NULL;
#else
		request[i].buffer = malloc(BUFFER_SIZE * sizeof(char));
		request[i].size = BUFFER_SIZE;
#endif
	}
}

// Check to find available slot in the i/o request array for the new i/o
// request.
// Return the 'index' of the available slot in the array, or return '-1' if all
// the slots are active and allocated.
static int find_slot() {
	static int i = 0;

	for (; i < MAX_REQS; i++) {
		if (request[i].state == 0)
			return i;

		// If the request is in active state, then check if it has finished.
		// Update the state based on the return valuew of aio_error().
		if (request[i].state == EINPROGRESS) {
			request[i].state = aio_error(&request[i].aiocbp);

			switch (request[i].state) {
				case 0:
#if MALLOC_ON
					if (request[i].buffer != NULL) {
						free(request[i].buffer);
						request[i].buffer = NULL;
					}
#endif
					return i;
					break;

				case EINPROGRESS:
					break;

				case ECANCELED:
					assertf(0, "Request cacelled");
					break;

				default:
					assertf(0, "AIO_ERROR %d", request[i].state);
					break;
			}
		}
	}

  i = 0;
	return -1;
}

// Add new I/O request in the array
// Arguments:
//	fd	   - File descriptor
//	data   - Data to be transfered to the device
//	size   - Size of the data
//	offset - Write the data to the specific offset in the file
//	
void req_add(int fd, char *data, size_t size, uint64_t offset) {
	int slot;					// Find available slot for the request

	slot = find_slot();

	// Wait here until find an available slot for the request
	while (slot == -1) {
		slot = find_slot();
	}

	// Create and initialize the aiocb structure.
	// If we don't init to 0, we have undefined behavior.
	// E.g. through sigevent op.aio_sigevent there could be a callback function
	// being set, that the program tries to call - which will then fail.
    struct aiocb obj = {0};

	obj.aio_fildes = fd;
	obj.aio_offset = offset;
#if MALLOC_ON
	request[slot].buffer = malloc(size * sizeof(char));
#else
	if (size > request[slot].size) {
		char *ptr_new = realloc(request[slot].buffer, size);
		request[slot].buffer = ptr_new;
		request[slot].size = size;
	}
#endif
	memcpy(request[slot].buffer, data, size);
	obj.aio_buf = request[slot].buffer;
	obj.aio_nbytes = size;            

	request[slot].state = EINPROGRESS;
	request[slot].aiocbp = obj;

#ifdef ASSERT
	int check = aio_write(&request[slot].aiocbp);
	assertf(check == 0, "Write failed");
#else
	aio_write(&request[slot].aiocbp);
#endif
}

// Traverse tthe array to check if all the i/o requests have been completed.  We
// check the state of the i/o request and update the state of each request.
// Return 1 if all the requests are completed succesfully
// Return 0, otherwise
int is_all_req_completed() {
	int i;

	for (i = 0; i < MAX_REQS; i++) {
		if (request[i].state == EINPROGRESS) {
			request[i].state = aio_error(&request[i].aiocbp);

			switch (request[i].state) {
				case 0:
#if MALLOC_ON
					if (request[i].buffer != NULL) {
						free(request[i].buffer);
						request[i].buffer = NULL;
					}
#endif
					break;

				case EINPROGRESS:
					return 0;
					break;

				case ECANCELED:
					assertf(0, "Request cacelled");
					break;

				default:
					assertf(0, "AIO_ERROR");
					break;
			}
		}
	}

	return 1;
}
