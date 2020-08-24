#pragma once

#include "Stream2.h"

// An interface defining which functions of CStream2 are allowed to use in models.
class CStream2ModelInterface : CStream2
{
	using CStream2::AddTimePoint;
};

