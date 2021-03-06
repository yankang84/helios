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

#include "FissionReaction.hpp"

namespace Helios {

using namespace AceReaction;

Fission::Fission(const AceIsotopeBase* isotope, const Ace::NeutronReaction& ace_reaction) :
		GenericReaction(isotope, ace_reaction) {/* */}

void Fission::print(std::ostream& out) const {
	out << " - Fission Reaction" << endl;
	Log::printLine(out,"*");
	out << endl;
	/* Print the cosine and energy sampler */
	GenericReaction::print(out);
}

void ChanceFission::print(std::ostream& out) const {
	out << " - Fission Reaction (chance)" << endl;
	vector<Reaction*> reactions = chance_sampler->getReactions();
	for(vector<Reaction*>::const_iterator it = reactions.begin() ; it != reactions.end() ; ++it)
		(*it)->print(out);
}

Fission::~Fission() {/* */}

DelayedFission::DelayedFission(const std::vector<Ace::DLYBlock::BasicData>& delayed_data,
		const std::vector<Ace::EnergyDistribution>& energy_dist, const Ace::NeutronReaction& ace_reaction) : Reaction(18) {
	EnergySamplerFactory sampler_factory(0);
	/* Create probability container */
	for(size_t i = 0 ; i < delayed_data.size() ; ++i)
		probs.push_back(PrecursorProbability(delayed_data[i]));
	/* Create energy sampler container */
	for(size_t i = 0 ; i < energy_dist.size() ; ++i)
		energy_sampler.push_back(sampler_factory.createLaw(energy_dist[i].laws[0], ace_reaction));
	/* Sanity check */
	assert(probs.size() == energy_sampler.size());
}

void DelayedFission::print(std::ostream& out) const {
	out << "Probability Table (delayed data) " << endl;
	for(size_t i = 0 ; i < probs.size() ; ++i) {
		out << "Group = " << i << " : ";
		for(size_t j = 0 ; j < probs[i].energy.size() ; ++j)
			out << scientific << probs[i].energy[j] << " " << probs[i].prob[j] << " ; ";
		out << endl;
	}
}

DelayedFission::~DelayedFission() {
	for(size_t i = 0 ; i < energy_sampler.size() ; ++i)
		delete energy_sampler[i];
}
} /* namespace Helios */
