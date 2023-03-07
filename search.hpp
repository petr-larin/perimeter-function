//
// search.hpp:
// Numeric functions used in theory of guaranteed search.
//
// Relevant bibliography:
//
// [1] Larin, P. M.  O nevozmozhnosti garantirovannogo poiska
//     v dostatochno bolshoi oblasti. -- Moscow State Univ. -- 
//     Dep. v VINITI 26.05.1998, No 1629-B1998. -- In Russian.
//     (On impossibility of guaranteed search in a sufficiently
//     large domain -- publ. by VINITI, www.viniti.ru)
//
// [2] Larin, P. M.  O nerazreshimosti zadach garantirovannogo
//     poiska v dostatochno bolshoi oblasti // Vestnik Moskovskogo
//     universiteta.  Seriya 15.  Vychislitelnaya matematika i
//     kibernetika. -- 2000 -- No. 1. -- pp. 44-47. -- In Russian.
//     (On insolvability of guaranteed search problems in a
//     sufficiently large domain // Moscow University Bulletin.
//     Ser. 15.  Computational mathematics and cybernetics)
//
// [3] Larin, P. M.  Funktsiya perimetra vypuklykh mnozhestv. --
//     Moscow State Univ. -- Dep. v VINITI 09.04.2004, No 594-B2004.
//     -- In Russian. (Perimeter function of convex domains --
//     publ. by VINITI, www.viniti.ru)
//

#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <cmath>
#include <list>
#include <stdexcept>
#include <vector>

namespace search
{
	//
	// The standard denominations are used:
	// w = velocity ratio,
	// r = radius of detection,
	// z = area, argument of perimeter functions,
	// p = perimeter, argument of inverse perimeter functions,
	// a = radius (of a circle or sphere),
	// a, b = sides of a rectangle.
	//

	//
	// (1) General definitions
	//

	const double pi (3.1415926535897932384626433832795);

	//
	// (2) Functions calculating the gain in the area of
	//     the residual domain in the discrete search model,
	//     "gain functions"
	//

	//
	// Standard 2-dimensional F(w,r)
	//
	// 0 <= w <= 1
	// 0 <= r
	//

	double f (double w, double r);

	//
	// 3-dimensional G(w,r)
	//
	// 0 <= w <= 1
	// 0 <= r
	//

	double g (double w, double r);

	//
	// 2-dimensional on a sphere H_a(w,r)
	//
	// 0 <= w <= 1
	// 0 <= r <= pi*a
	// 0 < a
	//

	double h (double w, double r, double a = 1.0);

	//
	// (3) Perimeter functions and inverse perimeter functions
	//

	//
	// Perimeter function of the plane
	//
	// 0 <= z
	//

	double pf_plane (double z);

	//
	// Inverse perimeter function of the plane
	//
	// 0 <= p
	//

	double ipf_plane (double p);

	//
	// Perimeter function of a plane angle
	//
	// 0 <= z
	// 0 < theta < 2*pi
	//

	double pf_angle (double z, double theta);

	//
	// Inverse perimeter function of a plane angle
	//
	// 0 <= p
	// 0 < theta < 2*pi
	//
	
	double ipf_angle (double p, double theta);

	//
	// Perimeter function of a sphere
	//
	// 0 <= z <= 4*pi*a*a, z < positive infinity
	// 0 < a
	//

	double pf_sphere (double z, double a = 1.0);

	//
	// Inverse perimeter function of a sphere
	//
	// 0 <= p <= 2*pi*a, p < positive infinity
	// 0 < a
	//

	double ipf_sphere (double p, double a = 1.0);

	//
	// Perimeter function of a circle
	//
	// 0 <= z <= pi*a*a, z < positive infinity
	// 0 <= a
	//

	double pf_circle (double z, double a = 1.0);

	//
	// Inverse perimeter function of a circle
	//
	// 0 <= p <= 2*a, p < positive infinity
	// 0 <= a
	//

	double ipf_circle (double p, double a = 1.0);

	//
	// Outer perimeter function of a circle
	//
	// 0 <= z
	// 0 <= a
	// z and a should not be positive infinity simultaneously
	//

	double opf_circle (double z, double a = 1.0);

	//
	// Inverse outer perimeter function of a circle
	//
	// 0 <= p
	// 0 <= a
	// p and a should not be positive infinity simultaneously
	//

	double iopf_circle (double p, double a = 1.0);

	//
	// Perimeter function of a rectangle
	//
	// 0 <= z <= a*b, z < positive infinity
	// 0 <= a < positive infinity
	// 0 <= b < positive infinity
	//

	double pf_rectangle (double z, double a = 1.0, double b = 1.0);

	//
	// Inverse perimeter function of a rectangle
	//
	// 0 <= p <= min(a,b)
	// 0 <= a < positive infinity
	// 0 <= b < positive infinity
	//

	double ipf_rectangle (double p, double a = 1.0, double b = 1.0);

	//
	// Outer perimeter function of a rectangle
	//
	// 0 <= z
	// 0 <= a < positive infinity
	// 0 <= b < positive infinity
	//

	double opf_rectangle (double z, double a = 1.0, double b = 1.0);

	//
	// Inverse outer perimeter function of a rectangle
	//
	// 0 <= p
	// 0 <= a < positive infinity
	// 0 <= b < positive infinity
	//

	double iopf_rectangle (double p, double a = 1.0, double b = 1.0);

	//
	// Perimeter function of the 3D space
	//
	// 0 <= z
	//

	double pf_3d (double z);

	//
	// Inverse perimeter function of the 3D space
	//
	// 0 <= p
	//

	double ipf_3d (double p);

	//
	// Perimeter function of a 3D sphere
	//
	// 0 <= z <= (4/3)*pi*a*a*a, z < positive infinity
	// 0 <= a
	//

	double pf_sphere_3d (double z, double a = 1.0);

	//
	// (4) Representation of a convex polygon whose perimeter
	//     function is to be calculated.  The clients of this
	//     class create the polygon by adding vertices.  The
	//     main purpose of this class is to be passed to the
	//     constructor of the convex_polygon_pf class calculating
	//     the perimeter function.
	//
	
	class convex_polygon {

	public:

		class point;
		class const_iterator;

		//
		// Default constructor, copy constructor, destructor
		// and operator =
		//

		convex_polygon ();
		convex_polygon (const convex_polygon&);
		~convex_polygon ();
		convex_polygon& operator = (const convex_polygon&);

		//
		// Add to the polygon the specified vertex.  This member
		// function simply adds the new vertex to the list and
		// doesn't check that the resulting list makes a convex set.
		// This is to ensure fast processing of polygons with large
		// number of vertices, e.g. we read a number of vertices
		// from a file and then calculate the convex hull only once.
		//

		void add_vertex (const point& new_vertex);

		//
		// Delete all vertices from the polygon
		//

		void reset ();

		//
		// Starting and ending position of the iterator
		// to be used with the nested class const_iterator
		//

		const_iterator begin () const;
		const_iterator end () const;

		//
		// Get the number of vertices in the polygon
		//

		unsigned num_vertices () const;

		//
		// Get the area of the polygon.  It is assumed that
		// convex_hull has been called before calling this function,
		// otherwise the behavior is unpredictable.
		//

		double area () const;

		//
		// Replace the polygon with its convex hull.  This function
		// removes the vertices that lie inside the convex hull of
		// the polygon.  It should be called after adding one or more
		// vertices prior to outputting the polygon to the clients
		// that assume that the polygon is convex, e.g. when
		// redrawing the polygon.  convex_hull() must be called
		// before passing the object to the convex_polygon_pf
		// constructor, otherwise the behavior will be unpredictable.
		// 
		// Sample code:
		//
		//		search::convex_polygon cp;
		//		double x, y;
		//		while (client_object.read_xy (x, y))
		//		{
		//			search::convex_polygon::point pt (x, y);
		//			cp.add_vertex (pt);
		//		}
		//		cp.convex_hull ();
		//		search::convex_polygon_pf pf (cp);
		//		double z (0.125);
		//		std::cout << "z=" << z << "; pf(z)=" << pf(z) << '\n';}
		//

		void convex_hull ();

		//
		// Representation of a 2-dimensional point
		//

		class point {

		public:

			//
			// Constructors, destructor and common operations
			//

			point ();
			point (double, double);
			~point ();
			bool operator == (const point&) const;
			bool operator != (const point&) const;
			point operator - () const;
			point operator + (const point&) const;
			point operator - (const point&) const;
			double operator * (const point&) const;
			point operator * (double) const;
			point operator / (double) const;

			//
			// Scalar product
			//

			double operator ^ (const point&) const;

			//
			// Distance between 0 and *this
			//

			double abs () const;

			//
			// Angle (*this, 0, x-axis)
			//

			double arg () const;

			//
			// Rotation by pi/2 counterclockwise
			//

			point ortho () const;

			//
			// Angle (p, *this, q)
			//

			double angle (const point& p, const point& q) const;

			//
			// Area of a triangle (*this, p, q)
			//

			double area (const point& p, const point& q) const;

			//
			// Area of a triangle (*this, p, q) with a sign
			//

			double sign_area (const point& p, const point& q) const;

			//
			// Distance between *this and the line pq.
			// If abs(p - q) == 0, distance between *this and p.
			//

			double dist (const point& p, const point& q) const;

			//
			// Normalized distance between p and the projection of *this onto pq:
			// distance = 0 if the projection coincides with p,
			// distance = 1 if the projection coincides with q,
			// distance = 0 if abs (p - q) == 0.
			//

			double proj (const point& p, const point& q) const;

			double x{}, y{};
		};

	private:
		class vertex;
	public:

		//
		// Bidirectional readonly iterator in the list of vertices.
		// Clients should use this class to access the info about
		// the vertices of the polygon.
		//

		class const_iterator {
		public:
			const_iterator ();
			const_iterator (const const_iterator&);
			~const_iterator ();
			const_iterator& operator = (const const_iterator&);
			point operator * () const;
			const point* operator -> () const;
			bool operator == (const const_iterator&) const;
			bool operator != (const const_iterator&) const;
			const_iterator& operator ++ ();
			const_iterator& operator -- ();
		private:
			explicit const_iterator (std::list<vertex>::const_iterator);
			std::list<vertex>::const_iterator iter;
			friend convex_polygon;
		};

	private:

		//
		// List of vertices
		//

		std::list<vertex> vertices;

		//
		// vertex = point + state.
		// state is only used in convex_hull() in the algorithm
		// of finding of the convex hull of the convex polygon.
		//

		class vertex {
		public:
			vertex (const point&);
			~vertex ();
			enum states {unused, used, first};
			point coord;
			states state;
		};

		friend const_iterator;
	};

	//
	// (5) Representation of a perimeter function of a convex
	//     polygon.  The only constructor of convex_polygon_pf
	//     accepts a parameter of the class convex_polygon.
	//

	class convex_polygon_pf {

	public:

		//
		// Constructor and destructor.  The cp parameter is only
		// used during the constructor call when the information
		// about the polygon is read and stored.  After the
		// constructor has finished, anything that happens to
		// the cp object (e.g. new vertices or destruction) has
		// no effect on the created object.
		//
		// cp.convex_hull() must be called before passing cp to
		// the constructor.
		//

		explicit convex_polygon_pf (const convex_polygon& cp);
		~convex_polygon_pf ();

		//
		// Info about the polygon: number of vertices, area and
		// 1/2 of the area.
		//

		unsigned num_vertices () const;
		double area () const;
		double half_area () const;

		//
		// Perimeter function itself.
		// These 2 functions are identical.
		//
		// 0 <= z <= area()
		//

		double operator () (double z);
		double pf (double z);

		//
		// Inverse perimeter function
		//
		// 0 <= p <= maximum()
		//

		double ipf (double p);

		//
		// Maximum of the perimeter function
		//

		double maximum ();

		//
		// Number of smooth segments in the perimeter function
		//

		unsigned num_segments ();

		//
		// Parameters of each smooth segment.
		//
		// a(i): 0 <= i <= num_segments(),
		// theta(i): 1 <= i <= num_segments(),
		// zeta(i): 1 <= i <= num_segments().
		//
		// If num_segments() is even, all the segments have the
		// following form:
		//
		// pf(z) = sqrt(2*theta(i)*(z+zeta(i))), where
		// a(i-1) <= z <= a(i).
		//
		// If num_segments() is odd, all the segments except
		// the middle segment (whose index is
		// k = (num_segments() + 1)/2) have the above mentioned
		// form, but the middle segment has the form
		//
		// pf(z) = zeta(k) = const, where
		// a(k-1) <= z <= a(k).
		//

		double a (unsigned i);
		double theta (unsigned i);
		double zeta (unsigned i);

		//
		// The shortest curve dividing the polygon into 2 parts
		// with equal areas.  All arguments are outbound.
		//
		// Return value:
		// the length of the curve.  If the return value is 0,
		// all the outbound arguments are not changed.
		//
		// is_arc:
		// returns true if the curve is a circular arc,
		// returns false if the curve is a linear segment.
		//
		// start:
		// returns the starting point of the curve.
		//
		// end:
		// returns the end point of the curve.
		//
		// center:
		// returns the center of the arc if is_arc = true,
		// is not changed if is_arc = false.
		//
		// If is_arc = true, the arc goes counterclockwise
		// from start to end.
		//

		double shortest (
			bool& is_arc, convex_polygon::point& start,
			convex_polygon::point& end, convex_polygon::point& center);

	private:

		class partial_pf;
		class eff_perimeter;
		class cyclic_uint;
		class side;
		class partial_pf_node;

		//
		// Copying and assignment aren't supported
		//

		convex_polygon_pf (const convex_polygon_pf&);
		//convex_polygon_pf& operator = (const convex_polygon_pf&);

		//
		// Operators [] for the array of the sides
		//

		side& operator [] (cyclic_uint);
		const side& operator [] (cyclic_uint) const;

		//
		// Area of the sub-polygon defined by the points
		// q[index_1], q[index_1 + 1], ... , q[index_2 - 1],
		// where q[index] denotes (*this)[index].q
		//

		double area (cyclic_uint index_1, cyclic_uint index_2) const;

		//
		// Area of the sub-polygon defined by the points
		// point, q[index_1], q[index_1 + 1], ... , q[index_2 - 1],
		// where q[index] denotes (*this)[index].q
		//

		double area (
			cyclic_uint index_1, cyclic_uint index_2,
			const convex_polygon::point& point) const;

		//
		// Construct the perimeter function 
		//

		void find_pf ();

		//
		// Find only the maximum of the perimeter function
		// (should be a bit faster than find_pf?)
		//

		void find_pf_max ();

		//
		// Info about the polygon: number of vertices, area,
		// 1/2 of the area and the array of the sides.
		//

		unsigned num_vertices_v;
		const double area_v, half_area_v;
		std::vector<side> sides;

		//
		// The structure representing the perimeter function:
		// an array of "partial" perimeter functions
		//

		partial_pf* function;

		//
		// Temporary structure used during the calculation
		// of the perimeter function.
		//

		partial_pf_node* tmp_function;

		//
		// Flags indicating whether the perimeter function, its
		// maximum and the shortest curve have been found
		//

		bool pf_ok, pf_max_ok, sc_ok;

		//
		// Attributes of the perimeter function:
		// number of segments and maximum
		//

		unsigned num_segments_v;
		double maximum_v;

		//
		// "Partial" perimeter function, that is, perimeter
		// function of a pair of sides.  [a, b] is its definition
		// domain, theta and zeta are its parameters, pfa and pfb
		// are its values at a and b.
		//

		class partial_pf {

		public:

			//
			// Trivial constructors
			//

			partial_pf ();
			partial_pf (double, double, double);
			partial_pf (double, double, double, double);

			//
			// Non-trivial constructor that constructs the
			// "partial" perimeter function for the pair of the 
			// sides referenced by index_1 and index_2.
			// If ep != 0, it also checks whether the shortest
			// curve dividing the polygon into 2 parts with equal
			// areas could be drawn between these sides, and
			// fills the structure pointed to by ep with the
			// corresponding info.
			//

			partial_pf (
				const convex_polygon_pf& cp, cyclic_uint index_1,
				cyclic_uint index_2, eff_perimeter* ep = 0);

			~partial_pf ();

			//
			// "Partial" perimeter function itself, and
			// inverse "partial" perimeter function
			//

			double pf (double) const;
			double ipf (double) const;

			//
			// Returns true if there exists at least one point z
			// that belongs to the definition domains of *this
			// and ppf for which ppf.pf(z) < this->pf(z),
			// and false otherwise.
			//
			// If the return value = true, the left and right
			// outbound variables define the segment
			// [left, right] where ppf.pf(z) <= this->pf(z).
			// In addition, the f_left outbound variable returns
			// true if left = this->a, and false otherwise,
			// and f_right returns true if right = this->b,
			// and false otherwise.
			//

			bool begin (
				const partial_pf& ppf, double& left, double& right,
				bool& f_left, bool& f_right);

			//
			// Returns true
			// (1) if there exists at least one point z
			// that belongs to the definition domains of *this
			// and ppf for which ppf.pf(z) > this->pf(z),
			// or (2) if ppf.b < this->b,
			// and false otherwise.
			//
			// Regardless of the return value, the right outbound
			// variable is set to
			// sup {z: ppf.pf(z) < this->pf(z)},
			// where sup is taken over the intersection of the
			// definition domains.
			//
			// If the return value is true, the f_left outbound
			// variable returns true if right = this->a, and
			// false otherwise.
			//

			bool end (const partial_pf& ppf, double& right, bool& f_left);

			//
			// Find the root of the equation (*this) = ppf
			//

			double root (const partial_pf& ppf) const;

			enum ppf_form {constant, sqrt, none};
			ppf_form form{};
			double a{}, b{}, theta{}, zeta{}, pfa{}, pfb{};

			friend eff_perimeter;
		};

		//
		// Effective perimeter (a curve dividing the polygon
		// into 2 parts).  May be either a linear segment or a
		// circular arc.
		//

		class eff_perimeter {
		public:
			partial_pf::ppf_form form = partial_pf::none;
			convex_polygon::point start, end, center;
		};

		//
		// The shortest curve dividing the polygon into 2 parts
		// with equal areas.
		//

		eff_perimeter shortest_curve;

		//
		// Index for the array of the polygon's sides.
		// Uses arithmetics modulo number of sides.
		//

		class cyclic_uint {
		public:
			explicit cyclic_uint (const convex_polygon_pf*, unsigned = 0);
			~cyclic_uint ();
			bool operator == (cyclic_uint) const;
			bool operator != (cyclic_uint) const;
			void operator = (unsigned);
			cyclic_uint& operator = (const cyclic_uint& rhs) { uint = rhs.uint; return *this; }; // added
			operator unsigned () const;
			cyclic_uint operator ++ ();
			cyclic_uint operator ++ (int);
		private:
			const unsigned umod;
			unsigned uint;
		};

		//
		// A side of the polygon.  p and q are adjacent vertices.
		//

		class side {
		public:
			side ();
			side (const convex_polygon::point&, const convex_polygon::point&);
			~side ();
			convex_polygon::point p, q;
		};

		//
		// List node:
		// partial_pf_node = partial_pf + next
		//

		class partial_pf_node : public partial_pf {
		public:
			partial_pf_node (double, double, double);
			~partial_pf_node ();
			partial_pf_node* next;
		};
		friend partial_pf;
		friend partial_pf_node;
	};

	//
	// Inline functions
	//

	inline bool
	equal (double x, double y)
	{
		static const double working_precision (1.0e-10);
		return fabs (x - y) < working_precision;
	}

	inline void
	trim (double& x)
	{
		if (equal (x, 0.0))
		{
			x = 0.0;
		}
	}

	inline
	convex_polygon::convex_polygon ()
	{
	}

	inline
	convex_polygon::convex_polygon (const convex_polygon& rhs)
	: vertices (rhs.vertices)
	{
	}

	inline
	convex_polygon::~convex_polygon ()
	{
	}

	inline convex_polygon&
	convex_polygon::operator = (const convex_polygon& rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}

		vertices = rhs.vertices;
		return *this;
	}

	inline void
	convex_polygon::add_vertex (const point& coord)
	{
		vertices.push_back (vertex (coord));
	}

	inline void
	convex_polygon::reset ()
	{
		vertices.clear ();
	}

	inline convex_polygon::const_iterator
	convex_polygon::begin () const
	{
		return const_iterator (vertices.begin ());
	}

	inline convex_polygon::const_iterator
	convex_polygon::end () const
	{
		return const_iterator (vertices.end ());
	}

	inline unsigned
	convex_polygon::num_vertices () const
	{
		return unsigned(vertices.size ());
	}	inline convex_polygon::point::point ()
	{
	}

	inline
	convex_polygon::point::point (double x, double y)
	: x (x), y (y)
	{
	}

	inline
	convex_polygon::point::~point ()
	{
	}

	inline bool
	convex_polygon::point::operator == (const point& p) const
	{
		return (x == p.x && y == p.y);
	}

	inline bool
	convex_polygon::point::operator != (const point& p) const
	{
		return (x != p.x || y == p.y);
	}

	inline convex_polygon::point
	convex_polygon::point::operator - () const
	{
		return point (-x, -y);
	}

	inline convex_polygon::point
	convex_polygon::point::operator + (const point& p) const
	{
		return point (x + p.x, y + p.y);
	}

	inline convex_polygon::point
	convex_polygon::point::operator - (const point& p) const
	{
		return point (x - p.x, y - p.y);
	}

	inline double
	convex_polygon::point::operator * (const point& p) const
	{
		return x*p.x + y*p.y;
	}

	inline convex_polygon::point
	convex_polygon::point::operator * (double d) const
	{
		return point (x*d, y*d);
	}
	
	inline convex_polygon::point
	convex_polygon::point::operator / (double d) const
	{
		return point (x/d, y/d);
	}

	inline double
	convex_polygon::point::operator ^ (const point& p) const
	{
		return x*p.y - y*p.x;
	}

	inline double
	convex_polygon::point::abs () const
	{
		return sqrt (x*x + y*y);
	}

	inline double
	convex_polygon::point::arg () const
	{
		return atan2 (y, x);
	}

	inline convex_polygon::point
	convex_polygon::point::ortho () const
	{
		return point (-y, x);
	}
			
	inline double
	convex_polygon::point::angle (const point& p, const point& q) const
	{
		point v1 (p - *this), v2 (q - *this);
		double angle (atan2 (v1^v2, v1*v2));
		trim (angle);

		if (angle < 0.0)
		{
			angle += 2.0*pi;
		}

		return angle;
	}

	inline double
	convex_polygon::point::area (const point& p, const point& q) const
	{
		return fabs (sign_area (p, q));
	}

	inline double
	convex_polygon::point::sign_area (const point& p, const point& q) const
	{
		return (p - *this)^(q - *this)/2.0;
	}

	inline double
	convex_polygon::point::dist (const point& p, const point& q) const
	{
		const point v (q - p);
		const double vabs (v.abs ());

		if (vabs == 0.0)
		{
			return (*this - p).abs ();
		}
		else
		{
			return fabs ((v.ortho ()/vabs)*(*this - q));
		}
	}
			
	inline double
	convex_polygon::point::proj (const point& p, const point& q) const
	{
		const point v (q - p);
		const double vabs (v.abs ());

		if (vabs == 0.0)
		{
			return (*this - p).abs ();
		}
		else
		{
			return ((v/vabs)*((*this - p)/vabs));
		}
	}

	inline
	convex_polygon::const_iterator::const_iterator ()
	{
	}

	inline
	convex_polygon::const_iterator::const_iterator (
		const convex_polygon::const_iterator& rhs)
	: iter (rhs.iter)
	{
	}

	inline
	convex_polygon::const_iterator::~const_iterator ()
	{
	}

	inline convex_polygon::const_iterator&
	convex_polygon::const_iterator::operator = (const convex_polygon::const_iterator& rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}

		iter = rhs.iter;
		return *this;
	}

	inline convex_polygon::point
	convex_polygon::const_iterator::operator * () const
	{
		return iter->coord;
	}

	inline const convex_polygon::point*
	convex_polygon::const_iterator::operator -> () const
	{
		return &iter->coord;
	}

	inline bool
	convex_polygon::const_iterator::operator == (const const_iterator& rhs) const
	{
		return iter == rhs.iter;
	}
	
	inline bool
	convex_polygon::const_iterator::operator != (const const_iterator& rhs) const
	{
		return iter != rhs.iter;
	}

	inline convex_polygon::const_iterator&
	convex_polygon::const_iterator::operator ++ ()
	{
		++iter;
		return *this;
	}

	inline convex_polygon::const_iterator&
	convex_polygon::const_iterator::operator -- ()
	{
		--iter;
		return *this;
	}

	inline convex_polygon::const_iterator::const_iterator (
		std::list<convex_polygon::vertex>::const_iterator iter)
	: iter (iter)
	{
	}

	inline
	convex_polygon::vertex::vertex (const point& coord)
	: coord (coord), state (unused)
	{
	}

	inline
	convex_polygon::vertex::~vertex ()
	{
	}

	inline
	convex_polygon_pf::~convex_polygon_pf ()
	{
		delete [] function;
	}

	inline unsigned
	convex_polygon_pf::num_vertices () const
	{
		return num_vertices_v;
	}

	inline double
	convex_polygon_pf::area () const
	{
		return area_v;
	}

	inline double
	convex_polygon_pf::half_area () const
	{
		return half_area_v;
	}

	inline double
	convex_polygon_pf::operator () (double z)
	{
		return pf (z);
	}

	inline convex_polygon_pf::side&
	convex_polygon_pf::operator [] (cyclic_uint index)
	{
		return sides [index];
	}

	inline const convex_polygon_pf::side&
	convex_polygon_pf::operator [] (cyclic_uint index) const
	{
		return sides [index];
	}

	inline
	convex_polygon_pf::partial_pf::partial_pf ()
	{
	}

	inline
	convex_polygon_pf::partial_pf::partial_pf (double a, double b, double pfa)
	: form (constant), a (a), b (b), theta (0.0), zeta (pfa),
	  pfa (pfa), pfb (pfa)
	{
	}

	inline
	convex_polygon_pf::partial_pf::partial_pf (
		double a, double b, double theta, double zeta)
	: form (sqrt), a (a), b (b), theta (theta), zeta (zeta),
	  pfa (pf (a)), pfb (pf (b))
	{
	}

	inline
	convex_polygon_pf::partial_pf::~partial_pf ()
	{
	}

	inline double
	convex_polygon_pf::partial_pf::pf (double z) const
	{
		return form == constant ? pfa : ::sqrt (2.0*theta*(z + zeta));
	}

	inline double
	convex_polygon_pf::partial_pf::ipf (double p) const
	{
		return form == constant ? a : (p/2.0)*(p/theta) - zeta;
	}

	inline
	convex_polygon_pf::cyclic_uint::cyclic_uint (
		const convex_polygon_pf* cppf, unsigned uint)
	: umod (cppf->num_vertices ()), uint (uint%umod)
	{
	}

	inline
	convex_polygon_pf::cyclic_uint::~cyclic_uint ()
	{
	}

	inline void
	convex_polygon_pf::cyclic_uint::operator = (unsigned rhs)
	{
		uint = rhs%umod;
	}

	inline
	convex_polygon_pf::cyclic_uint::operator unsigned () const
	{
		return uint;
	}

	inline bool
	convex_polygon_pf::cyclic_uint::operator == (cyclic_uint rhs) const
	{
		return uint == rhs;
	}

	inline bool
	convex_polygon_pf::cyclic_uint::operator != (cyclic_uint rhs) const
	{
		return uint != rhs;
	}

	inline convex_polygon_pf::cyclic_uint
	convex_polygon_pf::cyclic_uint::operator ++ ()
	{
		uint = ++uint%umod;
		return *this;
	}

	inline convex_polygon_pf::cyclic_uint
	convex_polygon_pf::cyclic_uint::operator ++ (int)
	{
		const cyclic_uint cuint (*this);
		++(*this);
		return cuint;
	}

	inline
	convex_polygon_pf::side::side ()
	{
	}

	inline
	convex_polygon_pf::side::side (
		const convex_polygon::point& p, const convex_polygon::point& q)
	: p (p), q (q)
	{
	}

	inline
	convex_polygon_pf::side::~side ()
	{
	}

	inline convex_polygon_pf::partial_pf_node::partial_pf_node (
		double a, double b, double pfa)
	: partial_pf (a, b, pfa), next (0)
	{
	}

	inline
	convex_polygon_pf::partial_pf_node::~partial_pf_node ()
	{
		delete next;
	}

} // namespace search

#endif // SEARCH_HPP