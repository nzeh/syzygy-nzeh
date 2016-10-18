#ifndef SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_BUILDER_H_
#define SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_BUILDER_H_

#include <unordered_set>
#include <syzygy/refinery/prob_mem_analyzer/factor_graph.h>

namespace refinery {
    namespace ProbMemAnalysis {

        class HypothesisHash {

        public:
            size_t operator()(const FactorGraph::HypothesisPtr &) const;

        private:
            std::hash<Address>                   addressHash;
            std::hash<Size>                      sizeHash;
            std::hash<TypeId>                    typeHash;
            std::hash<FactorGraph::Hypothesis *> ptrHash;
        };

        class HypothesisEqual {

        public:
            bool operator()(
                const FactorGraph::HypothesisPtr &, const FactorGraph::HypothesisPtr &) const;
        };

        class FactorHash {

        public:
            size_t operator()(const FactorGraph::FactorPtr &) const;

        private:
            std::hash<FactorGraph::Vertex *> ptrHash;
        };

        class FactorEqual {

        public:
            bool operator()(const FactorGraph::FactorPtr &, const FactorGraph::FactorPtr &) const;
        };

        class FactorGraphBuilder {

        public:
            void initialize();

            bool addHypothesis(FactorGraph::HypothesisPtr *);
            bool addFactor(FactorGraph::FactorPtr *, const FactorGraph::VertexPtrs &);

            FactorGraphPtr getGraph();

            using HypothesisCatalogue =
                std::unordered_set<FactorGraph::HypothesisPtr, HypothesisHash, HypothesisEqual>;
            using FactorCatalogue =
                std::unordered_set<FactorGraph::FactorPtr, FactorHash, FactorEqual>;

        private:
            FactorGraphPtr      _graph;
            HypothesisCatalogue _hypotheses;
            FactorCatalogue     _factors;
        };
    }
}

#endif // SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_BUILDER_H_