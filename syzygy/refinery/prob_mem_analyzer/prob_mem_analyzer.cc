#include <syzygy/refinery/prob_mem_analyzer/factor_graph_builder.h>
#include <syzygy/refinery/prob_mem_analyzer/factor_graph_inference.h>
#include <syzygy/refinery/prob_mem_analyzer/prob_mem_analyzer.h>
#include <syzygy/refinery/types/typed_data.h>
#include <syzygy/refinery/types/type_repository.h>

namespace refinery {
    namespace ProbMemAnalysis {

        // Wraps a pair of hypotheses representing the declared type of the data in a given
        // address range and whether the data in this address range looks like an object of this
        // type.
        struct TypeHypothesisPair {
            FactorGraph::HypothesisPtr declared_type;
            FactorGraph::HypothesisPtr content_type;
        };

        TypeHypothesisPair destructureBlockIfNew(FactorGraphBuilder &, const TypedData &);

        // Takes an address range representing an array and takes it apart generating hypotheses
        // for its individual array elements.
        void destructureArrayBlock(FactorGraphBuilder &builder,
            const TypedData &data, const TypeHypothesisPair &array_hypothesis_pair) {
            ArrayTypePtr array_type;
            if (data.type()->CastTo(&array_type)) {
                auto elem_type = array_type->GetElementType();
                auto num_elems = data.GetRange().size() / elem_type->size();
                for (auto i = 0; i < num_elems; ++i) {
                    TypedData elem;
                    if (data.GetArrayElement(i, &elem)) {
                        auto elem_hypothesis_pair = destructureBlockIfNew(builder, elem);
                        if (array_hypothesis_pair.declared_type && elem_hypothesis_pair.declared_type) {
                            FactorGraph::FactorPtr declared_type_factor =
                                std::make_shared<FactorGraph::DecompositionFactor>(
                                    FactorGraph::Factor::Weights({ 0, 0, 0, 0 }));
                            builder.addFactor(&declared_type_factor, {
                                array_hypothesis_pair.declared_type,
                                elem_hypothesis_pair.declared_type
                            });
                        }
                        if (array_hypothesis_pair.content_type && elem_hypothesis_pair.content_type) {
                            FactorGraph::FactorPtr content_type_factor =
                                std::make_shared<FactorGraph::DecompositionFactor>(
                                    FactorGraph::Factor::Weights({ 0, 0, 0, 0 }));
                            builder.addFactor(&content_type_factor, {
                                array_hypothesis_pair.content_type,
                                elem_hypothesis_pair.content_type
                            });
                        }
                    }
                    else {
                        // TODO(nzeh): Can this fail? Do I have to deal with it?
                        // I think we should probably just quietly ignore this.
                    }
                }
            }
            else {
                // TODO(nzeh): Indicate failure somehow.  Can this fail?
                // Ditto.
            }
        }

        // Takes an address range representing an object of some UDT and takes it apart into
        // its individual fields.
        // NOTE: VTABLE pointers are ignored for now.
        void destructureUdtBlock(FactorGraphBuilder &builder,
            const TypedData &data, TypeHypothesisPair &udt_hypothesis_pair) {
            UserDefinedTypePtr udt_type;
            if (data.type()->CastTo(&udt_type)) {
                size_t num_fields;
                if (data.GetFieldCount(&num_fields)) {
                    for (auto i = 0; i < num_fields; ++i) {
                        FieldPtr field;
                        TypedData field_data;
                        if (data.GetField(i, &field) && data.GetField(i, &field_data) &&
                            field->kind() != UserDefinedType::Field::VFPTR_KIND) {
                            auto field_hypothesis_pair = destructureBlockIfNew(builder, field_data);
                            if (udt_hypothesis_pair.declared_type && field_hypothesis_pair.declared_type) {
                                FactorGraph::FactorPtr declared_type_factor =
                                    std::make_shared<FactorGraph::DecompositionFactor>(
                                        FactorGraph::Factor::Weights({ 0, 0, 0, 0 }));
                                builder.addFactor(&declared_type_factor, {
                                    udt_hypothesis_pair.declared_type,
                                    field_hypothesis_pair.declared_type
                                });
                            }
                            if (udt_hypothesis_pair.content_type && field_hypothesis_pair.content_type) {
                                FactorGraph::FactorPtr content_type_factor =
                                    std::make_shared<FactorGraph::DecompositionFactor>(
                                        FactorGraph::Factor::Weights({ 0, 0, 0, 0 }));
                                builder.addFactor(&content_type_factor, {
                                    udt_hypothesis_pair.content_type,
                                    field_hypothesis_pair.content_type
                                });
                            }
                        }
                        else {
                            // TODO(nzeh): Can this fail? Do I have to deal with it?
                            // I think we should probably just quietly ignore this.
                        }
                    }
                }
                else {
                    // TODO(nzeh): How do we deal with failure here?
                }
            }
            else {
                // TODO(nzeh): How can this fail at this point?
            }
        }

        // Takes an address storing a pointer, dereferences it and destructures the referenced
        // object.
        void destructurePtrBlock(FactorGraphBuilder &builder,
            const TypedData &data, const TypeHypothesisPair &ptr_hypothesis_pair) {
            TypedData referenced_data;
            if (data.Dereference(&referenced_data)) {
                auto target_hypothesis_pair = destructureBlockIfNew(builder, referenced_data);
                FactorGraph::FactorPtr pointer_factor =
                    std::make_shared<FactorGraph::PointerFactor>(
                        FactorGraph::Factor::Weights({ 0, 0, 0, 0, 0, 0, 0, 0 }));
                builder.addFactor(&pointer_factor, {
                    ptr_hypothesis_pair.declared_type,
                    ptr_hypothesis_pair.content_type,
                    target_hypothesis_pair.declared_type
                });
            }
            else {
                // TODO(nzeh): For now, we're just ignoring failure here.
            }
        }

        // Takes apart a typed address range into its components and chases pointers.  If we already
        // have an identical hypothesis in the graph, it doesn't do anything.
        TypeHypothesisPair destructureBlockIfNew(FactorGraphBuilder &builder,
            const TypedData &data) {
            auto range = data.GetRange();
            auto type = data.type()->type_id();
            size_t available_bytes;
            TypeHypothesisPair hypothesis_pair;

            hypothesis_pair.declared_type =
                std::make_shared<FactorGraph::DeclaredTypeHypothesis>(range, type);
            if (builder.addHypothesis(&hypothesis_pair.declared_type)) {
                if (data.bit_source()->GetFrom(range, &available_bytes, nullptr) &&
                    available_bytes == range.size()) {
                    hypothesis_pair.content_type =
                        std::make_shared<FactorGraph::ContentTypeHypothesis>(range, type);
                    builder.addHypothesis(&hypothesis_pair.content_type);
                    FactorGraph::FactorPtr link_hypotheses =
                        std::make_shared<FactorGraph::DeclarationContentFactor>(
                            FactorGraph::Factor::Weights({ 0, 0, 0, 0 }));
                    builder.addFactor(&link_hypotheses, {
                        hypothesis_pair.declared_type,
                        hypothesis_pair.content_type
                    });
                }
                if (data.IsArrayType())
                    destructureArrayBlock(builder, data, hypothesis_pair);
                else if (data.IsUserDefinedType())
                    destructureUdtBlock(builder, data, hypothesis_pair);
                else if (data.IsPointerType())
                    destructurePtrBlock(builder, data, hypothesis_pair);
            }
            return hypothesis_pair;
        }

        // Builds the subgraph of the factor graph obtained by taking apart arrays and UDTs
        // and chasing pointers.
        bool buildDestructuringSubgraph(scoped_refptr<TypeRepository> type_repository,
            ProcessState *process_state, FactorGraphBuilder &builder) {
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

        // Builds the subgraph of the factor graph representing conflicts between hypotheses
        // (e.g., overlapping address ranges of incompatible types).
        //
        // NOTE: This plays heavily with the fact that superclasses are represented as
        // member fields of a subclass.  Thus, when there are hypotheses with overlapping
        // address ranges, they corroborate each other if and only if one is obtained as a
        // member (of a member of a member ...) of the other; otherwise, they contradict each
        // other.
        void buildConflictSubgraph(FactorGraphBuilder &builder) {
            //auto hypotheses = builder.hypotheses();

            // TODO(nzeh): This needs to be replaced with something smarter than this n^2
            // exhaustive search.
            //for (auto hyp1 = hypotheses.begin(), end = hypotheses.end(); hyp1 != end; ++hyp1)
            //    for (auto hyp2 = next(hyp1); hyp2 != end; ++hyp2)
            //        if ((*hyp1)->addressRange().Intersects((*hyp2)->addressRange())) {
            //            // TODO(nzeh): Implement this
            //        }

        }

        // Builds a factor graph from a given type repository and process state
        bool buildFactorGraph(scoped_refptr<TypeRepository> type_repository,
            ProcessState *process_state, FactorGraphPtr *graph) {
            FactorGraphBuilder builder;

            if (buildDestructuringSubgraph(type_repository, process_state, builder)) {
                buildConflictSubgraph(builder);
                *graph = builder.getGraph();
                return true;
            }
            else {
                return false;
            }
        }

        // Turns a given type repository and process state into a factor graph whose nodes
        // represent hypotheses about memory contents along with probabilities that these
        // hypotheses are true.
        bool runProbMemAnalysis(scoped_refptr<TypeRepository> type_repository,
            ProcessState *process_state, FactorGraphPtr *graph) {
            if (buildFactorGraph(type_repository, process_state, graph)) {
                inferProbabilities(*graph);
                return true;
            }
            else {
                return false;
            }
        }
    }
}