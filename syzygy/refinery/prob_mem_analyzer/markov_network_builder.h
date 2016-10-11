#ifndef SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_MARKOV_NETWORK_BUILDER_H_
#define SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_MARKOV_NETWORK_BUILDER_H_

#include <unordered_set>
#include <syzygy/refinery/prob_mem_analyzer/markov_network.h>

namespace refinery {
    namespace ProbMemAnalysis {
        namespace MarkovNetwork {

            class HypothesisHash {

            public:
                size_t operator()(const HypothesisPtr &) const;

            private:
                std::hash<Address> addressHash;
                std::hash<Size>    sizeHash;
                std::hash<TypeId>  typeHash;
            };

            class HypothesisEqual {

            public:
                bool operator()(const HypothesisPtr &, const HypothesisPtr &) const;
            };

            class FactorHash {

            public:
                size_t operator()(const FactorPtr &) const;

            private:
                std::hash<Hypothesis *> ptrHash;
            };

            class FactorEqual {

            public:
                bool operator()(const FactorPtr &, const FactorPtr &) const;
            };

            class NetworkBuilder {

            public:
                void initialize();

                bool addHypothesis(HypothesisPtr *);
                bool addFactor(FactorPtr *);

                NetworkPtr getNetwork();

                using HypothesisCatalogue =
                    std::unordered_set<HypothesisPtr, HypothesisHash, HypothesisEqual>;
                using FactorCatalogue =
                    std::unordered_set<FactorPtr, FactorHash, FactorEqual>;

                HypothesisCatalogue &hypotheses();
                FactorCatalogue &factors();

            private:
                NetworkPtr          _network;
                HypothesisCatalogue _hypotheses;
                FactorCatalogue     _factors;
            };
        }
    }
}

#endif // SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_MARKOV_NETWORK_BUILDER_H_