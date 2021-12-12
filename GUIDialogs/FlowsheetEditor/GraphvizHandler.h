/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include <graphviz/cgraph.h>
#include <string>

class CFlowsheet;

class CGraphvizHandler
{
public:
	enum class EStyle { SIMPLE, WITH_PORTS };		// Style of rendering.
	enum class ELayout { HORIZONTAL, VERTICAL };	// Direction to place units.

private:
	const CFlowsheet* m_flowsheet{};	// Pointer to flowsheet.

	EStyle m_style{};	// Rendering style.
	ELayout m_layout{};	// Rendering layout direction.

public:
	CGraphvizHandler(const CFlowsheet* _flowsheet);

	// Returns current rendering style.
	[[nodiscard]] EStyle Style() const;
	// Sets new rendering style.
	void SetStyle(EStyle _style);
	// Returns current rendering layout direction.
	[[nodiscard]] ELayout Layout() const;
	// Sets new rendering layout direction.
	void SetLayout(ELayout _layout);

	// Saves current flowsheet as a file of a given type (image or dot). Returns true on success.
	[[nodiscard]] bool SaveToFile(const std::string& _fileName) const;

private:
	// Creates new graphviz graph and returns a non-owning pointer to it.
	[[nodiscard]] Agraph_t* CreateGraph() const;
};

