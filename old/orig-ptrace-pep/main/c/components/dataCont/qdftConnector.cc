/*
 * qdftConnector.cc
 *
 *  Created on: Aug 17, 2011
 *      Author: cornelius moucha
 */

#include "qdftConnector.hh"
#include "qdft/qdft.hh"
#include "log_qdftConnector_cc.h"

/*
struct qdftmanager_s
{
  qdft::data_manager<> *dm;
};
*/
struct qdftmanager_s
{
  qdft::data_managers<> dm;
};

// Initialize the global manager
//static qdftmanager_ptr qdftmanager = (qdftmanager_ptr)malloc(sizeof(qdftmanager_t));
static qdftmanager_t qdftmanager;

// Get a new data name
char* qdftGet_new_data(size_t q, int mode)
{
	return strdup(qdftmanager.dm.new_data(q,(mode==INIT_DATA_MODE_KNOWN?qdft::known:qdft::unknown)).c_str());
}


// Initialize the graph
/*
qdftmanager_ptr qdftInit(size_t qi, const char* c)
{
  qdftmanager_ptr qdftmanager=(qdftmanager_ptr)malloc(sizeof(qdftmanager_t));
  qdft::data_manager<> *dm = new qdft::data_manager<>;
  qdftmanager.dm.dm=dm;
  qdftmanager.dm.dm->init(qi,c);
  return qdftmanager;
}
*/
void qdftInit(const char* d, size_t qi, const char* c)
{
  try{
	  qdftmanager.dm.init(d, qi,c);
  } catch (std::invalid_argument&){
	  qdftmanager.dm.remove(d, INT_MAX, c); //if the container already exists, we delete it and create a new one with the same name
										//except for FFOX, this(=invoking init on the same container twice) should never happen
	  qdftmanager.dm.init(d, qi,c);
  }
}

// Transfer q amount of data from c_src to c_dst
// qa is transfer size but may be greater than q
//size_t qdftTransfer(qdftmanager_ptr qdftmanager, size_t qa, const char* c_src, const char* c_dst)
//{
//  return qdftmanager.dm.dm->transfer(qa, c_src, c_dst);
//}
size_t qdftTransfer(const char* d, size_t qa, const char* c_src, const char* c_dst)
{
  return qdftmanager.dm.transfer(d, qa, c_src, c_dst);
}

// Remove q amount of data from c
/*
void qdftRemove(qdftmanager_ptr qdftmanager, size_t q, const char* c)
{
  qdftmanager.dm.dm->remove(q, c);
}
*/
void qdftRemove(const char* d, size_t q, const char* c)
{
  qdftmanager.dm.remove(d, q, c);
}

// Get amount of data in c
/*
size_t qdftGet_quantity(qdftmanager_ptr qdftmanager, const char* c)
{
  return qdftmanager.dm.dm->get_quantity(c);
}
*/
size_t qdftGet_quantity(const char* d, const char* c)
{
  return qdftmanager.dm.get_quantity(d, c);
}

// Get initial amount of data
size_t qdftGet_data_amount(const char* d)
{
  return qdftmanager.dm.get_data_amount(d);
}

// Set initial amount of data
void qdftSet_data_amount(const char* d, size_t q){
	try {
		qdftmanager.dm.set_data_amount(d,q);
	} catch (std::invalid_argument&) {
		qdftmanager.dm.new_data(q, (INIT_DATA_MODE==INIT_DATA_MODE_KNOWN?qdft::known:qdft::unknown), d);
	}
}


// Print out the graph in GraphViz format
/*
void qdftShow_graph(qdftmanager_ptr qdftmanager)
{
  qdftmanager.dm.dm->show_graph();
}
*/
void qdftShow_graphs()
{
  qdftmanager.dm.show_graphs();
}

// Saving for pretty printing
void qdftSave(const char* fname)
{
  qdftmanager.dm.save(fname);
}


// Revert the last transfer
/*
void qdftRevert(qdftmanager_ptr qdftmanager, size_t oldVal, const char* src, const char* dst)
{
  qdftmanager.dm.dm->set_edge_quantity(oldVal,src,dst);
}
*/
void qdftRevert_last_transfer(const char* d)
{
  qdftmanager.dm.revert_last_transfer(d);
}
