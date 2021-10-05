#ifndef __ASYNCIO_H__
#define __ASYNCIO_H__

#include <aio.h>
#include <stdint.h>
#include "../include/sharedDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

	// Application-defined structure for tracking I/O requests
	struct ioRequest {    
		int            state;	// Status of request
		char *		   buffer;  // Internal buffer
		struct aiocb   aiocbp;	// Asynchronous I/O control block
#if !MALLOC_ON
		size_t         size;
#endif
	};

    extern struct ioRequest request[MAX_REQS];

	// Initialize the array of I/O requests for the asynchronous I/O
	void	req_init(void);
	
	// Add new I/O request in the list
	// Arguments:
	//	fd	   - File descriptor
	//	reqNum - Request Number
	//	data   - Data to be transfered to the device
	//	size   - Size of the data
	//	offset - Write the data to the specific offset in the file
	void	req_add(int fd, char *data, size_t size, uint64_t offset);

	// Traverse tthe array to check if all the i/o requests have been completed.  We
	// check the state of the i/o request and update the state of each request.
	// Return 1 if all the requests are completed succesfully
	// Return 0, otherwise
	int		is_all_req_completed(void);
	
#ifdef __cplusplus
}
#endif

#endif // __ASYNCIO_H__ 
