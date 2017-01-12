/// A not-particularly efficient (or is it?) implementation of a directed Graph,
/// but a convenient one.
#ifndef CIC_GRAPH_H
#define CIC_GRAPH_H

#include <string>
using std::string;
#include <map>
using std::map;
#include <set>
using std::set;
#include <sstream>
using std::stringstream;

namespace cic {

template <typename ValueType> struct Connections {
  set<ValueType *> predecessors;
  set<ValueType *> successors;
};

template <typename ValueType> class Graph {
protected:
  map<ValueType *, Connections<ValueType>> Nodes;

public:
  string name;
  ValueType *beginNode;

  typedef typename map<ValueType *, Connections<ValueType>>::iterator
      nodes_iterator;
  typedef typename map<ValueType *, Connections<ValueType>>::const_iterator
      const_nodes_iterator;

  typedef typename set<ValueType *>::iterator succ_iterator;
  typedef typename set<ValueType *>::const_iterator const_succ_iterator;
  typedef typename set<ValueType *>::iterator pred_iterator;
  typedef typename set<ValueType *>::const_iterator const_pred_iterator;

  void addNode(ValueType *Value) { Nodes[Value]; }

  /// Checks if the graph contains node A.
  bool contains(ValueType *A) { return Nodes.find(A) != Nodes.end(); }

  /// Checks if the graph contains node A.
  bool hasNode(ValueType *A) { return contains(A); }

  void addEdge(ValueType *From, ValueType *To) {
    Nodes[From].successors.insert(To);
    Nodes[To].predecessors.insert(From);
  }

  /// Makes sure there are no edges pointing to `Value` and then removes it
  /// from `Nodes`.
  void removeNode(ValueType *Value) {
    for (auto &Node : Nodes) {
      Node.second.succesors.erase(Value);
      Node.second.predecessors.erase(Value);
    }
    Nodes.erase(Value);
  }

  void removeEdge(ValueType *From, ValueType *To) {
    Nodes.at(From).successors.erase(To);
    Nodes.at(To).predecessors.erase(From);
  }

  /// Checks if node B is a successor of node A.
  bool hasSuccessor(ValueType *A, ValueType *B) {
    return Nodes[A].successors.find(B) != Nodes[A].successors.end();
  }

  /// Checks if node B is a predecessor of node A.
  bool hasPredecessor(ValueType *A, ValueType *B) {
    return Nodes[A].predecessors.find(B) != Nodes[A].predecessors.end();
  }

  /// Gets the set of successors of node A.
  const set<ValueType *> &getSuccessors(ValueType *A) const {
    return Nodes.at(A).successors;
  }

  /// Gets the set of predecessors of node A.
  const set<ValueType *> &getPredecessors(ValueType *A) const {
    return Nodes.at(A).predecessors;
  }

  /// Construct a subgraph containing only the specified nodes and the
  /// connections between them.
  Graph getSubgraph(const set<ValueType *> nodes) const {
    Graph<ValueType> result;
    for (auto node : nodes) {
      result.addNode(node);
    }

    for (auto parent : nodes) {
      auto &successors = getSuccessors(parent);
      for (auto succ : successors) {
        if (result.contains(succ)) {
          result.addEdge(parent, succ);
        }
      }
    }

    return result;
  }

  /// Get the number of nodes of the graph.
  unsigned size() const { return Nodes.size(); }

  /// Get the number of nodes of the graph.
  unsigned numOfNodes() const { return size(); }

  /// Get the number of edges of the graph.
  unsigned numOfEdges() const {
    unsigned sum = 0;
    for (auto &Node : Nodes) {
      sum += Node.second.successors.size();
    }
    return sum;
  }

  // Generate a .dot format representation of the digraph.
  string toDot() const {
    stringstream ss;
    ss << "digraph \"" << name << "\" {\n";
    ss << "  label=\"" << name << "\";\n";
    for (auto Node : Nodes) {
      auto *parent = Node.first;
      ss << "  Node" << parent << " [shape=record];\n";
      for (auto *succ : getSuccessors(parent)) {
        ss << "  Node" << parent << " -> Node" << succ << ";\n";
      }
    }
    ss << "}";
    return ss.str();
  }
  succ_iterator succ_begin(ValueType *A) {
    return Nodes.at(A).successors.begin();
  }
  succ_iterator succ_end(ValueType *A) { return Nodes.at(A).successors.end(); }

  pred_iterator pred_begin(ValueType *A) {
    return Nodes.at(A).predecessors.begin();
  }
  pred_iterator pred_end(ValueType *A) {
    return Nodes.at(A).predecessors.end();
  }

  /// Remove all nodes from the graph.
  void clear() { Nodes.clear(); }

  nodes_iterator nodes_begin() { return Nodes.begin(); }
  nodes_iterator nodes_end() { return Nodes.end(); }

  /// Gets the set of successors of node A.
  const map<ValueType *, Connections<ValueType>> &getNodes() const {
    return Nodes;
  }
};
}

#endif
