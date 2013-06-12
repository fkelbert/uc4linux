// Copyright (C) 2011, 2012 Johan Oudinet <oudinet@lri.fr>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
#ifndef QDFT_TEST_LIB_CC
# define QDFT_TEST_LIB_CC
# include <vector>
# include <qdft/qdft.hh>

int main() {
  // A vector with 3 different data
  typedef qdft::data_managers<>	data_managers_t;
  data_managers_t dmanagers;
	qdft::quantity_type qold;

	std::cerr << "### Create new data: " << std::endl;
	qdft::dname_type d1 = dmanagers.new_data (5, qdft::last);
	qdft::dname_type d2 = dmanagers.new_data (10, qdft::max);
	qdft::dname_type d3 = dmanagers.new_data (8, qdft::last);
	qdft::dname_type d4 = dmanagers.new_data (12, qdft::last);
	std::cerr << "d1: " << d1 << std::endl;
	std::cerr << "d2: " << d2 << std::endl;
	std::cerr << "d3: " << d3 << std::endl;
	std::cerr << "d4: " << d4 << std::endl;

  // Set initial amount of data per data
	std::cerr << "#########################################################" << std::endl
						<< "########## Set initial amount of data per data ##########" << std::endl
						<< "#########################################################" << std::endl;
  dmanagers.init (d1, 5, "c0");
  // dmanagers.init (d1, 3, "c1"); // Throw an invalid_argument exception
  dmanagers.init (d2, 10, "d1");
  dmanagers.init (d3, 8, "e1");
  dmanagers.init (d4, 12, "f1");

	dmanagers.show_graphs ();

  // test transfer
	std::cerr << "###################################" << std::endl
						<< "########## Test transfer ##########" << std::endl
						<< "###################################" << std::endl;
  dmanagers.transfer (d1, 2, "c0", "c1");
  dmanagers.transfer (d1, 1, "c0", "c1");
  dmanagers.transfer (d2, 5, "d1", "d2");
  dmanagers.transfer (d2, 5, "d2", "d1");
  dmanagers.transfer (d3, 6, "e1", "e2");
  dmanagers.transfer (d4, 3, "f1", "f2");
  qold = dmanagers.transfer (d4, 2, "f1", "f2");
  dmanagers.transfer (d4, 5, "f1", "f3");
  dmanagers.transfer (d4, 5, "f2", "f4");
  dmanagers.transfer (d4, 5, "f3", "f4");

	dmanagers.show_graphs ();

  // test decrease
	std::cerr << "###################################" << std::endl
						<< "########## Test decrease ##########" << std::endl
						<< "###################################" << std::endl;
	// std::cerr << "#### test remove of c0 ####" << std::endl;
  dmanagers.remove (d1, 5, "c0");
  dmanagers.remove (d2, 5, "d2");
  dmanagers.remove (d3, 4, "e2");
  dmanagers.remove (d4, 5, "f4");
  dmanagers.remove (d4, 5, "f4");

	dmanagers.show_graphs ();

  // test adding again
	std::cerr << "#######################################" << std::endl
						<< "########## Test adding again ##########" << std::endl
						<< "#######################################" << std::endl;
	// std::cerr << "#### test adding again ####" << std::endl;
  dmanagers.transfer (d1, 5, "c1", "c0");
  dmanagers.transfer (d2, 5, "d1", "d2");

  dmanagers.transfer (d3, 2, "e2", "e3");
  dmanagers.remove (d3, 1, "e2");
  dmanagers.transfer (d3, 1, "e3", "e4");
  dmanagers.remove (d3, 1, "e3");
  dmanagers.transfer (d3, 1, "e4", "e5");
  dmanagers.transfer (d3, 1, "e4", "e6");
  dmanagers.transfer (d3, 1, "e4", "e7");
  dmanagers.remove (d3, 1, "e4");
  dmanagers.remove (d3, 1, "e5");
  dmanagers.remove (d3, 1, "e6");
  dmanagers.remove (d3, 1, "e7");

  // dmanagers.set_edge_quantity (d4, qold, "f1", "f2");
  dmanagers.transfer (d4, 5, "f3", "f4");
	dmanagers.revert_last_transfer (d4);
	
	std::cerr << "test get_quantity: 1 = "
						<< dmanagers.get_quantity (d3, "e2") << std::endl;

	dmanagers.show_graphs ();

	// test get total amount of data
	std::cerr << "test get_data_amount: 8 = "
						<< dmanagers.get_data_amount (d3) << std::endl;

	std::cerr << "#######################################" << std::endl
						<< "## Test updating unknown data amount ##" << std::endl
						<< "#######################################" << std::endl;
	qdft::dname_type d5 = dmanagers.new_data (100, qdft::unknown, "unknown_mode");
  dmanagers.init (d5, 30, "u0");
  dmanagers.init (d5, 80, "u1");
	dmanagers.set_data_amount (d5, 10);
	dmanagers.set_data_amount (d5, 10);
  dmanagers.init (d5, 10, "u2");
	dmanagers.set_data_amount (d5, 100);
	assert (210 == dmanagers.get_data_amount (d5));

	// Test updating data for known mode
	std::cerr << "#######################################" << std::endl
						<< "#### Test updating last data amount ###" << std::endl
						<< "#######################################" << std::endl;
	qdft::dname_type d6 = dmanagers.new_data (100, qdft::last, "last_mode");
	// d6 = dmanagers.new_data (100, qdft::last, "data5"); // throw an exception
  dmanagers.init (d6, 30, "g0");
  dmanagers.init (d6, 80, "g1");
	// old data amount: 100
	assert (100 == dmanagers.get_data_amount (d6));
	dmanagers.set_data_amount (d6, 10);
	dmanagers.set_data_amount (d6, 10);
	// new data amount: 10
	assert (10 == dmanagers.get_data_amount (d6));
	dmanagers.transfer (d6, 10, "g0", "g2");
	dmanagers.transfer (d6, 10, "g1", "g2");
  dmanagers.init (d6, 10, "g3");
	dmanagers.transfer (d6, 10, "g1", "g4");
	dmanagers.transfer (d6, 10, "g3", "g4");
	dmanagers.set_data_amount (d6, 100);
  dmanagers.init (d6, 90, "g5");
	dmanagers.transfer (d6, 10, "g1", "g6");
	dmanagers.transfer (d6, 10, "g3", "g6");
	dmanagers.transfer (d6, 90, "g5", "g6");
	// Amount of data in g6: 100
	assert (100 == dmanagers.get_quantity (d6, "g6"));
	dmanagers.show_graphs ();	

	std::cerr << "#######################################" << std::endl
						<< "#### Test updating max data amount ####" << std::endl
						<< "#######################################" << std::endl;
	qdft::dname_type d7 = dmanagers.new_data (100, qdft::max, "max_mode");
  dmanagers.init (d7, 30, "h0");
  dmanagers.init (d7, 80, "h1");
	dmanagers.set_data_amount (d7, 10);
  dmanagers.init (d7, 10, "h2");
  dmanagers.init (d7, 10, "h3");
	dmanagers.set_data_amount (d7, 10);
  dmanagers.init (d7, 10, "h4");
  dmanagers.init (d7, 10, "h5");
	dmanagers.set_data_amount (d7, 10);
  dmanagers.init (d7, 10, "h6");
  dmanagers.remove (d7, 10, "h2");
  dmanagers.remove (d7, 10, "h6");
	dmanagers.set_data_amount (d7, 100);
	assert (190 == dmanagers.get_data_amount (d7));

	dmanagers.show_graphs ();	
	dmanagers.save ("/tmp/graphs.dot");	

	std::cerr << "#######################################" << std::endl
						<< "#### Test several use of a temporary container ####" << std::endl
						<< "#######################################" << std::endl;
	qdft::dname_type d8 = dmanagers.new_data (100, qdft::last);
  dmanagers.init (d8, 100, "employees");
  dmanagers.transfer (d8, 47, "employees", "TMPCONT");
  dmanagers.transfer (d8, 47, "TMPCONT", "t4");
  dmanagers.remove (d8, 2147483647, "TMPCONT");
  dmanagers.transfer (d8, 28, "employees", "TMPCONT");
  dmanagers.transfer (d8, 28, "TMPCONT", "t1");
  dmanagers.remove (d8, 2147483647, "TMPCONT");
  dmanagers.transfer (d8, 37, "t4", "TMPCONT");
  dmanagers.transfer (d8, 37, "TMPCONT", "t1");
	dmanagers.show_graphs ();	
  dmanagers.remove (d8, 2147483647, "TMPCONT");

}

#endif // ! QDFT_TEST_LIB_CC
