
/*
	SPDX-FileCopyrightText: 2011-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: MIT
*/


// Excel include.
#include <read-excel/book.hpp>
#include <read-excel/exceptions.hpp>

// C++ include.
#include <iostream>


int main()
{
	try {
		Excel::Book book( "sample.xls" );

		Excel::Sheet * sheet = book.sheet( 0 );

		std::wcout << L"There is output of the \"sample.xls\" Excel file."
			<< std::endl << std::endl;

		std::wcout << L"A1 : " << sheet->cell( 0, 0 ).getString()
			<< std::endl;
		std::wcout << L"A2 : " << sheet->cell( 1, 0 ).getString()
			<< L" B2 : " << sheet->cell( 1, 1 ).getDouble() << std::endl;
		std::wcout << L"A3 : " << sheet->cell( 2, 0 ).getString()
			<< L" B3 : " << sheet->cell( 2, 1 ).getDouble() << std::endl;
		std::wcout << L"A4 : " << sheet->cell( 3, 0 ).getString()
			<< L" B4 : " << sheet->cell( 3, 1 ).getFormula().getDouble()
			<< std::endl;
		std::wcout << L"A5 : " << sheet->cell( 4, 0 ).getString()
			<< std::endl << L"Date mode is : "
			<< ( book.dateMode() == Excel::Book::DateMode::Dec31_1899 ?
					L"count of days since 31 December 1899 :" :
					L"count of days since 01 January 1904 :" )
			<< L" B5 : " << sheet->cell( 4, 1 ).getDouble()
			<< " days." << std::endl;

		std::wcout << std::endl << L"Thats all. And thanks for using this library."
			<< std::endl;
	}
	catch( const Excel::Exception & x )
	{
		std::wcout << x.whatAsWString() << std::endl;
	}
	catch( const std::exception & )
	{
		std::wcout << L"Can't open file." << std::endl;
	}

	return 0;
}
