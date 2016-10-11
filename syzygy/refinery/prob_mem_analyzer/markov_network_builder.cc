#include <syzygy/refinery/prob_mem_analyzer/markov_network_builder.h>

namespace refinery {
    namespace ProbMemAnalysis {
        namespace MarkovNetwork {

            size_t HypothesisHash::operator()(const HypothesisPtr &hypothesis) const {
                size_t hash = 17;
                hash = hash * 31 + addressHash(hypothesis->addressRange().start());
                hash = hash * 31 + sizeHash(hypothesis->addressRange().size());
                hash = hash * 31 + typeHash(hypothesis->type());
                return hash;
            }

            bool HypothesisEqual::operator()(
                const HypothesisPtr &a, const HypothesisPtr &b) const {
                return (*a == *b);
            }

            size_t FactorHash::operator()(const FactorPtr &factor) const {
                size_t hash = 17;
                for (auto hypothesis : const_cast<Factor &>(*factor))
                    hash = hash * 31 + ptrHash(hypothesis.get());
                return hash;
            }

            bool FactorEqual::operator()(
                const FactorPtr &a, const FactorPtr &b) const {
                return (*a == *b);
            }

            void NetworkBuilder::initialize() {
                _network = std::make_shared<Network>();
                _hypotheses.clear();
                _factors.clear();
            }

            bool NetworkBuilder::addHypothesis(HypothesisPtr *hypothesis) {
                auto result = _hypotheses.insert(*hypothesis);
                *hypothesis = *result.first;
                return result.second;
            }

            bool NetworkBuilder::addFactor(FactorPtr *factor) {
                auto result = _factors.insert(*factor);
                *factor = *result.first;
                return result.second;
            }

            NetworkPtr NetworkBuilder::getNetwork() {
                return _network;
            }

            NetworkBuilder::HypothesisCatalogue &NetworkBuilder::hypotheses() {
                return _hypotheses;
            }

            NetworkBuilder::FactorCatalogue &NetworkBuilder::factors() {
                return _factors;
            }
        }
    }
}