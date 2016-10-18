#include <syzygy/refinery/prob_mem_analyzer/factor_graph_builder.h>

namespace refinery {
    namespace ProbMemAnalysis {

        size_t HypothesisHash::operator()(const FactorGraph::HypothesisPtr &hypothesis) const {
            auto type_hypothesis = hypothesis->castToTypeHypothesis();
            if (type_hypothesis) {
                size_t hash = 17;
                hash = hash * 31 + addressHash(type_hypothesis->addressRange().start());
                hash = hash * 31 + sizeHash(type_hypothesis->addressRange().size());
                hash = hash * 31 + typeHash(type_hypothesis->type());
                return hash;
            }
            else {
                return ptrHash(hypothesis.get());
            }
        }

        bool HypothesisEqual::operator()(
            const FactorGraph::HypothesisPtr &a, const FactorGraph::HypothesisPtr &b) const {
            return (*a == *b);
        }

        size_t FactorHash::operator()(const FactorGraph::FactorPtr &factor) const {
            size_t hash = 17;
            for (auto hypothesis : factor->neighbours())
                hash = hash * 31 + ptrHash(hypothesis->otherEndpoint(factor.get()));
            return hash;
        }

        bool FactorEqual::operator()(
            const FactorGraph::FactorPtr &a, const FactorGraph::FactorPtr &b) const {
            return (*a == *b);
        }

        void FactorGraphBuilder::initialize() {
            _graph = std::make_shared<FactorGraph>();
            _hypotheses.clear();
            _factors.clear();
        }

        bool FactorGraphBuilder::addHypothesis(FactorGraph::HypothesisPtr *hypothesis) {
            auto result = _hypotheses.insert(*hypothesis);
            *hypothesis = *result.first;
            if (result.second)
                _graph->addVertex(*hypothesis);
            return result.second;
        }

        bool FactorGraphBuilder::addFactor(FactorGraph::FactorPtr *factor,
            const FactorGraph::VertexPtrs &neighbours) {
            auto result = _factors.insert(*factor);
            *factor = *result.first;
            if (result.second) {
                _graph->addVertex(*factor);
                for (auto neighbour : neighbours) {
                    auto edge = std::make_shared<FactorGraph::Edge>(factor->get(),
                        neighbour.get());
                    _graph->addEdge(edge);
                }
            }
            return result.second;
        }

        FactorGraphPtr FactorGraphBuilder::getGraph() {
            return _graph;
        }
    }
}