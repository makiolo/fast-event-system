#include <boost/graph/astar_search.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random.hpp>
#include <boost/random.hpp>
#include <boost/graph/graphviz.hpp>
#include <sys/time.h>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <tuple>
#include <math.h>  // for sqrt

// auxiliary types
struct location
{
	float y, x;  // lat, long
};

template <class Name, class LocMap>
class city_writer
{
public:
	city_writer(Name n, LocMap l, float _minx, float _maxx, float _miny, float _maxy, unsigned int _ptx, unsigned int _pty)
		: name(n)
		, loc(l)
		, minx(_minx)
		, maxx(_maxx)
		, miny(_miny)
		, maxy(_maxy)
		, ptx(_ptx)
		, pty(_pty)
	{
	}

	template <class Vertex>
	void operator()(std::ostream& out, const Vertex& v) const
	{
		float px = 1 - (loc[v].x - minx) / (maxx - minx);
		float py = (loc[v].y - miny) / (maxy - miny);
		out << "[label=\"" << name[v] << "\", pos=\"" << static_cast<unsigned int>(ptx * px) << "," << static_cast<unsigned int>(pty * py) << "\", fontsize=\"11\"]";
	}

private:
	Name name;
	LocMap loc;
	float minx, maxx, miny, maxy;
	unsigned int ptx, pty;
};

template <class WeightMap>
class time_writer
{
public:
	time_writer(WeightMap w)
		: wm(w)
	{

	}

	template <class Edge>
	void operator()(std::ostream& out, const Edge& e) const
	{
		out << "[label=\"" << wm[e] << "\", fontsize=\"11\"]";
	}

private:
	WeightMap wm;
};

template <class Graph, class CostType, class LocMap>
class distance_heuristic : public boost::astar_heuristic<Graph, CostType>
{
public:
	using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;

	distance_heuristic(LocMap l, Vertex goal)
		: m_location(l)
		, m_goal(goal)
	{

	}

	CostType operator()(Vertex u)
	{
		CostType dx = m_location[m_goal].x - m_location[u].x;
		CostType dy = m_location[m_goal].y - m_location[u].y;
		return std::sqrt(dx * dx + dy * dy);
	}

private:
	LocMap m_location;
	Vertex m_goal;
};


struct found_goal
{
};

// visitor that terminates when we find the goal
template <class Vertex>
class astar_goal_visitor : public boost::default_astar_visitor
{
public:
	astar_goal_visitor(Vertex goal)
		: m_goal(goal)
	{
	}

	template <class Graph>
	void examine_vertex(Vertex u, Graph& g)
	{
		if (u == m_goal)
			throw found_goal();
	}

private:
	Vertex m_goal;
};


int main(int argc, char** argv)
{
	using mygraph_t = boost::adjacency_list<
							boost::listS,
							boost::vecS,
							boost::undirectedS,
							boost::no_property,								// vertex property
							boost::property<boost::edge_weight_t, float>	// edge property
						>;
	using WeightMap = boost::property_map<mygraph_t, boost::edge_weight_t>::type;
	using vertex =  mygraph_t::vertex_descriptor;
	using edge_descriptor = mygraph_t::edge_descriptor;
	using edge = std::pair<int, int>;

	// states
	enum nodes
	{
		Troy,
		LakePlacid,
		Plattsburgh,
		Massena,
		Watertown,
		Utica,
		Syracuse,
		Rochester,
		Buffalo,
		Ithaca,
		Binghamton,
		Woodstock,
		NewYork,
		N
	};

	const char* name[] = {
		"Troy",
		"Lake Placid",
		"Plattsburgh",
		"Massena",
		"Watertown",
		"Utica",
		"Syracuse",
		"Rochester",
		"Buffalo",
		"Ithaca",
		"Binghamton",
		"Woodstock",
	   	"New York"
	};
	location locations[] = {// lat/long
		{42.73, 73.68},
		{44.28, 73.99},
		{44.70, 73.46},
		{44.93, 74.89},
		{43.97, 75.91},
		{43.10, 75.23},
		{43.04, 76.14},
		{43.17, 77.61},
		{42.89, 78.86},
		{42.44, 76.50},
		{42.10, 75.91},
		{42.04, 74.11},
		{40.67, 73.94}
	};

	// relations
	edge edge_array[] = {
		edge(Troy, Utica),
		edge(Troy, LakePlacid),
		edge(Troy, Plattsburgh),
		edge(LakePlacid, Plattsburgh),
		edge(Plattsburgh, Massena),
		edge(LakePlacid, Massena),
		edge(Massena, Watertown),
		edge(Watertown, Utica),
		edge(Watertown, Syracuse),
		edge(Utica, Syracuse),
		edge(Syracuse, Rochester),
		edge(Rochester, Buffalo),
		edge(Syracuse, Ithaca),
		edge(Ithaca, Binghamton),
		edge(Ithaca, Rochester),
		edge(Binghamton, Troy),
		edge(Binghamton, Woodstock),
		edge(Binghamton, NewYork),
		edge(Syracuse, Binghamton),
		edge(Woodstock, Troy),
		edge(Woodstock, NewYork)
	};
	unsigned int num_edges = sizeof(edge_array) / sizeof(edge);
	float weights[] = { // estimated travel time (mins)
		96,
		134,
		143,
		65,
		115,
		133,
		117,
		116,
		74,
		56,
		84,
		73,
		69,
		70,
		116,
		147,
		173,
		183,
		74,
		71,
		124
	};

	// create graph
	mygraph_t g(N);
	WeightMap weightmap = boost::get(boost::edge_weight, g);
	for (std::size_t j = 0; j < num_edges; ++j)
	{
		edge_descriptor e;
		bool inserted;
		std::tie(e, inserted) = boost::add_edge(edge_array[j].first, edge_array[j].second, g);
		weightmap[e] = weights[j];
	}

	// pick random start/goal
	boost::mt19937 gen(time(0));
	vertex start = boost::random_vertex(g, gen);
	vertex goal = boost::random_vertex(g, gen);

	std::cout << "Start vertex: " << name[start] << std::endl;
	std::cout << "Goal vertex: " << name[goal] << std::endl;

	std::ofstream dotfile;
	dotfile.open("city.dot");
	boost::write_graphviz(
		dotfile,
		g,
		city_writer<const char**, location*>(
			name,
			locations,
			73.46,
			78.86,
			40.67,
			44.93,
			480,
			400
		),
		time_writer<WeightMap>(weightmap)
	);

	std::vector<mygraph_t::vertex_descriptor> p(boost::num_vertices(g));
	std::vector<float> d(boost::num_vertices(g));
	try
	{
		// call astar named parameter interface
		boost::astar_search_tree(
			g,
			start,
			distance_heuristic<mygraph_t, float, location*>(locations, goal),
			boost::predecessor_map(boost::make_iterator_property_map(p.begin(), boost::get(boost::vertex_index, g)))
				.distance_map(boost::make_iterator_property_map(d.begin(), boost::get(boost::vertex_index, g)))
				.visitor(astar_goal_visitor<vertex>(goal))
		);
	}
	catch (found_goal fg)
	{
		// found a path to the goal
		std::list<vertex> shortest_path;
		for (vertex v = goal;; v = p[v])
		{
			shortest_path.push_front(v);
			if (p[v] == v)
			{
				break;
			}
		}

		std::cout << "Shortest path from " << name[start] << " to " << name[goal] << ": ";
		std::cout << name[start];

		auto spi = shortest_path.begin();
		for (++spi; spi != shortest_path.end(); ++spi)
		{
			std::cout << " -> " << name[*spi];
		}
		std::cout << std::endl << "Total travel time: " << d[goal] << std::endl;
		return 0;
	}

	std::cout << "Didn't find a path from " << name[start] << "to" << name[goal] << "!\n";
	return 0;
}

