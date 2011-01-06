/*********************************************************\
 *  File: FileZip.h                                      *
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


#ifndef __PLGENERAL_FILE_ZIP_H__
#define __PLGENERAL_FILE_ZIP_H__
#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "PLGeneral/File/FileImpl.h"
#include "PLGeneral/File/ZipHandle.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace PLGeneral {


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    ZIP implementation of 'FileImpl'
*/
class FileZip : public FileImpl {


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
	friend class FileObject;


	//[-------------------------------------------------------]
	//[ Private functions                                     ]
	//[-------------------------------------------------------]
	private:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] cUrl
		*    URL of the file or directory
		*  @param[in] sZipFile
		*    Path to the ZIP-file
		*  @param[in] sPathInZip
		*    Path inside the ZIP-file
		*  @param[in] pAccess
		*    Additional file access information (can be a null pointer)
		*/
		FileZip(const Url &cUrl, const String &sZipFile, const String &sPathInZip, const FileAccess *pAccess);

		/**
		*  @brief
		*    Destructor
		*/
		virtual ~FileZip();


	//[-------------------------------------------------------]
	//[ Private virtual FileImpl functions                    ]
	//[-------------------------------------------------------]
	private:
		virtual bool Exists() const;
		virtual bool IsFile() const;
		virtual bool IsDirectory() const;
		virtual bool CopyTo(const String &sDest, bool bOverwrite) const;
		virtual bool MoveTo(const String &sDest);
		virtual bool Rename(const String &sName);
		virtual bool CreateNewFile(bool bAlways);
		virtual bool CreateNewDirectory();
		virtual bool Delete();
		virtual bool DeleteDirectory();
		virtual void Close();
		virtual bool Open(uint32 nAccess);
		virtual bool IsOpen() const;
		virtual bool IsReadable() const;
		virtual bool IsWritable() const;
		virtual bool IsEof() const;
		virtual int GetC();
		virtual bool PutC(int nChar);
		virtual String GetS();
		virtual int PutS(const String &sString);
		virtual uint32 Read(void *pBuffer, uint32 nSize, uint32 nCount);
		virtual uint32 Write(const void *pBuffer, uint32 nSize, uint32 nCount);
		virtual bool Flush();
		virtual bool Seek(int32 nOffset, File::ESeek nLocation);
		virtual int32 Tell() const;
		virtual uint32 GetSize() const;
		virtual FileSearchImpl *CreateSearch();


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		String	  m_sZipFile;	/**< File name of ZIP-file */
		String	  m_sPathInZip;	/**< Path inside the ZIP-file */
		uint32	  m_nAccess;	/**< File access modes (see EAccess) */
		ZipHandle m_cZipFile;	/**< ZIP file handle */


};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // PLGeneral


#endif // __PLGENERAL_FILE_ZIP_H__
