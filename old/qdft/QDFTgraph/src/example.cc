// Copyright (C) 2011 Johan Oudinet <oudinet@lri.fr>
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
#ifndef QDFT_EXAMPLE_CC
# define QDFT_EXAMPLE_CC
# include <vector>
# include <qdft/qdft.hh>

int main() {
  // A vector with 3 different data
  typedef std::vector<qdft::data_manager<> >	data_managers_t;
  data_managers_t dmanagers (4);
	qdft::quantity_type qold;
  // Set initial amount of data per data
  dmanagers[0].init (5, "c0");
  dmanagers[1].init (10, "d1");
  dmanagers[2].init (8, "e1");
  dmanagers[3].init (12, "f1");

  // test transfer
  dmanagers[0].transfer (2, "c0", "c1");
  dmanagers[0].transfer (1, "c0", "c1");
  dmanagers[1].transfer (5, "d1", "d2");
  dmanagers[1].transfer (5, "d2", "d1");
  dmanagers[2].transfer (6, "e1", "e2");
  dmanagers[3].transfer (3, "f1", "f2");
  qold = dmanagers[3].transfer (2, "f1", "f2");
  dmanagers[3].transfer (5, "f1", "f3");
  dmanagers[3].transfer (5, "f2", "f4");
  dmanagers[3].transfer (5, "f3", "f4");

  size_t i = 0;
  for (data_managers_t::const_iterator it = dmanagers.begin ();
       it != dmanagers.end (); ++it)
    {
      std::cerr << "// Data Flow Graph for data " << i++ << std::endl;
      it->show_graph (std::cout);
    }

  // test decrease
	std::cerr << "#### test remove of c0 ####" << std::endl;
  dmanagers[0].remove (5, "c0");
  dmanagers[1].remove (5, "d2");
  dmanagers[2].remove (4, "e2");
  dmanagers[3].remove (5, "f4");
  dmanagers[3].remove (5, "f4");

  i = 0;
  for (data_managers_t::const_iterator it = dmanagers.begin ();
       it != dmanagers.end (); ++it)
    {
      std::cerr << "// Data Flow Graph for data " << i++ << std::endl;
      it->show_graph (std::cout);
    }

  // test adding again
	std::cerr << "#### test adding again ####" << std::endl;
  dmanagers[0].transfer (5, "c1", "c0");
  dmanagers[1].transfer (5, "d1", "d2");

  dmanagers[2].transfer (2, "e2", "e3");
  dmanagers[2].remove (1, "e2");
  dmanagers[2].transfer (1, "e3", "e4");
  dmanagers[2].remove (1, "e3");
  dmanagers[2].transfer (1, "e4", "e5");
  dmanagers[2].transfer (1, "e4", "e6");
  dmanagers[2].transfer (1, "e4", "e7");
  dmanagers[2].remove (1, "e4");
  dmanagers[2].remove (1, "e5");
  dmanagers[2].remove (1, "e6");
  dmanagers[2].remove (1, "e7");

  dmanagers[3].set_edge_quantity (qold, "f1", "f2");
	
	std::cerr << "test get_quantity: 1 = "
						<< dmanagers[2].get_quantity ("e2") << std::endl;
  i = 0;
  for (data_managers_t::const_iterator it = dmanagers.begin ();
       it != dmanagers.end (); ++it)
    {
      std::cerr << "// Data Flow Graph for data " << i++ << std::endl;
      it->show_graph (std::cout);
    }
}

#endif // ! QDFT_EXAMPLE_CC
