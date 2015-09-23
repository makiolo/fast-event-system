//
// Event_VX.h
//
// $Id: //poco/1.4/Foundation/include/Poco/Event_VX.h#1 $
//
// Library: Foundation
// Package: Threading
// Module:  Event
//
// Definition of the EventImpl class for VxWorks.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#ifndef Foundation_Event_VX_INCLUDED
#define Foundation_Event_VX_INCLUDED


#include "Poco/Foundation.h"
#include "Poco/Exception.h"
#include <semLib.h>
#include <atomic>

namespace Poco {


class poco_API EventImpl
{
protected:
	EventImpl(bool autoReset);
	~EventImpl();
	void setImpl();
	void waitImpl();
	bool waitImpl(long milliseconds);
	void resetImpl();

private:
	bool          _auto;
	std::atomic<bool> _state;
	SEM_ID        _sem;
};


} // namespace Poco


#endif // Foundation_Event_VX_INCLUDED
