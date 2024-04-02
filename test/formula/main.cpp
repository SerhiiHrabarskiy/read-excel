
/*
	SPDX-FileCopyrightText: 2011-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: MIT
*/

// Excel include.
#include <read-excel/formula.hpp>
#include <read-excel/record.hpp>
#include <read-excel/string.hpp>

// unit test helper.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <test/doctest/doctest.h>
#include <test/stream/stream.hpp>
#include <test/helper/helper.hpp>


const auto data1 = make_data(
	0x06u, 0x00u, 0x16u, 0x00u,
	0x01u, 0x00u, 0x02u, 0x00u, 0x00u, 0x00u,
	0x08u, 0x07u, 0x06u, 0x05u, 0x04u, 0x03u, 0x02u, 0x01u,
	0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u
); // data1

const auto data2 = make_data(
	0x06u, 0x00u, 0x16u, 0x00u,
	0x02u, 0x00u, 0x03u, 0x00u, 0x00u, 0x00u,
	0x01u, 0x00u, 0x01u, 0x00u, 0x00u, 0x00u, 0xFFu, 0xFFu,
	0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u
); // data2

const auto data3 = make_data(
	0x06u, 0x00u, 0x16u, 0x00u,
	0x03u, 0x00u, 0x04u, 0x00u, 0x00u, 0x00u,
	0x02u, 0x00u, 0x2Au, 0x00u, 0x00u, 0x00u, 0xFFu, 0xFFu,
	0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u
); // data3

const auto data4 = make_data(
	0x06u, 0x00u, 0x16u, 0x00u,
	0x04u, 0x00u, 0x05u, 0x00u, 0x00u, 0x00u,
	0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xFFu, 0xFFu,
	0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,

	0x07u, 0x02u, 0x12u, 0x00u,
	0x0Fu, 0x00u, 0x00u, 0x74u, 0x68u, 0x69u, 0x73u, 0x20u,
	0x69u, 0x73u, 0x20u, 0x72u, 0x65u, 0x64u, 0x20u, 0x69u,
	0x6Eu, 0x6Bu
); // data4

const auto data5 = make_data(
	0x06u, 0x00u, 0x16u, 0x00u,
	0x02u, 0x00u, 0x03u, 0x00u, 0x00u, 0x00u,
	0x01u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xFFu, 0xFFu,
	0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u
); // data5


//
// test_formula
//

TEST_CASE( "test_formula" )
{
	union {
		double asDouble;
		uint64_t asLongLong;
	} un;

	{
		TestStream stream( &data1[ 0 ], 26 );

		Excel::Record record( stream );

		Excel::Formula formula( record );

		REQUIRE( formula.valueType() == Excel::Formula::DoubleValue );
		REQUIRE( formula.getRow() == 0x01 );
		REQUIRE( formula.getColumn() == 0x02 );

		un.asLongLong = 0x0102030405060708;

		REQUIRE( formula.getDouble() == un.asDouble );
	}

	{
		TestStream stream( &data2[ 0 ], 26 );

		Excel::Record record( stream );

		Excel::Formula formula( record );

		REQUIRE( formula.valueType() == Excel::Formula::BooleanValue );
		REQUIRE( formula.getRow() == 0x02 );
		REQUIRE( formula.getColumn() == 0x03 );

		un.asLongLong = 0xFFFF000000010001;

		REQUIRE( formula.getBoolean() == true );
	}

	{
		TestStream stream( &data3[ 0 ], 26 );

		Excel::Record record( stream );

		Excel::Formula formula( record );

		REQUIRE( formula.valueType() == Excel::Formula::ErrorValue );
		REQUIRE( formula.getRow() == 0x03 );
		REQUIRE( formula.getColumn() == 0x04 );

		un.asLongLong = 0xFFFF0000002A0002;

		REQUIRE( formula.getErrorValue() == Excel::Formula::NA );
	}

	{
		TestStream stream( &data4[ 0 ], 48 );

		Excel::Record record( stream );

		Excel::Formula formula( record );

		Excel::Record stringRecord( stream );
		std::vector< int32_t > borders;

		formula.setString( Excel::loadString( stringRecord.dataStream(),
			borders ) );

		REQUIRE( formula.valueType() == Excel::Formula::StringValue );
		REQUIRE( formula.getRow() == 0x04 );
		REQUIRE( formula.getColumn() == 0x05 );

		un.asLongLong = 0xFFFF000000000000;

		REQUIRE( formula.getString() == L"this is red ink" );
	}

	{
		TestStream stream( &data5[ 0 ], 26 );

		Excel::Record record( stream );

		Excel::Formula formula( record );

		REQUIRE( formula.valueType() == Excel::Formula::BooleanValue );
		REQUIRE( formula.getRow() == 0x02 );
		REQUIRE( formula.getColumn() == 0x03 );

		un.asLongLong = 0xFFFF000000010001;

		REQUIRE( formula.getBoolean() == false );
	}
}
