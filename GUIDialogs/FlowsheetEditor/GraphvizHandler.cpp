/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "GraphvizHandler.h"
#include "Flowsheet.h"
#include "BaseUnit.h"
#include <graphviz/gvc.h>

CGraphvizHandler::CGraphvizHandler(const CFlowsheet* _flowsheet)
	: m_flowsheet{ _flowsheet }
{
}

CGraphvizHandler::EStyle CGraphvizHandler::Style() const
{
	return m_style;
}

void CGraphvizHandler::SetStyle(EStyle _style)
{
	m_style = _style;
}

CGraphvizHandler::ELayout CGraphvizHandler::Layout() const
{
	return m_layout;
}

void CGraphvizHandler::SetLayout(ELayout _layout)
{
	m_layout = _layout;
}

bool CGraphvizHandler::SaveToFile(const std::string& _fileName) const
{
	// whether saving was successful
	bool success = false;
	// try to crate and fill graph
	if (const auto graph = CreateGraph())
	{
		// try to create context
		if (GVC_t* context = gvContext())
		{
			// try to apply dot layout engine
			if (!gvLayout(context, graph, "dot"))
			{
				// get and check file extension
				if (const auto ext = std::filesystem::path{ _fileName }.extension().string(); !ext.empty())
				{
					// file rendering itself
					success = !gvRenderFilename(context, graph, ext.substr(1).c_str(), _fileName.c_str());
				}
				// clear layout
				gvFreeLayout(context, graph);
			}
			// clear context
			gvFreeContext(context);
		}
		// clear graph
		agclose(graph);
	}
	return success;
}

Agraph_t* CGraphvizHandler::CreateGraph() const
{
	// temporary z-string values, needed because graphviz functions take non-const char* as parameters
	char empty[]         = "";
	char attrNodesep[]   = "nodesep";
	char attrFontsize[]  = "fontsize";
	char attrRankdir[]   = "rankdir";
	char attrShape[]     = "shape";
	char attrLabel[]     = "label";
	char attrHeadport[]  = "headport";
	char attrTailport[]  = "tailport";
	char valNodesep[]    = "0.4";
	char valFontsize[]   = "20";
	char valRankdirLR[]  = "LR";
	char valRankdirTB[]  = "TB";
	char valShapeBox[]   = "box";
	char valShapePlain[] = "plaintext";

	// create directed graph
	constexpr Agdesc_t directed{ 1,0,0,1,0,0,0,0 };
	Agraph_t* graph = agopen(empty, directed, nullptr);
	agsafeset(graph, attrNodesep, valNodesep, empty);
	agsafeset(graph, attrFontsize, valFontsize, empty);
	switch (m_layout)
	{
	case ELayout::HORIZONTAL: agsafeset(graph, attrRankdir, valRankdirLR, empty); break;
	case ELayout::VERTICAL: agsafeset(graph, attrRankdir, valRankdirTB, empty); break;
	}

	// temporary storage of nodes
	std::map<std::string, Agnode_t*> nodes;

	// list units
	for (const auto& u : m_flowsheet->GetAllUnits())
	{
		if (!u->GetModel()) continue;

		char* nodeName = agstrdup(graph, u->GetKey().data());
		auto* node = agnode(graph, nodeName, 1);
		agstrfree(graph, nodeName);

		switch (m_style)
		{
		case EStyle::SIMPLE:
		{
			char* nodeLabel = agstrdup(graph, u->GetName().data());
			agsafeset(node, attrShape, valShapeBox, empty);
			agsafeset(node, attrLabel, nodeLabel, empty);
			agstrfree(graph, nodeLabel);
			break;
		}
		case EStyle::WITH_PORTS:
		{
			agsafeset(node, attrShape, valShapePlain, empty);
			const auto& ports = u->GetModel()->GetPortsManager();
			std::string html = "<TABLE BORDER = '1' CELLBORDER = '0' CELLSPACING = '0' CELLPADDING = '4'><TR>";
			if (!ports.GetAllInputPorts().empty())
			{
				html += "<TD><TABLE BORDER='0' CELLBORDER='1' CELLSPACING='0' CELLPADDING='4'>";
				for (const auto& port : ports.GetAllInputPorts())
					html += "<TR><TD PORT='" + port->GetName() + "'>" + port->GetName() + "</TD></TR>";
				html += "</TABLE></TD>";
			}
			html += "<TD>" + u->GetName() + "</TD>";
			if (!ports.GetAllOutputPorts().empty())
			{
				html += "<TD><TABLE BORDER='0' CELLBORDER='1' CELLSPACING='0' CELLPADDING='4'>";
				for (const auto& port : ports.GetAllOutputPorts())
					html += "<TR><TD PORT='" + port->GetName() + "'>" + port->GetName() + "</TD></TR>";
				html += "</TABLE></TD>";
			}
			html += "</TR></TABLE>";
			char* nodeHtml = agstrdup_html(graph, html.data());
			agsafeset(node, attrLabel, nodeHtml, empty);
			agstrfree(graph, nodeHtml);
			break;
		}
		}

		nodes[u->GetKey()] = node;
	}

	// list streams
	for (auto& c : m_flowsheet->GenerateConnectionsDescription())
	{
		const auto* stream = m_flowsheet->GetStream(c.stream);
		const auto* unitI = m_flowsheet->GetUnit(c.unitI);
		const auto* unitO = m_flowsheet->GetUnit(c.unitO);

		if (!stream || !unitI || !unitO || !nodes[unitO->GetKey()] || !nodes[unitI->GetKey()]) continue;

		char* edgeName = agstrdup(graph, stream->GetName().data());
		auto* edge = agedge(graph, nodes[unitO->GetKey()], nodes[unitI->GetKey()], edgeName, 1);
		agsafeset(edge, attrLabel, edgeName, empty);
		agstrfree(graph, edgeName);

		switch (m_style)
		{
		case EStyle::SIMPLE: break;
		case EStyle::WITH_PORTS:
		{
			agsafeset(edge, attrHeadport, c.portI.data(), empty);
			agsafeset(edge, attrTailport, c.portO.data(), empty);
		}
		}
	}

	return graph;
}
