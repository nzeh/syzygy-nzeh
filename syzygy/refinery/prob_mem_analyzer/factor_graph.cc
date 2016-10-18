#include <syzygy/refinery/prob_mem_analyzer/factor_graph.h>

namespace refinery {
    namespace ProbMemAnalysis {

        void FactorGraph::addVertex(const VertexPtr &vertex) {
            _vertices.push_back(vertex);
        }

        void FactorGraph::addEdge(const EdgePtr &edge) {
            _edges.push_back(edge);
            edge->connectToEndpoints();
        }

        const FactorGraph::VertexPtrs &FactorGraph::vertices() {
            return _vertices;
        }

        const FactorGraph::EdgePtrs &FactorGraph::edges() {
            return _edges;
        }

        const FactorGraph::EdgeCPtrs &FactorGraph::Vertex::neighbours() const {
            return _neighbours;
        }

        void FactorGraph::Vertex::addNeighbour(Edge *edge) {
            _neighbours.push_back(edge);
        }

        FactorGraph::Edge::Edge(Vertex *x, Vertex *y)
            : _endpoints({ x, y }),
            _out_messages({ {{0.5, 0.5}, {0.5, 0.5}} }),
            _in_messages({ {{0.5, 0.5}, {0.5, 0.5}} }) {}

        void FactorGraph::Edge::sendMessage(Vertex *source, const Message &message) {
            _in_messages[source == _endpoints[0] ? 0 : 1] = message;
        }

        const FactorGraph::Edge::Message &FactorGraph::Edge::receiveMessage(Vertex *target) {
            return _out_messages[target == _endpoints[0] ? 1 : 0];
        }

        void FactorGraph::Edge::resetMessages() {
            _out_messages = _in_messages = { {{0.5, 0.5}, {0.5, 0.5}} };
        }

        bool FactorGraph::Edge::propagateMessages() {
            auto converged = true;
            for (auto i = 0; i < 2; ++i)
                for (auto j = 0; j < 2; ++j) {
                    auto ratio = _out_messages[i][j] / _in_messages[i][j];
                    if (ratio < 0.99 || ratio > 1.01)
                        converged = false;
                }
            _out_messages = _in_messages;
            return converged;
        }

        FactorGraph::Vertex *FactorGraph::Edge::otherEndpoint(const Vertex *vertex) const {
            return (vertex == _endpoints[0] ? _endpoints[1] : _endpoints[0]);
        }

        void FactorGraph::Edge::connectToEndpoints() {
            _endpoints[0]->addNeighbour(this);
            _endpoints[1]->addNeighbour(this);
        }

        FactorGraph::Hypothesis::Hypothesis(Kind kind)
            : _kind(kind) {}

        double FactorGraph::Hypothesis::probability() const {
            return _probability;
        }

        FactorGraph::TypeHypothesis *FactorGraph::Hypothesis::castToTypeHypothesis() {
            return nullptr;
        }

        const FactorGraph::TypeHypothesis *FactorGraph::Hypothesis::castToTypeHypothesis() const {
            return nullptr;
        }

        FactorGraph::Hypothesis::Kind FactorGraph::Hypothesis::kind() const {
            return _kind;
        }

        void FactorGraph::Hypothesis::sendMessages() {
            for (auto edge : neighbours()) {
                Edge::Message prod = { 1, 1 };
                for (auto factor : neighbours())
                    if (factor != edge) {
                        auto msg = factor->receiveMessage(this);
                        prod[0] *= msg[0];
                        prod[1] *= msg[1];
                    }
                // We normalize messages so they don't drift off to 0 or infinity.  Logically,
                // this should not make a difference, but numerically and as far as convergence
                // is concerned, it may.
                auto total = prod[0] + prod[1];
                prod[0] /= total;
                prod[1] /= total;
                edge->sendMessage(this, prod);
            }
        }

        void FactorGraph::Hypothesis::computeProbability() {
            Edge::Message prod = { 1, 1 };
            for (auto edge : neighbours()) {
                auto msg = edge->receiveMessage(this);
                prod[0] *= msg[0];
                prod[1] *= msg[1];
            }
            _probability = prod[1] / (prod[0] + prod[1]);
        }

        bool FactorGraph::Hypothesis::operator==(const FactorGraph::Hypothesis &other) {

            // Hypotheses of different kinds are not equal
            if (_kind != other._kind)
                return false;

            // Observations (facts) are equal only if they are the same observation
            if (_kind == Kind::Observation)
                return (this == &other);

            // Type hypotheses of the same kind are equal if they refer to the same address range
            // and type
            auto type_hypothesis = castToTypeHypothesis();
            auto other_type_hypothesis = other.castToTypeHypothesis();
            return (
                type_hypothesis->addressRange() == other_type_hypothesis->addressRange() &&
                type_hypothesis->type() == other_type_hypothesis->type());
        }

        FactorGraph::TypeHypothesis::TypeHypothesis(Kind kind, const AddressRange &range, TypeId type)
            : Hypothesis(kind), _address_range(range), _type(type) {}

        const AddressRange &FactorGraph::TypeHypothesis::addressRange() const {
            return _address_range;
        }

        TypeId FactorGraph::TypeHypothesis::type() const {
            return _type;
        }

        FactorGraph::TypeHypothesis *FactorGraph::TypeHypothesis::castToTypeHypothesis() {
            return this;
        }

        const FactorGraph::TypeHypothesis *FactorGraph::TypeHypothesis::castToTypeHypothesis() const {
            return this;
        }

        FactorGraph::Factor::Factor(Kind kind, Weights &&weights)
            : _kind(kind), _weights(std::move(weights)) {}

        FactorGraph::Factor::Factor(Kind kind, const Weights &weights)
            : _kind(kind), _weights(weights) {}

        FactorGraph::Factor::Kind FactorGraph::Factor::kind() const {
            return _kind;
        }

        bool FactorGraph::Factor::operator==(const Factor &other) {

            // Factors of different kinds are different
            if (_kind != other._kind)
                return false;

            // Factors of the same kind are the same if they have exactly the same set of
            // neighbours.
            for (auto x = neighbours().begin(), y = other.neighbours().begin(),
                xend = neighbours().end(), yend = other.neighbours().end();
                x != xend || y != yend; ++x, ++y) {
                if (x == xend || y == yend)
                    return false;
                if ((*x)->otherEndpoint(this) != (*y)->otherEndpoint(&other))
                    return false;
            }
            return true;
        }

        FactorGraph::Edge::Message FactorGraph::Factor::summarizeMessages(unsigned int index) {

            // The index'th bit, the bits at position index and above, and the bits below position
            // index
            auto this_bit = 1U << index;
            auto upper_bits = std::numeric_limits<unsigned int>::max() << index;
            auto lower_bits = std::numeric_limits<unsigned int>::max() - upper_bits;

            Edge::Message msg;
            for (auto i = 0; i < 2; ++i) {

                // The value to be expected in the index'th bit position of the weight index for
                // this weight to be taken into account for message msg[i]
                auto this_bit_value = i * this_bit;

                msg[i] = 0;

                // msg[i] is a sum of _weights.size() / 2 weight-message products.
                for (auto j = 0U, count = _weights.size() / 2; j < count; ++j) {

                    // The index of the jth weight is obtained by shifting up the upper bits of
                    // j and splicing in this_bit_value between the upper and lower bits.
                    auto k = (j & upper_bits << 1) + this_bit_value + (j & lower_bits);

                    // Now multiply the kth weight with the messages corresponding to the values
                    // of its bits.
                    auto point_weight = _weights[k];
                    for (auto not_i = 0U, bit = 1U, degree = neighbours().size();
                        not_i < degree; ++not_i, bit <<= 1)
                        if (not_i != index) {
                            auto val = (k & bit ? 1 : 0);
                            point_weight *= neighbours()[not_i]->receiveMessage(this)[val];
                        }
                    msg[i] += point_weight;
                }
            }
            return msg;
        }

        void FactorGraph::Factor::sendMessages() {
            for (auto i = 0U, degree = neighbours().size(); i < degree; ++i)
                neighbours()[i]->sendMessage(this, summarizeMessages(i));
        }

        void FactorGraph::Factor::computeProbability() {}
    }
}
