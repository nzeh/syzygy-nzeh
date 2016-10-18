#ifndef SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_BUILDER_H_
#define SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_BUILDER_H_

#include <unordered_set>
#include <syzygy/refinery/prob_mem_analyzer/factor_graph.h>

namespace refinery {
    namespace ProbMemAnalysis {

        // A hash function and equality operator for hypothesis pointers, needed to store them
        // in a hash table of hypotheses already in the graph.
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

        // A hash function and equality operator for factor pointers, needed to store them in
        // a hash table of factors already in the graph.
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

        // Helper class for building a FactorGraph.  Makes sure every hypothesis an factor is
        // added only once.
        class FactorGraphBuilder {

        public:
            // Initialize the builder with an empty graph.
            void initialize();

            // Functions to add hypotheses and factors to the graph.  They return true if the
            // addition actually took place, that is, the hypothesis or factor didn't already
            // exist.
            bool addHypothesis(FactorGraph::HypothesisPtr *);
            bool addFactor(FactorGraph::FactorPtr *, const FactorGraph::VertexPtrs &);

            // Get the constructed graph.
            FactorGraphPtr getGraph();

        private:
            using HypothesisCatalogue =
                std::unordered_set<FactorGraph::HypothesisPtr, HypothesisHash, HypothesisEqual>;

            using FactorCatalogue =
                std::unordered_set<FactorGraph::FactorPtr, FactorHash, FactorEqual>;

            FactorGraphPtr      _graph;
            HypothesisCatalogue _hypotheses;
            FactorCatalogue     _factors;
        };
    }
}

#endif // SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_BUILDER_H_