
/*
	SPDX-FileCopyrightText: 2011-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: MIT
*/

#ifndef COMPOUNDFILE__HEADER_HPP__INCLUDED
#define COMPOUNDFILE__HEADER_HPP__INCLUDED

// CompoundFile include.
#include "sat.hpp"
#include "compoundfile_exceptions.hpp"
#include "utils.hpp"

// Excel include.
#include "../stream.hpp"

// C++ include.
#include <istream>
#include <cstring>


namespace CompoundFile {

//
// Header
//

//! Header of the compound file.
class Header {

	friend class File;

public:
	Header();
	Header( std::istream & stream );

	//! \return Byte order in the file.
	Excel::Stream::ByteOrder byteOrder() const;

	//! \return Size of a sector.
	int32_t sectorSize() const;

	//! \return Size of a short-sector.
	int32_t shortSectorSize() const;

	//! \return Total number of sectors used for the sector allocation table.
	int32_t sectorsInSAT() const;

	//! \return SecID of first sector of the directory stream.
	SecID dirStreamSecID() const;

	//! \return Minimum size of a standard stream.
	int32_t streamMinSize() const;

	//! \return SecID of first sector of the short-sector allocation table.
	SecID ssatFirstSecID() const;

	//! \return Total number of sectors used for the short-sector allocation table.
	int32_t sectorsInSSAT() const;

	//! \return SecID of first sector of the master sector allocation table.
	SecID msatFirstSecID() const;

	//! \return Total number of sectors used for the master sector allocation table.
	int32_t sectorsInMSAT() const;

protected:
	//! Load header from the file.
	void load( std::istream & stream );

private:
	//! Byte order.
	Excel::Stream::ByteOrder m_byteOrder;
	//! Size of a sector.
	int32_t m_sectorSize;
	//! Size of a short-sector.
	int32_t m_shortSectorSize;
	//! Total number of sectors used for the sector allocation table.
	int32_t m_sectorsInSAT;
	//! SecID of first sector of the directory stream.
	SecID m_dirStreamSecID;
	//! Minimum size of a standard stream.
	int32_t m_streamMinSize;
	//! SecID of first sector of the short-sector allocation table.
	SecID m_ssatFirstSecID;
	//! Total number of sectors used for the short-sector allocation table.
	int32_t m_sectorsInSSAT;
	//! SecID of first sector of the master sector allocation table.
	SecID m_msatFirstSecID;
	//! Total number of sectors used for the master sector allocation table.
	int32_t m_sectorsInMSAT;
}; // class Header

inline
Header::Header()
	:	m_byteOrder( Excel::Stream::LittleEndian )
	,	m_sectorSize( 0 )
	,	m_shortSectorSize( 0 )
	,	m_sectorsInSAT( 0 )
	,	m_dirStreamSecID( 0 )
	,	m_streamMinSize( 0 )
	,	m_ssatFirstSecID( 0 )
	,	m_sectorsInSSAT( 0 )
	,	m_msatFirstSecID( 0 )
	,	m_sectorsInMSAT( 0 )
{
}

inline
Header::Header( std::istream & stream )
	:	m_byteOrder( Excel::Stream::LittleEndian )
	,	m_sectorSize( 0 )
	,	m_shortSectorSize( 0 )
	,	m_sectorsInSAT( 0 )
	,	m_dirStreamSecID( 0 )
	,	m_streamMinSize( 0 )
	,	m_ssatFirstSecID( 0 )
	,	m_sectorsInSSAT( 0 )
	,	m_msatFirstSecID( 0 )
	,	m_sectorsInMSAT( 0 )
{
	load( stream );
}

inline Excel::Stream::ByteOrder
Header::byteOrder() const
{
	return m_byteOrder;
}

inline int32_t
Header::sectorSize() const
{
	return m_sectorSize;
}

inline int32_t
Header::shortSectorSize() const
{
	return m_shortSectorSize;
}

inline int32_t
Header::sectorsInSAT() const
{
	return m_sectorsInSAT;
}

inline SecID
Header::dirStreamSecID() const
{
	return m_dirStreamSecID;
}

inline int32_t
Header::streamMinSize() const
{
	return m_streamMinSize;
}

inline SecID
Header::ssatFirstSecID() const
{
	return m_ssatFirstSecID;
}

inline int32_t
Header::sectorsInSSAT() const
{
	return m_sectorsInSSAT;
}

inline SecID
Header::msatFirstSecID() const
{
	return m_msatFirstSecID;
}

inline int32_t
Header::sectorsInMSAT() const
{
	return m_sectorsInMSAT;
}

//! Compound document file identifier
static const char cIdentifier[] = {
	static_cast< char > ( 0xD0u ),
	static_cast< char > ( 0xCFu ),
	static_cast< char > ( 0x11u ),
	static_cast< char > ( 0xE0u ),
	static_cast< char > ( 0xA1u ),
	static_cast< char > ( 0xB1u ),
	static_cast< char > ( 0x1Au ),
	static_cast< char > ( 0xE1u ),
	static_cast< char > ( 0x00u ) };

//! Byte order identifier.
static const char cLittleEndian[] = {
	static_cast< char > ( 0xFEu ),
	static_cast< char > ( 0xFFu ),
	static_cast< char > ( 0x00u ) };

inline void
Header::load( std::istream & stream )
{
	{
		char fileID[] =
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

		stream.read( fileID, 8 );

		if( std::strcmp( cIdentifier, fileID ) != 0 )
			throw Exception( L"Wrong file identifier. "
				L"It isn't a compound file." );
	}

	stream.seekg( 20, std::ios::cur );

	{
		char byteOrder[] = { 0x00, 0x00, 0x00 };

		stream.read( byteOrder, 2 );

		if( std::strcmp( byteOrder, cLittleEndian ) == 0 )
			m_byteOrder = Excel::Stream::LittleEndian;
		else
			m_byteOrder = Excel::Stream::BigEndian;
	}

	{
		int16_t sectorSizePower = 0;

		readData( stream, sectorSizePower, 2 );

		m_sectorSize = 1 << sectorSizePower;
	}

	{
		int16_t shortSectorSizePower = 0;

		readData( stream, shortSectorSizePower, 2 );

		m_shortSectorSize = 1 << shortSectorSizePower;
	}

	stream.seekg( 10, std::ios::cur );

	{
		int32_t sectorsInSAT = 0;

		readData( stream, sectorsInSAT, 4 );

		m_sectorsInSAT = sectorsInSAT;
	}

	{
		int32_t dirStreamSecID = 0;

		readData( stream, dirStreamSecID, 4 );

		m_dirStreamSecID = dirStreamSecID;
	}

	stream.seekg( 4, std::ios::cur );

	{
		int32_t streamMinSize = 0;

		readData( stream, streamMinSize, 4 );

		m_streamMinSize = streamMinSize;
	}

	{
		int32_t ssatFirstSecID = 0;

		readData( stream, ssatFirstSecID, 4 );

		m_ssatFirstSecID = ssatFirstSecID;
	}

	{
		int32_t sectorsInSSAT = 0;

		readData( stream, sectorsInSSAT, 4 );

		m_sectorsInSSAT = sectorsInSSAT;
	}

	{
		int32_t msatFirstSecID = 0;

		readData( stream, msatFirstSecID, 4 );

		m_msatFirstSecID = msatFirstSecID;
	}

	{
		int32_t sectorsInMSAT = 0;

		readData( stream, sectorsInMSAT, 4 );

		m_sectorsInMSAT = sectorsInMSAT;
	}
}

} /* namespace CompoundFile */

#endif // COMPOUNDFILE__HEADER_HPP__INCLUDED
