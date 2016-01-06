/**
	Basic CSV file reader / writer class
	Version 0.1, 06/01/2016
	-> Crossplatform / standard ASCII support
	-> CSVFile.h

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

#ifndef CSVFILE_H
#define CSVFILE_H

#include <stdlib.h>
#include <stdio.h>

typedef enum {
	CSV_NOERROR = 0,
	CSV_FILEERROR,
	CSV_MEMORYERROR,
	CSV_EOF,
}CSV_ERRORS;

class CSVFile
{
public:
	/**
	 * \fn CSVFile(const char * filename = NULL)
	 * \brief Create an empty CSV file
	 * \param[in] filename CSV file path for read/write operations
	 */
	CSVFile(const char * filename = NULL);

	/**
	 * \fn CSVFile(int noRows, int noColumns, int noComments)
	 * \brief Create a pre-sized CSV file
	 * \param[in] noRows desired number of rows
	 * \param[in] noColumns desired number of columns
	 * \param[in] noComments desired number of comments
	 */
	CSVFile(int noRows, int noColumns, int noComments);

	/**
	 * \fn ~CSVFile()
	 * \brief Release a CSV file
	 */
	~CSVFile();

	/**
	 * \fn CSV_ERRORS read(bool keepInMem = false)
	 * \brief Read a CSV file from disk
	 * \param[in] keepInMem preserve file content in memory for further accesses
	 * \return first error occured while reading
	 */
	CSV_ERRORS read(bool keepInMem = false);

	/**
	 * \fn CSV_ERRORS write()
	 * \brief Write a CSV file to disk
	 * \return first error occured while reading
	 */
	CSV_ERRORS write();

	/**
	 * \fn CSV_ERRORS assess(int & countRows, int & countColumns, int & countComments, int & countLineChars, bool keepInMem = false)
	 * \brief Get usefull statistics about a CSV file without parsing it totally
	 * \param[out] countRows number of rows
	 * \param[out] countColumns number of columns
	 * \param[out] countComments number of comments
	 * \param[out] countLineChars maximum number of characters on a single line
	 * \param[in] keepInMem preserve file content in memory for further accesses
	 * \return first error occured while reading
	 */
	CSV_ERRORS assess(int & countRows, int & countColumns, int & countComments, int & countLineChars, bool keepInMem = false);

	/**
	 * \fn void setFilename(const char * filename)
	 * \brief Set the CSV filename
	 * \param[in] filename CSV file path for read/write operations
	 */
	void setFilename(const char * filename);

	/**
	 * \fn const char * getFilename()
	 * \brief Get the CSV filename
	 * \return CSV file path for read/write operations
	 */
	const char * getFilename() {return path;}

	/**
	 * \fn void setComment(int index, const char * comment)
	 * \brief Add a comment line in the CSV file
	 * \param[in] index index of the comment
	 * \param[in] comment desired comment string
	 */
	void setComment(int index, const char * comment);

	/**
	 * \fn const char * getComment(int index)
	 * \brief Get a comment line from the CSV file
	 * \param[in] index index of the comment
	 * \return desired comment string
	 */
	const char * getComment(int index);

	/**
	 * \fn int getNoComments()
	 * \brief Get the number of comments in the CSV file
	 * \return number of comments
	 */
	int getNoComments() {return noComments;}

	/**
	 * \fn void setSeparator(char separator)
	 * \brief Set the character used to separate the fields (default: ';')
	 * \param[in] separator separator character
	 */
	void setSeparator(char separator) {this->separator = separator;}

	/**
	 * \fn char getSeparator()
	 * \brief Get the character used to separate the fields
	 * \return separator character
	 */
	char getSeparator() {return separator;}

	/**
	 * \fn void setRem(char rem)
	 * \brief Set the character used to indicate a comment (default: '#')
	 * \param[in] rem comment character
	 */
	void setRem(char rem) {this->rem = rem;}

	/**
	 * \fn char getRem()
	 * \brief Get the character used to indicate a comment
	 * \return comment character
	 */
	char getRem() {return rem;}

	/**
	 * \fn void setEOL(const char * eol);
	 * \brief Set the string used to indicate a newline (default: '\r\n')
	 * \param[in] eol null terminated end-of-line string
	 */
	void setEOL(const char * eol);

	/**
	 * \fn const char * getEOL()
	 * \brief Get the string used to indicate a newline
	 * \return null terminated end-of-line string
	 */
	const char * getEOL() { return eol;}

	/**
	 * \fn int getNoRows()
	 * \brief Get the number of rows in the CSV file
	 * \return number of rows
	 */
	int getNoRows() {return noRows;}

	/**
	 * \fn int getNoColumns()
	 * \brief Get the number of columns in the CSV file
	 * \return number of columns
	 */
	int getNoColumns() {return noColumns;}

	/**
	 * \fn void setCell(int row, int column, const char * data);
	 * \brief Set the specified cell string (copy the string)
	 * \param[in] row cell's row
	 * \param[in] column cell's column
	 * \param[in] data cell's string
	 */
	void setCell(int row, int column, const char * data);

	/**
	 * \fn const char * getCell(int row, int column)
	 * \brief Get the specified cell string
	 * \param[in] row cell's row
	 * \param[in] column cell's column
	 * \return desired cell string or null if not set
	 */
	const char * getCell(int row, int column);

private:
	FILE * file;
	char * path;
	char * ramFile;
	long long ramFileLen;

	char *** rows;
	char ** comments;
	char separator;
	char rem;
	char substitute;
	char eol[4];
	int eolLen;
	int noRows, noAllocatedRows;
	int noColumns, noAllocatedColumns;
	int noComments, noAllocatedComments;

private:
	CSV_ERRORS load();
	void unload();

	CSV_ERRORS reallocate(int noRows, int noColumns, int noComments);
	void freeContent();
	void secureString(char * string);

};

#endif