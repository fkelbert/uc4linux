// Copyright (C) 2011, 2012 Johan Oudinet <oudinet@cs.tum.edu>
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
#ifndef QDFT_MANAGER_HXX
# define QDFT_MANAGER_HXX
# include <queue>
# include <boost/property_map/property_map.hpp>
# include <boost/graph/graphviz.hpp>
# include <boost/graph/boykov_kolmogorov_max_flow.hpp>
# include <boost/graph/iteration_macros.hpp>
# include "manager.hh"

namespace qdft {
  namespace detail {

    // Map that assume reverse edge are already present in G
    template <class Graph>
    class reverse_edge_map :
      public boost::put_get_helper<typename boost::graph_traits<Graph>::edge_descriptor&,
				   reverse_edge_map<Graph> >
    {
    public:
      typedef boost::graph_traits<Graph> Traits;
      typedef typename Traits::edge_descriptor edge_descriptor;

      typedef edge_descriptor	key_type;
      typedef edge_descriptor	value_type;
      typedef edge_descriptor&	reference;
      typedef boost::lvalue_property_map_tag	category;

      reverse_edge_map (const Graph& g) : g_(g) {}

      reference operator[] (const key_type& e) const
      {
				bool ok;
				boost::tie (rev_, ok) = edge (target(e, g_), source(e, g_), g_);
				assert (ok); // reverse edge must exist
				return rev_;
      }

    private:
      const Graph& g_;
      mutable value_type rev_;

    };

    // Adaptor to create easily a Lvalue map
    template <class Graph>
    reverse_edge_map<Graph>
    make_reverse_edge_map (const Graph& g)
    {
      return reverse_edge_map<Graph> (g);
    }

		/**
		 * Compute the out_degree without counting edges with 0 capacity
		 *
		 * @param u vertex descriptor
		 * @param g graph
		 *
		 * @return the number of edges starting from u
		 */
		template <class BidirectionalGraph>
		inline typename boost::graph_traits<BidirectionalGraph>::degree_size_type
		real_out_degree(const typename boost::graph_traits<BidirectionalGraph>::vertex_descriptor u,
										const BidirectionalGraph& g)
		{
			typedef boost::graph_traits<BidirectionalGraph> Traits;
			typename Traits::degree_size_type res = 0;
			typename Traits::out_edge_iterator ei, ei_end;
			for (boost::tie (ei, ei_end) = out_edges (u, g);
					 ei != ei_end; ++ei)
				if (g[*ei].transfered > 0)
					++res;
			return res;
		}

		/**
		 * Compute the in_degree without counting edges with 0 capacity
		 *
		 * @param u vertex descriptor
		 * @param g graph
		 *
		 * @return the number of edges ending in u
		 */
		template <class BidirectionalGraph>
		inline typename boost::graph_traits<BidirectionalGraph>::degree_size_type
		real_in_degree(const typename boost::graph_traits<BidirectionalGraph>::vertex_descriptor u,
										const BidirectionalGraph& g)
		{
			typedef boost::graph_traits<BidirectionalGraph> Traits;
			typename Traits::degree_size_type res = 0;
			typename Traits::in_edge_iterator ei, ei_end;
			for (boost::tie (ei, ei_end) = in_edges (u, g);
					 ei != ei_end; ++ei)
				if (g[*ei].transfered > 0)
					++res;
			return res;
		}

  } // end namespace detail

	// Create a new data manager with q amount of data
  template <class Graph>
	data_manager<Graph>::data_manager (const quantity_type& q,
																		 const mode_type& m,
																		 const dname_type& d)
	{
		m_ = m;
		has_just_transfered_ = false;
		src_ = add_vertex (detail::node (next_id_ (), q, d), g_);
		current_src_ = add_vertex (detail::node (next_id_ (), q,
																						 "src_", false), g_);
		add_edge (src_, current_src_, detail::transition (q), g_);
		add_edge (current_src_, src_, g_); // reverse edge for max flow
	}

	// Set the amount of data
	template <class Graph>
	void
	data_manager<Graph>::set_data_amount (const quantity_type& q)
	{
		vertex_descriptor s;
		g_[current_src_].fake = true;
		s = add_vertex (detail::node (next_id_ (), q, "src_", false), g_);
		switch (m_)
			{
			case unknown:
						add_edge (src_, s,
											detail::transition (q),
											g_);
						add_edge (s, src_, g_); // reverse edge for max flow
						g_[src_].amount += q;
				break;
			case max:
				add_edge (current_src_, s,
									detail::transition (std::min(g_[current_src_].amount,q)),
									g_);
				add_edge (s, current_src_, g_); // reverse edge for max flow
				if (q > g_[current_src_].amount)
					{
						add_edge (src_, s,
											detail::transition (q - g_[current_src_].amount),
											g_);
						add_edge (s, src_, g_); // reverse edge for max flow
						g_[src_].amount += q - g_[current_src_].amount;
					}
				break;
			case last:
				add_edge (s, current_src_,
									detail::transition (std::min(g_[current_src_].amount,q)),
									g_);
				add_edge (current_src_, s, g_); // reverse edge for max flow
				remove_edge (src_, current_src_, g_);
				remove_edge (current_src_, src_, g_);
				add_edge (src_, s, detail::transition (q), g_);
				add_edge (s, src_, g_); // reverse edge for max flow
				if (q < g_[current_src_].amount) // cut amount of data greater than q
					{
						BGL_FORALL_VERTICES_T(v, g_, Graph)
							if (!g_[v].fake && g_[v].amount > q)
								g_[v].amount = q;
					}
				g_[src_].amount = q;
				break;
			default:
				throw std::domain_error ("Do not know how to update data in this mode");
			}
		bool ok;
		edge_descriptor e;
		boost::tie(e, ok) = edge (src_, current_src_, g_);
		if (ok)
			g_[src_].amount -= g_[e].transfered;
		remove_vertex_rec_(current_src_);
		current_src_ = s;
	}

  // Initialize the container c with qi amount of data
  template <class Graph>
  void
  data_manager<Graph>::init (const quantity_type& qi, const cname_type& c)
  {
    vertex_descriptor s;
		if (qi > g_[current_src_].amount)
			throw std::domain_error ("init: qi must be lower than the current amount of data");
    typename c2v_type::const_iterator it = c2v_.find (c);
    if (it != c2v_.end ()) // c must be new
			throw std::invalid_argument ("init: c must be new");
    s = add_vertex (detail::node (next_id_ (), qi, c, false), g_);
    c2v_[c] = s;		// reverse mapping from c to vertex
    add_edge (current_src_, s, detail::transition (qi), g_);
    // Add also the reverse edge for max_flow algorithms
    add_edge (s, current_src_, g_);
  }

  // Transfer q amount of data from c_src to c_dst
  template <class Graph>
  quantity_type
  data_manager<Graph>::transfer (const quantity_type& qa,
				 const cname_type& c_src,
				 const cname_type& c_dst)
  {
    // c_src must exist (otherwise no transfer) but c_dst can be new
    vertex_descriptor u, v;
    bool ok;
    edge_descriptor e;
    typename c2v_type::const_iterator it = c2v_.find (c_src);
    typename c2v_type::const_iterator jt = c2v_.find (c_dst);
		// if c_src doesn't exist or is equal to c_dst, then no transfer
    if ((it == c2v_.end ()) || (it == jt))
			return 0;
    u = it->second;
    if (jt != c2v_.end ())
      {
        v = jt->second;
        boost::tie (e, ok) = edge (u, v, g_);
				if (qa == 0) // for a transfer of 0, simply returns the edge capacity
					return ok? g_[e].transfered : 0;
      }
    else // create new node for c_dst
      {
				if (qa == 0) // no need to create a new node to transfer 0
					return 0;
				v = add_vertex (detail::node (next_id_ (), 0, c_dst, false), g_);
				c2v_[c_dst] = v;		// reverse mapping from c to vertex
        ok = false; // No edge going to v as it is a new vertex
      }
    quantity_type qcs = g_[u].amount;
    quantity_type sized = std::min (qcs, qa);
    quantity_type old_capacity = 0;
    bool update_needed = true;
    if (ok)
      {
				// Update existing edge capacity
				old_capacity = g_[e].transfered;
				quantity_type qtransfer = std::min (old_capacity + sized,
																						qcs);
				if (qtransfer != old_capacity)
					{
						olde_ = e;
						oldq_ = g_[e].transfered;
						g_[e].transfered = qtransfer;
					}
				else // Graph is still the same
					update_needed = false;
      }
    else
      {
				// Add a new edge from c_src to c_dst
				oldq_ = 0;
				boost::tie(olde_, ok) = add_edge (u, v, detail::transition (sized), g_);

				// And also its reverse edge for max_flow algorithms
				add_edge (v, u, g_);
      }
    // Recompute max_flow from source to v if the graph changed
    if (update_needed)
			{
				has_just_transfered_ = true;

				update_data_of_ (v);
			}
		return old_capacity;
  }

  // Remove q amount of data from c
  template <class Graph>
  void
  data_manager<Graph>::remove (const quantity_type& q, const cname_type& c)
  {
    // TODO: Be careful that if you remove a node or an edge here, it
    // may invalidate some vertex descriptors present in c2v_ (i.e.,
    // choose the correct container between vecS, listS and setS).

    vertex_descriptor u;
    typename c2v_type::iterator it = c2v_.find (c);
    if (it == c2v_.end ()) {	// c does not exist, do nothing
			return;
    }
    u = it->second;
    g_[u].fake = true;
    if (g_[u].amount > q)
      {
				if (detail::real_in_degree(u, g_) == 1 && detail::real_out_degree(u, g_) == 0)
					{ // No need to create a new vertex. Simply use u
						edge_descriptor e;
						in_edge_iterator ei, ei_end;
						boost::tie (ei, ei_end) = in_edges (u, g_);
						while (g_[*ei].transfered == 0)
							++ei;
						assert (g_[*ei].transfered == g_[u].amount);
						g_[*ei].transfered -= q;
						g_[u].fake = false;
						g_[u].amount -= q;
					}
				else
					{
						vertex_descriptor u_new;
						u_new = add_vertex (detail::node (next_id_ (),
																							g_[u].amount - q,
																							g_[u].name, false),
																g_);
						add_edge (u, u_new, detail::transition (g_[u].amount - q), g_);
						add_edge (u_new, u, g_); // fake edge for max_flow algorithm
						it->second = u_new; // update reference to c
					}
      }
    else // No more data inside c
      {
				c2v_.erase (c);
				remove_vertex_rec_ (u);
      }

  }

	// Set the edge capacity to a specific value
  template <class Graph>
	void
	data_manager<Graph>::set_edge_quantity (const quantity_type& q,
																					const cname_type& c_src,
																					const cname_type& c_dst)
	{
    vertex_descriptor u, v;
    bool ok;
    edge_descriptor e;
    typename c2v_type::const_iterator it = c2v_.find (c_src);
    typename c2v_type::const_iterator jt = c2v_.find (c_dst);
		if (it == c2v_.end () || jt == c2v_.end ())
			return;
		u = it->second;
		v = jt->second;
		boost::tie(e, ok) = edge (u, v, g_);
		if (ok && q != g_[e].transfered)
			{
				// set new capacity to q
				g_[e].transfered = q;
				// update max flow
				update_data_of_ (target (e, g_));
			}
	}

	// Revert the last transfered edge value to its previous value.
  template <class Graph>
	void
	data_manager<Graph>::revert_last_transfer ()
	{
		vertex_descriptor u, v;
		edge_descriptor re;
		bool ok;

		assert (has_just_transfered_);
		std::cerr << "olde_: " << olde_ << " "
							<< "oldq_: " << oldq_ << std::endl;
		std::cerr << "newq: " << g_[olde_].transfered << std::endl;
		u = source (olde_, g_);
		v = target (olde_, g_);
		boost::tie (re, ok) = edge(v, u, g_);
		assert (ok);
		if (oldq_ == 0 && g_[re].transfered == 0)
			{ // Remove these two edges
				remove_edge (u, v, g_);
				remove_edge (v, u, g_);
			}
		else
			g_[olde_].transfered = oldq_;
		// update max flow
		update_data_of_ (v);
	}

  // Get amount of data in c
  template <class Graph>
  quantity_type
  data_manager<Graph>::get_quantity (const cname_type& c) const
  {
    typename c2v_type::const_iterator it = c2v_.find (c);
    if (it == c2v_.end ()) // container is not inside the graph
			return 0;
    else
			return g_[it->second].amount;
  }

	// Get the total amount of data
  template <class Graph>
	quantity_type
	data_manager<Graph>::get_data_amount () const
	{
		quantity_type res = 0;
		BGL_FORALL_OUTEDGES_T(src_, e, g_, Graph)
			{
				res += g_[e].transfered;
			}
		g_[src_].amount = res;
		return res;
	}

  // Function to print out the internal graph
  template <class Graph>
  void data_manager<Graph>::show_graph (std::ostream& out) const
  {
    boost::dynamic_properties dp;
    // vertex_index works only with vecS vertex container
    // dp.property ("node_id", boost::get (boost::vertex_index, g_));
    dp.property ("node_id", boost::get (&::qdft::detail::node::id, g_));
    dp.property ("name", boost::get (&::qdft::detail::node::name, g_));
    dp.property ("amount", boost::get (&::qdft::detail::node::amount, g_));
    dp.property ("fake", boost::get (&::qdft::detail::node::fake, g_));
    dp.property ("transfered",
    		 boost::get (&::qdft::detail::transition::transfered, g_));
    write_graphviz_dp(out, g_, dp);
  }

  // Remove vertex u from the internal graph
  template <class Graph>
  void data_manager<Graph>::remove_vertex_ (vertex_descriptor u)
  {
		has_just_transfered_ = false;
		clear_vertex (u, g_);
		remove_vertex (u, g_);
  }

  // Try to remove vertex u and continue recursively
  template <class Graph>
  void
  data_manager<Graph>::remove_vertex_rec_ (vertex_descriptor u)
  {
		std::queue<vertex_descriptor> qv;
		qv.push (u);
		bool deleted = false;	 // True if at least one vertex is deleted
		do {
			// Consider deleting u
			u = qv.front ();
			qv.pop ();
			// \bug Cannot rely on in/out_degree because of fake edges added for maxFlow
			vertex_descriptor u_pred; // u's father
			if (detail::real_out_degree (u, g_) == 0)
				{
					// Consider u's parents for removing
					BGL_FORALL_INEDGES_T(u, e, g_, Graph)
						{
							if (g_[e].transfered > 0 && g_[source (e, g_)].fake)
								qv.push (source (e, g_));
						}
					// Safely remove u from g_
					remove_vertex_ (u);
					deleted = true;
				}
			else if (detail::real_out_degree (u, g_) == 1
							 && detail::real_in_degree (u, g_) == 1)
				{
					// s --e1-> u --e2-> t (and may also have s --e3-> t)
					in_edge_iterator ei, ei_end;
					boost::tie (ei, ei_end) = in_edges (u, g_);
					while (g_[*ei].transfered == 0)
						++ei;
					u_pred = source (*ei, g_);
					edge_descriptor e1, e2, e3;
					vertex_descriptor s;
					// find e2
					out_edge_iterator ei2, ei2_end;
					boost::tie (ei2, ei2_end) = out_edges (u, g_);
					while (g_[*ei2].transfered == 0)
						++ei2;
					e2 = *ei2;
					// find e1
					in_edge_iterator ei1, ei1_end;
					boost::tie (ei1, ei1_end) = in_edges (u, g_);
					while (g_[*ei1].transfered == 0)
						++ei1;
					e1 = *ei1;
					// define s
					s = source (e1, g_);
					bool ok;
					boost::tie (e3, ok) = edge (s, target (e2, g_), g_);
					if (ok) // Update e3
						{
							g_[e3].transfered = std::min (g_[e2].transfered + g_[e3].transfered, g_[s].amount);
						}
					else // Create e3 only if s != t
						{
							vertex_descriptor t = target (e2, g_);
							if (s != t)
								{
									add_edge (s, t,
														detail::transition (std::min(g_[e2].transfered,
																												 g_[s].amount)),
														g_);
									add_edge (t, s, g_); // fake edge for max_flow algo
								}
						}
					// And remove u from g_
					remove_vertex_ (u);
					if (g_[u_pred].fake)
						qv.push (u_pred);
					deleted = true;
				} // end else if
		} while (!qv.empty ());
		if (deleted) // at least one vertex was deleted
			{
				// need to update indexes
				next_id_.reset ();
				BGL_FORALL_VERTICES_T(v, g_, Graph)
					{
						g_[v].id = next_id_ ();
					}
			}
	}

	// Compute max flow from source to u
  template <class Graph>
	void data_manager<Graph>::update_data_of_ (vertex_descriptor u)
	{
		g_[u].amount =
			boykov_kolmogorov_max_flow(g_,
																 // CapacityEdgeMap
																 boost::get (&::qdft::detail::transition::transfered, g_),
																 // ResidualCapacityEdgeMap,
																 boost::get (&::qdft::detail::transition::residual, g_),
																 // ReverseEdgeMap
																 detail::make_reverse_edge_map (g_),
																 // IndexMap
																 boost::get (&::qdft::detail::node::id, g_),
																 boost::vertex(0, g_),
																 u);
		if (g_[u].amount == 0)
			remove_vertex_rec_ (u);
	}
} // end namespace qdft

#endif // ! QDFT_MANAGER_HXX
