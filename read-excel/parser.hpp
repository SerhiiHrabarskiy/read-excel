
/*
	SPDX-FileCopyrightText: 2011-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: MIT
*/

#pragma once

// C++ include.
#include <string>

// Excel include.
#include "storage.hpp"
#include "sheet.hpp"

#include "compoundfile/compoundfile.hpp"
#include "compoundfile/compoundfile_exceptions.hpp"

namespace Excel {

//
// Parser
//

//! Parser of XLS file.
class Parser final {
public:
	//! Load WorkBook from stream.
	static void loadBook( std::istream & fileStream, IStorage & storage,
		const std::string & fileName = "<custom-stream>" );

	//! Store document date mode.
	static void handleDateMode( Record & r, IStorage & storage );

	//! Load sheets from file.
	static void loadGlobals( std::vector< BoundSheet > & boundSheets, 
		Stream & stream, IStorage & storage );

	//! Load boundsheet.
	static BoundSheet parseBoundSheet( Record & record, BOF::BiffVersion ver );

	//! Load WorkSheets.
	static void loadWorkSheets( const std::vector< BoundSheet > & boundSheets,
		Stream & stream, IStorage& storage );

	//! Parse shared string table.
	static void parseSST( Record & record, IStorage & storage );

	//! Load WorkSheet.
	static void loadSheet( size_t sheetIdx, const BoundSheet & boundSheet,
		Stream & stream, IStorage & storage );

	//! Handle label SST.
	static void handleLabelSST( Record & record, size_t sheetIdx, IStorage & storage );

	//! Handle label.
	static void handleLabel( Record & record, size_t sheetIdx, IStorage & storage );

	//! Handle RK.
	static void handleRK( Record & record, size_t sheetIdx, IStorage & storage );

	//! Handle MULRK.
	static void handleMULRK( Record & record, size_t sheetIdx, IStorage & storage );

	//! Handle NUMBER.
	static void handleNUMBER( Record & record, size_t sheetIdx, IStorage & storage );

	//! Handle FORMULA.
	static void handleFORMULA( Record & record, Stream & stream, size_t sheetIdx,
		IStorage & storage );
}; // class Parser

inline void
Parser::loadBook( std::istream & fileStream, IStorage & storage,
	const std::string & fileName )
{
	static_assert( sizeof( double ) == 8,
		"Unsupported platform: double has to be 8 bytes." );

	try {
		CompoundFile::File file( fileStream, fileName );
		auto stream = file.stream( 
			file.hasDirectory( L"Workbook" ) ? file.directory( L"Workbook" ) 
			                                 : file.directory( L"Book") );

		std::vector< BoundSheet > boundSheets;

		loadGlobals( boundSheets, *stream, storage );

		loadWorkSheets( boundSheets, *stream, storage );
	}
	catch( const CompoundFile::Exception & x )
	{
		throw Exception( x.whatAsWString() );
	}
}

inline void
Parser::handleDateMode( Record & r, IStorage & storage )
{
	uint16_t mode = 0;

	r.dataStream().read( mode, 2 );

	storage.onDateMode( mode );
}

inline void
Parser::loadGlobals( std::vector< BoundSheet > & boundSheets, 
	Stream & stream, IStorage & storage )
{
	BOF bof;

	while( true )
	{
		Record r( stream );

		switch( r.code() )
		{
			case XL_BOF :
				bof.parse( r );
				break;

			case XL_FILEPASS :
				throw Exception( L"This file is protected. Decryption is not implemented yet." );

			case XL_SST :
				parseSST( r, storage );
				break;

			case XL_BOUNDSHEET :
				boundSheets.push_back( parseBoundSheet( r, bof.version() ) );
				break;

			case XL_DATEMODE :
				handleDateMode( r, storage );
				break;

			case XL_EOF :
				return;

			case XL_UNKNOWN :
				throw Exception( L"Wrong format." );

			default:
				break;
		}
	}
}

inline BoundSheet
Parser::parseBoundSheet( Record & record, BOF::BiffVersion ver )
{
	int32_t pos = 0;
	int16_t sheetType = 0;
	std::wstring sheetName;

	record.dataStream().read( pos, 4 );

	record.dataStream().read( sheetType, 2 );

	sheetName = loadString( record.dataStream(), record.borders(), 1, ver );

	return BoundSheet( pos,
		BoundSheet::convertSheetType( sheetType ),
		sheetName );
}

inline void
Parser::loadWorkSheets( const std::vector< BoundSheet > & boundSheets,
	Stream & stream, IStorage& storage )
{
	for( size_t i = 0; i < boundSheets.size(); ++i )
	{
		if( boundSheets[i].sheetType() == BoundSheet::WorkSheet )
		{
			storage.onSheet( i, boundSheets[i].sheetName() );
			loadSheet( i, boundSheets[i], stream, storage );
		}
	}
}

inline void
Parser::parseSST( Record & record, IStorage & storage )
{
	int32_t totalStrings = 0;
	int32_t uniqueStrings = 0;

	record.dataStream().read( totalStrings, 4 );
	record.dataStream().read( uniqueStrings, 4 );

	std::vector< std::wstring > sst( uniqueStrings );

	for( int32_t i = 0; i < uniqueStrings; ++i )
		storage.onSharedString( uniqueStrings, i,
			loadString( record.dataStream(), record.borders() ) );
}

inline void
Parser::loadSheet( size_t sheetIdx, const BoundSheet & boundSheet,
	Stream & stream, IStorage & storage )
{
	stream.seek( boundSheet.BOFPosition(), Stream::FromBeginning );
	BOF bof;

	{
		Record record( stream );

		bof.parse( record );
	}

	if( bof.version() != BOF::BIFF8 )
		throw Exception( L"Unsupported BIFF version. BIFF8 is supported only." );

	while( true )
	{
		Record record( stream );

		switch( record.code() )
		{
			case XL_LABELSST :
				handleLabelSST( record, sheetIdx, storage );
				break;

			case XL_LABEL :
				handleLabel( record, sheetIdx, storage );
				break;

			case XL_RK :
			case XL_RK2 :
				handleRK( record, sheetIdx, storage );
				break;

			case XL_MULRK :
				handleMULRK( record, sheetIdx, storage );
				break;

			case XL_NUMBER :
				handleNUMBER( record, sheetIdx, storage );
				break;

			case XL_FORMULA :
				handleFORMULA( record, stream, sheetIdx, storage );
				break;

			case XL_EOF :
				return;

			case XL_UNKNOWN :
				throw Exception( L"Wrong format." );

			default:
				break;
		}
	}
}

inline void
Parser::handleLabelSST( Record & record, size_t sheetIdx, IStorage & storage )
{
	int16_t row = 0;
	int16_t column = 0;
	int16_t xfIndex = 0;
	int32_t sstIndex = 0;

	record.dataStream().read( row, 2 );
	record.dataStream().read( column, 2 );
	record.dataStream().read( xfIndex, 2 );
	record.dataStream().read( sstIndex, 4 );

	storage.onCellSharedString( sheetIdx, row, column, sstIndex );
}

inline void
Parser::handleLabel( Record & record, size_t sheetIdx, IStorage & storage )
{
	int16_t row = 0;
	int16_t column = 0;

	record.dataStream().read( row, 2 );
	record.dataStream().read( column, 2 );
	record.dataStream().seek( 2, Excel::Stream::FromCurrent );
	const auto data = loadString( record.dataStream(), record.borders(), 2, BOF::BIFF7 );

	storage.onCell( sheetIdx, row, column, data );
}

//
// doubleFromRK
//

inline double
doubleFromRK( uint32_t rk )
{
	double num = 0;

	if( rk & 0x02 )
	{
		// int32_t
		num = (double) (rk >> 2);
	}
	else
	{
		// hi words of IEEE num
		*((uint32_t *)&num+1) = rk & 0xFFFFFFFC;
		*((uint32_t *)&num) = 0;
	}

	if( rk & 0x01 )
		// divide by 100
		num /= 100;

	return num;
} // doubleFromRK

inline void
Parser::handleRK( Record & record, size_t sheetIdx, IStorage & storage )
{
	int16_t row = 0;
	int16_t column = 0;
	uint32_t rk = 0;

	record.dataStream().read( row, 2 );
	record.dataStream().read( column, 2 );

	record.dataStream().seek( 2, Stream::FromCurrent );
	record.dataStream().read( rk, 4 );

	storage.onCell( sheetIdx, row, column, doubleFromRK( rk ) );
}

inline void
Parser::handleMULRK( Record & record, size_t sheetIdx, IStorage & storage )
{
	int16_t row = 0;
	int16_t colFirst = 0;
	int16_t colLast = 0;

	record.dataStream().read( row, 2 );
	record.dataStream().read( colFirst, 2 );

	int32_t pos = record.dataStream().pos();

	record.dataStream().seek( -2, Stream::FromEnd );

	record.dataStream().read( colLast, 2 );

	record.dataStream().seek( pos, Stream::FromBeginning );

	const int16_t rkCount = colLast - colFirst + 1;

	for( int16_t i = 0; i < rkCount; ++i )
	{
		uint32_t rk = 0;

		record.dataStream().seek( 2, Stream::FromCurrent );

		record.dataStream().read( rk, 4 );

		storage.onCell( sheetIdx, row, colFirst + i, doubleFromRK( rk ) );
	}
}

inline void
Parser::handleNUMBER( Record & record, size_t sheetIdx, IStorage & storage )
{
	int16_t row = 0;
	int16_t column = 0;

	record.dataStream().read( row, 2 );
	record.dataStream().read( column, 2 );

	record.dataStream().seek( 2, Stream::FromCurrent );

	union {
		double m_asDouble;
		uint64_t m_asLongLong;
	} doubleAndLongLong;

	record.dataStream().read( doubleAndLongLong.m_asLongLong, 8 );

	storage.onCell( sheetIdx, row, column, doubleAndLongLong.m_asDouble );
}

inline void
Parser::handleFORMULA( Record & record, Stream & stream, size_t sheetIdx, IStorage & storage )
{
	Formula formula( record );

	if( formula.valueType() == Formula::StringValue )
	{
		Record stringRecord( stream );

		std::vector< int32_t > borders;

		formula.setString( loadString( stringRecord.dataStream(), borders ) );
	}

	storage.onCell( sheetIdx, formula );
}

} /* namespace Excel */
