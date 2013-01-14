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

#include "AceIsotope.hpp"
#include "AceReader/ACEReader.hpp"
#include "AceReader/NeutronTable.hpp"
#include "AceReader/Conf.hpp"
#include "AceReaction/AceReactionBase.hpp"
#include "AceReaction/FissionReaction.hpp"
#include "../../Common/XsSampler.hpp"
#include "../../Environment/McEnvironment.hpp"

using namespace std;
using namespace Ace;

namespace Helios {

AceIsotopeBase* AceIsotopeFactory::createIsotope(const Ace::NeutronTable& table) const {
	/* Create child grid */
	const ChildGrid* child_grid = master_grid->pushGrid(table.getEnergyGrid().begin(), table.getEnergyGrid().end());
	/* Create isotope */
	return new AceIsotopeBase(table, child_grid);
}

double AceIsotopeBase::energy_freegas_threshold = 400.0; /* By default, 400.0 kT*/
double AceIsotopeBase::awr_freegas_threshold = 1.0;      /* By default, only H */

/* Create NU sampler based on the information on the ACE data */
static AceReaction::NuSampler* buildNuSampler(const Ace::NUBlock::NuData* nu_data) {
	typedef Ace::NUBlock AceNu;
	/* Get type */
	int type = nu_data->getType();
	/* Polynomial type */
	if(type == AceNu::flag_pol)
		return new AceReaction::PolynomialNu(dynamic_cast<const AceNu::Polynomial*>(nu_data));
	/* Tabular type */
	return new AceReaction::TabularNu(dynamic_cast<const AceNu::Tabular*>(nu_data));
}

void AceIsotopeBase::setFissionReaction(const Ace::NeutronTable& _table) {

	/* Fission MT for get the NU sampler */
	int fission_mt(18);

	/* Check if the isotope contains a fission cross section */
	if(reactions.check_all("18")) {
		/* We just got one fission reaction */
		fissile = true;
		/* Get fission cross section */
		fission_xs = reactions.get_xs(18);
		/* Check size */
		assert(fission_xs.size() == total_xs.size());
		/* Set fission cross section */
		fission_reaction = getReaction(18);

	} else if(reactions.check_all("19-21,38")) {

		/* We got chance fission reactions */
		fissile = true;
		/* Get fission cross section */
		fission_xs = reactions.get_xs(18);
		/* Check size */
		assert(fission_xs.size() == total_xs.size());

		/* MT of chance fission reaction */
		int mts[4] = {19, 20, 21, 38};
		vector<int> chance_mts(mts, mts + 4);

		/* Array for the secondary particle reaction sampler */
		vector<pair<Reaction*,const CrossSection*> > chance_array;

		/* Push chance reactions */
		for(vector<int>::const_iterator it = chance_mts.begin() ; it != chance_mts.end() ; ++it) {
			/* Get MT */
			int mt = (*it);

			/* Get the reaction from the container */
			ReactionContainer::const_iterator it_reaction = reactions.get_mt(mt);
			if(it_reaction != reactions.end()) {
				/* Reaction exist */
				const NeutronReaction& ace_reaction = (*it_reaction);
				/* Push reaction */
				chance_array.push_back( make_pair(getReaction(mt), &(ace_reaction.getXs()) ));
			}
		}

		/* Set fission cross section */
		fission_reaction = new AceReaction::ChanceFission(chance_array, fission_xs, child_grid);

		/* Push reaction into local map (since is not created in a "legal" way) */
		reaction_map[18] = fission_reaction;

		/* Put fission MT */
		fission_mt = (*chance_array.begin()).first->getId();
	}

	/* Set the fission probability (in case the information is available) */
	if(fissile) {
		/* Get fission reaction */
		const Ace::NeutronReaction& ace_reaction = (*reactions.get_mt(fission_mt));

		/* Get distribution of emerging particles from the NU-block */
		const Ace::NUBlock* nu_block = _table.block<NUBlock>();

		/* Sanity check */
		if(not nu_block) {
			/* Print warning */
			Log::warn() << left << "No information on NU block for fission reaction with mt = " << fission_mt
						<< " for isotope " << getUserId() << Log::endl;

			/* Set the isotope as non-fissile */
			fissile = false;
			return;
		} else {
			/* Get the NU data related to this fission reaction */
			vector<Ace::NUBlock::NuData*> nu_data = nu_block->clone();

			/* TODO - For now just sample particles with prompt spectrum */
			if(nu_data.size() >= 2)
				total_nu = buildNuSampler(nu_data[1]);
			else if(nu_data.size() == 1)
				total_nu = buildNuSampler(nu_data[0]);
			else
				throw(AceModule::AceError(getUserId(), "Cannot create reaction for mt = " + toString(ace_reaction.getMt()) +
					" : Information in NU block is not available" ));
		}
	}
}

AceIsotopeBase::AceIsotopeBase(const Ace::NeutronTable& _table, const ChildGrid* _child_grid) : Isotope(_table.getReactions().name()),
	reactions(_table.getReactions()), aweight(reactions.awr()), temperature(reactions.temp()), child_grid(_child_grid),
	fission_reaction(0), total_nu(0), secondary_sampler(0) {

	/* Total microscopic cross section of this isotope */
	total_xs = reactions.get_xs(1);

	/* Elastic cross section */
	elastic_xs = reactions.get_xs(2);
	/* Set elastic reaction */
	elastic_scattering = getReaction(2);

	/* Set fission stuff */
	setFissionReaction(_table);

	/* Set the absorption cross section */
	absorption_xs = reactions.get_xs(27);
	/* Check size */
	if(absorption_xs.size() != 0) {
		if(absorption_xs.size() != total_xs.size())
			throw(AceModule::AceError(getUserId(), "Absorption and total cross section don't have the same size"));
	} else {
		/* The only case when this happens is with 1002 isotope */
		absorption_xs = CrossSection(total_xs.size());
	}

	/* 	Calculate inelastic cross section */
	inelastic_xs = total_xs - absorption_xs - elastic_xs;

	/* Array for the secondary particle reaction sampler */
	vector<pair<Reaction*,const CrossSection*> > reaction_array;

	/* Loop over the scattering reactions (skipping fission) */
	for(Ace::ReactionContainer::const_iterator it = reactions.begin() ; it != reactions.end() ; ++it) {
		/* Get angular distribution type */
		int angular_data = (*it).getAngular().getKind();

		/* If the reaction does not contains angular data, we reach the end of "secondary" particle reactions */
		if(angular_data == Ace::AngularDistribution::no_data) break;

		/* Get MT of the reaction */
		int mt = (*it).getMt();

		/* We shouldn't include elastic and fission here */
		if((mt < 18 || mt > 21) && mt != 38 && mt != 2) {
				/* Create reaction */
				reaction_array.push_back(make_pair(getReaction(mt), &(*it).getXs()));
		}
	}

	/* Create the sampler */
	if(reaction_array.size() > 0)
		secondary_sampler = new XsSampler<Reaction*>(reaction_array);

}

/* Auxiliary function to get the probability of a reaction */
double AceIsotopeBase::getProb(Energy& energy, const Ace::CrossSection& xs) const {
	double factor;
	size_t idx = child_grid->index(energy,factor);
	double prob = factor * (xs[idx + 1] - xs[idx]) + xs[idx];
	double total = factor * (total_xs[idx + 1] - total_xs[idx]) + total_xs[idx];
	return prob / total;
}

double AceIsotopeBase::getAbsorptionProb(Energy& energy) const {
	return getProb(energy, absorption_xs);
}

double AceIsotopeBase::getFissionProb(Energy& energy) const {
	return getProb(energy, fission_xs);
}

double AceIsotopeBase::getElasticProb(Energy& energy) const {
	return getProb(energy, elastic_xs);
}

double AceIsotopeBase::getTotalXs(Energy& energy) const {
	double factor;
	size_t idx = child_grid->index(energy, factor);
	double total = factor * (total_xs[idx + 1] - total_xs[idx]) + total_xs[idx];
	return total;
}

double AceIsotopeBase::getFissionXs(Energy& energy) const {
	double factor;
	size_t idx = child_grid->index(energy, factor);
	double fission = factor * (fission_xs[idx + 1] - fission_xs[idx]) + fission_xs[idx];
	return fission;
}

Reaction* AceIsotopeBase::inelastic(Energy& energy, Random& random) const {
	/*
	 * Check if there is a sampler. This method shouldn't be executed when
	 * there aren't non-elastic reactions, but who knows...
	 */
	if(!secondary_sampler) return elastic_scattering;
	/* Get inelastic reaction from the sampler */
	double factor;
	size_t idx = child_grid->index(energy, factor);
	double inel = factor * (inelastic_xs[idx + 1] - inelastic_xs[idx]) + inelastic_xs[idx];
	return secondary_sampler->sample(idx, inel * random.uniform(), factor);
};

Reaction* AceIsotopeBase::getReaction(InternalId mt) {
	/* Static instance of the reaction factory */
	static AceReaction::AceReactionFactory reaction_factory;

	/* Check on local map */
	map<int,Reaction*>::const_iterator rea = reaction_map.find(mt);
	if(rea != reaction_map.end())
		return (*rea).second;

	/* Get the reaction from the container */
	ReactionContainer::const_iterator it_reaction = reactions.get_mt(mt);

	if(it_reaction != reactions.end()) {
		/* Reaction exist */
		const NeutronReaction& ace_reaction = (*it_reaction);
		/* Return reaction */
		Reaction* reaction = reaction_factory.createReaction(this, ace_reaction);
		/* Push reaction into local map */
		reaction_map[mt] = reaction;
		/* And return it... */
		return reaction;
	} else {
		/* Reaction can't be found */
		throw(AceModule::AceError(reactions.name(),"Reaction mt = " + toString(mt) + " does not exist"));
	}

}

void AceIsotopeBase::print(std::ostream& out) const {
	out << "isotope = " <<  setw(9) << reactions.name()
		<< " ; awr = " << setw(9) << aweight << " ; temperature = " << temperature / Constant::boltz << " K ";
}

AceIsotopeBase::~AceIsotopeBase() {
	/* Delete NU-sampler */
	delete total_nu;
	/* Delete reactions */
	for(map<int,Reaction*>::const_iterator it = reaction_map.begin() ; it != reaction_map.end() ; ++it)
		delete (*it).second;
	/* Delete sampler */
	delete secondary_sampler;
};

}


