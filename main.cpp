/**
	Basic CSV file reader / writer class
	Version 0.1, 06/01/2016
	-> Crossplatform / standard ASCII support
	-> main.cpp

	The MIT License (MIT)

	Copyright (c) 2016 Frédéric Meslin
	Email: fredericmeslin@hotmail.com
	Website: www.fredslab.net
	Twitter: @marzacdev

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CSVFile.h"

int main(int argc, char * argv[])
{
	printf("Testing constructors / destructors\n");
	CSVFile * csv1 = new CSVFile("csv1.csv");
	delete csv1;
	CSVFile * csv2 = new CSVFile(0, 0, 0);
	delete csv2;
	CSVFile * csv3 = new CSVFile(32, 16, 8);
	delete csv3;

	printf("Testing write\n");
	CSVFile * csv4 = new CSVFile(8, 8, 4);
	csv4->setComment(0, "First comment");
	csv4->setComment(1, "Second comment");
	csv4->setComment(3, "Forth comment after empty comment");
	csv4->setCell(0, 0, "Cell 0x0");
	csv4->setCell(1, 1, "Cell 1x1");
	csv4->setCell(0, 1, "Cell 0x1");
	csv4->setCell(1, 0, "Cell 1x0");
	csv4->setCell(2, 2, "Bad ; , Cell \r\n \n");
	csv4->setCell(0, 1, "Cell 0x1");
	csv4->setFilename("csv4.csv");
	printf("Problem!\n");
	csv4->write();
	delete csv4;

	printf("Testing assess\n");
	CSVFile * csv5 = new CSVFile("csv4.csv");
	int countRows;
	int countColumns;
	int countComments;
	int countLineChars;
	csv5->assess(countRows, countColumns, countComments, countLineChars);
	printf("Rows %i, Columns %i, Comments %i, Chars %i\n", countRows, countColumns, countComments, countLineChars);

	printf("Testing read\n");
	csv5->read();

	printf("Testing re-write\n");
	csv5->setFilename("csv5.csv");
	csv5->write();
	delete csv5;

	printf("End of tests\n");
	return 0;
}
