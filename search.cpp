//
// search.cpp:
// Implementation of the functions and classes defined
// in search.hpp.
//
// V2 - bug fixed in 
// 			partial_pf (
//				const convex_polygon_pf&, cyclic_uint,
//				cyclic_uint, eff_perimeter*);
// 
// Build options
//
// The SEARCH_CPP_THROW_RANGE parameter controls the way
// the functions react to the wrong arguments:
//
// If SEARCH_CPP_THROW_RANGE is defined, the functions will throw
// (1) std::invalid_argument if any of the arguments is NaN;
// (2) std::out_of_range if any of the arguments is a finite
// number of +/- infinity but is out of the specified range.
//
// If SEARCH_CPP_THROW_RANGE is not defined, the functions will
// return NaN in all of the above cases.
//

#define SEARCH_CPP_THROW_RANGE

#include <limits>
#include <string>

#include "search.hpp"

namespace search
{
	//
	// Some internal definitions only used in the
	// implementation of namespace search
	//

	//
	// Name of the namespace used to report errors
	//

	static const std::string name_of_namespace ("search::");

	//
	// Positive infinity
	//

	static const double pos_infinity (std::numeric_limits<double>::infinity());

	//
	// Quiet NaN
	// Note: this initialization is due to the STLport bug.
	// Replace when possible with
	// const double qnan (std::numeric_limits<double>::quiet_nan());
	//

	static const double qnan (pos_infinity/pos_infinity);

	//
	// Auxiliary function used by **pf_circle functions.
	// It calculates
	// aux_1(beta) = beta - tan(beta) + (pi/2 - beta)*tan^2(beta).
	//

	double aux_1 (double beta);

	//
	// Auxiliary function used by **pf_sphere_3d functions.
	// It calculates
	// aux_2(beta) = 1 - cos(beta).
	//

	double aux_2 (double beta);

#ifdef SEARCH_CPP_THROW_RANGE

	//
	// Range check for inbound arguments
	//

	bool out_of_range (bool cond, const std::string& name_of_fun)
	{
		if (!cond)
		{
			std::string what (name_of_namespace);
			what += name_of_fun;
			throw std::out_of_range (what);
		}
		else
		{
			return false;
		}
	}

	//
	// Check for NaN for inbound arguments (3 overloads)
	//

	bool is_nan (double x, const std::string& name_of_fun)
	{
		if (_isnan (x))
		{
			std::string what (name_of_namespace);
			what += name_of_fun;
			throw std::invalid_argument (what);
		}
		else
		{
			return false;
		}
	}

	bool is_nan (double x, double y, const std::string& name_of_fun)
	{
		if (_isnan (x) || _isnan (y))
		{
			std::string what (name_of_namespace);
			what += name_of_fun;
			throw std::invalid_argument (what);
		}
		else
		{
			return false;
		}
	}

	bool is_nan (double x, double y, double z, const std::string& name_of_fun)
	{
		if (_isnan (x) || _isnan (y) || _isnan (z))
		{
			std::string what (name_of_namespace);
			what += name_of_fun;
			throw std::invalid_argument (what);
		}
		else
		{
			return false;
		}
	}

#else // SEARCH_CPP_THROW_RANGE

	//
	// Range check for inbound arguments
	//

	bool out_of_range (bool cond, const std::string& name_of_fun)
	{
		return !cond;
	}

	//
	// Check for NaN for inbound arguments (3 overloads)
	//

	bool is_nan (double x, const std::string& name_of_fun)
	{
		return _isnan (x) != 0;
	}

	bool is_nan (double x, double y, const std::string& name_of_fun)
	{
		return _isnan (x) || _isnan (y);
	}

	bool is_nan (double x, double y, double z, const std::string& name_of_fun)
	{
		return _isnan (x) || _isnan (y) || _isnan (z);
	}

#endif // SEARCH_CPP_THROW_RANGE

} // namespace search

//
// The implementation of gain functions and perimeter functions
// follows, and it is pretty self-explanatory.  Some comments
// are given when necessary. For more info, refer to theory of
// guaranteed search.
//

double search::f (double w, double r)
{
	static const std::string name_of_fun ("f(double,double)");

	if (is_nan (w, r, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= w && w <= 1.0, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= r, name_of_fun))
	{
		return qnan;
	}

	return 2.0*r*(w*(pi - acos (w)) + sqrt (1.0 - w*w));
}

double search::g (double w, double r)
{
	static const std::string name_of_fun ("g(double,double)");

	if (is_nan (w, r, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= w && w <= 1.0, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= r, name_of_fun))
	{
		return qnan;
	}

	const double tmp (1.0 + w);
	return pi*r*r*tmp*tmp;
}

double search::h (double w, double r, double a)
{
	static const std::string name_of_fun ("h(double,double,double)");

	if (is_nan (w, r, a, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= w && w <= 1.0, name_of_fun))
	{
		return qnan;
	}

	// this check may be taken away if needed:
	// the function will work on any value of r,
	// provided that r/a doesn't evaluate to infinity
	if (out_of_range (0.0 <= r && r <= pi*a, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 < a, name_of_fun))
	{
		return qnan;
	}

	r /= a;

	const double r_abs (fabs (r));
	const double r_limit (1.0e-10);

	if (w == 1.0 && r_abs <= r_limit)
	{
		// A very special case: w=1 and |r|<<1.
		// The general algorithm is in trouble when
		// r^4 evaluates to 0, that is, |r^4| ~ 1e-310.
		// We'll use
		//    h(1,r) = 4/sqrt(3) + 2*pi*r + O(r^3)
		// in the vicinity of r = 0 where this approximation
		// is accurate enough.

		const double lim (4.0/sqrt (3.0));

		if (r > 0.0)
		{
			return a*(lim + 2.0*pi*r);
		}
		else
		if (r == 0.0)
		{
			// For h(1,0) we'll return 0, while, strictly
			// speaking, h(1,0) is not defined:
			//    lim h(w,0) = 0          as w -> 1, but
			//    lim h(1,r) = 4/sqrt(3)  as r -> +0.
			// We must return something, and I prefer
			// to return 0 because h(w,r) is odd in respect to r.

			return 0.0;
		}
		else // r < 0
		{
			return a*(-lim + 2.0*pi*r);
		}
	}

	// depending on the relation between w and r,
	// different algorithms will be used

	const double r_limit_1 (1.0e-5);
	const double r_limit_2 (2.0e-5);
	const double r_limit_3 (pi/2.0);
	int wr_case;

	if (r_abs > r_limit_3)
	{
		wr_case = 1;
	}
	else
	if (r_abs > r_limit_2)
	{
		wr_case = 2;
	}
	else
	if (r_abs*r_abs > (1.0 - w)/100.0) // 100 is empirical
	{
		wr_case = 2;
	}
	else
	if (r_abs > r_limit_1)
	{
		wr_case = 3;
	}
	else
	{
		// wr_case = 4; can handle this case immediately:
		return a*f (w, r);
	}

	// intermediate values

	const double w2 (w*w);
	const double r2 (r*r);
	const double c (cos (r));
	const double s (sin (r));
	const double ws (w*s);
	const double ws_r (r==0.0 ? w : ws/r);

	double x;	// x = 1 - (w*sin(r)/r)^2
	double y;	// y = cos(r) - w*sin(r)/r
	double z;	// z = cos(r) - w^2*sin(r)/r

	if (wr_case == 1)
	{
		x = 1.0 - ws_r*ws_r;
		y = c - ws_r;
		z = c - ws_r*w;
	}
	else // wr_case is 2 or 3
	{
		// Expand into power series to ensure
		// uniform accuracy over the range -pi/2 <= r <= pi/2.
		// I used Mathematica 2.2 to get the coefficients.

		double p[12];	// even powers of r
		p[0] = r2;

		for (int ind = 1; ind < 12; ++ind)
		{
			p[ind] = p[ind-1]*r2;
		}

		x =	(1.0 - w)*(1.0 + w) + w2*(
			+ p[0]       / 3.0
			- p[1] * 2.0 / 45.0
			+ p[2]       / 315.0
			- p[3] * 2.0 / 14175.0
			+ p[4] * 2.0 / 467775.0
			- p[5] * 4.0 / 42567525.0
			+ p[6]       / 638512875.0
			- p[7] * 2.0 / 97692469875.0
			+ p[8] * 2.0 / 9280784638125.0
			- p[9] * 4.0 / 2143861251406875.0
			+ p[10]* 2.0 / 147926426347074375.0
			- p[11]* 4.0 / 48076088562799171875.0
		);

		y = (1.0 - w)
			- p[0] * (3.0 - w) / 6.0
			+ p[1] * (5.0 - w) / 120.0
			- p[2] * (7.0 - w) / 5040.0
			+ p[3] * (9.0 - w) / 362880.0
			- p[4] * (11.0 - w)/ 39916800.0
			+ p[5] * (13.0 - w)/ 6227020800.0
			- p[6] * (15.0 - w)/ 1307674368000.0
			+ p[7] * (17.0 - w)/ 355687428096000.0
			- p[8] * (19.0 - w)/ 121645100408832000.0
			+ p[9] * (21.0 - w)/ 51090942171709440000.0;
 
		z = (1.0 - w2)
			- p[0] * (3.0 - w2) / 6.0
			+ p[1] * (5.0 - w2) / 120.0
			- p[2] * (7.0 - w2) / 5040.0
			+ p[3] * (9.0 - w2) / 362880.0
			- p[4] * (11.0 - w2)/ 39916800.0
			+ p[5] * (13.0 - w2)/ 6227020800.0
			- p[6] * (15.0 - w2)/ 1307674368000.0
			+ p[7] * (17.0 - w2)/ 355687428096000.0
			- p[8] * (19.0 - w2)/ 121645100408832000.0
			+ p[9] * (21.0 - w2)/ 51090942171709440000.0;
	}

	const double h1 (2.0*ws*(pi - acos (ws_r)));
	const double h2 ((2.0*y*z + 2.0*r2*x*x) /
		(sqrt (x)*sqrt (z*z + r2*x*x)));
	const double h3 ( -2.0*y*c/(sqrt (x)));

	double result (h1 + (h2 + h3)/r);

	if (wr_case == 3)
	{
		result = (
			result * (r_limit_2 - r_abs) +
			f (w, r)* (r_abs - r_limit_1)
				) /
			(r_limit_2 - r_limit_1);
	}

	return a*result;
}

double search::pf_plane (double z)
{
	static const std::string name_of_fun ("pf_plane(double)");

	if (is_nan (z, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= z, name_of_fun))
	{
		return qnan;
	}

	return 2.0*sqrt (pi)*sqrt (z);
}

double search::ipf_plane (double p)
{
	static const std::string name_of_fun ("ipf_plane(double)");

	if (is_nan (p, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= p, name_of_fun))
	{
		return qnan;
	}

	return (p/(4.0*pi))*p;
}

double search::pf_angle (double z, double theta)
{
	static const std::string name_of_fun ("pf_angle(double,double)");

	if (is_nan (z, theta, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= z, name_of_fun))
	{
		return qnan;
	}
	
	if (out_of_range (0.0 < theta && theta < 2.0*pi, name_of_fun))
	{
		return qnan;
	}

	return sqrt (2.0*std::min (theta, pi))*sqrt (z);
}

double search::ipf_angle (double p, double theta)
{
	static const std::string name_of_fun ("ipf_angle(double,double)");

	if (is_nan (p, theta, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= p, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 < theta && theta < 2.0*pi, name_of_fun))
	{
		return qnan;
	}

	return p/(2.0*std::min (theta, pi))*p;
}

double search::pf_sphere (double z, double a)
{
	static const std::string name_of_fun ("pf_sphere(double,double)");

	if (is_nan (z, a, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= z && z <= 4.0*pi*a*a && z < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 < a, name_of_fun))
	{
		return qnan;
	}

	return 2.0*sqrt (z)*sqrt (pi - z/(4.0*a*a));
}

double search::ipf_sphere (double p, double a)
{
	static const std::string name_of_fun ("ipf_sphere(double,double)");

	if (is_nan (p, a, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= p && p <= 2.0*pi*a && p < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 < a, name_of_fun))
	{
		return qnan;
	}

	const double tmp (p/(2.0*pi*a));
	return (2.0*pi)*(a*(1.0 - sqrt (1.0 - tmp*tmp))*a);
}

double search::aux_1 (double beta)
{
	const int size (22);
	double b[size];
	b[1] = beta;

	for (int ind = 2; ind < size; ++ind)
	{
		b[ind] = b[ind-1]*beta;
	}

	const double num (
		+ b[2] * pi      / 2.0
		- b[3] * 4.0     / 3.0
		- b[4] * pi      / 6.0
		+ b[5] * 8.0     / 15.0
		+ b[6] * pi      / 45.0
		- b[7] * 8.0     / 105.0
		- b[8] * pi      / 630.0
		+ b[9] * 16.0    / 2835.0
		+ b[10]* pi      / 14175.0
		- b[11]* 8.0     / 31185.0
		- b[12]* pi      / 467775.0
		+ b[13]* 16.0	 / 2027025.0
		+ b[14]* pi * 2.0/ 42567525.0
		- b[15]* 16.0    / 91216125.0
		- b[16]* pi      / 1277025750.0
		+ b[17]* 32.0    / 10854718875.0
		+ b[18]* pi      / 97692469875.0
		- b[19]* 8.0     / 206239658625.0
		- b[20]* pi      / 9280784638125.0
		+ b[21]* 16.0    / 38979295480125.0
		);

	const double c (cos (beta));
	return num/(c*c);
}

//
// Note: in the next 4 functions **pf_circle
// the following system is solved:
//
// (1) p/a   = +/- (pi - 2*beta)*tan(beta),
// (2) z/a^2 = beta - tan(beta) + (pi/2 - beta)*tan^2(beta).
//
// In perimeter functions, z is known and p is sought.
// In inverse perimeter functions, p is known and z is sought.
// In some cases, the substitute beta = alpha - pi/2 is used.
//

double search::pf_circle (double z, double a)
{
	static const std::string name_of_fun ("pf_circle(double,double)");

	if (is_nan (z, a, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= z && z <= pi*a*a && z < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= a, name_of_fun))
	{
		return qnan;
	}

	if (a == 0.0)
	{
		return 0.0;
	}

	double z_norm (z/a);
	z_norm /= a;

	if (z_norm == 0.0)
	{
		return sqrt (2.0*pi)*sqrt (z);
	}

	z = z_norm;

	if (z > pi/2.0)
	{
		z = pi - z;
	}

	if (z == 0.0)
	{
		return 0.0;
	}

	double beta_left (0.0);
	double beta_right (pi/2.0);
	double beta;
	double beta_left_old;
	double beta_right_old;
	double result;

	if (z < pi/2.0 - 1.0)
	{
		for (;;)
		{
			beta = (beta_left + beta_right)/2.0;
			beta_left_old = beta_left;
			beta_right_old = beta_right;

			if (aux_1 (beta) > z)
			{
				beta_right = beta;

				if (beta_right >= beta_right_old)
				{
					break;
				}
			}
			else
			{
				beta_left = beta;

				if (beta_left <= beta_left_old)
				{
					break;
				}
			}
		}

		result = (pi - 2.0*beta)*sin (beta)/cos (beta);
	}
	else
	{
		for (;;)
		{
			beta = (beta_left + beta_right)/2.0;
			beta_left_old = beta_left;
			beta_right_old = beta_right;

			if (beta == pi/2.0)
			{
				break;
			}

			const double t (sin (beta)/cos (beta));

			if (beta - t + t*(pi/2.0 - beta)*t > z)
			{
				beta_right = beta;

				if (beta_right >= beta_right_old)
				{
					break;
				}
			}
			else
			{
				beta_left = beta;

				if (beta_left <= beta_left_old)
				{
					break;
				}
			}
		}

		const double c (sin (pi/2.0 - beta));

		if (c == 0.0)
		{
			result = 2.0;
		}
		else
		{
			result = (pi - 2.0*beta)*sin (beta)/c;
		}
	}

	return a*result;
}

double search::ipf_circle (double p, double a)
{
	static const std::string name_of_fun ("ipf_circle(double,double)");

	if (is_nan (p, a, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= p && p <= 2.0*a && p < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= a, name_of_fun))
	{
		return qnan;
	}

	if (a == 0.0)
	{
		return 0.0;
	}

	const double p_norm (p/a);

	if (p_norm == 0.0)
	{
		return (p/(2.0*pi))*p;
	}

	p = p_norm;

	double beta_left (0.0);
	double beta_right (pi/2.0);
	double beta;
	double beta_left_old;
	double beta_right_old;
	double result;

	for (;;)
	{
		beta = (beta_left + beta_right)/2.0;
		beta_left_old = beta_left;
		beta_right_old = beta_right;

		if (beta == pi/2.0)
		{
			break;
		}

		if ((pi - 2.0*beta)*sin (beta)/cos (beta) > p)
		{
			beta_right = beta;

			if (beta_right >= beta_right_old)
			{
				break;
			}
		}
		else
		{
			beta_left = beta;

			if (beta_left <= beta_left_old)
			{
				break;
			}
		}
	}

	if (beta < pi/4.0)
	{
		result = aux_1 (beta);
	}
	else
	{
		if (beta == pi/2.0)
		{
			result = pi/2.0;
		}
		else
		{
			const double t (sin (beta)/cos (beta));
			result = beta - t + t*(pi/2.0 - beta)*t;
		}
	}

	return a*result*a;
}

double search::opf_circle (double z, double a)
{
	static const std::string name_of_fun ("opf_circle(double,double)");

	if (is_nan (z, a, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (z < pos_infinity || a < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= z, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= a, name_of_fun))
	{
		return qnan;
	}

	if (z == pos_infinity)
	{
		return pos_infinity;
	}

	if (a == 0.0)
	{
		return 2.0*sqrt (pi)*sqrt (z);
	}

	double z_norm (z/a);
	z_norm /= a;

	if (z_norm == 0.0)
	{
		return sqrt (2.0*pi)*sqrt(z);
	}

	if (z_norm == pos_infinity)
	{
		return 2.0*(sqrt (pi)*sqrt (z) - a);
	}

	z = z_norm;

	double result;

	if (z < pi/2.0 - 1.0)
	{
		double beta_left (-pi/2.0);
		double beta_right (0.0);
		double beta;
		double beta_left_old;
		double beta_right_old;

		for (;;)
		{
			beta = (beta_left + beta_right)/2.0;
			beta_left_old = beta_left;
			beta_right_old = beta_right;

			if (aux_1 (beta) < z)
			{
				beta_right = beta;

				if (beta_right >= beta_right_old)
				{
					break;
				}
			}
			else
			{
				beta_left = beta;

				if (beta_left <= beta_left_old)
				{
					break;
				}
			}
		}

		result = (2.0*beta - pi)*sin (beta)/cos (beta);
	}
	else
	{
		double alpha_left (0.0);
		double alpha_right (pi/2.0);
		double alpha;
		double alpha_left_old;
		double alpha_right_old;

		for (;;)
		{
			alpha = (alpha_left + alpha_right)/2.0;
			alpha_left_old = alpha_left;
			alpha_right_old = alpha_right;

			const double t (sin (alpha)/cos (alpha));
			const double tt (t*t);

			if (tt == 0.0)
			{
				alpha = 0.0;
				break;
			}

			if (-pi/2.0 + alpha + 1/t + (pi - alpha)/(t*t) < z)
			{
				alpha_right = alpha;

				if (alpha_right >= alpha_right_old)
				{
					break;
				}
			}
			else
			{
				alpha_left = alpha;

				if (alpha_left <= alpha_left_old)
				{
					break;
				}
			}
		}

		result = 2.0*(pi - alpha)*cos (alpha)/sin (alpha);
	}

	return a*result;
}

double search::iopf_circle (double p, double a)
{
	static const std::string name_of_fun ("iopf_circle(double,double)");

	if (is_nan (p, a, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (p < pos_infinity || a < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= p, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= a, name_of_fun))
	{
		return qnan;
	}

	if (p == pos_infinity)
	{
		return pos_infinity;
	}

	if (a == 0.0)
	{
		return (p/(4.0*pi))*p;
	}

	double p_norm (p/a);

	if (p_norm == 0.0)
	{
		return (p/(2.0*pi))*p;
	}

	if (p_norm == pos_infinity)
	{
		const double tmp (p/2.0 + a);
		return tmp*(1.0/pi)*tmp;
	}
	
	p = p_norm;

	double result;

	if (p < pi/2.0)
	{
		double beta_left (-pi/2.0);
		double beta_right (0.0);
		double beta;
		double beta_left_old;
		double beta_right_old;

		for (;;)
		{
			beta = (beta_left + beta_right)/2.0;
			beta_left_old = beta_left;
			beta_right_old = beta_right;

			if ((2*beta - pi)*sin (beta)/cos (beta) < p)
			{
				beta_right = beta;

				if (beta_right >= beta_right_old)
				{
					break;
				}
			}
			else
			{
				beta_left = beta;

				if (beta_left <= beta_left_old)
				{
					break;
				}
			}
		}

		result = aux_1 (beta);
	}
	else
	{
		double alpha_left (0.0);
		double alpha_right (pi/2.0);
		double alpha;
		double alpha_left_old;
		double alpha_right_old;

		for (;;)
		{
			alpha = (alpha_left + alpha_right)/2.0;
			alpha_left_old = alpha_left;
			alpha_right_old = alpha_right;

			const double t (sin (alpha)/cos (alpha));

			if (t == 0.0)
			{
				alpha = 0.0;
				break;
			}

			if (2.0*(pi - alpha)/t < p)
			{
				alpha_right = alpha;

				if (alpha_right >= alpha_right_old)
				{
					break;
				}
			}
			else
			{
				alpha_left = alpha;

				if (alpha_left <= alpha_left_old)
				{
					break;
				}
			}
		}

		const double t (sin (alpha)/cos (alpha));
		result = -pi/2.0 + alpha + 1.0/t + (pi - alpha)/(t*t);
	}

	return a*result*a;
}

double search::pf_rectangle (double z, double a, double b)
{
	static const std::string name_of_fun ("pf_rectangle(double,double,double)");

	if (is_nan (z, a, b, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= a && a < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= b && b < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= z && z <= a*b && z < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (a > b)
	{
		std::swap (a, b);
	}

	const double a_2 (a/2.0);
	const double ab_2 (a_2*b);

	if (z > ab_2)
	{
		z = ab_2 - (z - ab_2);
	}

	if (z < (a/pi)*a)
	{
		return sqrt (pi)*sqrt (z);
	}
	else
	{
		return a;
	}
}

double search::ipf_rectangle (double p, double a, double b)
{
	static const std::string name_of_fun ("ipf_rectangle(double,double,double)");

	if (is_nan (p, a, b, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= a && a < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= b && b < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (a > b)
	{
		std::swap (a, b);
	}

	if (out_of_range (0.0 <= p && p <= a, name_of_fun))
	{
		return qnan;
	}

	return (p/pi)*p;
}

//
// Note: in the next 2 functions *opf_rectangle
// the following systems are solved:
//
// (1) 2*z = r^2*(2*pi - beta + sin(beta)),
// (2) b = 2*r*sin(beta/2),
// (3) p = (2*pi - beta)*r,
// and
// (1) 2*z = r^2*(2*pi - beta + sin(beta)) - sqrt(a^2 + b^2),
// (2) sqrt(a^2 + b^2) = 2*r*sin(beta/2),
// (3) p = (2*pi - beta)*r.
//
// In opf_rectangle, z is known and p is sought.
// In iopf_rectangle, p is known and z is sought.
//

double search::opf_rectangle (double z, double a, double b)
{
	static const std::string name_of_fun ("opf_rectangle(double,double,double)");

	if (is_nan (z, a, b, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= a && a < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= b && b < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= z, name_of_fun))
	{
		return qnan;
	}

	if (z == pos_infinity)
	{
		return pos_infinity;
	}

	if (a > b)
	{
		std::swap (a, b);
	}

	if (b == 0.0)
	{
		return 2.0*sqrt (pi)*sqrt (z);
	}
	else
	{
		if (z <= pi*(b/8.0)*b)
		{
			return sqrt (2.0*pi)*sqrt (z);
		}
		else
		{
			double result1, result2;

			double beta_left (0.0);
			double beta_right (search::pi);
			double beta;
			double beta_left_old;
			double beta_right_old;
			double r;

			for (;;)
			{
				beta = (beta_left + beta_right)/2.0;
				beta_left_old = beta_left;
				beta_right_old = beta_right;
				r = b/(2.0*sin (beta/2.0));

				if (r*(2.0*pi - beta + sin (beta))*(r/2.0) < z)
				{
					beta_right = beta;

					if (beta_right >= beta_right_old)
					{
						break;
					}
				}
				else
				{
					beta_left = beta;

					if (beta_left <= beta_left_old)
					{
						break;
					}
				}
			}

			result1 = (2.0*pi - beta)*r;

			double diag;

			if ((1.0/b)*(1.0/b) == 0.0)
			{
				const double k (1.0e-170);
				const double ak (a*k);
				const double bk (b*k);
				diag = sqrt (ak*ak + bk*bk)/k;
			}
			else
			{
				if (b*b == 0.0)
				{
					const double k (1.0e+170);
					const double ak (a*k);
					const double bk (b*k);
					diag = sqrt (ak*ak + bk*bk)/k;
				}
				else
				{
					diag = sqrt (a*a + b*b);
				}
			}

			beta_left = 0.0;
			beta_right = pi;

			for (;;)
			{
				beta = (beta_left + beta_right)/2.0;
				beta_left_old = beta_left;
				beta_right_old = beta_right;
				r = diag/(2.0*sin (beta/2.0));

				if (r*(2.0*pi - beta + sin (beta))*(r/2.0) - (a/2.0)*b < z)
				{
					beta_right = beta;

					if (beta_right >= beta_right_old)
					{
						break;
					}
				}
				else
				{
					beta_left = beta;

					if (beta_left <= beta_left_old)
					{
						break;
					}
				}
			}

			result2 = (2.0*pi - beta)*r;

			return std::min (result1, result2);
		}
	}
}

double search::iopf_rectangle (double p, double a, double b)
{
	static const std::string name_of_fun ("iopf_rectangle(double,double,double)");

	if (is_nan (p, a, b, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= a && a < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= b && b < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= p, name_of_fun))
	{
		return qnan;
	}

	if (p == pos_infinity)
	{
		return pos_infinity;
	}

	if (a > b)
	{
		std::swap (a, b);
	}

	if (b == 0.0)
	{
		return (p/(4.0*pi))*p;
	}
	else
	{
		if (p <= pi*b/2.0)
		{
			return (p/(2.0*pi))*p;
		}
		else
		{
			double result1, result2;

			double beta_left (0.0);
			double beta_right (search::pi);
			double beta;
			double beta_left_old;
			double beta_right_old;
			double r;

			for (;;)
			{
				beta = (beta_left + beta_right)/2.0;
				beta_left_old = beta_left;
				beta_right_old = beta_right;
				r = b/(2.0*sin (beta/2.0));

				if ((2.0*pi - beta)*r < p)
				{
					beta_right = beta;

					if (beta_right >= beta_right_old)
					{
						break;
					}
				}
				else
				{
					beta_left = beta;

					if (beta_left <= beta_left_old)
					{
						break;
					}
				}
			}

			result1 = r*(2.0*pi - beta + sin (beta))*(r/2.0);

			double diag;

			if ((1.0/b)*(1.0/b) == 0.0)
			{
				const double k (1.0e-170);
				const double ak (a*k);
				const double bk (b*k);
				diag = sqrt (ak*ak + bk*bk)/k;
			}
			else
			{
				if (b*b == 0.0)
				{
					const double k (1.0e+170);
					const double ak (a*k);
					const double bk (b*k);
					diag = sqrt (ak*ak + bk*bk)/k;
				}
				else
				{
					diag = sqrt (a*a + b*b);
				}
			}

			if (p <= pi*(diag/2.0))
			{
				return result1;
			}

			beta_left = 0.0;
			beta_right = pi;

			for (;;)
			{
				beta = (beta_left + beta_right)/2.0;
				beta_left_old = beta_left;
				beta_right_old = beta_right;
				r = diag/(2.0*sin (beta/2.0));

				if ((2.0*pi - beta)*r < p)
				{
					beta_right = beta;

					if (beta_right >= beta_right_old)
					{
						break;
					}
				}
				else
				{
					beta_left = beta;

					if (beta_left <= beta_left_old)
					{
						break;
					}
				}
			}

			result2 = r*(2.0*pi - beta + sin (beta))*(r/2.0) - (a/2.0)*b;

			return std::max (result1, result2);
		}
	}
}

double search::pf_3d (double z)
{
	static const std::string name_of_fun ("pf_3d(double)");

	if (is_nan (z, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= z, name_of_fun))
	{
		return qnan;
	}

	return pow (6.0*sqrt (pi)*z, 2.0/3.0);
}

double search::ipf_3d (double p)
{
	static const std::string name_of_fun ("ipf_3d(double)");

	if (is_nan (p, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= p, name_of_fun))
	{
		return qnan;
	}

	return sqrt ((p/(36.0*pi))*p*p);
}

double search::aux_2 (double beta)
{
	const int size (10);
	double b[size];
	b[0] = beta*beta;

	for (int ind = 1; ind < size; ++ind)
	{
		b[ind] = b[ind-1]*b[0];
	}

	const double result (
		+ b[0] / 2.0
		- b[1] / 24.0
		+ b[2] / 720.0 
		- b[3] / 40320.0
		+ b[4] / 3628800.0 
		- b[5] / 479001600.0
		+ b[6] / 87178291200.0
		- b[7] / 20922789888000.0
		+ b[8] / 6402373705728000.0
		- b[9] / 2432902008176640000.0
		);

	return result;
}

double search::pf_sphere_3d (double z, double a)
{
	static const std::string name_of_fun ("pf_sphere_3d(double,double)");

	if (is_nan (z, a, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= z && z <= (4.0*pi/3.0)*a*a*a && z < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= a, name_of_fun))
	{
		return qnan;
	}

	if (a == 0.0)
	{
		return 0.0;
	}

	double z_norm (z/a);
	z_norm /= a;
	z_norm /= a;

	if (z_norm == 0.0)
	{
		return pow (3.0*sqrt (2.0*pi)*z, 2.0/3.0);
	}

	z = z_norm;

	if (z > 2.0*pi/3.0)
	{
		z = 4.0*pi/3.0 - z;
	}

	if (z == 0.0)
	{
		return 0.0;
	}

	double beta_left (0.0);
	double beta_right (pi/2.0);
	double beta;
	double beta_left_old;
	double beta_right_old;
	double result;

	if (z < pi/2.0 - 1.0)
	{
		for (;;)
		{
			beta = (beta_left + beta_right)/2.0;
			beta_left_old = beta_left;
			beta_right_old = beta_right;

			double z_beta;
			const double s (sin (beta));

			if (s == 0.0)
			{
				z_beta = 2.0;
			}
			else
			{
				const double tmp (aux_2 (beta));
				const double c (1.0 - tmp);
				const double ct (c/s);

				z_beta = (1.0 - s)*(1.0 - s)*(2.0 + s) +
					tmp*tmp*(2.0 + c)*ct*ct*ct;
			}

			z_beta *= pi/3.0;

			if (z_beta < z)
			{
				beta_right = beta;

				if (beta_right >= beta_right_old)
				{
					break;
				}
			}
			else
			{
				beta_left = beta;

				if (beta_left <= beta_left_old)
				{
					break;
				}
			}
		}

		const double s (sin (beta));

		if (s == 0.0)
		{
			result = 1.0;
		}
		else
		{
			const double c (cos (beta));
			const double ct (c/s);

			result = aux_2 (beta)*ct*ct;
		}

		result *= 2.0*pi;
	}
	else
	{
		for (;;)
		{
			beta = (beta_left + beta_right)/2.0;
			beta_left_old = beta_left;
			beta_right_old = beta_right;

			double z_beta;
			double s (sin (beta));

			const double tmp (aux_2 (beta));
			const double c (1.0 - tmp);
			const double t (s/c);

			z_beta = pi*(tmp*tmp*(2.0 + c) +
				(1.0 - s)*(1.0 - s)*(2.0 + s)*t*t*t)/3.0;

			if (z_beta > z)
			{
				beta_right = beta;

				if (beta_right >= beta_right_old)
				{
					break;
				}
			}
			else
			{
				beta_left = beta;

				if (beta_left <= beta_left_old)
				{
					break;
				}
			}
		}

		const double t (tan (beta));

		result = 2.0*pi*(1 - sin (beta))*t*t;
	}

	return a*result*a;
}

double
search::convex_polygon::area () const
{
	if (num_vertices () < 3)
	{
		return 0.0;
	}

	double area (0.0);
	std::list<vertex>::const_iterator iter (vertices.begin ());
	point origin (iter->coord);
	++iter;
	point last (iter->coord);
	++iter;

	for (; iter != vertices.end (); ++iter)
	{
		area += origin.area (last, iter->coord);
		last = iter->coord;
	}

	return area;
}

void
search::convex_polygon::convex_hull ()
{
	if (num_vertices () < 3) return; // nothing to do

	// in the case num_vertices = 3 nothing to do either,
	// but the resulting order of vertices is important

	std::list<vertex> hull; // will hold the convex hull

	// find the lower right point

	std::list<vertex>::iterator iter (vertices.begin ()), select (iter);

	point lr_point (iter->coord); // lower right point
	++iter;

	for (; iter != vertices.end (); ++iter)
	{
		point cur (iter->coord);

		if (cur.y < lr_point.y || (cur.y == lr_point.y && cur.x > lr_point.x))
		{
			lr_point = cur;
			select = iter;
		}
	}

	// mark the vertex as "first" and add it to the list
	select->state = vertex::first;
	hull.push_front (vertex (lr_point));

	// 2 last added to the convex hull
	point last_added (lr_point), previous (lr_point);

	// initial location exactly on the left from last_added
	previous.x = previous.x - 1.0;

	// main loop

	for (;;)
	{
		double
			min_ang (6.29), // > 2*pi
			max_dist (0.0);

		// find new vertice for the convex hull
		for (iter = vertices.begin (); iter != vertices.end (); ++iter)
		{
			// check if vertex is already a part of the hull
			if (iter->state == vertex::used)
			{
				continue;
			}

			// check for duplicates;
			// skip if iter points to last_added;
			// also, make sure the first iteration is correct
			if (iter->coord == last_added)
			{
				continue;
			}

			// calculate the angle between the three points
			double
				ang (last_added.angle (
					last_added*2.0 - previous, iter->coord)),
				dist ((iter->coord - last_added).abs ());

			if (ang < min_ang || (ang == min_ang && dist > max_dist))
			{
				min_ang = ang;
				max_dist = dist;

				// save the position
				select = iter;
			}
		}

		// if the selected vertice is "first," we're done
		if (select->state == vertex::first)
		{
			break;
		}
		else
		{
			// add the selected vertice to the convex hull
			// and prepare to the next iteration

			hull.push_front (vertex (select->coord));
			select->state = vertex::used;
			previous = last_added;
			last_added = select->coord;
		}
	}

	vertices.swap (hull);
}

search::convex_polygon_pf::convex_polygon_pf (
	const convex_polygon& cp)
	: num_vertices_v (cp.num_vertices ()), area_v (cp.area ()),
	  half_area_v (area_v / 2.0), sides (num_vertices_v),
	  function (0), tmp_function (0),
	  pf_ok (false), pf_max_ok (false), sc_ok (false),
	  maximum_v (0.0), num_segments_v (0)
{
	// fill in the array of cp sides

	// since cp.convex_hull has stored the vertices in clockwise order,
	// the sides will be stored in clockwise order, too

	convex_polygon::const_iterator iter (cp.begin ());

	if (num_vertices () > 2)
	{
		convex_polygon::point first (*iter);
		cyclic_uint index (this);

		unsigned skipped (0);

		while (index != num_vertices () - 1)
		{
			convex_polygon::point p (*iter);
			++iter;
			convex_polygon::point q (*iter);

			// no sides with (p - q).abs = 0 will be present
			if ((p - q).abs () == 0.0)
			{
				++index;
				++skipped;
				continue;
			}

			(*this) [index++] = side (p, q);
		}

		if ((*iter - first).abs () != 0.0)
		{
			(*this) [index] = side (*iter, first);
		}
		else
		{
			++skipped;
		}

		num_vertices_v -= skipped;
	}
}

double
search::convex_polygon_pf::pf (double z)
{
	static const std::string name_of_fun ("convex_polygon_pf::pf(double)");

	if (is_nan (z, name_of_fun))
	{
		return qnan;
	}

	if (out_of_range (0.0 <= z && z <= area () && z < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	if (!pf_ok)
	{
		find_pf ();
	}

	if (z > half_area ())
	{
		z = area () - z;
	}

	//replace with less trivial search for large num_segments()
	for (unsigned index = 0; index < num_segments (); ++index)
	{
		if (z <= function [index].b)
		{
			return function [index].pf (z);
		}
	}

	return qnan; // unreachable
}

double
search::convex_polygon_pf::ipf (double p)
{
	static const std::string name_of_fun ("convex_polygon_pf::ipf(double)");

	if (is_nan (p, name_of_fun))
	{
		return qnan;
	}

	if (!pf_ok)
	{
		find_pf ();
	}

	if (out_of_range (0.0 <= p && p <= maximum () && p < pos_infinity, name_of_fun))
	{
		return qnan;
	}

	for (unsigned index = 0; index < num_segments (); ++index)
	{
		if (function [index].pfb >= p)
		{
			return function [index].ipf (p);
		}
	}

	return qnan; // unreachable
}

double
search::convex_polygon_pf::maximum ()
{
	if (!pf_max_ok)
	{
		find_pf_max ();
	}

	return maximum_v;
}

unsigned
search::convex_polygon_pf::num_segments ()
{
	if (!pf_ok)
	{
		find_pf ();
	}

	return num_segments_v;
}

double
search::convex_polygon_pf::a (unsigned index)
{
	static const std::string name_of_fun ("convex_polygon_pf::a(unsigned)");

	if (!pf_ok)
	{
		find_pf ();
	}

	const unsigned num_segments (this->num_segments ());

	if (out_of_range (index <= num_segments, name_of_fun))
	{
		return qnan;
	}

	const unsigned max_index ((num_segments - 1) >> 1);

	if (index <= max_index)
	{
		return function [index].a;
	}
	else
	if (index == max_index + 1)
	{
		if ((num_segments & 1) == 0)
		{
			return half_area ();
		}
		else
		{
			return area() - function [num_segments - index].a;
		}
	}
	else
	{
		return area() - function [num_segments - index].a;
	}
}

double
search::convex_polygon_pf::theta (unsigned index)
{
	static const std::string name_of_fun ("convex_polygon_pf::theta(unsigned)");

	if (!pf_ok)
	{
		find_pf ();
	}

	const unsigned num_segments (this->num_segments ());

	if (out_of_range (1 <= index && index <= num_segments, name_of_fun))
	{
		return qnan;
	}

	--index;

	const unsigned max_index ((num_segments - 1) >> 1);

	if (index <= max_index)
	{
		return function [index].theta;
	}
	else
	{
		return -function [num_segments - index - 1].theta;
	}
}

double
search::convex_polygon_pf::zeta (unsigned index)
{
	static const std::string name_of_fun ("convex_polygon_pf::zeta(unsigned)");

	if (!pf_ok)
	{
		find_pf ();
	}

	const unsigned num_segments (this->num_segments ());

	if (out_of_range (1 <= index && index <= num_segments, name_of_fun))
	{
		return qnan;
	}

	--index;

	const unsigned max_index ((num_segments - 1) >> 1);

	if (index <= max_index)
	{
		return function [index].zeta;
	}
	else
	{
		return -area () - function [num_segments - index - 1].zeta;
	}
}

double
search::convex_polygon_pf::shortest (
	bool& is_arc, convex_polygon::point& start,
	convex_polygon::point& end, convex_polygon::point& center)
{
	if (!sc_ok)
	{
		find_pf_max ();
	}

	if (maximum () == 0.0)
	{
		return 0.0;
	}

	is_arc = (shortest_curve.form == partial_pf::sqrt);

	start = shortest_curve.start;
	end = shortest_curve.end;

	if (is_arc)
	{
		center = shortest_curve.center;
	}

	return maximum ();
}

double
search::convex_polygon_pf::area (
	cyclic_uint index_1, cyclic_uint index_2) const
{
	double area (0.0);
	++index_1;

	if (index_1 == index_2)
	{
		return 0.0;
	}

	const convex_polygon::point point ((*this) [index_1].p);

	for (++index_1; index_1 != index_2; ++index_1)
	{
		area += point.area ((*this) [index_1].p, (*this) [index_1].q);
	}

	return area;
}

double
search::convex_polygon_pf::area (
	cyclic_uint index_1, cyclic_uint index_2,
	const convex_polygon::point& point) const
{
	double area (0.0);
	
	for (++index_1; index_1 != index_2; ++index_1)
	{
		area += point.area ((*this) [index_1].p, (*this) [index_1].q);
	}

	return area;
}

void
search::convex_polygon_pf::find_pf ()
{
	// fictious node to avoid handling holes in the definition domain,
	// also, this will be a "stub" node in the case num_vertices < 3
	tmp_function = new partial_pf_node (0.0, half_area (), sqrt (10.0*pi*area ()));

	if (num_vertices () > 2)
	{
		cyclic_uint index_1 (this), index_2 (this);

		// check all pairs of sides
		for (index_1 = 1; index_1 != 0; ++index_1)
		{
			for (index_2 = 0; index_2 != index_1; ++index_2)
			{
				// "partial" perimeter function of the two sides
				const partial_pf ppf (*this, index_1, index_2);

				// empty definition domain
				if (ppf.form == partial_pf::none)
				{
					continue;
				}

				// iterators
				partial_pf_node (*iter)(tmp_function), (*save)(0);

				// find left "insertion point"

				double left, right;
				bool f_left, f_right;

				new_loop:

				for (; iter != 0; iter = iter->next)
				{
					if (iter->begin (ppf, left, right, f_left, f_right))
					{
						break;
					}
				}

				// ppf isn't represented in the perimeter function
				if (iter == 0)
				{
					continue;
				}

				if (!f_left)
				{
					// left "insertion point" doesn't coincide with
					// the left boundary of the iter definition domain:
					// adding a new node

					partial_pf_node* new_node = new partial_pf_node (*iter);
					iter->b = left;
					iter->pfb = iter->pf (left);
					iter->next = new_node;
					iter = new_node;
				}

				if (!f_right)
				{
					// right insertion already found, adding a new node

					partial_pf_node* new_node = new partial_pf_node (*iter);
					*((partial_pf*)(iter)) = ppf;
					iter->a = left;
					iter->pfa = iter->pf (left);
					iter->b = right;
					iter->pfb = iter->pf (right);
					iter->next = new_node;
					new_node->a = right;
					new_node->pfa = new_node->pf (right);
					iter = new_node->next;

					goto new_loop;
				}

				// insert ppf; right boundary is not known yet
				*((partial_pf*)(iter)) = ppf;
				iter->a = left;
				iter->pfa = iter->pf (left);

				// save so as to be able to fill in the right boundary
				save = iter;

				// another iterator storing previous position
				partial_pf_node* prev = iter;

				iter = iter->next;

				// find right "insertion point"
				while (iter != 0)
				{
					if (iter->end (ppf, right, f_left))
					{
						break;
					}
					else
					{
						// node totally removed
						prev->next = iter->next;
						iter->next = 0;
						delete iter;
						iter = prev->next;
					}
				}

				// fill in right boundary
				save->b = right;
				save->pfb = save->pf (right);

				if (iter != 0 && !f_left)
				{
					// right "insertion point" doesn't coincide with
					// the beginning of a node; shifting the beginning.
					iter->a	= right;
					iter->pfa = iter->pf (right);
				}

				// while iter != 0
				goto new_loop;
			}
		}
	}

	pf_ok = pf_max_ok = true;

	// count the number of segments

	partial_pf_node (*iter)(tmp_function);

	for (; iter != 0; iter = iter->next)
	{
		++num_segments_v;
	}

	function = new partial_pf [num_segments_v];

	for (
		iter = tmp_function, num_segments_v = 0;
		iter != 0;
		iter = iter->next, ++num_segments_v)
	{
		function [num_segments_v] = partial_pf (*iter);
	}

	maximum_v = function [num_segments_v - 1].pfb;

	if (function [num_segments_v - 1].form == partial_pf::constant)
	{
		num_segments_v = num_segments_v*2 - 1;
	}
	else
	{
		num_segments_v = num_segments_v*2;
	}

	delete tmp_function;
}

void
search::convex_polygon_pf::find_pf_max ()
{
	if (num_vertices () < 3)
	{
		maximum_v = 0.0;
		pf_max_ok = true;
		
		// sc_ok remains false

		return;
	}

	cyclic_uint
		index_1 (this), index_2 (this),
		save_index_1 (this), save_index_2 (this);

	// will accumulate the maximum
	double max (sqrt (pi*area ()));

	// check all pairs of sides
	for (index_1 = 1; index_1 != 0; ++index_1)
	{
		for (index_2 = 0; index_2 != index_1; ++index_2)
		{
			// "partial" perimeter function of the two sides
			const partial_pf ppf (*this, index_1, index_2);

			// empty definition domain
			if (ppf.form == partial_pf::none)
			{
				continue;
			}

			// arg max outside definition domain
			if (ppf.b < half_area ())
			{
				continue;
			}

			// update
			if (ppf.pfb < max)
			{
				max = ppf.pfb;
				save_index_1 = index_1;
				save_index_2 = index_2;
			}
		}
	}

	// "partial" perimeter function of the two sides
	partial_pf ppf (*this, save_index_1, save_index_2, &shortest_curve);

	maximum_v = max;
	pf_max_ok = sc_ok = true;
}

search::convex_polygon_pf::partial_pf::partial_pf (
	const convex_polygon_pf& pf, cyclic_uint index_1,
	cyclic_uint index_2, eff_perimeter* shortest_curve)
{
	const side side_1 (pf [index_1]), side_2 (pf [index_2]);
	const convex_polygon::point
	pq1 (side_1.q - side_1.p),
	pq2 (side_2.q - side_2.p);
	theta = convex_polygon::point (0.0, 0.0).angle (pq1, -pq2);

	if (theta == 0.0)
	{
		const double
			p2 (side_2.p.proj (side_1.p, side_1.q)),
			q2 (side_2.q.proj (side_1.p, side_1.q));

		if (p2 <= 0.0 || q2 >= 1.0)
		{
			form = none;
			return;
		}

		const convex_polygon::point
			r (p2 < 1.0 ?
				side_1.p + pq1 * p2 :
				side_2.p - pq1 * (p2 - 1.0)),
			s (q2 < 0.0 ?
				side_2.q - pq1 * q2 :
				side_1.p + pq1 * q2);

		const double
			area_r (pf.area (index_1, index_2, r)),
			area_s (pf.area (index_2, index_1, s));

			a = std::min (area_r, area_s);
			b = std::min (pf.area () - a, pf.half_area ());

		if (a == b)
		{
			form = none;
			return;
		}

		pfb = pfa = (side_1.p).dist (side_2.p, side_2.q);
		theta = 0.0;
		zeta = pfa;
		form = constant;

		if (shortest_curve != 0)
		{
			if (b < pf.half_area ())
			{
				shortest_curve->form = none;
			}
			else
			{
				shortest_curve->form = constant;

				const convex_polygon::point
					r1 (side_1.p + pq1 * r.proj (side_1.p, side_1.q)),
					s1 (side_1.p + pq1 * s.proj (side_1.p, side_1.q)),
					rs (s1 - r1);

				const double rsa (rs.abs ());

				if (pfa*rsa == 0.0)
				{
					shortest_curve->start = shortest_curve->end = r;
				}
				else
				{
					const convex_polygon::point t (
						(r1 + s1 + rs*(area_s - area_r)/(rsa*pfa))/2.0); // bug fixed - was (area_r - area_s)

					const double tp (t.proj (r1, s1));
					shortest_curve->start = r1 + rs*tp;
					shortest_curve->end = side_2.p + pq2*(shortest_curve->start).proj (side_2.p, side_2.q);
				}
			}
		}
	}
	else
	{
		const convex_polygon::point
			o (0.0, 0.0),
			r ((pq1*o.sign_area (side_2.p, side_2.q) -
				pq2*o.sign_area (side_1.p, side_1.q))/
				o.sign_area (pq1, pq2));

		double
			p1 ((side_1.p - r).abs ()),
			q1 ((side_1.q - r).abs ()),
			p2 ((side_2.p - r).abs ()),
			q2 ((side_2.q - r).abs ());

		if (theta < pi) // q1p2
		{
			if (p1 <= p2 || q1 >= q2)
			{
				form = none;
				return;
			}

			if (side_1.q == side_2.p)
			{
				q1 = p2 = 0.0;
			}

			double
				r_min (std::max (q1, p2)),
				r_max (std::min (p1, q2));

			cyclic_uint index (index_2);

			for (++index; index != index_1; ++index)
			{
				const double proj (r.proj (pf [index].p, pf [index].q));

				if (0.0 < proj && proj < 1.0)
				{
					r_max = std::min (
						r_max, r.dist (pf [index].p, pf [index].q));
				}
			}

			if (r_min >= r_max)
			{
				form = none;
				return;
			}

			zeta = r.area (side_1.q, side_2.p) - pf.area (index_1, index_2);
			a = r_min*r_min*theta/2.0 - zeta;
			b = r_max*r_max*theta/2.0 - zeta;

			if (a > pf.half_area ())
			{
				form = none;
				return;
			}

			form = sqrt;
			pfa = r_min*theta;
			pfb = r_max*theta;

			if (b > pf.half_area ())
			{
				b = pf.half_area ();
				pfb	= this->pf (b);
			}

			if (a == b)
			{
				form = none;
				return;
			}

			if (shortest_curve != 0)
			{
				if (b < pf.half_area ())
				{
					shortest_curve->form = none;
				}
				else
				{
					shortest_curve->form = sqrt;
					shortest_curve->center = r;

					const double rad (pfb/theta);
					shortest_curve->start = r + (side_1.p - r)*rad/p1;
					shortest_curve->end = r + (side_2.q - r)*rad/q2;
				}
			}
		}
		else
		if (theta > pi) // p1q2
		{
			if (p1 >= p2 || q1 <= q2)
			{
				form = none;
				return;
			}

			if (side_1.p == side_2.q)
			{
				p1 = q2 = 0.0;
			}

			double
				r_min (std::max (p1, q2)),
				r_max (std::min (q1, p2));

			cyclic_uint index (index_1);

			for (++index; index != index_2; ++index)
			{
				const double proj (r.proj (pf [index].p, pf [index].q));

				if (0.0 < proj && proj < 1.0)
				{
					r_max = std::min (
						r_max, r.dist (pf [index].p, pf [index].q));
				}
			}

			if (r_min >= r_max)
			{
				form = none;
				return;
			}

			theta = 2.0*pi - theta;
			zeta = r.area (side_1.p, side_2.q) - pf.area (index_2, index_1);
			a = r_min*r_min*theta/2.0 - zeta;
			b = r_max*r_max*theta/2.0 - zeta;

			if (a > pf.half_area ())
			{
				form = none;
				return;
			}

			form = sqrt;
			pfa = r_min * theta;
			pfb = r_max * theta;

			if (b > pf.half_area ())
			{
				b = pf.half_area ();
				pfb	= this->pf (b);
			}

			if (a == b)
			{
				form = none;
				return;
			}

			if (shortest_curve != 0)
			{
				if (b < pf.half_area ())
				{
					shortest_curve->form = none;
				}
				else
				{
					shortest_curve->form = sqrt;
					shortest_curve->center = r;

					const double rad (pfb/theta);
					shortest_curve->start = r + (side_2.p - r)*rad/p2;
					shortest_curve->end = r + (side_1.q - r)*rad/q1;
				}
			}
		}
		else // collinear
		{
			form = none;
			return;
		}
	}
}

bool
search::convex_polygon_pf::partial_pf::begin (
	const partial_pf& ppf, double& left, double& right,
		bool& f_left, bool& f_right)
{
	if (b <= ppf.a || a >= ppf.b)
	{
		return false;
	}

	double delta (theta - ppf.theta);

	if (equal (delta, 0.0))
	{
		return false;
	}

	bool
		possibly_left (a >= ppf.a),
		possibly_right (b <= ppf.b);

	const double
		com_a (std::max (a, ppf.a)),
		com_b (std::min (b, ppf.b)),
		pf_com_a (pf (com_a)),
		pf_com_b (pf (com_b)),
		ppf_com_a (ppf.pf (com_a)),
		ppf_com_b (ppf.pf (com_b));

	double
		delta_a (pf_com_a - ppf_com_a),
		delta_b (pf_com_b - ppf_com_b);

		trim (delta_a);
		trim (delta_b);

	if (delta_a < 0.0)
	{
		if (delta_b <= 0.0)
		{
			return false;
		}
		else
		{
			left = root (ppf);
			right = com_b;

			if (left == right)
			{
				return false;
			}

			f_left = false;
			f_right = possibly_right;
		}
	}
	else
	if (delta_a == 0.0)
	{
		if (delta_b <= 0.0)
		{
			return false;
		}
		else
		{
			left = com_a;
			right = com_b;
			f_left = possibly_left;
			f_right = possibly_right;
		}
	}
	else // pf_com_a > ppf_om_a
	{
		if (delta_b < 0.0)
		{
			left = com_a;
			right = root (ppf);

			if (left == right)
			{
				return false;
			}

			f_left = possibly_left;
			f_right = false;
		}
		else
		{
			left = com_a;
			right = com_b;
			f_left = possibly_left;
			f_right = possibly_right;
		}
	}

	return true;
}

bool
search::convex_polygon_pf::partial_pf::end (
	const partial_pf& ppf, double& right, bool& f_left)
{
	if (a >= ppf.b)
	{
		right = ppf.b;
		f_left = false;
		return true;
	}

	if (b > ppf.b && pf (ppf.b) >= ppf.pfb)
	{
		right = ppf.b;
		f_left = false;
		return true;
	}

	bool possibly_left (a >= ppf.a);

	const double
		com_a (std::max (a, ppf.a)),
		com_b (std::min (b, ppf.b)),
		pf_com_a (pf (com_a)),
		pf_com_b (pf (com_b)),
		ppf_com_a (ppf.pf (com_a)),
		ppf_com_b (ppf.pf (com_b));

	if (pf_com_a > ppf_com_a)
	{
		if (pf_com_b >= ppf_com_b)
		{
			right = com_b;
			return false;
		}
		else
		{
			right = root (ppf);
			f_left = false;
		}
	}
	else
	if (pf_com_a == ppf_com_a)
	{
		if (pf_com_b >= ppf_com_b)
		{
			right = com_b;
			return false;
		}
		else
		{
			right = com_a;
			f_left = possibly_left;
		}
	}
	else
	{
		right = com_a;
		f_left = possibly_left;
	}

	return true;
}

double
search::convex_polygon_pf::partial_pf::root (
	const partial_pf& ppf) const
{
	if (form == constant)
	{
		if (ppf.form == sqrt)
		{
			return (pfa/2.0)*(pfa/ppf.theta) - ppf.zeta;
		}
		else
		{
			// ??? shouldn't be reachable
			return std::max (a, ppf.a);
		}
	}
	else
	{
		if (ppf.form == sqrt)
		{
			if (theta == ppf.theta)
			{
				return std::max (a, ppf.a);
			}

			return (ppf.theta*ppf.zeta - theta*zeta)/(theta - ppf.theta);
		}
		else
		{
			return (ppf.pfa/2.0)*(ppf.pfa/theta) - zeta;
		}
	}
}

#ifdef SEARCH_CPP_THROW_RANGE
#undef SEARCH_CPP_THROW_RANGE
#endif