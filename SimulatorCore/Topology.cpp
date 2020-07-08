/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Topology.h"
#include "DyssolUtilities.h"
#include <map>

CTopology::CTopology(size_t _nVertices)
{
	m_vAdjList.resize(_nVertices);
}

void CTopology::SetVertices(size_t _nVertices)
{
	m_vAdjList.clear();
	m_vAdjList.resize(_nVertices);
}

void CTopology::AddEdge(size_t _nV1, size_t _nV2)
{
	if (_nV1 < m_vAdjList.size())
		if (!VectorContains(m_vAdjList[_nV1], _nV2))
			m_vAdjList[_nV1].push_back(_nV2);
}

size_t CTopology::VerticesNum() const
{
	return m_vAdjList.size();
}

size_t CTopology::EdgesNum() const
{
	size_t cnt = 0;
	for (const auto& v : m_vAdjList)
		cnt += v.size();
	return cnt;
}

bool CTopology::Analyse(u_matr_t& _vOrder, std::vector<u_pair_vect_t>& _vTears) const
{
	_vOrder = StronglyConnectedComponents();         // get partitions and basic order
	_vTears.clear();
	_vTears.resize(_vOrder.size());
	u_matr_t vTearStreams = GetTearStreams(_vOrder); // get list of tear streams
	for (size_t i = 0; i < _vOrder.size(); ++i)      // get order for each partition
	{
		if (_vOrder[i].size() > 1) // partition with cycle
		{
			u_matr_t vSubGraph(_vOrder[i].size());	         // graph
			u_vect_t iDir = _vOrder[i];				         // vector of to translate indices reduced -> main
			u_vect_t iInv(VerticesNum(), _vOrder[i].size()); // vector of to translate indices main -> reduced
			for (size_t j = 0; j < iDir.size(); ++j)
				iInv[iDir[j]] = j;
			// build reduced graph for this partition
			for (size_t j = 0; j < _vOrder[i].size(); ++j)
			{
				const size_t v = _vOrder[i][j];
				for (size_t k = 0; k < m_vAdjList[v].size(); ++k)
				{
					const size_t w = m_vAdjList[v][k];
					if (VectorContains(_vOrder[i], w))
						vSubGraph[iInv[v]].push_back(iInv[w]);
				}
			}
			// remove tear streams from reduced graph
			for (size_t v = 0; v < vTearStreams.size(); ++v)
			{
				for (size_t w = 0; w < vTearStreams[v].size(); ++w)
				{
					const size_t v_new = iInv[v];
					size_t tear_new = iInv[vTearStreams[v][w]];
					if (v_new >= _vOrder[i].size() || tear_new >= _vOrder[i].size())
						continue;
					auto it = std::find(vSubGraph[v_new].begin(), vSubGraph[v_new].end(), tear_new);
					if (it != vSubGraph[v_new].end())
					{
						vSubGraph[v_new].erase(it);
						_vTears[i].push_back(u_pair_t(v, vTearStreams[v][w])); // set tear stream to its position
					}
				}
			}
			if (_vTears[i].empty()) return false; // cannot perform topological sort for cyclic graph
			// sort reduced graph
			u_vect_t vNewOrder = TopologicalSort(vSubGraph);
			for (size_t j = 0; j < vNewOrder.size(); ++j)
				_vOrder[i][j] = iDir[vNewOrder[j]];
		}
	}

	// checks
	size_t cnt1 = 0;
	for (auto& tear : _vTears)
		cnt1 += tear.size();
	size_t cnt2 = 0;
	for (auto& tearStream : vTearStreams)
		cnt2 += tearStream.size();
	const bool bRes1 = cnt1 == cnt2;

	bool bRes2 = true;
	for (size_t i = 0; i < _vOrder.size(); ++i)
		if (_vOrder[i].size() <= 1 && !_vTears[i].empty() || _vOrder[i].size() > 1 && _vTears[i].empty())
			bRes2 = false;

	// combine
	for (size_t i = 0; _vTears.size() > 1 && i < _vTears.size() - 1;)
	{
		if (_vTears[i].empty() && _vTears[i + 1].empty())
		{
			_vOrder[i].push_back(_vOrder[i + 1].front());
			_vOrder.erase(_vOrder.begin() + i + 1);
			_vTears.erase(_vTears.begin() + i + 1);
		}
		else
			++i;
	}

	return bRes1 & bRes2;
}

bool CTopology::DeepFirstSearch(const u_matr_t& _graph, size_t _v, size_t _w)
{
	b_vect_t vVisited(_graph.size(), false);
	return DeepFirstSearchUtil(_graph, _v, _w, vVisited);
}

bool CTopology::DeepFirstSearchUtil(const u_matr_t& _graph, size_t _v, size_t _w, b_vect_t& _visited)
{
	_visited[_v] = true;
	for (size_t i = 0; i < _graph[_v].size(); ++i)
		if (!_visited[_graph[_v][i]])
		{
			if (_graph[_v][i] == _w) return true;
			if (DeepFirstSearchUtil(_graph, _graph[_v][i], _w, _visited)) return true;
		}
	return false;
}

CTopology::u_matr_t CTopology::GetInvAdjList(const u_matr_t& _list) const
{
	u_matr_t vIAL(VerticesNum());
	for (size_t i = 0; i < _list.size(); ++i)
		for (size_t j = 0; j < _list[i].size(); ++j)
			if (!VectorContains(vIAL[_list[i][j]], i))
				vIAL[_list[i][j]].push_back(i);
	return vIAL;
}

CTopology::u_matr_t CTopology::GetWeightedAdjMatrix(const u_matr_t& _SCC) const
{
	// remove edges, connecting strongly connected components (partitions), from adjacency list
	u_matr_t reducedList(VerticesNum());							   // reduced adjacency list, containing only edges within partitions
	for (size_t iSrc = 0; iSrc < m_vAdjList.size(); ++iSrc)            // for all src nodes
		for (size_t iDst : m_vAdjList[iSrc])				           // for all dst nodes, connected to src node
			for (const auto& k : _SCC)						           // for all partitions
				if(VectorContains(k, iSrc) && VectorContains(k, iDst)) // both src ans dst are in partition
					reducedList[iSrc].push_back(iDst);				   // put it into reduces list

	// weighted adjacency matrix; weights depend on the number of inlet and outlet streams, connected to the issuing unit
	// in the simplest case: weight = 10 * in_number + (max_out_number + 1 - out_number)
	u_matr_t vWAdjMatr(VerticesNum(), u_vect_t(VerticesNum(), 0));
	// inverted adjacency list
	u_matr_t vIAL = GetInvAdjList(reducedList);
	// calculate coefficients
	size_t kLo = 1; // lower coefficient
	for (auto& v : reducedList)
		kLo = std::max(kLo, v.size());
	++kLo;
	const auto kHi = size_t(std::pow(10., int(std::floor(std::log10(double(kLo)))) + 1)); // higher coefficient
	// hi weights
	for (size_t i = 0; i < reducedList.size(); ++i)
		for (size_t j = 0; j < reducedList[i].size(); ++j)
			vWAdjMatr[i][reducedList[i][j]] = kLo - reducedList[i].size();
	// low weights
	for (size_t i = 0; i < vIAL.size(); ++i)
		for (size_t j = 0; j < reducedList[i].size(); ++j)
			vWAdjMatr[i][reducedList[i][j]] += kHi * vIAL[i].size();

	// set some minimum weight for all edges, which connect partitions
	for (size_t i = 0; i < m_vAdjList.size(); ++i)
		for (size_t j = 0; j < m_vAdjList[i].size(); ++j)
			if (!vWAdjMatr[i][m_vAdjList[i][j]])
				vWAdjMatr[i][m_vAdjList[i][j]] = 1;

	return vWAdjMatr;
}

CTopology::u_matr_t CTopology::StronglyConnectedComponents() const
{
	u_matr_t vSCC;
	u_vect_t vInd(m_vAdjList.size(), -1);
	u_vect_t vLow(m_vAdjList.size(), -1);
	u_vect_t vStack;
	size_t nIndex = 0;
	for (size_t i = 0; i < vInd.size(); ++i)
		if (vInd[i] == size_t(-1))
			StronglyConnectedComponentsUtil(i, nIndex, vInd, vLow, vStack, vSCC);
	std::reverse(vSCC.begin(), vSCC.end());
	return vSCC;
}

void CTopology::StronglyConnectedComponentsUtil(size_t _nV, size_t& _nIndex, u_vect_t& _vInd, u_vect_t& _vLow, u_vect_t& _vStack, u_matr_t& _vSCC) const
{
	_vInd[_nV] = _nIndex;
	_vLow[_nV] = _nIndex;
	_nIndex++;
	_vStack.push_back(_nV);

	for (size_t i = 0; i < m_vAdjList[_nV].size(); ++i)
	{
		const size_t nW = m_vAdjList[_nV][i];
		if (_vInd[nW] == size_t(-1))
		{
			StronglyConnectedComponentsUtil(nW, _nIndex, _vInd, _vLow, _vStack, _vSCC);
			_vLow[_nV] = std::min(_vLow[_nV], _vLow[nW]);
		}
		else if (VectorContains(_vStack, nW))
			_vLow[_nV] = std::min(_vLow[_nV], _vInd[nW]);
	}

	if (_vLow[_nV] == _vInd[_nV])
	{
		_vSCC.push_back(u_vect_t());
		for (;;)
		{
			size_t nW = _vStack.back();
			_vStack.pop_back();
			_vSCC.back().push_back(nW);
			if (nW == _nV)
				break;
		}
	}
}

CTopology::u_vect_t CTopology::TopologicalSort(const u_matr_t& _graph)
{
	b_vect_t vVisited(_graph.size(), false);
	u_vect_t vOrder;
	for (size_t i = 0; i < vVisited.size(); ++i)
		if (!vVisited[i])
			TopologicalSortUtil(_graph, i, vVisited, vOrder);
	std::reverse(vOrder.begin(), vOrder.end());
	return vOrder;
}

void CTopology::TopologicalSortUtil(const u_matr_t& _graph, size_t _v, b_vect_t& _vVisited, u_vect_t& _vOrder)
{
	_vVisited[_v] = true;
	for (size_t i = 0; i < _graph[_v].size(); ++i)
	{
		const size_t w = _graph[_v][i];
		if (!_vVisited[w])
			TopologicalSortUtil(_graph, w, _vVisited, _vOrder);
	}
	_vOrder.push_back(_v);
}

// TODO: replace with Edmonds' algorithm to get the proper order
CTopology::u_matr_t CTopology::MinSpanningTree(const u_matr_t& _vWeights)
{
	u_vect_t vMST(_vWeights.size(), -1);                                    // minimum spanning tree
	u_vect_t vWeight(_vWeights.size(), std::numeric_limits<size_t>::max()); // array of minimum weights
	b_vect_t vIncluded(_vWeights.size(), false);                            // vertices already included in MST

	vWeight[0] = 0;     // 1st vertex as root

	for (size_t i = 0; i < _vWeights.size(); ++i)
	{
		// find min not included weight
		size_t min = std::numeric_limits<size_t>::max();
		size_t v = -1;
		for (size_t j = 0; j < _vWeights.size(); ++j)
			if (!vIncluded[j] && vWeight[j] < min)
				min = vWeight[j], v = j;
		if(v == size_t(-1)) return u_matr_t();	// graph is not connected

		vIncluded[v] = true; // add vertex to the MST

		// update weights and MST
		for (size_t w = 0; w < _vWeights.size(); w++)
		{
			if (_vWeights[v][w] && !vIncluded[w] && _vWeights[v][w] < vWeight[w])
			{
				vMST[w] = v;
				vWeight[w] = _vWeights[v][w];
			}
			if (_vWeights[w][v] && !vIncluded[w] && _vWeights[w][v] < vWeight[w])
			{
				vMST[w] = v;
				vWeight[w] = _vWeights[w][v];
			}
		}
	}

	u_matr_t vRet(vMST.size());
	for (size_t i = 0; i < vMST.size(); ++i)
		if (vMST[i] != size_t(-1))
		{
			if (_vWeights[i][vMST[i]])
				vRet[i].push_back(vMST[i]);
			else
				vRet[vMST[i]].push_back(i);
		}
	return vRet;
}

CTopology::u_matr_t CTopology::FindAdjacentNodeLoops(const u_matr_t& _weights)
{
	u_matr_t vANL(_weights.size());
	for (size_t i = 0; i < _weights.size(); ++i)
		for (size_t j = i; j < _weights[i].size(); ++j)
			if (_weights[i][j] && _weights[j][i])
			{
				if (_weights[i][j] >= _weights[j][i] && !VectorContains(vANL[i], j))
					vANL[i].push_back(j);
				else if (!VectorContains(vANL[j], i))
					vANL[j].push_back(i);
			}
	return vANL;
}

CTopology::u_matr_t CTopology::GetTearStreams(const u_matr_t& _SCC) const
{
	u_matr_t vWeights = GetWeightedAdjMatrix(_SCC);
	u_matr_t vLoops = FindAdjacentNodeLoops(vWeights); // find bi-directionally coupled nodes
	//remove torn streams
	for (size_t i = 0; i < vLoops.size(); ++i)
		for (size_t j = 0; j < vLoops[i].size(); ++j)
			vWeights[i][vLoops[i][j]] = 0;
	u_matr_t vInDAG = MinSpanningTree(vWeights);       // edges in directed acyclic graph
	if(vInDAG.empty()) return u_matr_t();              // cannot find minimum spanning tree for this graph
	std::multimap<size_t, u_pair_t> vOutDAG;           // edges outside the DAG
	// copy all which are not in InDAG and Loops
	for (size_t i = 0; i < m_vAdjList.size(); ++i)
		for (size_t j = 0; j < m_vAdjList[i].size(); ++j)
			if (!VectorContains(vInDAG[i], m_vAdjList[i][j]) && !VectorContains(vLoops[i], m_vAdjList[i][j]))
				vOutDAG.insert(std::pair<size_t, u_pair_t>(vWeights[i][m_vAdjList[i][j]], u_pair_t(i, m_vAdjList[i][j])));
	for (auto it = vOutDAG.begin(); it != vOutDAG.end();)
	{
		const size_t v = (*it).second.first;
		const size_t w = (*it).second.second;
		vInDAG[v].push_back(w);
		if (DeepFirstSearch(vInDAG, w, v))	// check for cycle
		{
			vInDAG[v].pop_back();
			++it;
		}
		else
			vOutDAG.erase(it++);
	}
	for (auto& it : vOutDAG)
		vLoops[it.second.first].push_back(it.second.second);
	return vLoops;
}
