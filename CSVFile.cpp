/**
	Basic CSV file reader / writer class
	Version 0.1, 06/01/2016
    -> Crossplatform / standard ASCII support
    -> CSVFile.cpp
	
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

/*****************************************************************************/
CSVFile::CSVFile(const char * filename) :
	file(NULL), path(NULL),
	ramFile(NULL), ramFileLen(0),
	rows(NULL), comments(NULL),
	separator(';'), rem('#'), substitute(':'),
	noRows(0), noAllocatedRows(0),
	noColumns(0), noAllocatedColumns(0),
	noComments(0), noAllocatedComments(0)
{
	if (!filename) return;
	path = strdup(filename);
	setEOL("\r\n");
}

CSVFile::CSVFile(int noRows, int noColumns, int noComments) :
	file(NULL), path(NULL),
	ramFile(NULL), ramFileLen(0),
	rows(NULL), comments(NULL),
	separator(';'), rem('#'), substitute(':'),
	noRows(0), noAllocatedRows(0),
	noColumns(0), noAllocatedColumns(0),
	noComments(0), noAllocatedComments(0)
{
// Pre-allocate memory
	reallocate(noRows, noColumns, noComments);
	setEOL("\r\n");
}

CSVFile::~CSVFile() 
{
// Close file
	unload();
	if (file) fclose(file);
	if (path) free(path);

// Clean-up content
	freeContent();
	for (int r = 0; r < noAllocatedRows; r++) 
		if (rows[r]) free(rows[r]);
	if (rows) free(rows);
}

/*****************************************************************************/
CSV_ERRORS CSVFile::reallocate(int noRows, int noColumns, int noComments)
{
// Allocate rows
	if (noRows > noAllocatedRows) {
	// Allocate more memory
		int lastNoRows = noAllocatedRows;
		noAllocatedRows = 0;
		rows = (char ***) realloc(rows, sizeof(char *) * noRows);
		if (!rows) return CSV_MEMORYERROR;
		noAllocatedRows = noRows;
		for (int r = lastNoRows; r < noRows; r++)
			rows[r] = NULL;
	}else{
	// Clean-up memory
		for (int r = noRows; r < this->noRows; r++) {
			for (int c = 0; c < this->noColumns; c++) {
				char * p = rows[r][c];
				if (p) {free(p); rows[r][c] = NULL;}
			}
			free(rows[r]);
		}
	}
	this->noRows = noRows;
	
// Allocate columns
	if (noColumns > noAllocatedColumns) {
	// Allocate more memory
		int lastNoColumns = noAllocatedColumns;
		noAllocatedColumns = 0;
		for (int r = 0; r < noRows; r++) {
			rows[r] = (char **) realloc(rows[r], sizeof(char *) * noColumns);
			if (!rows[r]) return CSV_MEMORYERROR;
			for (int c = lastNoColumns; c < noColumns; c++)
				rows[r][c] = NULL;
		}
		noAllocatedColumns = noColumns;
	}else{
	// Clean-up memory
		for (int r = 0; r < this->noRows; r++)
			for (int c = noColumns; c < this->noColumns; c++) {
				char * p = rows[r][c];
				if (p) {free(p); rows[r][c] = NULL;}
			}
	}
	this->noColumns = noColumns;

// Allocate comments
	if (noComments > noAllocatedComments) {
	// Allocate more memory
		int lastNoComments = noAllocatedComments;
		noAllocatedComments = 0;
		comments = (char **) realloc(comments, sizeof(char *) * noComments);
		if (!comments) return CSV_MEMORYERROR;
		noAllocatedComments = noComments;
		for (int c = lastNoComments; c < noComments; c++)
			comments[c] = NULL;
	}else{
	// Clean-up memory
		for (int c = noComments; c < this->noComments; c++) {
			char * p = comments[c];
			if (p) {free(p); comments[c] = NULL;}
		}
	}
	this->noComments = noComments;
	return CSV_NOERROR;
}

void CSVFile::freeContent()
{
// Free cells
	for (int r = 0; r < noAllocatedRows; r++) {
		for (int c = 0; c < noAllocatedColumns; c++) {
			char * p = rows[r][c];
			if (p) {free(p); rows[r][c] = NULL;}
		}
	}
// Free comments	
	for (int c = 0; c < noAllocatedComments; c++) {
		char * p = comments[c];
		if (p) {free(p); comments[c] = NULL;}
	}
}

/*****************************************************************************/
void CSVFile::secureString(char * string)
{
	if (!string) return;
// Substitute characters used to format
	int c;
	while((c = *string++)) {
		if ((c < ' ' && c != '\t') || (c == separator) || (c == rem))
			string[-1] = substitute;
	}
}

/*****************************************************************************/
CSV_ERRORS CSVFile::read(bool keepInMem)
{
// Allocate memory
	int countRows;
	int countColumns;
	int countComments;
	int countLineChars;
	CSV_ERRORS error = assess(countRows, countColumns, countComments, countLineChars, true);
	if (error) return error;
	error = reallocate(countRows, countColumns, countComments);
	if (error) return error;
	freeContent();

// Allocate parsing buffers
	if (!countRows || !countLineChars) return CSV_NOERROR;
	char commentBuffer[8 + countLineChars];
	char cellBuffer[8 + countLineChars];	
	int commentLength = 0;
	int cellLength = 0;
	
// Initialise the parser
	int row = 0;
	int column = 0;
	int comment = 0;
	bool commentOnLine = false;

// Parse the file
	for (long long k = 0; k < ramFileLen; k++) {
		int c = ramFile[k];
		if (c == '\r' || c == '\n') {
		// New CSV line
			if (commentOnLine) {
				if (commentLength) {
					commentBuffer[commentLength] = 0;
					comments[comment++] = strdup(commentBuffer);
					commentLength = 0;
				}else comment++;
			}
			if (cellLength) {
				cellBuffer[cellLength] = 0;
				rows[row][column] = strdup(cellBuffer);
				cellLength = 0;
				column++;
			}
			if (column) row ++;
			commentOnLine = false;
			column = 0;
		}else{
		// Collect the data
			if (commentOnLine) {
				commentBuffer[commentLength++] = c;
			}else{
				if (c == rem) commentOnLine = true;
				else if (c == separator) {
					if (cellLength) {
						cellBuffer[cellLength] = 0;
						rows[row][column] = strdup(cellBuffer);
						cellLength = 0;
					}
					column++;
				}else{
					cellBuffer[cellLength++] = c;
				}
			}
		}
	}
	
// Unload the file
	if (!keepInMem) unload();
	return CSV_NOERROR;
}

CSV_ERRORS CSVFile::write()
{
// Open the CSV file
	file = fopen(path, "wb");
	if (!file) return CSV_FILEERROR;
	clearerr(file);

// Write comments
	for (int c = 0; c < noComments; c++) {
		fwrite(&rem, 1, 1, file);
		if (comments[c]) fwrite(comments[c], strlen(comments[c]), 1, file);
		fwrite(eol, eolLen, 1, file);
	}

// Write rows
	for (int r = 0; r < noRows; r++) {
		for (int c = 0; c < noColumns; c++) {
			if (rows[r][c])	fwrite(rows[r][c], strlen(rows[r][c]), 1, file);
			if (c != noColumns - 1) fwrite(&separator, 1, 1, file);
		}
		fwrite(eol, eolLen, 1, file);
	}

// Detect any error
	if (ferror(file)) {
		fclose(file);
		file = NULL;
		return CSV_FILEERROR;
	}
	
// Close the CSV file	
	fclose(file);
	file = NULL;
	return CSV_NOERROR;
}

/*****************************************************************************/
CSV_ERRORS CSVFile::assess(int & countRows, int & countColumns, int & countComments, int & countLineChars, bool keepInMem)
{
// Load the file
	load();

// Count all elements	
	int cRow = 0;
	int cColumn = 0;
	int cColumnLocal = 0;
	int cComment = 0;
	int lastLine = 0;
	int lineMaxLen = 0;
	bool commentOnLine = false;
	bool emptyLine = true;
	for (long long k = 0; k < ramFileLen; k++) {
		int c = ramFile[k];
		if (c == '\r' || c == '\n') {
		// Count rows and columns
			if (!emptyLine) cRow ++;
			if (++ cColumnLocal > cColumn)
				cColumn = cColumnLocal;
				
		// Count line length
			lastLine = k - lastLine;
			if (lastLine > lineMaxLen) 
				lineMaxLen = lastLine;
			lastLine = k;
			
		// Prepare next line
			cColumnLocal = 0;
			commentOnLine = false;
			emptyLine = true;
		}else if (c != ' ' || c != '\t') {
		// Detect content
			if (!commentOnLine) {
				if (c == rem) {
					commentOnLine = true;
					cComment ++;
				}else if (c == separator) {
					cColumnLocal ++;
					emptyLine = false;
				}else emptyLine = false;
			}
		}
	}
	
// Unload the file
	if (!keepInMem) unload();
	countRows = cRow;
	countColumns = cColumn;
	countComments = cComment;
	countLineChars = lineMaxLen;
	return CSV_NOERROR;
}

/*****************************************************************************/
CSV_ERRORS CSVFile::load()
{
// Open the CSV file
	if (ramFile) return CSV_NOERROR;
	file = fopen(path, "rb");
	if (!file) return CSV_FILEERROR;
	
// Allocate file buffer
	fseek(file, 0, SEEK_END);
	long long len = ftell(file);
	ramFile = (char *) malloc(len);
	if (!ramFile) return CSV_MEMORYERROR;
	ramFileLen = len;
	
// Load complete file
	fseek(file, 0, SEEK_SET);
	fread(ramFile, ramFileLen, 1, file);
	
// Detect any error
	if (ferror(file)) {
		unload();
		fclose(file);
		file = NULL;
		return CSV_FILEERROR;
	}
	
// Close the CSV file	
	fclose(file);
	file = NULL;
	return CSV_NOERROR;
}

void CSVFile::unload()
{
	if (!ramFile) return;
	free(ramFile);
	ramFile = NULL;
	ramFileLen = 0;
}

/*****************************************************************************/
void CSVFile::setFilename(const char * filename)
{
	if (path) free(path);
	if (filename) path = strdup(filename);
}

void CSVFile::setEOL(const char * eol)
{
	strncpy(this->eol, eol, 4);
	this->eol[3] = 0;
	eolLen = strlen(this->eol);
}

/*****************************************************************************/	
void CSVFile::setComment(int index, const char * comment)
{
	if (index < 0 || index >= noComments) return;
	if (comments[index]) free(comments[index]);
	char * ns = strdup(comment);
	secureString(ns);
	comments[index] = ns;
}

const char * CSVFile::getComment(int index)
{
	if (index < 0 || index >= noComments) return NULL;
	return comments[index];
}

/*****************************************************************************/
const char * CSVFile::getCell(int row, int column)
{
	if (row < 0 || row >= noRows) return NULL;
	if (column < 0 || column >= noColumns) return NULL;
	return rows[row][column];
}

void CSVFile::setCell(int row, int column, const char * data)
{
	if (row < 0 || row >= noRows) return;
	if (column < 0 || column >= noColumns) return;
	if (rows[row][column]) free(rows[row][column]);
	char * ns = strdup(data);
	secureString(ns);
	rows[row][column] = ns;
}
	
