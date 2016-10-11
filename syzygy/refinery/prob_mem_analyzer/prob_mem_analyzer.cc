#include <syzygy/refinery/prob_mem_analyzer/markov_network_builder.h>
#include <syzygy/refinery/prob_mem_analyzer/markov_network_inference.h>
#include <syzygy/refinery/prob_mem_analyzer/prob_mem_analyzer.h>
#include <syzygy/refinery/types/typed_data.h>
#include <syzygy/refinery/types/type_repository.h>

namespace refinery {
    namespace ProbMemAnalysis {

        MarkovNetwork::HypothesisPtr destructureBlockIfNew(MarkovNetwork::NetworkBuilder &,
            const TypedData &);

        void destructureArrayBlock(MarkovNetwork::NetworkBuilder &builder,
            const TypedData &data, const MarkovNetwork::HypothesisPtr &hypothesis) {
            ArrayTypePtr array_type;
            if (data.type()->CastTo(&array_type)) {
                auto elem_type = array_type->GetElementType();
                auto num_elems = data.GetRange().size() / elem_type->size();
                for (auto i = 0; i < num_elems; ++i) {
                    TypedData elem;
                    if (data.GetArrayElement(i, &elem)) {
                        // TODO(nzeh): How does this handle fields that are VTABLE
                        // pointers?  How do we handle VTABLE pointers as indicators
                        // that what we're seeing here is really an object of the type
                        // we think is here?
                        auto elem_hypothesis = destructureBlockIfNew(builder, elem);
                        auto elem_factor = std::make_shared<MarkovNetwork::Factor>(
                            MarkovNetwork::Hypotheses({ hypothesis, elem_hypothesis }));
                        builder.addFactor(&elem_factor);
                    }
                    else {
                        // TODO(nzeh): Can this fail? Do I have to deal with it?
                    }
                }
            }
            else {
                // TODO(nzeh): Indicate failure somehow.  Can this fail?
            }
        }

        void destructureUdtBlock(MarkovNetwork::NetworkBuilder &builder,
            const TypedData &data, const MarkovNetwork::HypothesisPtr &hypothesis) {
            UserDefinedTypePtr udt_type;
            if (data.type()->CastTo(&udt_type)) {
                size_t num_fields;
                if (data.GetFieldCount(&num_fields)) {
                    for (auto i = 0; i < num_fields; ++i) {
                        TypedData field;
                        if (data.GetField(i, &field)) {
                            auto field_hypothesis = destructureBlockIfNew(builder, field);
                            auto field_factor = std::make_shared<MarkovNetwork::Factor>(
                                MarkovNetwork::Hypotheses({ hypothesis, field_hypothesis }));
                        }
                        else {
                            // TODO(nzeh): How do we deal with failure here?
                        }
                    }
                }
                else {
                    // TODO(nzeh): How can this fail at this point?
                }
            }
            else {
                // TODO(nzeh): indicate failure somehow.  Can this fail?
            }
        }

        void destructurePtrBlock(MarkovNetwork::NetworkBuilder &builder,
            const TypedData &data, const MarkovNetwork::HypothesisPtr &hypothesis) {
            TypedData referenced_data;
            if (data.Dereference(&referenced_data)) {
                auto target_hypothesis = destructureBlockIfNew(builder, referenced_data);
                auto pointer_factor = std::make_shared<MarkovNetwork::Factor>(
                    MarkovNetwork::Hypotheses({ hypothesis, target_hypothesis }));
                builder.addFactor(&pointer_factor);
                // TODO(nzeh): Add factor matrix
            }
        }

        MarkovNetwork::HypothesisPtr destructureBlockIfNew(MarkovNetwork::NetworkBuilder &builder,
            const TypedData &data) {
            auto range      = data.GetRange();
            auto type       = data.type()->type_id();
            auto hypothesis = std::make_shared<MarkovNetwork::Hypothesis>(range, type);
            if (builder.addHypothesis(&hypothesis)) {
                if (data.IsArrayType())
                    destructureArrayBlock(builder, data, hypothesis);
                else if (data.IsUserDefinedType())
                    destructureUdtBlock(builder, data, hypothesis);
                else if (data.IsPointerType())
                    destructurePtrBlock(builder, data, hypothesis);
            }
            return hypothesis;
        }

        bool buildDestructuringSubnetwork(scoped_refptr<TypeRepository> type_repository,
            ProcessState *process_state, MarkovNetwork::NetworkBuilder &builder) {
            TypedBlockLayerPtr typed_blocks;

            if (process_state->FindLayer(&typed_blocks)) {
                for (auto block : *typed_blocks) {
                    auto type = type_repository->GetType(block->data().type_id());
                    if (type == nullptr) {
                        // TODO(nzeh): Don't just fail wholesale when a type lookup fails
                        return false;
                    }
                    TypedData data(process_state, type, block->range().start());
                    destructureBlockIfNew(builder, data);
                }
                return true;
            }
            else {
                return false;
            }
        }

        void buildConflictSubgraph(MarkovNetwork::NetworkBuilder &builder) {
            auto hypotheses = builder.hypotheses();

            // TODO(nzeh): This needs to be replaced with something smarter than this n^2
            // exhaustive search.
            for (auto hyp1 = hypotheses.begin(), end = hypotheses.end(); hyp1 != end; ++hyp1)
                for (auto hyp2 = next(hyp1); hyp2 != end; ++hyp2)
                    if ((*hyp1)->addressRange().Intersects((*hyp2)->addressRange())) {
                        // TODO(nzeh): Implement this
                    }

        }

        bool buildNetwork(scoped_refptr<TypeRepository> type_repository,
            ProcessState *process_state, MarkovNetwork::NetworkPtr *network) {
            MarkovNetwork::NetworkBuilder builder;
            TypedBlockLayerPtr typed_blocks;

            if (buildDestructuringSubnetwork(type_repository, process_state, builder)) {
                buildConflictSubgraph(builder);
                *network = builder.getNetwork();
                return true;
            }
            else {
                return false;
            }
        }

        bool runProbMemAnalysis(scoped_refptr<TypeRepository> type_repository,
            ProcessState *process_state, MarkovNetwork::NetworkPtr *network) {
            if (buildNetwork(type_repository, process_state, network)) {
                MarkovNetwork::inferProbabilities(*network);
                return true;
            }
            else {
                return false;
            }
        }
    }
}