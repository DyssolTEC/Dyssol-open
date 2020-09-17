#pragma once

#include "Stream.h"

// An interface defining which functions of CStream are allowed to use in models.
class CStreamInterface : CStream
{
	using CBaseStream::AddTimePoint;
};

