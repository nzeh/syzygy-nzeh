#ifndef SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_PROB_MEM_ANALYZER_H_
#define SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_PROB_MEM_ANALYZER_H_

#include <syzygy/refinery/process_state/process_state.h>
#include <syzygy/refinery/prob_mem_analyzer/markov_network.h>

namespace refinery {
    namespace ProbMemAnalysis {
        bool runProbMemAnalysis(scoped_refptr<TypeRepository>, ProcessState &,
            MarkovNetwork::NetworkPtr *);
    }
}

#endif // SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_PROB_MEM_ANALYZER_H_