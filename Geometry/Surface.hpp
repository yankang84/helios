/*
 Copyright (c) 2012, Esteban Pellegrino
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SURFACE_HPP_
#define SURFACE_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <fstream>

#include "../Common.hpp"

namespace Helios {

	/* Define the Cell class */
	class Cell;

	class Surface {

	public:

		/* Information about the surfaces */
		enum SurfaceInfo {
			NONE       = 0,
			REFLECTING = 1
		};

		/* Sense of the coordinate respect this surface */
		inline bool sense(const Coordinate& pos) const {return (function(pos) >= 0);}
		/* Return a normal vector at some point on the surface */
		virtual void normal(const Coordinate& point, Direction& vnormal) const = 0;
		/* Determine distance to intersection with the surface. Returns whether it hits and pass back what the distance is. */
		virtual bool intersect(const Coordinate& pos, const Direction& dir, const bool& sense, double& distance) = 0;

		/* Return the user ID associated with this surface. */
		const SurfaceId& getUserId() const {return surfid;}
		/* Return the internal ID associated with this surface. */
		const InternalSurfaceId& getInternalId() const {return int_surfid;}
		/* Get the extra information of the surface */
		SurfaceInfo getFlags() const {return flag;}
		/* Set different options for the surfaces */
		void setFlags(SurfaceInfo new_flag) {flag = new_flag;}

	protected:

		/* Surface constructor function */
		typedef Surface(*(*Constructor)(const SurfaceId&, const std::vector<double>&));
		/* Friendly factory */
		friend class SurfaceFactory;
		/* Friendly printer */
		friend std::ostream& operator<<(std::ostream& out, const Surface& q);

		/* Create surface from user id */
		Surface(const SurfaceId& surfid);
		/* Create surface from user id and flags */
		Surface(const SurfaceId& surfid, SurfaceInfo flag);
		/* Prevent copy */
		Surface(const Surface& surface);
		Surface& operator= (const Surface& other);

		/* Add a neighbor cell of this surface */
		void addNeighborCell(const bool& sense, Cell* cell);
		/* Get neighbor cells of this surface */
		const std::vector<Cell*>& getNeighborCell(const bool& sense) const;

		/* Mathematically define a surface as a collection of points that satisfy this equation */
		virtual double function(const Coordinate& pos) const = 0;
		/* Print internal parameters of the surface */
		virtual void print(std::ostream& out) const = 0;

		/* Get the name of this surface */
		virtual std::string name() const = 0;
		/* Get the constructor function (used by the parser) */
		virtual Constructor constructor() const = 0;

		virtual ~Surface() {/* */};

	private:
		/* Static counter, incremented by one each time a surface is created */
		static size_t counter;
		/* Internal identification of this surface */
		InternalSurfaceId int_surfid;
		/* User's identification of this surface */
		SurfaceId surfid;
		/* Information about the surface */
		SurfaceInfo flag;
		/* Neighbor cells */
		std::vector<Cell*> neighbor_pos;
		std::vector<Cell*> neighbor_neg;
	};

	inline void Surface::addNeighborCell(const bool& sense, Cell* cell) {
		if(sense)
			neighbor_pos.push_back(cell);
		else
			neighbor_neg.push_back(cell);
	}

	inline const std::vector<Cell*>& Surface::getNeighborCell(const bool& sense) const {
		if(sense)
			return neighbor_pos;
		else
			return neighbor_neg;
	}

	class SurfaceFactory {

		/* Static instance of the factory */
		static SurfaceFactory factory;

		/* Map of surfaces types and constructors */
		std::map<std::string, Surface::Constructor> constructor_table;

		/* Private constructor */
		SurfaceFactory() {/* */};
		void operator=(SurfaceFactory&);
		SurfaceFactory(const SurfaceFactory&);
		virtual ~SurfaceFactory() {/* */}

	public:
		/* Access the factory, reference to the static singleton */
		static SurfaceFactory& access() {return factory;}

		/* Exception */
		class BadSurfaceCreation : public std::exception {
			std::string reason;
		public:
			BadSurfaceCreation(const std::string& type) {
				reason = "Cannot create type " + type;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadSurfaceCreation() throw() {/* */};
		};

		/* Register a new surface */
		void registerSurface(const Surface& surface);

		/* Create a new surface */
		Surface* createSurface(const std::string& type, const SurfaceId& surid, const std::vector<double>& coeffs) const;

	};

	/* Output surface information */
	std::ostream& operator<<(std::ostream& out, const Surface& q);

} /* namespace Helios */

#endif /* SURFACE_HPP_ */
