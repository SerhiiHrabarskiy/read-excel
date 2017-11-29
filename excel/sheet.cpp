
/*!
	\file
	\brief Excel WorkSheet.

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
#include "sheet.hpp"
#include "record.hpp"
#include "bof.hpp"
#include "formula.hpp"
#include "string.hpp"
#include "exceptions.hpp"


namespace Excel {

//
// BoundSheet
//

//! Sheet's type.
enum SheetType {
	//! Worksheet.
	WorkSheet = 0x0000,
	//! Excel 4.0 macro sheet.
	MacroSheet = 0x0100,
	//! Chart.
	Chart = 0x0200,
	//! Visual Basic module.
	VisualBasicModule = 0x0600
}; // enum SheetType

BoundSheet::BoundSheet( int32_t pos,
	SheetType type, const std::wstring & name )
	:	m_BOFPosition( pos )
	,	m_sheetType( type )
	,	m_sheetName( name )
{
}

int32_t
BoundSheet::BOFPosition() const
{
	return m_BOFPosition;
}

BoundSheet::SheetType
BoundSheet::sheetType() const
{
	return m_sheetType;
}

const std::wstring &
BoundSheet::sheetName() const
{
	return m_sheetName;
}

BoundSheet::SheetType
BoundSheet::convertSheetType( int16_t type )
{
	return ( (SheetType) ( type & 0xFF00 ) );
}


//
// Sheet
//

Sheet::Sheet( const std::vector< std::wstring > & sst )
	:	m_sst( sst )
	,	m_columnsCount( 0 )
{
}

void
Sheet::initCells( size_t rows, size_t columns )
{
	m_cells.resize( rows );

	for( std::vector< std::vector< Cell > >::iterator it = m_cells.begin(),
		last = m_cells.end(); it != last; ++it )
	{
		it->resize( columns );
	}
}

void
Sheet::initCell( size_t row, size_t column )
{
	if( m_cells.size() < row + 1 )
		m_cells.resize( row + 1 );

	if( m_cells[ row ].size() < column + 1 )
	{
		if( column + 1 > m_columnsCount )
			m_columnsCount = column + 1;

		for( std::vector< std::vector< Cell > >::iterator it = m_cells.begin(),
			last = m_cells.end(); it != last; ++it )
		{
			it->resize( m_columnsCount );
		}
	}
}

const Cell &
Sheet::cell( size_t row, size_t column ) const
{
	if( m_cells.size() > 0 )
	{
		if( m_cells.size() - 1 < row || m_cells[ 0 ].size() - 1 < column )
			return m_dummyCell;
		else
			return m_cells[ row ][ column ];
	}

	return m_dummyCell;
}

size_t
Sheet::rowsCount() const
{
	return m_cells.size();
}

size_t
Sheet::columnsCount() const
{
	return m_columnsCount;
}

void
Sheet::load( const BoundSheet & boundSheet,
	Stream & stream )
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
			case XL_DIMENSION :
				//handleDimensions( bof, record );
				break;

			case XL_LABELSST:
				handleLabelSST( record );
				break;

			case XL_RK:
			case XL_RK2:
				handleRK( record );
				break;

			case XL_MULRK:
				handleMULRK( record );
				break;

			case XL_NUMBER:
				handleNUMBER( record );
				break;

			case XL_FORMULA:
				handleFORMULA( record, stream );
				break;

			case XL_EOF:
				return;

			default:
				break;
		}
	}
}

void
Sheet::handleDimensions( const BOF & bof,
	Record & record )
{
	if( bof.version() == BOF::BIFF8 )
	{
		int32_t firstRow = 0;
		int32_t lastRow = 0;
		int16_t firstColumn = 0;
		int16_t lastColumn = 0;

		record.dataStream().read( firstRow, 4 );
		record.dataStream().read( lastRow, 4 );
		record.dataStream().read( firstColumn, 2 );
		record.dataStream().read( lastColumn, 2 );

		initCells( lastRow - 1, lastColumn - 1 );
	}
	else
	{
		int16_t firstRow = 0;
		int16_t lastRow = 0;
		int16_t firstColumn = 0;
		int16_t lastColumn = 0;

		record.dataStream().read( firstRow, 2 );
		record.dataStream().read( lastRow, 2 );
		record.dataStream().read( firstColumn, 2 );
		record.dataStream().read( lastColumn, 2 );

		initCells( lastRow - 1, lastColumn - 1 );
	}
}

void
Sheet::handleLabelSST( Record & record )
{
	int16_t row = 0;
	int16_t column = 0;
	int16_t xfIndex = 0;
	int32_t sstIndex = 0;

	record.dataStream().read( row, 2 );
	record.dataStream().read( column, 2 );
	record.dataStream().read( xfIndex, 2 );
	record.dataStream().read( sstIndex, 4 );

	initCell( row, column );

	m_cells[ row ][ column ].setData( m_sst[ sstIndex ] );
}


namespace /* anonymous */ {

//
// doubleFromRK
//

double
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

} /* namespace anonymous */

void
Sheet::handleRK( Record & record )
{
	int16_t row = 0;
	int16_t column = 0;
	uint32_t rk = 0;

	record.dataStream().read( row, 2 );
	record.dataStream().read( column, 2 );

	record.dataStream().seek( 2, Stream::FromCurrent );
	record.dataStream().read( rk, 4 );

	initCell( row, column );

	m_cells[ row ][ column ].setData( doubleFromRK( rk ) );
}

void
Sheet::handleMULRK( Record & record )
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

	initCell( row, colLast );

	for( int16_t i = 0; i < rkCount; ++i )
	{
		uint32_t rk = 0;

		record.dataStream().seek( 2, Stream::FromCurrent );

		record.dataStream().read( rk, 4 );

		m_cells[ row ][ colFirst + i ].setData( doubleFromRK( rk ) );
	}
}

void
Sheet::handleNUMBER( Record & record )
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

	initCell( row, column );

	m_cells[ row ][ column ].setData( doubleAndLongLong.m_asDouble );
}

void
Sheet::handleFORMULA( Record & record, Stream & stream )
{
	Formula formula( record );

	if( formula.valueType() == Formula::StringValue )
	{
		Record stringRecord( stream );

		if( stringRecord.code() == 0x0221 )
			stringRecord = Record( stream );

		std::vector< int32_t > borders;

		formula.setString( loadString( stringRecord.dataStream(), borders ) );
	}

	initCell( formula.getRow(), formula.getColumn() );

	m_cells[ formula.getRow() ][ formula.getColumn() ].setData( formula );
}

} /* namespace Excel */
