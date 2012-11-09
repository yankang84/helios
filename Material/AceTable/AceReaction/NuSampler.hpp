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

#ifndef NUSAMPLER_HPP_
#define NUSAMPLER_HPP_

#include "../../../Common/Interpolate.hpp"
#include "../AceReader/TyrDistribution.hpp"

namespace Helios {

namespace AceReaction {
	/*
	 * Classes to sample the number of particles released on a reaction. Could be
	 * a fixed value or energy dependent.
	 */

	/* Fixed NU (No dependency with particle's energy) */
	class FixedNu {
		double number;
	public:
		FixedNu(const Ace::TyrDistribution& tyr) : number(tyr.getTyr()) {/* */}
		double getNu(double energy) {return number;}
		~FixedNu() {/* */}
	};

	/* Get NU from a table */
	class TabularNu {
		std::vector<double> energies; /* tabular energies points */
		std::vector<double> nu;       /* Values of NU */
	public:
		TabularNu(const Ace::TyrDistribution& tyr) : energies(tyr.getEnergies()), nu(tyr.getNu()) {/* */}
		double getNu(double energy) {
			/* Get interpolation data */
			std::pair<size_t,double> res = interpolate(energies.begin(), energies.end(), energy);
			/* Index */
			size_t idx = res.first;
			/* Interpolation factor */
			double factor = res.second;
			/* Return interpolated NU */
			return factor * (nu[idx + 1] - nu[idx]) + nu[idx];
		}
		~TabularNu() {/* */}
	};
}

}


#endif /* NUSAMPLER_HPP_ */
