#ifndef SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_H_
#define SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_H_

#include <memory>
#include "base/macros.h"
#include <syzygy/refinery/core/address.h>
#include <syzygy/refinery/types/type.h>

// Everything "MarkovNetwork" in here should be renamed to "FactorGraph" because that's how we
// represent things.  I only started thinking about it as a Markov network (or rather
// Markov random field) when I started on this.

namespace refinery {
    namespace ProbMemAnalysis {

        // The factor graph used for inferring probabilities of address-type annotations
        class FactorGraph {

        public:
            FactorGraph() = default;

            class Vertex;
            using VertexPtr = std::shared_ptr<Vertex>;
            using VertexConstPtr = std::shared_ptr<const Vertex>;
            using Vertices = std::vector<VertexPtr>;

            class Hypothesis;
            using HypothesisPtr = std::shared_ptr<Hypothesis>;
            using HypothesisConstPtr = std::shared_ptr<const Hypothesis>;

            class TypeHypothesis;
            using TypeHypothesisPtr = std::shared_ptr<TypeHypothesis>;
            using TypeHypothesisConstPtr = std::shared_ptr<const TypeHypothesis>;

            class DeclaredTypeHypothesis;
            class ContentTypeHypothesis;
            
            class Observation;
            using ObservationPtr = std::shared_ptr<Observation>;
            using ObservationConstPtr = std::shared_ptr<const Observation>;

            class Factor;
            using FactorPtr = std::shared_ptr<Factor>;

            class DecompositionFactor;
            class PointerFactor;
            class ContentFactor;
            class DeclarationContentFactor;

            void addVertex(const VertexPtr &);

        private:
            Vertices _vertices;

            DISALLOW_COPY_AND_ASSIGN(FactorGraph);
        };

        using FactorGraphPtr = std::shared_ptr<FactorGraph>;

        // A vertex of the factor graph.  All vertices know about is their neighbours.
        class FactorGraph::Vertex {

        public:
            virtual ~Vertex() = default;

            const Vertices &neighbours() const;

        protected:
            Vertex() = default;
            explicit Vertex(Vertices &&);
            explicit Vertex(const Vertices &);

            void addNeighbour(const VertexPtr &);

        private:
            friend class Factor;
            Vertices _neighbours;

            DISALLOW_COPY_AND_ASSIGN(Vertex);
        };

        // A vertex that represents some hypothesis or fact (Observation is a subclass
        // of Hypothesis and is guaranteed to be true with confidence 1 based on the structure
        // of the graph).  A hypothesis has an associated probability.
        class FactorGraph::Hypothesis : public FactorGraph::Vertex {

        public:
            void probability(double);
            double probability() const;

            virtual TypeHypothesisPtr castToTypeHypothesis();
            virtual TypeHypothesisConstPtr castToTypeHypothesis() const;
            virtual ObservationPtr castToObservation();
            virtual ObservationConstPtr castToObservation() const;

            enum class Kind {
                Observation,
                DeclaredTypeHypothesis,
                ContentTypeHypothesis,
            };

            Kind kind() const;

            bool operator==(const Hypothesis &);

        protected:
            Hypothesis(Kind);

        private:
            double _probability;
            Kind   _kind;

            DISALLOW_COPY_AND_ASSIGN(Hypothesis);
        };

        // A hypothesis that represents any type of association of a type with an address
        // range.
        class FactorGraph::TypeHypothesis
            : public FactorGraph::Hypothesis, std::enable_shared_from_this<TypeHypothesis> {

        public:
            const AddressRange &addressRange() const;
            TypeId type() const;

            TypeHypothesisPtr castToTypeHypothesis();
            TypeHypothesisConstPtr castToTypeHypothesis() const;

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
        class FactorGraph::Observation
            : public FactorGraph::Hypothesis, std::enable_shared_from_this<Observation> {

        public:
            Observation() = default;

            ObservationPtr castToObservation();
            ObservationConstPtr castToObservation() const;

        private:
            DISALLOW_COPY_AND_ASSIGN(Observation);
        };

        // A weight distribution (probability distribution without the requirement to normalize
        // to 1) over a set of Booleans.
        class WeightDistribution {

        public:
            explicit WeightDistribution(std::vector<double> &&);
            explicit WeightDistribution(const std::vector<double> &);
            double weight(const std::vector<bool> &) const;

        private:
            std::vector<double> _weights;
        };

        // A vertex that represents a factor, that is, a representation of a joint probability
        // distribution over a set of hypotheses.
        //
        // TODO(nzeh): Add methods that allow us to manipulate the probability distribution
        // associated with this factor during inference.
        class FactorGraph::Factor
            : public FactorGraph::Vertex, std::enable_shared_from_this<Factor> {

        public:

            enum class Kind {
                DecompositionFactor,
                PointerFactor,
                ContentFactor,
                DeclarationContentFactor,
            };

            Factor(Vertices &&, WeightDistribution &&, Kind);
            Factor(const Vertices &, const WeightDistribution &, Kind);

            Kind kind() const;

            bool operator==(const Factor &);

        private:
            WeightDistribution _weights;
            Kind               _kind;

            DISALLOW_COPY_AND_ASSIGN(Factor);
        };

        // A factor representing the association between hypotheses generated by decomposing
        // UDTs and arrays into their members.
        class FactorGraph::DecompositionFactor : public FactorGraph::Factor {

        public:
            DecompositionFactor(Vertices &&, WeightDistribution &&);
            DecompositionFactor(const Vertices &, const WeightDistribution &);

        private:
            DISALLOW_COPY_AND_ASSIGN(DecompositionFactor);
        };

        // A factor representing the association between hypotheses generated by dereferencing
        // pointers.
        class FactorGraph::PointerFactor : public FactorGraph::Factor {

        public:
            PointerFactor(Vertices &&, WeightDistribution &&);
            PointerFactor(const Vertices &, const WeightDistribution &);

        private:
            DISALLOW_COPY_AND_ASSIGN(PointerFactor);
        };

        // A factor representing the association between hypotheses generated by analyzing the
        // content of a given address range.
        class FactorGraph::ContentFactor : public FactorGraph::Factor {

        public:
            ContentFactor(Vertices &&, WeightDistribution &&);
            ContentFactor(const Vertices &, const WeightDistribution &);

        private:
            DISALLOW_COPY_AND_ASSIGN(ContentFactor);
        };

        // A factor used to link a DeclaredTypeHypothesis and a ContentTypeHypothesis for the
        // same type and address range.
        class FactorGraph::DeclarationContentFactor : public FactorGraph::Factor {

        public:
            DeclarationContentFactor(Vertices &&, WeightDistribution &&);
            DeclarationContentFactor(const Vertices &, const WeightDistribution &);

        private:
            DISALLOW_COPY_AND_ASSIGN(DeclarationContentFactor);
        };
    }
}

#endif // SYZYGY_REFINERY_PROB_MEMORY_ANALYZER_FACTOR_GRAPH_H_