#ifndef SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_H_
#define SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_H_

#include <array>
#include "base/macros.h"
#include <syzygy/refinery/core/address.h>
#include <syzygy/refinery/types/type.h>

#define DECLARE_CLASS(T)                  \
    class T;                              \
    using T##Ptr   = std::shared_ptr<T>;  \
    using T##Ptrs  = std::vector<T##Ptr>; \
    using T##CPtrs = std::vector<T *>;

namespace refinery {
    namespace ProbMemAnalysis {

        // The factor graph used for inferring probabilities of address-type annotations
        DECLARE_CLASS(FactorGraph)

        class FactorGraph {

        public:
            FactorGraph() = default;

            DECLARE_CLASS(Vertex)
            DECLARE_CLASS(Edge)
            DECLARE_CLASS(Hypothesis)
            DECLARE_CLASS(TypeHypothesis)
            DECLARE_CLASS(Factor)

            void addVertex(const VertexPtr &);
            void addEdge(const EdgePtr &);

            const VertexPtrs &vertices();
            const EdgePtrs &edges();

        private:
            VertexPtrs _vertices;
            EdgePtrs   _edges;

            DISALLOW_COPY_AND_ASSIGN(FactorGraph);
        };

        class FactorGraph::Vertex {

        public:
            Vertex() = default;
            virtual ~Vertex() = default;

            const EdgeCPtrs &neighbours() const;

            // These two methods are the key workers in the implementation of belief propagation.
            // Note: Other inference methods are possible and may be explored, but belief
            // propagation is so simple to implement that it's a good candidate to try first.

            // Calculate message sent to each neighbour from the messages received from all other
            // neighbours.
            virtual void sendMessages() = 0;

            // Compute the probability of this vertex from the messages received from its
            // neighbours.
            virtual void computeProbability() = 0;

        private:
            friend class FactorGraph::Edge;
            void addNeighbour(Edge *);

            EdgeCPtrs _neighbours;

            DISALLOW_COPY_AND_ASSIGN(Vertex);
        };

        // Edges between vertices in the graph.  Used to hold messages between vertices.
        class FactorGraph::Edge {

        public:
            Edge(Vertex *, Vertex *);

            // A message consists of a pair of weights whose relative values represent how strongly
            // the sender believes that the recipient should be true or false.
            using Message = std::array<double, 2>;

            // Send the given message from the given endpoint.
            void sendMessage(Vertex *, const Message &);

            // Receive the given message by the given endpoint.
            const Message &receiveMessage(Vertex *);

            // The current belief propagation implementation proceeds in synchronized rounds.
            // This requires each edge to distinguish between messages received from one endpoint
            // before the current round (_out_messages) and messages received during the current
            // round (_in_messages).  This method is called at the end of the current round to 
            // copy _in_messages to _out_messages in preparation for the next round.  It returns
            // true if the relative difference between the old _in_messages and the old
            // _out_messages is less than 1% (at which point we consider the belief propagation
            // to have converged).
            bool propagateMessages();

            // Resets messages to equal probability for true and false at the beginning of
            // belief propagation.
            void resetMessages();

            // Given one endpoint of the edge, returns the other endpoint.
            Vertex *otherEndpoint(const Vertex *) const;

        private:
            friend class FactorGraph;

            // Adds this edge to the neighbour lists of its two endpoints.
            void connectToEndpoints();

            // Two endpoints, two in-messages, two out-messages
            std::array<Vertex *, 2> _endpoints;
            std::array<Message, 2> _in_messages;
            std::array<Message, 2> _out_messages;
        };

        // A vertex that represents some hypothesis or fact.  (Facts are represented as hypotheses
        // whose incident factors have weights that ensure their probability is always 1.)
        class FactorGraph::Hypothesis : public FactorGraph::Vertex {

        public:
            enum Kind {
                Observation,
                DeclaredTypeHypothesis,
                ContentTypeHypothesis,
            };

            Hypothesis(Kind);

            // The probability of this hypothesis
            double probability() const;

            // Safe cast of this hypothesis to a type hypothesis
            virtual TypeHypothesis *castToTypeHypothesis();
            virtual const TypeHypothesis *castToTypeHypothesis() const;

            // The kind of this hypothesis
            Kind kind() const;

            // Two hypotheses are equal if they are of the same kind and (in case of a type
            // hypothesis) they concern the same address range and type.
            bool operator==(const Hypothesis &);

            void sendMessages() override;
            void computeProbability() override;

        private:
            double _probability;
            Kind   _kind;

            DISALLOW_COPY_AND_ASSIGN(Hypothesis);
        };

        // A hypothesis that represents any type of association of a type with an address
        // range.  Currently, we distinguish between the type of the content (Can it be
        // interpreted as a value of this type?) and the type derived via type propagation
        // and pointer chasing (the "declared" type) without inspecting the content.
        class FactorGraph::TypeHypothesis : public FactorGraph::Hypothesis {

        public:
            TypeHypothesis(Kind, const AddressRange &, TypeId);

            const AddressRange &addressRange() const;
            TypeId type() const;

            FactorGraph::TypeHypothesis *castToTypeHypothesis();
            const FactorGraph::TypeHypothesis *castToTypeHypothesis() const;

        private:
            AddressRange _address_range;
            TypeId       _type;

            DISALLOW_COPY_AND_ASSIGN(TypeHypothesis);
        };

        // A vertex that represents a factor, that is, a representation of a joint probability
        // distribution over a set of hypotheses.
        class FactorGraph::Factor : public FactorGraph::Vertex {

        public:
            enum Kind {
                DecompositionFactor,
                PointerFactor,
                ContentFactor,
                DeclarationContentFactor,
            };

            using Weights = std::vector<double>;

            Factor(Kind, Weights &&);
            Factor(Kind, const Weights &);

            Kind kind() const;

            bool operator==(const Factor &);

            // Computes a summary message to be sent to the ith neighbour from the messages
            // received from all neighbours except the ith.
            Edge::Message summarizeMessages(unsigned int);

            // Send messages to all neighbours based on the messages received from them.
            void sendMessages();

            // For a factor, this does nothing.
            void computeProbability();

        private:
            Weights _weights;
            Kind    _kind;

            DISALLOW_COPY_AND_ASSIGN(Factor);
        };
    }
}

#endif // SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_H_