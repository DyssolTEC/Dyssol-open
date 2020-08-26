#pragma once

#include "Stream.h"

// An interface defining which functions of CBaseStream are allowed to use in models.
class CStream2ModelInterface : CBaseStream
{
	using CBaseStream::AddTimePoint;
};

