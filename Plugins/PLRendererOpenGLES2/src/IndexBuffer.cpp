/*********************************************************\
 *  File: IndexBuffer.cpp                                *
 *
 *  Copyright (C) 2002-2013 The PixelLight Team (http://www.pixellight.org/)
 *
 *  This file is part of PixelLight.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 *  and associated documentation files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all copies or
 *  substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 *  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
\*********************************************************/


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include <PLCore/System/System.h>
#include "PLRendererOpenGLES2/Renderer.h"
#include "PLRendererOpenGLES2/Extensions.h"
#include "PLRendererOpenGLES2/IndexBuffer.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
using namespace PLCore;
namespace PLRendererOpenGLES2 {


//[-------------------------------------------------------]
//[ Public functions                                      ]
//[-------------------------------------------------------]
/**
*  @brief
*    Destructor
*/
IndexBuffer::~IndexBuffer()
{
	Clear();

	// Update renderer statistics
	static_cast<PLRenderer::RendererBackend&>(GetRenderer()).GetWritableStatistics().nIndexBufferNum--;
}

/**
*  @brief
*    Makes this index buffer to the renderers current index buffer
*/
bool IndexBuffer::MakeCurrent()
{
	// Is there an index buffer?
	if (!m_nIndexBuffer && !m_pData)
		return false; // Error!

	// Bind the index buffer
	if (m_nIndexBuffer) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nIndexBuffer);

		// Do we need to update the IBO?
		if (m_bUpdateIBO && m_pData) {
			m_bUpdateIBO = false;

			// Upload new data
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_nSize, m_pData);
		}
	} else if (m_pData) {
		// Nothing to do
	}

	// Done
	return true;
}

/**
*  @brief
*    Returns the dynamic data (none IBO)
*/
void *IndexBuffer::GetDynamicData() const
{
	return m_nIndexBuffer ? nullptr : m_pData;
}


//[-------------------------------------------------------]
//[ Private functions                                     ]
//[-------------------------------------------------------]
/**
*  @brief
*    Constructor
*/
IndexBuffer::IndexBuffer(PLRenderer::Renderer &cRenderer) : PLRenderer::IndexBuffer(cRenderer),
	m_nIndexBuffer(0),
	m_pData(nullptr),
	m_pLockedData(nullptr),
	m_bLockReadOnly(false),
	m_bUpdateIBO(false),
	m_nUsageAPI(0)
{
	// Update renderer statistics
	static_cast<PLRenderer::RendererBackend&>(cRenderer).GetWritableStatistics().nIndexBufferNum++;
}


//[-------------------------------------------------------]
//[ Public virtual PLRenderer::Buffer functions           ]
//[-------------------------------------------------------]
bool IndexBuffer::IsAllocated() const
{
	return (m_nIndexBuffer != 0 || m_pData != nullptr);
}

bool IndexBuffer::Allocate(uint32 nElements, PLRenderer::Usage::Enum nUsage, bool bManaged, bool bKeepData)
{
	// Get the renderer instance
	Renderer &cRenderer = static_cast<Renderer&>(GetRenderer());

	// Get API dependent type
	uint32 nElementSizeAPI;
	if (m_nElementType == UInt) {
		// "GL_OES_element_index_uint"-extension available?
		if (cRenderer.GetContext().GetExtensions().IsGL_OES_element_index_uint()) {
			nElementSizeAPI = sizeof(uint32);
		} else {
			// UInt is not supported by OpenGL ES 2.0

			// Error!
			return false;
		}
	} else if (m_nElementType == UShort) {
		nElementSizeAPI = sizeof(uint16);
	} else if (m_nElementType == UByte) {
		nElementSizeAPI = sizeof(uint8);
	} else {
		 // Error!
		return false;
	}

	// Check if we have to reallocate the buffer
	if (m_nSize == nElementSizeAPI*nElements && nUsage == m_nUsage && m_bManaged == bManaged)
		return true; // Done

	// Get API dependent usage
	if (nUsage == PLRenderer::Usage::Static)
		m_nUsageAPI = GL_STATIC_DRAW;
	else if (nUsage == PLRenderer::Usage::Dynamic)
		m_nUsageAPI = GL_DYNAMIC_DRAW;
	else if (nUsage == PLRenderer::Usage::WriteOnly)
		m_nUsageAPI = GL_STREAM_DRAW;
	else if (nUsage != PLRenderer::Usage::Software) {
		m_nUsageAPI = 0;

		// Error!
		return false;
	} else {
		m_nUsageAPI = 0;
	}

	// Update renderer statistics
	cRenderer.GetWritableStatistics().nIndexBufferMem -= m_nSize;

	// Init data
	uint8 *pDataBackup = nullptr;
	uint32 nSizeBackup = m_nSize;
	if (m_pData && ((m_nSize != nElementSizeAPI*nElements) || (m_bManaged && !bManaged))) {
		// Backup the current data
		if (bKeepData) {
			pDataBackup = new uint8[m_nSize];
			MemoryManager::Copy(pDataBackup, m_pData, m_nSize);
		}

		// Delete data
		if (m_nElementType == UShort)
			delete [] static_cast<uint16*>(m_pData);
		else
			delete [] static_cast<uint8*>(m_pData);
		m_pData = nullptr;
	}
	if (m_nIndexBuffer && nUsage == PLRenderer::Usage::Software) {
		if (bKeepData && !m_pData && !pDataBackup && Lock(PLRenderer::Lock::ReadOnly)) {
			// Backup the current data
			pDataBackup = new uint8[m_nSize];
			MemoryManager::Copy(pDataBackup, GetData(), m_nSize);
			Unlock();
		}

		// Delete index buffer object
		glDeleteBuffers(1, &m_nIndexBuffer);
		m_nIndexBuffer = 0;
	}
	ForceUnlock();
	m_nElements  = nElements;
	m_nSize		 = nElementSizeAPI*m_nElements;
	m_nUsage	 = nUsage;
	m_bManaged   = bManaged;

	// Create the index buffer
	bool bIBO = false;
	if (nUsage != PLRenderer::Usage::Software) {
		// Use IBO
		bIBO = true;
		if (!m_nIndexBuffer)
			glGenBuffers(1, &m_nIndexBuffer);
		GLint nElementArrayBufferBackup;
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &nElementArrayBufferBackup);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nSize, nullptr, m_nUsageAPI);

		// Error checking
		int nArrayObjectSize = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &nArrayObjectSize);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nElementArrayBufferBackup);
		if (nArrayObjectSize <= 0) {
			// Delete index buffer object
			glDeleteBuffers(1, &m_nIndexBuffer);
			m_nIndexBuffer = 0;

			// Error!
			return false;
		}
	}

	// No IBO or managed?
	if (!bIBO || bManaged) {
		if (!m_pData)
			m_pData = new uint8[m_nSize];
		m_bUpdateIBO = true;
	} else {
		m_bUpdateIBO = false;
	}

	// Restore old data if required
	if (pDataBackup) {
		// We can just copy the old data in...
		if (Lock(PLRenderer::Lock::WriteOnly)) {
			uint32 nSize = nSizeBackup;
			if (nSize > m_nSize)
				nSize = m_nSize;
			MemoryManager::Copy(GetData(), pDataBackup, nSize);
			Unlock();
		}

		// Cleanup
		delete [] pDataBackup;
	}

	// Update renderer statistics
	static_cast<PLRenderer::RendererBackend&>(GetRenderer()).GetWritableStatistics().nIndexBufferMem += m_nSize;

	// Done
	return true;
}

bool IndexBuffer::Clear()
{
	if (IsAllocated()) {
		ForceUnlock();
		if (m_nIndexBuffer) {
			glDeleteBuffers(1, &m_nIndexBuffer);
			m_nIndexBuffer = 0;
		}
		if (m_pData) {
			if (m_nElementType == UShort)
				delete [] static_cast<uint16*>(m_pData);
			else
				delete [] static_cast<uint8*>(m_pData);
			m_pData = nullptr;
		}

		// Update renderer statistics
		static_cast<PLRenderer::RendererBackend&>(GetRenderer()).GetWritableStatistics().nIndexBufferMem -= m_nSize;

		// Init
		m_nElements  = 0;
		m_nSize		 = 0;
		m_nUsage	 = PLRenderer::Usage::Unknown;
		m_nUsageAPI  = 0;
		m_bUpdateIBO = false;
	}

	// Done
	return true;
}

void *IndexBuffer::Lock(uint32 nFlag)
{
	// Check whether there's an index buffer
	if (!m_nIndexBuffer && !m_pData)
		return nullptr; // Error!

	// Check whether the index buffer is already locked
	m_nLockCount++;
	if (!m_pLockedData) {
		// Get lock mode
		if (nFlag == PLRenderer::Lock::ReadOnly)
			m_bLockReadOnly = true;
		else if (nFlag == PLRenderer::Lock::WriteOnly)
			m_bLockReadOnly = false;
		else if (nFlag == PLRenderer::Lock::ReadWrite)
			m_bLockReadOnly = false;
		else
			return nullptr; // Error!

		// Map the index buffer
		static_cast<PLRenderer::RendererBackend&>(GetRenderer()).GetWritableStatistics().nIndexBufferLocks++;
		m_nLockStartTime = System::GetInstance()->GetMicroseconds();
		if (m_pData)
			m_pLockedData = m_pData;
		else if (m_nIndexBuffer) {
			// "GL_OES_mapbuffer"-extension available? (there's only support for "write only")
			if (nFlag == PLRenderer::Lock::WriteOnly && static_cast<Renderer&>(GetRenderer()).GetContext().GetExtensions().IsGL_OES_mapbuffer()) {
				// Make this index buffer to the current one
				MakeCurrent();

				// Map
				m_pLockedData = glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
			}
		}

		// Lock valid?
		if (!m_pLockedData)
			m_nLockCount--;
	}

	// Return the locked data
	return m_pLockedData;
}

void *IndexBuffer::GetData()
{
	return m_pLockedData;
}

bool IndexBuffer::Unlock()
{
	// Check whether data is locked
	if (!m_pLockedData)
		return false; // Error!

	// Do we have to unlock the buffer now?
	m_nLockCount--;
	if (m_nLockCount)
		return true; // Nope, it's still used somewhere else...

	// Unlock the index buffer
	if (m_pData) {
		if (m_nIndexBuffer && !m_bLockReadOnly)
			m_bUpdateIBO = true;
	} else {
		// "GL_OES_mapbuffer"-extension available?
		if (m_nIndexBuffer && static_cast<Renderer&>(GetRenderer()).GetContext().GetExtensions().IsGL_OES_mapbuffer()) {
			// Make this index buffer to the current one
			MakeCurrent();

			// Unmap
			glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER);
		}
	}
	static_cast<PLRenderer::RendererBackend&>(GetRenderer()).GetWritableStatistics().nIndexBuffersSetupTime += System::GetInstance()->GetMicroseconds()-m_nLockStartTime;
	m_pLockedData   = nullptr;
	m_bLockReadOnly = false;

	// Done
	return true;
}


//[-------------------------------------------------------]
//[ Private virtual PLRenderer::Resource functions        ]
//[-------------------------------------------------------]
void IndexBuffer::BackupDeviceData(uint8 **ppBackup)
{
	// Backup data
	if (!m_pData && Lock(PLRenderer::Lock::ReadOnly)) {
		*ppBackup = new uint8[m_nSize];
		MemoryManager::Copy(*ppBackup, GetData(), m_nSize);
		ForceUnlock();
	} else {
		*ppBackup = nullptr;
	}

	// Destroy the index buffer
	if (m_nIndexBuffer) {
		glDeleteBuffers(1, &m_nIndexBuffer);
		m_nIndexBuffer = 0;
	}
}

void IndexBuffer::RestoreDeviceData(uint8 **ppBackup)
{
	if (*ppBackup) {
		GLint nElementArrayBufferBackup;
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &nElementArrayBufferBackup);
		glGenBuffers(1, &m_nIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nSize, nullptr, m_nUsageAPI);
		if (Lock(PLRenderer::Lock::WriteOnly)) {
			MemoryManager::Copy(GetData(), *ppBackup, m_nSize);
			Unlock();
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nElementArrayBufferBackup);

		// Delete memory
		delete [] *ppBackup;
	} else {
		if (m_bManaged) {
			GLint nElementArrayBufferBackup;
			glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &nElementArrayBufferBackup);
			glGenBuffers(1, &m_nIndexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nIndexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nSize, nullptr, m_nUsageAPI);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nElementArrayBufferBackup);
			m_bUpdateIBO = true;
		}
	}
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // PLRendererOpenGLES2
