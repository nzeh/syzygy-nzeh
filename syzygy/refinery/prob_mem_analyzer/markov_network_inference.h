#ifndef SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_MARKOV_NETWORK_INFERENCE_H_
#define SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_MARKOV_NETWORK_INFERENCE_H_

#include <syzygy/refinery/prob_mem_analyzer/markov_network.h>

namespace refinery {
    namespace ProbMemAnalysis {
        namespace MarkovNetwork {

            void inferProbabilities(NetworkPtr);
        }
    }
}

#endif // SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_MARKOV_NETWORK_INFERENCE_H_