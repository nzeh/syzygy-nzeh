#ifndef SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_INFERENCE_H_
#define SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_INFERENCE_H_

#include <syzygy/refinery/prob_mem_analyzer/factor_graph.h>

namespace refinery {
    namespace ProbMemAnalysis {

        void inferProbabilities(FactorGraphPtr);
    }
}

#endif // SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_INFERENCE_H_