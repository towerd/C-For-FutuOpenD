#include "Common.h"

#include <memory.h>

using namespace ftq;
using namespace std;

namespace ftq
{
	void APIProtoHeader::Init()
	{
		memset(this, 0, sizeof(*this));
		szHeaderFlag[0] = 'F';
		szHeaderFlag[1] = 'T';
	}
}

