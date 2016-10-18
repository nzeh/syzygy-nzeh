#include <syzygy/refinery/prob_mem_analyzer/factor_graph_inference.h>

namespace refinery {
    namespace ProbMemAnalysis {

        void resetMessages(const FactorGraphPtr &graph) {
            for (auto edge : graph->edges())
                edge->resetMessages();
        }

        void sendMessages(const FactorGraphPtr &graph) {
            for (auto vertex : graph->vertices())
                vertex->sendMessages();
        }

        bool propagateMessages(const FactorGraphPtr &graph) {
            auto converged = true;
            for (auto edge : graph->edges())
                converged |= edge->propagateMessages();
            return converged;
        }

        bool propagateBeliefs(const FactorGraphPtr &graph) {
            sendMessages(graph);
            return propagateMessages(graph);
        }

        void computeProbabilities(const FactorGraphPtr &graph) {
            for (auto vertex : graph->vertices())
                vertex->computeProbability();
        }

        void inferProbabilities(const FactorGraphPtr &graph) {
            resetMessages(graph);
            static const auto max_iterations = 1000U;
            static auto converged = false;
            for (auto i = 0U; !converged && i < max_iterations; ++i)
                converged = propagateBeliefs(graph);
            if (converged)
                computeProbabilities(graph);
        }
    }
}
