#include <syzygy/refinery/prob_mem_analyzer/markov_network.h>

namespace refinery {
    namespace ProbMemAnalysis {
        namespace MarkovNetwork {

            Hypothesis::Hypothesis(AddressRange address_range, TypeId type) :
                _address_range(address_range), _type(type), _probability(0) {}

            Hypothesis::Hypothesis(const Hypothesis &hypothesis) :
                _address_range(hypothesis._address_range), _type(hypothesis._type),
                _probability(hypothesis._probability) {}

            Hypothesis &Hypothesis::operator=(const Hypothesis &hypothesis) {
                _address_range = hypothesis._address_range;
                _type          = hypothesis._type;
                _probability   = hypothesis._probability;
                return *this;
            }

            AddressRange Hypothesis::addressRange() const {
                return _address_range;
            }

            TypeId Hypothesis::type() const {
                return _type;
            }

            void Hypothesis::setProbability(double probability) {
                _probability = probability;
            }

            double Hypothesis::getProbability() const {
                return _probability;
            }

            bool Hypothesis::operator==(const Hypothesis &other) const {
                return (_address_range == other._address_range && _type == other._type);
            }

            Factor::Factor(Hypotheses &&hypotheses)
                : _hypotheses(std::move(hypotheses)) {}

            Factor::Factor(const Hypotheses &hypotheses)
                : _hypotheses(hypotheses) {}

            Factor::Factor(Factor &&factor) :
                _hypotheses(std::move(factor._hypotheses)) {}

            Factor::Factor(const Factor &factor) :
                _hypotheses(factor._hypotheses) {}

            Factor &Factor::operator=(Factor &&factor) {
                _hypotheses = std::move(factor._hypotheses);
                return *this;
            }

            Factor &Factor::operator=(const Factor &factor) {
                _hypotheses = factor._hypotheses;
                return *this;
            }

            bool Factor::operator==(const Factor &other) const {
                return (_hypotheses == other._hypotheses);
            }

            Factor::Iterator Factor::begin() {
                return _hypotheses.begin();
            }

            Factor::Iterator Factor::end() {
                return _hypotheses.end();
            }

            Network::Network(Network &&network) :
                _hypotheses(std::move(network._hypotheses)),
                _factors(std::move(network._factors)) {}

            Network::Network(const Network &network) :
                _hypotheses(network._hypotheses), _factors(network._factors) {}

            void Network::addHypothesis(HypothesisPtr hypothesis) {
                _hypotheses.push_back(hypothesis);
            }

            void Network::addFactor(FactorPtr factor) {
                _factors.push_back(factor);
            }
        }
    }
}