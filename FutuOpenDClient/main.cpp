#include <iostream>
#include "NetCenter.h"
#include "QuoteHandler.h"
#include "Common.h"

using namespace ftq;

int main(int argc, const char * argv[]) {
    // insert code here...
	std::cout << GetMicroTimeStamp() << endl;
	QuoteHandler *pQuoteHandler = new QuoteHandler();
	NetCenter::Default()->Init(uv_default_loop());
	NetCenter::Default()->SetProtoHandler(pQuoteHandler);
	NetCenter::Default()->Connect("127.0.0.1", 11111);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    return 0;
}
