#ifndef SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_MARKOV_NETWORK_H_
#define SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_MARKOV_NETWORK_H_

#include <syzygy/refinery/core/address.h>
#include <syzygy/refinery/types/type.h>

namespace refinery {
    namespace ProbMemAnalysis {
        namespace MarkovNetwork {

            // A hypothesis associates a putative type with a given address range
            // and stores a probability that this association is correct.
            class Hypothesis {

            public:
                Hypothesis(AddressRange, TypeId);
                Hypothesis(const Hypothesis &);
                Hypothesis &operator=(const Hypothesis &);

                AddressRange addressRange() const;
                TypeId       type()         const;

                void   setProbability(double);
                double getProbability() const;

                bool operator==(const Hypothesis &) const;

            private:
                AddressRange _address_range;
                TypeId       _type;
                double       _probability;
            };

            using HypothesisPtr = std::shared_ptr<Hypothesis>;
            using Hypotheses    = std::vector<HypothesisPtr>;

            class Factor {

            public:
                explicit Factor(Hypotheses &&);
                explicit Factor(const Hypotheses &);
                Factor(Factor &&);
                Factor(const Factor &);
                Factor &operator=(Factor &&);
                Factor &operator=(const Factor &);

                bool operator==(const Factor &) const;

                using Iterator = Hypotheses::iterator;
                Iterator begin();
                Iterator end();

            private:
                Hypotheses _hypotheses;
            };

            using FactorPtr = std::shared_ptr<Factor>;
            using Factors   = std::vector<FactorPtr>;

            class Network {

            public:
                Network() = default;
                Network(Network &&);
                Network(const Network &);

                void addHypothesis(HypothesisPtr);
                void addFactor(FactorPtr);

            private:
                Hypotheses _hypotheses;
                Factors    _factors;
            };

            using NetworkPtr = std::shared_ptr<Network>;
        }
    }
}

#endif // SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_MARKOV_NETWORK_H_