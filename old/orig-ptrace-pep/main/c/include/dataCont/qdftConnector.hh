/*
 * qdftConnector.hh
 *
 *  Created on: Aug 17, 2011
 *      Author: cornelius moucha
 */

#ifndef qdftCONNECTOR_HH_
#define qdftCONNECTOR_HH_

#define INIT_DATA_MODE_KNOWN (0)
#define INIT_DATA_MODE_UNKNOWN (1)

#define INIT_DATA_MODE INIT_DATA_MODE_KNOWN


// method declarations in a C understandable way
#ifdef  __cplusplus
  extern "C" {
#endif

typedef struct qdftmanager_s qdftmanager_t;
typedef qdftmanager_t       *qdftmanager_ptr;
// KIT: commented for compiling under linux! why this is necessary in openbsd???
//typedef unsigned long size_t;

//static qdftmanager_ptr qdftmanager;

// Get a new data name
char* qdftGet_new_data(size_t q, int mode);

// Initialize the graph
void qdftInit(const char* d, size_t qi, const char* c);

// Transfer q amount of data from c_src to c_dst
// qa is transfer size but may be greater than q
size_t qdftTransfer(const char* d, size_t qa, const char* c_src, const char* c_dst);

// Remove q amount of data from c
void qdftRemove(const char* d, size_t q, const char* c);

// Get amount of data in c
size_t qdftGet_quantity(const char* d, const char* c);

// Get initial amount of data
size_t qdftGet_data_amount(const char* d);

// Set initial amount of data
void qdftSet_data_amount(const char* d, size_t q);


// Print out the graph in GraphViz format
void qdftShow_graphs();

// Save the graphs for pretty printing
void qdftSave(const char* fname);

// Revert the last transfer
//void qdftRevert(const char* d, size_t oldVal, const char* src, const char* dst);
void qdftRevert_last_transfer(const char* d);

#ifdef  __cplusplus
  }
#endif

#endif /* qdftCONNECTOR_HH_ */
