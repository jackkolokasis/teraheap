#include <asm-generic/errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <aio.h>
#include <unistd.h>
#include "../include/asyncIO.h"
#include <threads.h>//perpap

//struct ioRequest request[MAX_REQS];
static ioRequestMap_t *_requests_map;
static uint16_t _requests_queue_size;
static uint64_t _GC_THREADS;
// Initialize the array of I/O requests for the asynchronous I/O
void req_init(uint64_t gc_threads) {
	_GC_THREADS = gc_threads;
	if (_GC_THREADS > 8 ) {
	    _requests_queue_size = 5;
	} else {
	    _requests_queue_size = MAX_REQS / _GC_THREADS;//FIXME
	}
	//_requests_map = malloc(gc_threads * (sizeof(ioRequestMap_t) + queue_size * sizeof(ioRequest_t)));//FIXME
	_requests_map = malloc(gc_threads * sizeof(ioRequestMap_t));

	if(!_requests_map) {
	    perror("Allocation failure! No memory available for requests map.");   
	    exit(EXIT_FAILURE);
	}	
#if 1//perpap
	for (uint64_t tid = 0; tid < gc_threads; ++tid) {
                _requests_map[tid].request = malloc(_requests_queue_size * sizeof(ioRequest_t));
                if(!_requests_map[tid].request) {
	            perror("Allocation failure! No memory available for requests.");   
	            exit(EXIT_FAILURE);
	        }
		ioRequestMap_t *requests = &_requests_map[tid];
		//requests->size = queue_size;//FIXME
		for (uint16_t i = 0; i < /*MAX_REQS*/_requests_queue_size; i++) {
			requests->request[i].state = 0;
#if MALLOC_ON
			requests->request[i].buffer = NULL;
#else
			requests->request[i].buffer = malloc(BUFFER_SIZE * sizeof(char));
			if(!requests->request[i].buffer) {
			    perror("Allocation failure! No memory available for request buffer.");   
	                    exit(EXIT_FAILURE);
			}
			requests->request[i].size = BUFFER_SIZE;
#endif
		}
	}
#endif
#if 0
	for (uint8_t i = 0; i < MAX_REQS; i++) {
		request[i].state = 0;
#if MALLOC_ON
		request[i].buffer = NULL;
#else
		request[i].buffer = malloc(BUFFER_SIZE * sizeof(char));
		request[i].size = BUFFER_SIZE;
#endif
	}
#endif
}

// Check to find available slot in the i/o request array for the new i/o
// request.
// Return the 'index' of the available slot in the array, or return '-1' if all
// the slots are active and allocated.
static int find_slot(uint64_t worker_id) {
        ioRequestMap_t *requests = &_requests_map[worker_id];
        int *slot = &requests->slot;

	for (int i = *slot; i < /*MAX_REQS*/_requests_queue_size; ++i) {
		if (requests->request[i].state == 0){
			*slot = i;
			return i;
		}

		// If the request is in active state, then check if it has finished.
		// Update the state based on the return valuew of aio_error().
		if (requests->request[i].state == EINPROGRESS) {
			requests->request[i].state = aio_error(&requests->request[i].aiocbp);

			switch (requests->request[i].state) {
				case 0:
#if MALLOC_ON
					if (requests->request[i].buffer != NULL) {
						free(requests->request[i].buffer);
						requests->request[i].buffer = NULL;
					}

#endif
					*slot = i;
					return i;
					//break;

				case EINPROGRESS:
					break;

				case ECANCELED:
					assertf(0, "Request cacelled");
					break;

				default:
					assertf(0, "AIO_ERROR %d", requests->request[i].state);
					break;
			}
		}
	}

        *slot = 0;
	return -1;
}
#if 0
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
#endif
// Add new I/O request in the array
// Arguments:
//	fd	   - File descriptor
//	data   - Data to be transfered to the device
//	size   - Size of the data
//	offset - Write the data to the specific offset in the file
//	
void req_add(int fd, char *data, size_t size, uint64_t offset, uint64_t worker_id) {
	int slot;					// Find available slot for the request

	// Wait here until find an available slot for the request
	while ((slot = find_slot(worker_id)) == -1) {
	    ;
	}
        ioRequestMap_t *requests = &_requests_map[worker_id];
	// Create and initialize the aiocb structure.
	// If we don't init to 0, we have undefined behavior.
	// E.g. through sigevent op.aio_sigevent there could be a callback function
	// being set, that the program tries to call - which will then fail.
    struct aiocb obj = {0};

	obj.aio_fildes = fd;
	obj.aio_offset = offset;
#if MALLOC_ON
	requests->request[slot].buffer = malloc(size * sizeof(char));
#else	
	if (size > requests->request[slot].size) {
		char *ptr_new = realloc(requests->request[slot].buffer, size);
		if(!ptr_new) {
		    perror("ReAllocation failure! No memory available for request buffer.");
		    exit(EXIT_FAILURE);
		}
		requests->request[slot].buffer = ptr_new;
		requests->request[slot].size = size;
	}
#endif
	memcpy(requests->request[slot].buffer, data, size);
	obj.aio_buf = requests->request[slot].buffer;
	obj.aio_nbytes = size;            

	requests->request[slot].state = EINPROGRESS;
	requests->request[slot].aiocbp = obj;

#ifdef ASSERT
	int check = aio_write(&requests->request[slot].aiocbp);
	assertf(check == 0, "Write failed");
#else
	aio_write(&requests->request[slot].aiocbp);
#endif
}

// Traverse tthe array to check if all the i/o requests have been completed.  We
// check the state of the i/o request and update the state of each request.
// Return 1 if all the requests are completed succesfully
// Return 0, otherwise
int is_all_req_completed() {
        for (uint64_t tid = 0; tid < _GC_THREADS; ++tid ) {
		ioRequestMap_t *requests = &_requests_map[tid];//perpap
		for (/*int*/uint16_t i = 0; i < /*MAX_REQS*/_requests_queue_size; i++) {
			if (requests->request[i].state == EINPROGRESS) {
				requests->request[i].state = aio_error(&requests->request[i].aiocbp);

				switch (requests->request[i].state) {
					case 0:
#if MALLOC_ON
						if (requests->request[i].buffer != NULL) {
							free(requests->request[i].buffer);
							requests->request[i].buffer = NULL;
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
	}

	return 1;
}

int is_all_req_completed_parallel(uint32_t worker_id) {
	ioRequestMap_t *requests = &_requests_map[worker_id];
	for (/*int*/uint16_t i = 0; i < /*MAX_REQS*/_requests_queue_size; i++) {
		if (requests->request[i].state == EINPROGRESS) {
			requests->request[i].state = aio_error(&requests->request[i].aiocbp);

			switch (requests->request[i].state) {
				case 0:
#if MALLOC_ON
					if (requests->request[i].buffer != NULL) {
						free(requests->request[i].buffer);
						requests->request[i].buffer = NULL;
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
