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

#ifndef PARTICLESOURCE_HPP_
#define PARTICLESOURCE_HPP_

#include <vector>

#include "../Common/Common.hpp"
#include "../Common/Sampler.hpp"
#include "SourceDefinition.hpp"
#include "Distribution/Distribution.hpp"

namespace Helios {

	/* Sampling a particle (position, energy and angle) */
	class ParticleSampler {
		/* ID defined by the user for this sampler */
		SamplerId samplerid;
		/* Reference position of the sampler */
		Coordinate position;
		/* Reference direction of the sampler */
		Direction direction;
		/* Energy of the initial particle (1 MeV by default) */
		double energy;
		/* Initial weight */
		double weight;
		/* Initial state of the particle */
		Particle::State state;

		/* Sampling different stuff on phase space */
		std::vector<DistributionBase*> distributions;
	public:

		class Definition : public SourceDefinition {
			/* ID defined by the user for this sampler */
			SamplerId samplerid;
			/* Reference position of the sampler */
			Coordinate position;
			/* Reference direction of the sampler */
			Direction direction;
			/* Samplers IDs */
			std::vector<DistributionId> distributionIds;

			/* Sampling different stuff on phase space */
			std::vector<DistributionBase*> distributions;
		public:
			Definition(const SamplerId& samplerid, const Coordinate& position,
					   const Direction& direction, const std::vector<DistributionId>& distributionIds) :
					   SourceDefinition(SourceDefinition::SAMPLER), samplerid(samplerid),
					   position(position), direction(direction), distributionIds(distributionIds) {/* */}

			Direction getDirection() const {
				return direction;
			}

			std::vector<DistributionBase*> getDistributions() const {
				return distributions;
			}

			void setDistributions(std::vector<DistributionBase*> distributions) {
				this->distributions = distributions;
			}

			Coordinate getPosition() const {
				return position;
			}

			SamplerId getSamplerid() const {
				return samplerid;
			}

			std::vector<DistributionId> getDistributionIds() const {
				return distributionIds;
			}

			~Definition() {/* */}
		};

		/* Exception */
		class BadSamplerCreation : public std::exception {
			std::string reason;
		public:
			BadSamplerCreation(const SamplerId& distid, const std::string& msg) {
				reason = "Cannot create sampler " + toString(distid) + " : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadSamplerCreation() throw() {/* */};
		};

		ParticleSampler(const ParticleSampler::Definition* definition);

		/* Sample particle */
		void operator() (Particle& particle,Random& r) const {
			/* Set phase space coordinates of this sampler */
			particle.pos() = position;
			particle.dir() = direction;
			particle.evs() = energy;
			particle.eix() = 0;
			particle.wgt() = weight;
			particle.sta() = state;
			/* Apply distributions (if any) */
			for(std::vector<DistributionBase*>::const_iterator it = distributions.begin() ; it != distributions.end() ; ++it)
				(*(*it))(particle,r);
		}

		virtual ~ParticleSampler(){/* */};
	};

	class ParticleSource {

	public:

		class Definition : public SourceDefinition {
			/* Samplers IDs */
			std::vector<SamplerId> samplersIds;
			/* Weights of each sampler */
			std::vector<double> weights;
			/* Strength of the source on the problem */
			double strength;

			/* Samplers */
			std::vector<ParticleSampler*> samplers;
		public:
			Definition(const std::vector<SamplerId>& samplersIds, const std::vector<double>& weights, const double& strength) :
				SourceDefinition(SourceDefinition::SOURCE), samplersIds(samplersIds), weights(weights), strength(strength) {
				/* Check the weight input */
				if(this->weights.size() == 0) {
					this->weights.resize(this->samplersIds.size());
					/* Equal probability for all samplers */
					double prob = 1/(double)this->samplersIds.size();
					for(size_t i = 0 ; i < this->samplersIds.size() ; ++i)
						this->weights[i] = prob;
				}
			}

			std::vector<ParticleSampler*> getSamplers() const {
				return samplers;
			}

			void setSamplers(std::vector<ParticleSampler*> samplers) {
				this->samplers = samplers;
			}

			std::vector<SamplerId> getSamplersIds() const {
				return samplersIds;
			}

			std::vector<double> getWeights() const {
				return weights;
			}
			double getStrength() const {
				return strength;
			}
		};

		/* Exception */
		class BadSourceCreation : public std::exception {
			std::string reason;
		public:
			BadSourceCreation(const std::string& msg) {
				reason = "Cannot create source : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadSourceCreation() throw() {/* */};
		};

		/* Create the source */
		ParticleSource(const ParticleSource::Definition* definition);

		/* Sample a particle */
		Particle sample(Random& r) const {
			Particle particle;
			ParticleSampler* sampler = source_sampler->sample(0,r.uniform());
			(*sampler)(particle,r);
			return particle;
		}

		/* Sample a particle (override) */
		void sample(Particle& particle, Random& r) const {
			ParticleSampler* sampler = source_sampler->sample(0,r.uniform());
			(*sampler)(particle,r);
		}

		/* Get strength */
		double getStrength() const {
			return strength;
		}

		virtual ~ParticleSource() {delete source_sampler;};

	private:

		/* Sampler of ParticleSampler(s) */
		Sampler<ParticleSampler*>* source_sampler;
		/* Strength of this source */
		double strength;
	};

} /* namespace Helios */
#endif /* PARTICLESOURCE_HPP_ */