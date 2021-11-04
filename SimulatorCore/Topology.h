/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>
#include <cstddef>

class CTopology
{
private:
	typedef std::vector<std::vector<size_t>> u_matr_t;
	typedef std::vector<size_t> u_vect_t;
	typedef std::vector<bool> b_vect_t;
	typedef std::pair<size_t, size_t> u_pair_t;
	typedef std::vector<std::pair<size_t, size_t>> u_pair_vect_t;

	u_matr_t m_vAdjList;

public:
	CTopology() = default;
	CTopology(size_t _nVertices);
	~CTopology() = default;

	void SetVertices(size_t _nVertices);
	void AddEdge(size_t _nV1, size_t _nV2);
	size_t VerticesNum() const;
	size_t EdgesNum() const;

	// Performs topological analysis and returns calculation order and list of tear streams.
	bool Analyse(u_matr_t& _vOrder, std::vector<u_pair_vect_t>& _vTears) const;

private:
	// Returns true if there is a path from vertex v to w in directed Graph. Graph is an adjacency list. Does not handle self-loops.
	static bool DeepFirstSearch(const u_matr_t& _graph, size_t _v, size_t _w);
	static bool DeepFirstSearchUtil(const u_matr_t& _graph, size_t _v, size_t _w, b_vect_t& _visited);

	u_matr_t GetInvAdjList(const u_matr_t& _list) const;
	// Returns weighted adjacency matrix according to Roach's synthetic method.
	u_matr_t GetWeightedAdjMatrix(const u_matr_t& _SCC) const;

	// Uses Tarjan's Algorithm.
	u_matr_t StronglyConnectedComponents() const;
	void StronglyConnectedComponentsUtil(size_t _nV, size_t& _nIndex, u_vect_t& _vInd, u_vect_t& _vLow, u_vect_t& _vStack, u_matr_t& _vSCC) const;

	// Determines calculation order in acyclic connected graph.
	static u_vect_t TopologicalSort(const u_matr_t& _graph);
	static void TopologicalSortUtil(const u_matr_t& _graph, size_t _v, b_vect_t& _vVisited, u_vect_t& _vOrder);

	// Uses Prim's algorithm, takes not mirrored undirected weighted adjacency matrix of connected graph as a parameter. Returns adjacency list.
	static u_matr_t MinSpanningTree(const u_matr_t& _vWeights);

	// Returns list of edges, which should be torn to break adjacent-node loops (bi-directionally coupled nodes). Chooses edges with the highest weight from each pair, or the last one, if the weights are the same.
	static u_matr_t FindAdjacentNodeLoops(const u_matr_t& _weights);

	// Uses Roach's synthetic method.
	u_matr_t GetTearStreams(const u_matr_t& _SCC) const;
};

