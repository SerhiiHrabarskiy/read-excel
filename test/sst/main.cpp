
/*!
	\file
	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2011-2017 Igor Mironchik

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
*/

// Excel include.
#include <excel/sst.hpp>
#include <excel/record.hpp>

// unit test helper.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <test/doctest/doctest.h>
#include <test/helper/helper.hpp>
#include <test/stream/stream.hpp>


const auto data = make_data(
	0xFCu, 0x00u, 0x13u, 0x00u,
	0x03u, 0x00u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u, 0x00u,
	0x10u, 0x00u, 0x00u,
	0x53u, 0x54u, 0x53u, 0x54u, 0x53u, 0x54u, 0x53u, 0x54u,

	0x3Cu, 0x00u, 0x11u, 0x00u,
	0x01u,
	0x53u, 0x00u, 0x54u, 0x00u, 0x53u, 0x00u, 0x54u, 0x00u,
	0x53u, 0x00u, 0x54u, 0x00u, 0x53u, 0x00u, 0x54u, 0x00u,

	0x3Cu, 0x00u, 0x13u, 0x00u,
	0x10u, 0x00u, 0x00u,
	0x51u, 0x52u, 0x51u, 0x52u, 0x51u, 0x52u, 0x51u, 0x52u,
	0x51u, 0x52u, 0x51u, 0x52u, 0x51u, 0x52u, 0x51u, 0x52u,

	0x3Cu, 0x00u, 0x23u, 0x00u,
	0x10u, 0x00u, 0x01u,
	0x51u, 0x00u, 0x52u, 0x00u, 0x51u, 0x00u, 0x52u, 0x00u,
	0x51u, 0x00u, 0x52u, 0x00u, 0x51u, 0x00u, 0x52u, 0x00u,
	0x51u, 0x00u, 0x52u, 0x00u, 0x51u, 0x00u, 0x52u, 0x00u,
	0x51u, 0x00u, 0x52u, 0x00u, 0x51u, 0x00u, 0x52u, 0x00u
); // data


//
// test_sst
//

TEST_CASE( "test_sst" )
{
	TestStream testStream( &data[ 0 ], 106 );

	Excel::Record record( testStream );

	std::vector< std::wstring > strings =
		Excel::SharedStringTable::parse( record );

	REQUIRE( strings.size() == 3 );

	REQUIRE( strings[ 0 ] == L"STSTSTSTSTSTSTST" );
	REQUIRE( strings[ 1 ] == L"QRQRQRQRQRQRQRQR" );
	REQUIRE( strings[ 2 ] == L"QRQRQRQRQRQRQRQR" );
}
