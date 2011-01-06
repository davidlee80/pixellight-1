/*********************************************************\
 *  File: XmlDocument.cpp                                *
 *
 *  Copyright (C) 2002-2011 The PixelLight Team (http://www.pixellight.org/)
 *
 *  This file is part of PixelLight.
 *
 *  PixelLight is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PixelLight is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with PixelLight. If not, see <http://www.gnu.org/licenses/>.
\*********************************************************/


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "PLGeneral/File/File.h"
#include "PLGeneral/Xml/XmlParsingData.h"
#include "PLGeneral/Xml/XmlDeclaration.h"
#include "PLGeneral/Xml/XmlDocument.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace PLGeneral {


//[-------------------------------------------------------]
//[ Private static data                                   ]
//[-------------------------------------------------------]
const String XmlDocument::sErrorString[ErrorStringCount] =
{
	"No error",
	"Error",
	"Failed to open file",
	"Error parsing Element.",
	"Failed to read Element name",
	"Error reading Element value.",
	"Error reading Attributes.",
	"Error: empty tag.",
	"Error reading end tag.",
	"Error parsing Unknown.",
	"Error parsing Comment.",
	"Error parsing Declaration.",
	"Error document empty.",
	"Error null (0) or unexpected EOF found in input stream.",
	"Error parsing CDATA.",
	"Error when XmlDocument added to document, because XmlDocument can only be at the root."
};


//[-------------------------------------------------------]
//[ Public functions                                      ]
//[-------------------------------------------------------]
/**
*  @brief
*    Default constructor
*/
XmlDocument::XmlDocument() : XmlNode(Document),
	m_bError(false),
	m_nErrorID(-1),
	m_nTabSize(4),
	m_bUseMicrosoftBOM(false)
{
}

/**
*  @brief
*    Constructor
*/
XmlDocument::XmlDocument(const String &sName) : XmlNode(Document),
	m_bError(false),
	m_nErrorID(-1),
	m_nTabSize(4),
	m_bUseMicrosoftBOM(false)
{
	// Use the name as value
	m_sValue = sName;
}

/**
*  @brief
*    Copy constructor
*/
XmlDocument::XmlDocument(const XmlDocument &cSource) : XmlNode(Document)
{
	*this = cSource;
}

/**
*  @brief
*    Destructor
*/
XmlDocument::~XmlDocument()
{
}

/**
*  @brief
*    Copy operator
*/
XmlDocument &XmlDocument::operator =(const XmlDocument &cSource)
{
	// Delete all the children of this node
	Clear();

	// Copy data
	m_sValue			= cSource.m_sValue;
	m_bError			= cSource.m_bError;
	m_nErrorID			= cSource.m_nErrorID;
	m_sErrorDescription	= cSource.m_sErrorDescription;
	m_cErrorCursor		= cSource.m_cErrorCursor;
	m_nTabSize			= cSource.m_nTabSize;
	m_bUseMicrosoftBOM	= cSource.m_bUseMicrosoftBOM;
	for (const XmlNode *pNode=cSource.GetFirstChild(); pNode; pNode=pNode->GetNextSibling()) {
		XmlNode *pClone = pNode->Clone();
		if (pClone)
			LinkEndChild(*pClone);
	}
	return *this;
}

/**
*  @brief
*    Load a file using the current document value
*/
bool XmlDocument::Load(EEncoding nEncoding)
{
	return Load(m_sValue, nEncoding);
}

/**
*  @brief
*    Load a file using the given filename
*/
bool XmlDocument::Load(const String &sFilename, EEncoding nEncoding)
{
	// Check parameter
	if (sFilename.GetLength()) {
		// Set document value
		SetValue(sFilename);

		// Open file in binary mode so that the parser can normalize the end of lines (EOL)
		File cFile(sFilename);
		if (cFile.Exists() && cFile.Open(File::FileRead)) {
			return Load(cFile);
		} else {
			SetError(ErrorOpeningFile, 0, 0, EncodingUnknown);
		}
	}

	// Error!
	return false;
}

/**
*  @brief
*    Loads from a given file
*/
bool XmlDocument::Load(File &cFile, EEncoding nEncoding)
{
	// Set document value
	SetValue(cFile.GetUrl().GetNativePath());

	// Get data
	const uint32 nFileSize = cFile.GetSize();
	if (nFileSize) {
		char *pszData = new char[nFileSize+1];
		cFile.Read(pszData, 1, nFileSize);
		pszData[nFileSize] = '\0';

		// Process the buffer in place to normalize new lines. (See comment above.)
		// Copies from the 'pszData' to 'pszWriteData' pointer, where pszData can advance faster if
		// a newline-carriage return is hit.
		//
		// Wikipedia:
		// Systems based on ASCII or a compatible character set use either LF  (Line feed, '\n', 0x0A, 10 in decimal) or
		// CR (Carriage return, '\r', 0x0D, 13 in decimal) individually, or CR followed by LF (CR+LF, 0x0D 0x0A)...
		//        * LF:    Multics, Unix and Unix-like systems (GNU/Linux, AIX, Xenix, Mac OS X, FreeBSD, etc.), BeOS, Amiga, RISC OS, and others
		//        * CR+LF: DEC RT-11 and most other early non-Unix, non-IBM OSes, CP/M, MP/M, DOS, OS/2, Microsoft Windows, Symbian OS
		//        * CR:    Commodore 8-bit machines, Apple II family, Mac OS up to version 9 and OS-9
		const char *pszReadData = pszData;	// The read head
		char *pszWriteData = pszData;		// The write head
		const char CR = 0x0d;
		const char LF = 0x0a;
		while (*pszReadData) {
			if (*pszReadData == CR) {
				*pszWriteData++ = LF;
				pszReadData++;
				if (*pszReadData == LF)	// Check for CR+LF (and skip LF)
					pszReadData++;
			} else {
				*pszWriteData++ = *pszReadData++;
			}
		}
		*pszWriteData = '\0';

		// Parse data
		Parse(pszData, nullptr, nEncoding);

		// Cleanup the data
		delete [] pszData;

		// Done
		return !Error();
	} else {
		// Done
		return true;
	}
}

/**
*  @brief
*    Save a file using the current document value
*/
bool XmlDocument::Save()
{
	return Save(m_sValue);
}

/**
*  @brief
*    Save a file using the given filename
*/
bool XmlDocument::Save(const String &sFilename)
{
	// Open the file
	File cFile(sFilename);
	if (cFile.Open(File::FileWrite | File::FileCreate)) {
		// Set document value
		SetValue(sFilename);

		// Save
		Save(cFile);

		// Done
		return true;
	} else {
		// Error!
		SetError(ErrorOpeningFile, 0, 0, EncodingUnknown);
		return false;
	}
}

/**
*  @brief
*    Get the root element -- the only top level element -- of the document
*/
XmlElement *XmlDocument::GetRootElement()
{
	return GetFirstChildElement();
}

const XmlElement *XmlDocument::GetRootElement() const
{
	return GetFirstChildElement();
}

/**
*  @brief
*    If an error occurs, error will be set to true
*/
bool XmlDocument::Error() const
{
	return m_bError;
}

/**
*  @brief
*    Contains a textual (english) description of the error if one occurs
*/
String XmlDocument::GetErrorDesc(bool bLocation) const
{
	if (bLocation) {
		String sString = m_sErrorDescription;
		if (GetErrorRow() > 0)
			sString += String::Format(" Row: %d", GetErrorRow());
		if (GetErrorColumn() > 0)
			sString += String::Format(" Column: %d", GetErrorColumn());
		return sString;
	} else {
		return m_sErrorDescription;
	}
}

/**
*  @brief
*    Generally, you probably want the error string ('GetErrorDesc()') - but if you
*    prefer the error ID, this function will fetch it
*/
int XmlDocument::GetErrorID() const
{
	return m_nErrorID;
}

/**
*  @brief
*    Returns the location (if known) of the error
*/
int XmlDocument::GetErrorRow() const
{
	return m_cErrorCursor.nRow + 1;
}

/**
*  @brief
*    The column where the error occured
*/
int XmlDocument::GetErrorColumn() const
{
	return  m_cErrorCursor.nColumn + 1;
}

/**
*  @brief
*    Returns the tab size
*/
uint32 XmlDocument::GetTabSize() const
{
	return m_nTabSize;
}

/**
*  @brief
*    Sets the tab size
*/
void XmlDocument::SetTabSize(uint32 nTabSize)
{
	m_nTabSize = nTabSize;
}

/**
*  @brief
*    If you have handled the error, it can be reset with this call
*/
void XmlDocument::ClearError()
{
	m_bError			= false;
	m_nErrorID			= 0;
	m_sErrorDescription = "";
	m_cErrorCursor.nRow = m_cErrorCursor.nColumn = 0;
}


//[-------------------------------------------------------]
//[ Public virtual XmlBase functions                      ]
//[-------------------------------------------------------]
bool XmlDocument::Save(File &cFile, uint32 nDepth)
{
	for (XmlNode *pNode=GetFirstChild(); pNode; pNode=pNode->GetNextSibling()) {
		pNode->Save(cFile, nDepth);
		cFile.PutC('\n');
	}

	// Done
	return true;
}

String XmlDocument::ToString(uint32 nDepth) const
{
	String sXml;
	for (const XmlNode *pNode=GetFirstChild(); pNode; pNode=pNode->GetNextSibling())
		sXml += pNode->ToString(nDepth);
	return sXml;
}

const char *XmlDocument::Parse(const char *pszData, XmlParsingData *pData, EEncoding nEncoding)
{
	ClearError();

	// Parse away, at the document level. Since a document contains nothing but other tags, most of what happens here is skipping white space
	if (!pszData || !*pszData) {
		// Set error code
		SetError(ErrorDocumentEmpty, 0, 0, EncodingUnknown);

		// Error!
		return nullptr;
	}

	// Note that, for a document, this needs to come before the while space skip, so that parsing starts from the pointer we are given
	m_cCursor.Clear();
	if (pData) {
		m_cCursor.nRow    = pData->m_cCursor.nRow;
		m_cCursor.nColumn = pData->m_cCursor.nColumn;
	} else {
		m_cCursor.nRow    = 0;
		m_cCursor.nColumn = 0;
	}
	XmlParsingData cXmlParsingData(pszData, m_nTabSize, m_cCursor.nRow, m_cCursor.nColumn);
	m_cCursor = cXmlParsingData.Cursor();

	if (nEncoding == EncodingUnknown) {
		// Check for the Microsoft UTF-8 lead bytes
		const unsigned char *pU = (const unsigned char*)pszData;
		if (	*(pU+0) && *(pU+0) == UTF_LEAD_0
			 && *(pU+1) && *(pU+1) == UTF_LEAD_1
			 && *(pU+2) && *(pU+2) == UTF_LEAD_2 ) {
			nEncoding = EncodingUTF8;
			m_bUseMicrosoftBOM = true;
		}
	}

	pszData = SkipWhiteSpace(pszData, nEncoding);
	if (!pszData) {
		// Set error code
		SetError(ErrorDocumentEmpty, 0, 0, EncodingUnknown);

		// Error!
		return nullptr;
	}

	while (pszData && *pszData) {
		XmlNode *pNode = Identify(pszData, nEncoding);
		if (pNode) {
			pszData = pNode->Parse(pszData, &cXmlParsingData, nEncoding);
			LinkEndChild(*pNode);
		} else {
			// Get us out of here right now!
			break;
		}

		// Did we get nEncoding info?
		if (nEncoding == EncodingUnknown && pNode->ToDeclaration()) {
			const String sEncoding = pNode->ToDeclaration()->GetEncoding();
			if (!sEncoding.GetLength())
				nEncoding = EncodingUTF8;
			else if (StringEqual(sEncoding, "UTF-8", true, EncodingUnknown))
				nEncoding = EncodingUTF8;
			else if (StringEqual(sEncoding, "UTF8", true, EncodingUnknown))
				nEncoding = EncodingUTF8;	// Incorrect, but be nice
			else
				nEncoding = EncodingLegacy;
		}

		pszData = SkipWhiteSpace(pszData, nEncoding);
	}

	// Was this empty?
	if (!m_pFirstChild) {
		SetError(ErrorDocumentEmpty, 0, 0, nEncoding);

		// Error!
		return nullptr;
	}

	// All is well
	return pszData;
}


//[-------------------------------------------------------]
//[ Public virtual XmlNode functions                      ]
//[-------------------------------------------------------]
XmlNode *XmlDocument::Clone() const
{
	return new XmlDocument(*this);
}


//[-------------------------------------------------------]
//[ Private functions                                     ]
//[-------------------------------------------------------]
/**
*  @brief
*    Sets an error
*/
void XmlDocument::SetError(int nError, const char *pszErrorLocation, XmlParsingData *pData, EEncoding nEncoding)
{
	// The first error in a chain is more accurate - don't set again!
	if (!m_bError) {
		m_bError			= true;
		m_nErrorID			= nError;
		m_sErrorDescription = sErrorString[m_nErrorID];
		m_cErrorCursor.Clear();
		if (pszErrorLocation && pData) {
			pData->Stamp(pszErrorLocation, nEncoding);
			m_cErrorCursor = pData->Cursor();
		}
	}
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // PLGeneral
