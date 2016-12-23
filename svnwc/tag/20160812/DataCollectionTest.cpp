#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "c_DataCollection_PadHandler.h"

TEST_CASE("Protocol Pad", "[PAD]") {
	TCDataCollectionPadHandler pad;
	std::vector<TCString> TCDataCollectionPadHandler::*m_ptr = NULL;
	TCString packet = "AT66867106022056110,33251,125469984,0,102.6133426732902,26.343084828707404,0.0,0.0,,,-1,460018230389346,15608179872,12.5,0X0D 0X0A";
}