#include <syzygy/refinery/prob_mem_analyzer/factor_graph.h>

namespace refinery {
    namespace ProbMemAnalysis {

        void FactorGraph::addVertex(const VertexPtr &vertex) {
            _vertices.push_back(vertex);
        }

        const FactorGraph::Vertices &FactorGraph::Vertex::neighbours() const {
            return _neighbours;
        }

        FactorGraph::Vertex::Vertex(Vertices &&neighbours)
            : _neighbours(std::move(neighbours)) {}

        FactorGraph::Vertex::Vertex(const Vertices &neighbours)
            : _neighbours(neighbours) {}

        void FactorGraph::Vertex::addNeighbour(const VertexPtr &vertex) {
            _neighbours.push_back(vertex);
        }

        FactorGraph::Hypothesis::Hypothesis(Kind kind)
            : _kind(kind) {}

        void FactorGraph::Hypothesis::probability(double probability) {
            _probability = probability;
        }

        double FactorGraph::Hypothesis::probability() const {
            return _probability;
        }

        FactorGraph::TypeHypothesisPtr FactorGraph::Hypothesis::castToTypeHypothesis() {
            return TypeHypothesisPtr();
        }

        FactorGraph::TypeHypothesisConstPtr FactorGraph::Hypothesis::castToTypeHypothesis() const {
            return TypeHypothesisPtr();
        }

        FactorGraph::ObservationPtr FactorGraph::Hypothesis::castToObservation() {
            return ObservationPtr();
        }

        FactorGraph::ObservationConstPtr FactorGraph::Hypothesis::castToObservation() const {
            return ObservationPtr();
        }

        FactorGraph::Hypothesis::Kind FactorGraph::Hypothesis::kind() const {
            return _kind;
        }

        bool FactorGraph::Hypothesis::operator==(const FactorGraph::Hypothesis &other) {
            if (_kind != other._kind)
                return false;
            if (_kind == Kind::Observation)
                return (this == &other);
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

        FactorGraph::TypeHypothesisPtr FactorGraph::TypeHypothesis::castToTypeHypothesis() {
            return shared_from_this();
        }

        FactorGraph::TypeHypothesisConstPtr FactorGraph::TypeHypothesis::castToTypeHypothesis() const {
            return shared_from_this();
        }

        FactorGraph::DeclaredTypeHypothesis::DeclaredTypeHypothesis(
            const AddressRange &range, TypeId type)
            : TypeHypothesis(Kind::DeclaredTypeHypothesis, range, type) {}

        FactorGraph::ContentTypeHypothesis::ContentTypeHypothesis(
            const AddressRange &range, TypeId type)
            : TypeHypothesis(Kind::ContentTypeHypothesis, range, type) {}

        FactorGraph::ObservationPtr FactorGraph::Observation::castToObservation() {
            return shared_from_this();
        }

        FactorGraph::ObservationConstPtr FactorGraph::Observation::castToObservation() const {
            return shared_from_this();
        }

        WeightDistribution::WeightDistribution(std::vector<double> &&weights)
            : _weights(std::move(weights)) {}

        WeightDistribution::WeightDistribution(const std::vector<double> &weights)
            : _weights(weights) {}

        double WeightDistribution::weight(const std::vector<bool> &truth_assignment) const {
            auto delta = 1U;
            auto index = 0U;
            for (auto value : truth_assignment) {
                if (value)
                    index += delta;
                delta <<= 1;
            }
            return _weights[index];
        }

        FactorGraph::Factor::Factor(Vertices &&neighbours, WeightDistribution &&weights,
            Kind kind)
            : Vertex(std::move(neighbours)), _weights(std::move(weights)), _kind(kind) {
            for (auto vertex : neighbours)
                vertex->addNeighbour(shared_from_this());
        }

        FactorGraph::Factor::Factor(const Vertices &neighbours, const WeightDistribution &weights,
            Kind kind)
            : Vertex(neighbours), _weights(weights), _kind(kind) {
            for (auto vertex : neighbours)
                vertex->addNeighbour(shared_from_this());
        }

        FactorGraph::Factor::Kind FactorGraph::Factor::kind() const {
            return _kind;
        }

        bool FactorGraph::Factor::operator==(const Factor &other) {
            return (_kind == other._kind && _neighbours == other._neighbours);
        }

        FactorGraph::DecompositionFactor::DecompositionFactor(
            Vertices &&neighbours,
            WeightDistribution &&weights)
            : Factor(std::move(neighbours), std::move(weights), Kind::DecompositionFactor) {}

        FactorGraph::DecompositionFactor::DecompositionFactor(
            const Vertices &neighbours,
            const WeightDistribution &weights)
            : Factor(neighbours, weights, Kind::DecompositionFactor) {}

        FactorGraph::PointerFactor::PointerFactor(
            Vertices &&neighbours,
            WeightDistribution &&weights)
            : Factor(std::move(neighbours), std::move(weights), Kind::PointerFactor) {}

        FactorGraph::PointerFactor::PointerFactor(
            const Vertices &neighbours,
            const WeightDistribution &weights)
            : Factor(neighbours, weights, Kind::PointerFactor) {}

        FactorGraph::ContentFactor::ContentFactor(
            Vertices &&neighbours,
            WeightDistribution &&weights)
            : Factor(std::move(neighbours), std::move(weights), Kind::ContentFactor) {}

        FactorGraph::ContentFactor::ContentFactor(
            const Vertices &neighbours,
            const WeightDistribution &weights)
            : Factor(neighbours, weights, Kind::ContentFactor) {}

        FactorGraph::DeclarationContentFactor::DeclarationContentFactor(
            Vertices &&neighbours,
            WeightDistribution &&weights)
            : Factor(std::move(neighbours), std::move(weights), Kind::ContentFactor) {}

        FactorGraph::DeclarationContentFactor::DeclarationContentFactor(
            const Vertices &neighbours,
            const WeightDistribution &weights)
            : Factor(neighbours, weights, Kind::ContentFactor) {}
    }
}