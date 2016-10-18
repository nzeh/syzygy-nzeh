#ifndef SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_H_
#define SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_H_

#include <array>
#include "base/macros.h"
#include <syzygy/refinery/core/address.h>
#include <syzygy/refinery/types/type.h>

// Everything "MarkovNetwork" in here should be renamed to "FactorGraph" because that's how we
// represent things.  I only started thinking about it as a Markov network (or rather
// Markov random field) when I started on this.

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
            DECLARE_CLASS(DeclaredTypeHypothesis)
            DECLARE_CLASS(ContentTypeHypothesis)
            DECLARE_CLASS(Observation)
            DECLARE_CLASS(Factor)
            DECLARE_CLASS(DecompositionFactor)
            DECLARE_CLASS(PointerFactor)
            DECLARE_CLASS(ContentFactor)
            DECLARE_CLASS(DeclarationContentFactor)

            void addVertex(const VertexPtr &);
            void addEdge(const EdgePtr &);

            const VertexPtrs &vertices();
            const EdgePtrs &edges();

        private:
            VertexPtrs _vertices;
            EdgePtrs   _edges;

            DISALLOW_COPY_AND_ASSIGN(FactorGraph);
        };

        // A vertex of the factor graph.  All vertices know about is their neighbours.
        class FactorGraph::Vertex {

        public:
            virtual ~Vertex() = default;

            const EdgeCPtrs &neighbours() const;

            virtual void sendMessages() = 0;
            virtual void computeProbability() = 0;

        protected:
            Vertex() = default;

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

            using Message = std::array<double, 2>;

            void sendMessage(Vertex *, const Message &);
            const Message &receiveMessage(Vertex *);
            bool propagateMessages();
            void resetMessages();

            Vertex *otherEndpoint(const Vertex *) const;

        private:
            friend class FactorGraph;
            void connectToEndpoints();

            std::array<Vertex *, 2> _endpoints;
            std::array<Message, 2> _in_messages;
            std::array<Message, 2> _out_messages;
        };

        // A vertex that represents some hypothesis or fact (Observation is a subclass
        // of Hypothesis and is guaranteed to be true with confidence 1 based on the structure
        // of the graph).  A hypothesis has an associated probability.
        class FactorGraph::Hypothesis : public FactorGraph::Vertex {

        public:
            double probability() const;

            virtual TypeHypothesis *castToTypeHypothesis();
            virtual const TypeHypothesis *castToTypeHypothesis() const;
            virtual Observation *castToObservation();
            virtual const Observation *castToObservation() const;

            enum class Kind {
                Observation,
                DeclaredTypeHypothesis,
                ContentTypeHypothesis,
            };

            Kind kind() const;

            bool operator==(const Hypothesis &);

            void sendMessages();
            void computeProbability();

        protected:
            Hypothesis(Kind);

        private:
            double _probability;
            Kind   _kind;

            DISALLOW_COPY_AND_ASSIGN(Hypothesis);
        };

        // A hypothesis that represents any type of association of a type with an address
        // range.
        class FactorGraph::TypeHypothesis : public FactorGraph::Hypothesis {

        public:
            const AddressRange &addressRange() const;
            TypeId type() const;

            FactorGraph::TypeHypothesis *castToTypeHypothesis();
            const FactorGraph::TypeHypothesis *castToTypeHypothesis() const;

        protected:
            TypeHypothesis(Kind, const AddressRange &, TypeId);

        private:
            AddressRange _address_range;
            TypeId       _type;

            DISALLOW_COPY_AND_ASSIGN(TypeHypothesis);
        };

        // A hypothesis that represents that some part of the program thinks that a certain
        // address range holds an object of a certain type
        class FactorGraph::DeclaredTypeHypothesis : public FactorGraph::TypeHypothesis {

        public:
            DeclaredTypeHypothesis(const AddressRange &, TypeId);

        private:
            DISALLOW_COPY_AND_ASSIGN(DeclaredTypeHypothesis);
        };

        // A hypothesis that represents that the content of a certain address range contains
        // a (possibly corrupted) representation of an object of a certain type
        class FactorGraph::ContentTypeHypothesis : public FactorGraph::TypeHypothesis {

        public:
            ContentTypeHypothesis(const AddressRange &, TypeId);

        private:
            DISALLOW_COPY_AND_ASSIGN(ContentTypeHypothesis);
        };

        // A "hypothesis" that represents a fact.  It doesn't store any useful information
        // about the fact it represents but instead is used as an anchor linked to other hypotheses
        // using a factor whose weights ensure that its probability is always 1.  For example,
        // we represent the observed memory content of a certain address range as an Observation
        // and link it to a ContentTypeHypothesis with a factor whose weights express the degree
        // to which we believe that this address range is indeed of this type given the bit data
        // we found in this address range.
        class FactorGraph::Observation : public FactorGraph::Hypothesis {

        public:
            Observation() = default;

            FactorGraph::Observation *castToObservation();
            const FactorGraph::Observation *castToObservation() const;

        private:
            DISALLOW_COPY_AND_ASSIGN(Observation);
        };

        // A vertex that represents a factor, that is, a representation of a joint probability
        // distribution over a set of hypotheses.
        //
        // TODO(nzeh): Add methods that allow us to manipulate the probability distribution
        // associated with this factor during inference.
        class FactorGraph::Factor : public FactorGraph::Vertex {

        public:

            enum class Kind {
                DecompositionFactor,
                PointerFactor,
                ContentFactor,
                DeclarationContentFactor,
            };

            using Weights = std::vector<double>;

            Factor(Weights &&, Kind);
            Factor(const Weights &, Kind);

            Kind kind() const;

            bool operator==(const Factor &);

            Edge::Message summarizeMessages(unsigned int);
            void sendMessages();
            void computeProbability();

        private:
            Weights _weights;
            Kind    _kind;

            DISALLOW_COPY_AND_ASSIGN(Factor);
        };

        // A factor representing the association between hypotheses generated by decomposing
        // UDTs and arrays into their members.
        class FactorGraph::DecompositionFactor : public FactorGraph::Factor {

        public:
            DecompositionFactor(Weights &&);
            DecompositionFactor(const Weights &);

        private:
            DISALLOW_COPY_AND_ASSIGN(DecompositionFactor);
        };

        // A factor representing the association between hypotheses generated by dereferencing
        // pointers.
        class FactorGraph::PointerFactor : public FactorGraph::Factor {

        public:
            PointerFactor(Weights &&);
            PointerFactor(const Weights &);

        private:
            DISALLOW_COPY_AND_ASSIGN(PointerFactor);
        };

        // A factor representing the association between hypotheses generated by analyzing the
        // content of a given address range.
        class FactorGraph::ContentFactor : public FactorGraph::Factor {

        public:
            ContentFactor(Weights &&);
            ContentFactor(const Weights &);

        private:
            DISALLOW_COPY_AND_ASSIGN(ContentFactor);
        };

        // A factor used to link a DeclaredTypeHypothesis and a ContentTypeHypothesis for the
        // same type and address range.
        class FactorGraph::DeclarationContentFactor : public FactorGraph::Factor {

        public:
            DeclarationContentFactor(Weights &&);
            DeclarationContentFactor(const Weights &);

        private:
            DISALLOW_COPY_AND_ASSIGN(DeclarationContentFactor);
        };
    }
}

#endif // SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_H_