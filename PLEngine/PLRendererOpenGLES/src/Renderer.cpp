/*********************************************************\
 *  File: Renderer.cpp                                   *
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
#include <GLES2/gl2.h>
#include <PLGeneral/Log/Log.h>
#include <PLGeneral/Tools/Tools.h>
#include <PLGraphics/Image/Image.h>
#include <PLGraphics/Image/ImagePart.h>
#include <PLGraphics/Image/ImageBuffer.h>
#include <PLGraphics/Image/ImageEffects.h>
#include <PLRenderer/Renderer/SurfaceWindowHandler.h>
#include <PLRenderer/Renderer/Backend/DrawHelpersBackend.h>
#include "PLRendererOpenGLES/SurfaceWindow.h"
#include "PLRendererOpenGLES/SurfaceTextureBuffer.h"
#include "PLRendererOpenGLES/TextureBuffer2D.h"
#include "PLRendererOpenGLES/TextureBuffer3D.h"
#include "PLRendererOpenGLES/TextureBufferCube.h"
#include "PLRendererOpenGLES/IndexBuffer.h"
#include "PLRendererOpenGLES/VertexBuffer.h"
#include "PLRendererOpenGLES/ProgramGLSL.h"
#include "PLRendererOpenGLES/FontManager.h"
#include "PLRendererOpenGLES/ShaderLanguageGLSL.h"
#include "PLRendererOpenGLES/Renderer.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
using namespace PLGeneral;
using namespace PLMath;
using namespace PLGraphics;
namespace PLRendererOpenGLES {


//[-------------------------------------------------------]
//[ RTTI interface                                        ]
//[-------------------------------------------------------]
pl_implement_class(Renderer)


//[-------------------------------------------------------]
//[ Public functions                                      ]
//[-------------------------------------------------------]
/**
*  @brief
*    Default constructor
*/
Renderer::Renderer(EMode nMode, uint32 nZBufferBits, uint32 nStencilBits, uint32 nMultisampleAntialiasingSamples, String sDefaultShaderLanguage) : PLRenderer::RendererBackend(ModeShaders),	// Only shaders mode is supported by OpenGL ES 2.0
	m_pFontManager(new FontManager(*this)),
	m_pShaderLanguageGLSL(new ShaderLanguageGLSL(*this)),
	m_hDisplay(nullptr),
	m_hConfig(nullptr),
	m_hContext(nullptr),
	m_nDummyNativeWindow(nullptr),
	m_hDummySurface(nullptr)
	#ifdef LINUX
		, m_pDisplay(nullptr)
	#endif
{
	// This renderer implementation has just support for GLSL as shader language, so ignore sDefaultShaderLanguage

	// Output log information
	PL_LOG(Info, "Initialize OpenGL ES 2.0 renderer")

	// Set Z buffer bits and stencil buffer bits capabilities
	m_sCapabilities.nZBufferBits = nZBufferBits;
	m_sCapabilities.nStencilBits = nStencilBits;

	// Get X server display connection
	#ifdef LINUX
		m_pDisplay = XOpenDisplay(nullptr);
	#endif

	// Get display
	#ifdef LINUX
		m_hDisplay = eglGetDisplay((EGLNativeDisplayType)m_pDisplay);
	#else
		m_hDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	#endif
	if (m_hDisplay != EGL_NO_DISPLAY) {
		// Initialize EGL
		EGLint nEGLMajorVersion, nEGLMinorVersion;
		if (eglInitialize(m_hDisplay, &nEGLMajorVersion, &nEGLMinorVersion) == EGL_TRUE) {
			{ // Output log information
				PL_LOG(Info, String::Format("Initialized EGL version %d.%d", nEGLMajorVersion, nEGLMinorVersion))
				// EGL_VENDOR
				const char *pszResult = eglQueryString(m_hDisplay, EGL_VENDOR);
				PL_LOG(Info, String::Format("EGL vendor: %s", pszResult ? pszResult : "-"))
				// EGL_VERSION
				pszResult = eglQueryString(m_hDisplay, EGL_VERSION);
				PL_LOG(Info, String::Format("EGL version: %s", pszResult ? pszResult : "-"))
				// EGL_EXTENSIONS
				pszResult = eglQueryString(m_hDisplay, EGL_EXTENSIONS);
				PL_LOG(Info, String::Format("Supported EGL extensions: %s", pszResult ? pszResult : "-"))
				// EGL_CLIENT_APIS
				pszResult = eglQueryString(m_hDisplay, EGL_CLIENT_APIS);
				PL_LOG(Info, String::Format("Supported EGL client rendering APIs: %s", pszResult ? pszResult : "-"))
			}

			// Choose a EGL configuration
			m_hConfig = ChooseConfig(nMultisampleAntialiasingSamples);

			// We can only go on if a EGL configuration was chosen properly
			if (m_hConfig) {
				// Create context
				EGLint pContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
				m_hContext = eglCreateContext(m_hDisplay, m_hConfig, nullptr, pContextAttribs);
				if (m_hContext != EGL_NO_CONTEXT) {
					// Create the dummy native window
					#ifdef WIN32
					{
						HINSTANCE hModuleHandle = GetModuleHandle(nullptr);
						WNDCLASS sWindowClass;
						sWindowClass.hInstance		= hModuleHandle;
						sWindowClass.lpszClassName	= TEXT("PLOpenGLESDummyNativeWindow");
						sWindowClass.lpfnWndProc	= DefWindowProc;
						sWindowClass.style			= 0;
						sWindowClass.hIcon			= nullptr;
						sWindowClass.hCursor		= nullptr;
						sWindowClass.lpszMenuName	= nullptr;
						sWindowClass.cbClsExtra		= 0;
						sWindowClass.cbWndExtra		= 0;
						sWindowClass.hbrBackground	= nullptr;
						RegisterClass(&sWindowClass);
						m_nDummyNativeWindow = CreateWindow(TEXT("PLOpenGLESDummyNativeWindow"), TEXT("PFormat"), WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 8, 8, HWND_DESKTOP, nullptr, hModuleHandle, nullptr);
					}
					#endif
					#ifdef LINUX
						// Create dummy window
						XSetWindowAttributes sSetWindowAttributes;
						sSetWindowAttributes.event_mask   = 0;
						sSetWindowAttributes.border_pixel = 0;
						m_nDummyNativeWindow = XCreateWindow(m_pDisplay, DefaultRootWindow(m_pDisplay), 0, 0, 300, 300, 0,
															 CopyFromParent, InputOutput, CopyFromParent,
															 CWBorderPixel|CWEventMask, &sSetWindowAttributes);
					#endif

					// Create an EGL dummy surface
					m_hDummySurface = eglCreateWindowSurface(m_hDisplay, m_hConfig, m_nDummyNativeWindow, nullptr);
					if (m_hDummySurface == EGL_NO_SURFACE) {
						// Error!
						PL_LOG(Error, "Failed to create EGL dummy surface!")
					}

					// Make the internal dummy surface to the currently used one
					if (MakeCurrent(nullptr) == EGL_FALSE) {
						// Error!
						PL_LOG(Error, "Failed to make the EGL dummy surface to the current one!")
					}

					// Show some OpenGL ES information
					ShowOpenGLESInformation();

					// Setup the renderer capabilities
					SetupCapabilities();

					// Multisample antialiasing enabled properly?
					if (nMultisampleAntialiasingSamples > 1) {
						// Check multisample antialiasing states
						GLint nSampleBuffers = 0;
						glGetIntegerv(GL_SAMPLE_BUFFERS, &nSampleBuffers);
						if (nSampleBuffers != 1) {
							PL_LOG(Warning, String::Format("OpenGL ES 2.0 multisample antialiasing: 1 sample buffers requested but %d sample buffers received", nSampleBuffers))
						} else {
							GLint nSamples = 0;
							glGetIntegerv(GL_SAMPLES, &nSamples);
							if (nSamples != nMultisampleAntialiasingSamples)
								PL_LOG(Warning, String::Format("OpenGL ES 2.0 multisample antialiasing: %d samples per pixel requested but %d samples per pixel received", nMultisampleAntialiasingSamples, nSamples))
						}
					}
					m_sCapabilities.nMultisampleAntialiasingSamples = nMultisampleAntialiasingSamples;

					// Init data
					m_fDepthBoundsZMin = 0.0f;
					m_fDepthBoundsZMax = 0.0f;
					m_nVertexOffset    = 0;

					// The default value of '4' is NOT desired and would make tricky problems (memory leaks and so on)
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
					glPixelStorei(GL_PACK_ALIGNMENT,   1);

					// Init wrappers
					InitWrappers();

					// Allocate sampler states
					m_ppnSamplerState = new uint32*[m_sCapabilities.nMaxTextureUnits];
					for (uint32 i=0; i<m_sCapabilities.nMaxTextureUnits; i++) {
						m_ppnSamplerState[i] = new uint32[PLRenderer::Sampler::Number];
						MemoryManager::Set(m_ppnSamplerState[i], -1, sizeof(uint32)*PLRenderer::Sampler::Number);
					}

					// Allocate internal sampler states
					m_ppnInternalSamplerState = new uint32*[m_sCapabilities.nMaxTextureUnits];
					for (uint32 i=0; i<m_sCapabilities.nMaxTextureUnits; i++) {
						m_ppnInternalSamplerState[i] = new uint32[PLRenderer::Sampler::Number];
						MemoryManager::Set(m_ppnInternalSamplerState[i], -1, sizeof(uint32)*PLRenderer::Sampler::Number);
					}

					// Allocate current stuff
					m_ppCurrentTextureBuffer = new PLRenderer::TextureBuffer*[m_sCapabilities.nMaxTextureUnits];
					for (uint32 i=0; i<m_sCapabilities.nMaxTextureUnits; i++)
						m_ppCurrentTextureBuffer[i] = nullptr;

					// Setup current render states
					for (uint32 i=0; i<PLRenderer::RenderState::Number; i++) {
						uint32 nState = m_nRenderState[i];
						m_nRenderState[i] = m_nRenderState[i]+1;
						SetRenderState((PLRenderer::RenderState::Enum)i, nState);
					}

					// Reset render
					Reset();

					// Done
					return;
				} else {
					// Error!
					PL_LOG(Error, "Failed to create EGL context!")
				}
			} else {
				// Error!
				PL_LOG(Error, "Failed to choose EGL configuration!")
			}
		} else {
			// Error!
			PL_LOG(Error, "Failed to initialize EGL!")
		}

		// Cleanup
		eglTerminate(m_hDisplay);
	} else {
		// Error!
		PL_LOG(Error, "Failed to get EGL default display!")
	}
}

/**
*  @brief
*    Destructor
*/
Renderer::~Renderer()
{
	// Output log information
	PL_LOG(Info, "De-initialize OpenGL ES 2.0 renderer")

	// Destroy all renderer surfaces of this renderer
	while (m_lstSurfaces.GetNumOfElements())
		delete m_lstSurfaces[0];

	// Clear list of display modes
	for (uint32 i=0; i<m_lstDisplayModeList.GetNumOfElements(); i++)
		delete m_lstDisplayModeList[i];
	m_lstDisplayModeList.Clear();

	// Destroy the OpenGL ES renderer font manager while there's still an active OpenGL ES context (... font textures...)
	delete m_pFontManager;
	m_pFontManager = nullptr;

	// Destroy the GLSL shader language instance
	delete m_pShaderLanguageGLSL;
	m_pShaderLanguageGLSL = nullptr;

	// Destroy the draw helpers instance
	delete m_pDrawHelpers;
	m_pDrawHelpers = nullptr;

	// Destroy all renderer resources of this renderer
	while (m_lstResources.GetNumOfElements())
		delete m_lstResources[0];

	// Free sampler states
	if (m_ppnSamplerState) {
		for (uint32 i=0; i<m_sCapabilities.nMaxTextureUnits; i++)
			delete [] m_ppnSamplerState[i];
		delete [] m_ppnSamplerState;
		m_ppnSamplerState = nullptr;
	}

	// Free internal sampler states
	if (m_ppnInternalSamplerState) {
		for (uint32 i=0; i<m_sCapabilities.nMaxTextureUnits; i++)
			delete [] m_ppnInternalSamplerState[i];
		delete [] m_ppnInternalSamplerState;
		m_ppnInternalSamplerState = nullptr;
	}

	// Free current stuff
	if (m_ppCurrentTextureBuffer) {
		delete [] m_ppCurrentTextureBuffer;
		m_ppCurrentTextureBuffer = nullptr;
	}

	// Make 'nothing' current
	eglMakeCurrent(EGL_DEFAULT_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	// Destroy the EGL dummy surface
	if (eglDestroySurface(m_hDisplay, m_hDummySurface) == EGL_FALSE) {
		// Error!
		PL_LOG(Error, "Failed to destroy the used EGL dummy surface!")
	}
	m_hDummySurface = nullptr;

	// Destroy the EGL context
	if (eglDestroyContext(m_hDisplay, m_hContext) == EGL_FALSE) {
		// Error!
		PL_LOG(Error, "Failed to destroy the used EGL context!")
	}
	m_hContext = nullptr;

	// Terminate the EGL display
	if (eglTerminate(m_hDisplay) == EGL_FALSE) {
		// Error!
		PL_LOG(Error, "Failed to terminate the used EGL display!")
	}
	m_hDisplay = nullptr;
	m_hConfig  = nullptr;

	// Destroy the dummy native window
	#ifdef WIN32
		if (m_nDummyNativeWindow) {
			DestroyWindow(m_nDummyNativeWindow);
			UnregisterClass(TEXT("PLOpenGLESDummyNativeWindow"), GetModuleHandle(nullptr));
			m_nDummyNativeWindow = nullptr;
		}
	#endif
	#ifdef LINUX
		// Destroy the dummy native window
		if (m_nDummyNativeWindow)
			XDestroyWindow(m_pDisplay, m_nDummyNativeWindow);

		// Close the X server display connection
		if (m_pDisplay) {
			XCloseDisplay(m_pDisplay);
		}
	#endif

	// Release all resources allocated by the shader compiler
	glReleaseShaderCompiler();

	// Return EGL to it's state at thread initialization
	if (eglReleaseThread() == EGL_FALSE) {
		// Error!
		PL_LOG(Error, "Failed to release the EGL thread!")
	}
}

/**
*  @brief
*    Returns the used EGL display
*/
EGLDisplay Renderer::GetEGLDisplay() const
{
	return m_hDisplay;
}

/**
*  @brief
*    Returns the used EGL config
*/
EGLConfig Renderer::GetEGLConfig() const
{
	return m_hConfig;
}

/**
*  @brief
*    Returns the used EGL context
*/
EGLContext Renderer::GetEGLContext() const
{
	return m_hContext;
}

/**
*  @brief
*    Makes a given EGL surface to the currently used one
*/
EGLBoolean Renderer::MakeCurrent(EGLSurface hEGLSurface)
{
	// Use the EGL dummy surface?
	if (!hEGLSurface)
		hEGLSurface = m_hDummySurface;

	// Make the EGL surface to the current one
	return eglMakeCurrent(m_hDisplay, hEGLSurface, hEGLSurface, m_hContext);
}

/**
*  @brief
*    Returns a PL format as OpenGL ES format
*/
uint32 Renderer::GetOpenGLESPixelFormat(PLRenderer::TextureBuffer::EPixelFormat nFormat) const
{
	switch (nFormat) {
		case PLRenderer::TextureBuffer::L8:
		case PLRenderer::TextureBuffer::L16:
		case PLRenderer::TextureBuffer::L16F:
		case PLRenderer::TextureBuffer::L32F:
			return GL_LUMINANCE;

		case PLRenderer::TextureBuffer::A8:
			return GL_ALPHA;

		case PLRenderer::TextureBuffer::L4A4:
		case PLRenderer::TextureBuffer::L8A8:
			return GL_LUMINANCE_ALPHA;

		case PLRenderer::TextureBuffer::D16:
		case PLRenderer::TextureBuffer::D24:
		case PLRenderer::TextureBuffer::D32:
			return GL_DEPTH_COMPONENT;

		case PLRenderer::TextureBuffer::R8G8B8:
		case PLRenderer::TextureBuffer::R3G3B2:
		case PLRenderer::TextureBuffer::R5G6B5:
		case PLRenderer::TextureBuffer::DXT1:
			return GL_RGB;

		case PLRenderer::TextureBuffer::R5G5B5A1:
		case PLRenderer::TextureBuffer::R4G4B4A4:
		case PLRenderer::TextureBuffer::R8G8B8A8:
		case PLRenderer::TextureBuffer::R10G10B10A2:
		case PLRenderer::TextureBuffer::R16G16B16A16:
		case PLRenderer::TextureBuffer::DXT3:
		case PLRenderer::TextureBuffer::DXT5:
		case PLRenderer::TextureBuffer::R16G16B16A16F:
		case PLRenderer::TextureBuffer::R32G32B32A32F:
		default:
			return GL_RGBA;
	}
}

/**
*  @brief
*    Returns a PL data format as OpenGL ES data format
*/
uint32 Renderer::GetOpenGLESDataFormat(PLRenderer::TextureBuffer::EPixelFormat nFormat) const
{
	switch (nFormat) {
		case PLRenderer::TextureBuffer::D16:
			return GL_UNSIGNED_SHORT;

		case PLRenderer::TextureBuffer::D24:
		case PLRenderer::TextureBuffer::D32:
			return GL_UNSIGNED_INT;

		case PLRenderer::TextureBuffer::L8:
		case PLRenderer::TextureBuffer::A8:
		case PLRenderer::TextureBuffer::L4A4:
		case PLRenderer::TextureBuffer::L8A8:
		case PLRenderer::TextureBuffer::R3G3B2:
		case PLRenderer::TextureBuffer::R5G6B5:
		case PLRenderer::TextureBuffer::R5G5B5A1:
		case PLRenderer::TextureBuffer::R4G4B4A4:
		case PLRenderer::TextureBuffer::R8G8B8:
		case PLRenderer::TextureBuffer::R8G8B8A8:
		case PLRenderer::TextureBuffer::R10G10B10A2:
		case PLRenderer::TextureBuffer::DXT1:
		case PLRenderer::TextureBuffer::DXT3:
		case PLRenderer::TextureBuffer::DXT5:
		case PLRenderer::TextureBuffer::LATC1:
		case PLRenderer::TextureBuffer::LATC2:
			return GL_UNSIGNED_BYTE;

		case PLRenderer::TextureBuffer::L16:
		case PLRenderer::TextureBuffer::R16G16B16A16:
			return GL_UNSIGNED_SHORT;

		case PLRenderer::TextureBuffer::L16F:
		case PLRenderer::TextureBuffer::R16G16B16A16F:
		{
			// [TODO] GL_OES_texture_half_float
			// return GL_HALF_FLOAT_OES;
			return GL_UNSIGNED_BYTE;
		}

		case PLRenderer::TextureBuffer::L32F:
		case PLRenderer::TextureBuffer::R32G32B32A32F:
			return GL_FLOAT;

		default:
			return GL_UNSIGNED_BYTE;
	}
}

/**
*  @brief
*    Chooses the image source and internal texture buffer formats depending on various input parameters
*/
PLRenderer::TextureBuffer::EPixelFormat Renderer::ChooseFormats(PLGraphics::Image &cImage, PLRenderer::TextureBuffer::EPixelFormat nInternalFormat, uint32 nFlags,
																PLRenderer::TextureBuffer::EPixelFormat &nImageFormat, bool &bUsePreCompressedData) const
{
	PLRenderer::TextureBuffer::EPixelFormat nChosenInternalFormat = PLRenderer::TextureBuffer::Unknown;

	// Get image pixel format
	nImageFormat = PLRenderer::TextureBuffer::GetFormatFromImage(cImage, !(nFlags & PLRenderer::TextureBuffer::Compression));
	bUsePreCompressedData = false;
	if (PLRenderer::TextureBuffer::IsCompressedFormat(nImageFormat)) {
		// Currently, we don't support texture compression within the OpenGL ES renderer
		// Do not use texture buffer compression
		nImageFormat = PLRenderer::TextureBuffer::GetFormatFromImage(cImage, true);
	}

	// Get internal pixel format
	if (nInternalFormat != PLRenderer::TextureBuffer::Unknown) {
		nChosenInternalFormat = nInternalFormat;
		if (PLRenderer::TextureBuffer::IsCompressedFormat(nChosenInternalFormat)) {
			// Hm, the user want's to use a certain compressed format, but the desired format is NOT available...
			// we have to choose a fallback format.
			switch (nChosenInternalFormat) {
				case PLRenderer::TextureBuffer::LATC1:
					nChosenInternalFormat = PLRenderer::TextureBuffer::L8;
					break;

				case PLRenderer::TextureBuffer::LATC2:
					nChosenInternalFormat = PLRenderer::TextureBuffer::L8A8;
					break;

				case PLRenderer::TextureBuffer::DXT1:
					nChosenInternalFormat = PLRenderer::TextureBuffer::R8G8B8;
					break;

				case PLRenderer::TextureBuffer::DXT3:
				case PLRenderer::TextureBuffer::DXT5:
					nChosenInternalFormat = PLRenderer::TextureBuffer::R8G8B8A8;
					break;
			}
		}
	} else {
		if (nImageFormat == PLRenderer::TextureBuffer::Unknown)
			return PLRenderer::TextureBuffer::Unknown; // Error!
		nChosenInternalFormat = nImageFormat;
	}

	// Return the chosen internal format
	return nChosenInternalFormat;
}

// [TODO] Clean this up!
bool Renderer::SetShaderProgramTextureBuffer(int nStage, PLRenderer::TextureBuffer *pTextureBuffer)
{
	// Set all available texture stages to this setting?
	if (nStage < 0) {
		for (uint32 i=0; i<m_sCapabilities.nMaxTextureUnits; i++)
			SetTextureBuffer(i, pTextureBuffer);
	} else {
		// Check if the stage is correct
		if (nStage >= (signed)m_sCapabilities.nMaxTextureUnits)
			return false; // Error!

		// Is this texture buffer already set?
		if (m_ppCurrentTextureBuffer[nStage] == pTextureBuffer)
			return false; // Error!

		// Make this texture buffer to the renderers current one
		PLRenderer::TextureBuffer *pPreviousTextureBuffer = m_ppCurrentTextureBuffer[nStage];
		m_ppCurrentTextureBuffer[nStage] = pTextureBuffer;

		// Loop through all sampler states
		for (uint32 i=0; i<PLRenderer::Sampler::Number; i++) {
			// Make this state 'dirty'
			m_ppnInternalSamplerState[nStage][i]++;

			// Reset the state
			SetSamplerState(nStage, (PLRenderer::Sampler::Enum)i, m_ppnSamplerState[nStage][i]);
		}
	}

	// Done
	return true;
}


//[-------------------------------------------------------]
//[ Private functions                                     ]
//[-------------------------------------------------------]
/**
*  @brief
*    Chooses a EGL config
*/
EGLConfig Renderer::ChooseConfig(uint32 nMultisampleAntialiasingSamples) const
{
	// Try to find a working EGL configuration
	EGLConfig hConfig = nullptr;
	EGLint nConfigs = 0;
	bool bChooseConfigCapitulated = false;
	bool bMultisampleAntialiasing = false;
	uint32 nMultisampleAntialiasingSampleBuffers = 0;
	uint32 nMultisampleAntialiasingSamplesCurrent = nMultisampleAntialiasingSamples;
	do {
		// Get the current multisample antialiasing settings
		bMultisampleAntialiasing = (nMultisampleAntialiasingSamplesCurrent > 1);	// Multisample antialiasing with just one sample per per pixel isn't real multisample, is it? :D
		nMultisampleAntialiasingSampleBuffers = bMultisampleAntialiasing ? 1 : 0;

		// Set desired configuration
		const EGLint pConfigAttribs[] =
		{
			EGL_LEVEL,				0,										// Frame buffer level
			EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,							// Which types of EGL surfaces are supported
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,						// Which client APIs are supported
			EGL_DEPTH_SIZE,			EGL_DONT_CARE,							// Bits of Z in the depth buffer
			EGL_SAMPLE_BUFFERS,		nMultisampleAntialiasingSampleBuffers,	// Number of multisample buffers (enable/disable multisample antialiasing)
			EGL_SAMPLES,			nMultisampleAntialiasingSamplesCurrent,	// Number of samples per pixel (multisample antialiasing samples)
			EGL_BUFFER_SIZE,		16,
			EGL_NONE
		};

		// Choose exactly one matching configuration
		if (eglChooseConfig(m_hDisplay, pConfigAttribs, &hConfig, 1, &nConfigs) == EGL_FALSE || nConfigs < 1) {
			// Can we change something on the multisample antialiasing? (may be the cause that no configuration was found!)
			if (bMultisampleAntialiasing) {
				if (nMultisampleAntialiasingSamplesCurrent > 8)
					nMultisampleAntialiasingSamplesCurrent = 8;
				else if (nMultisampleAntialiasingSamplesCurrent > 4)
					nMultisampleAntialiasingSamplesCurrent = 4;
				else if (nMultisampleAntialiasingSamplesCurrent > 2)
					nMultisampleAntialiasingSamplesCurrent = 2;
				else if (nMultisampleAntialiasingSamplesCurrent == 2)
					nMultisampleAntialiasingSamplesCurrent = 0;
			} else {
				// Don't mind, forget it...
				bChooseConfigCapitulated = true;
			}
		}
	} while (nConfigs < 1 && !bChooseConfigCapitulated);

	// Done
	return hConfig;
}

/**
*  @brief
*    Shows some OpenGL ES information
*/
void Renderer::ShowOpenGLESInformation()
{
	GLint nValue = 0;

	// Show some OpenGL ES information
	PL_LOG(Info, String::Format("OpenGL ES 2.0 version: %s",		glGetString(GL_VERSION)	 ? (const char*)glGetString(GL_VERSION)	 : "-"))
	PL_LOG(Info, String::Format("OpenGL ES 2.0 vendor info: %s",	glGetString(GL_VENDOR)	 ? (const char*)glGetString(GL_VENDOR)	 : "-"))
	PL_LOG(Info, String::Format("OpenGL ES 2.0 renderer info: %s",	glGetString(GL_RENDERER) ? (const char*)glGetString(GL_RENDERER) : "-"))

	{ // Show some OpenGL ES shader information
		PL_LOG(Info, String::Format("OpenGL ES 2.0 shading language version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION) ? (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) : "-"))

		// GL_MAX_VERTEX_ATTRIBS
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nValue);
		PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum number of vertex attributes: %d", nValue))

		// GL_MAX_VERTEX_UNIFORM_VECTORS
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &nValue);
		PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum number of uniform vertex vectors: %d", nValue))

		// GL_MAX_FRAGMENT_UNIFORM_VECTORS
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &nValue);
		PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum number of uniform fragment vectors: %d", nValue))

		// GL_MAX_VARYING_VECTORS
		glGetIntegerv(GL_MAX_VARYING_VECTORS, &nValue);
		PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum number of varying vectors: %d", nValue))

		// GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &nValue);
		PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum number of combined texture image units: %d", nValue))

		// GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &nValue);
		PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum number of vertex texture image units: %d", nValue))

		// GL_MAX_TEXTURE_IMAGE_UNITS
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nValue);
		PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum number of texture image units: %d", nValue))
	}

	// GL_MAX_TEXTURE_SIZE
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &nValue);
	PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum texture size: %d", nValue))

	// GL_MAX_CUBE_MAP_TEXTURE_SIZE
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &nValue);
	PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum cube map texture size: %d", nValue))

	// GL_MAX_RENDERBUFFER_SIZE
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &nValue);
	PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum renderbuffer size: %d", nValue))

	{ // GL_MAX_VIEWPORT_DIMS
		GLint nDimension[2] = {0, 0};
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS, nDimension);
		PL_LOG(Info, String::Format("OpenGL ES 2.0 maximum viewport dimension: %dx%d", nDimension[0], nDimension[1]))
	}
}

/**
*  @brief
*    Initializes all wrappers
*/
void Renderer::InitWrappers()
{
	// Fill modes - not supported by OpenGL ES 2.0

	// Cull modes
	m_cPLE_CULLWrapper.Resize(PLRenderer::Cull::Number, false, false);
	m_cPLE_CULLWrapper += 0;		//  0: PLRenderer::Cull::None (glDisable(GL_CULL_FACE)/glEnable(GL_CULL_FACE) is used)
	m_cPLE_CULLWrapper += GL_CW;	//  1: PLRenderer::Cull::CW
	m_cPLE_CULLWrapper += GL_CCW;	//  2: PLRenderer::Cull::CCW

	// Blend functions
	m_cPLE_BLENDWrapper.Resize(PLRenderer::BlendFunc::Number, false, false);
	m_cPLE_BLENDWrapper += GL_ZERO;					//  0: PLRenderer::BlendFunc::Zero
	m_cPLE_BLENDWrapper += GL_ONE;					//  1: PLRenderer::BlendFunc::One
	m_cPLE_BLENDWrapper += GL_SRC_COLOR;			//  2: PLRenderer::BlendFunc::SrcColor
	m_cPLE_BLENDWrapper += GL_ONE_MINUS_SRC_COLOR;	//  3: PLRenderer::BlendFunc::InvSrcColor
	m_cPLE_BLENDWrapper += GL_SRC_ALPHA;			//  4: PLRenderer::BlendFunc::SrcAlpha
	m_cPLE_BLENDWrapper += GL_ONE_MINUS_SRC_ALPHA;	//  5: PLRenderer::BlendFunc::InvSrcAlpha
	m_cPLE_BLENDWrapper += GL_SRC_ALPHA_SATURATE;	//  6: PLRenderer::BlendFunc::SrcAlphaSat
	m_cPLE_BLENDWrapper += GL_DST_COLOR;			//  7: PLRenderer::BlendFunc::DstColor
	m_cPLE_BLENDWrapper += GL_ONE_MINUS_DST_COLOR;	//  8: PLRenderer::BlendFunc::InvDstColor
	m_cPLE_BLENDWrapper += GL_DST_ALPHA;			//  9: PLRenderer::BlendFunc::DstAlpha
	m_cPLE_BLENDWrapper += GL_ONE_MINUS_DST_ALPHA;	// 10: PLRenderer::BlendFunc::InvDstAlpha

	// Comparison functions
	m_cPLE_CMPWrapper.Resize(PLRenderer::Compare::Number, false, false);
	m_cPLE_CMPWrapper += GL_NEVER;		//  0: PLRenderer::Compare::Never
	m_cPLE_CMPWrapper += GL_LESS;		//  1: PLRenderer::Compare::Less
	m_cPLE_CMPWrapper += GL_EQUAL;		//  2: PLRenderer::Compare::Equal
	m_cPLE_CMPWrapper += GL_LEQUAL;		//  3: PLRenderer::Compare::LessEqual
	m_cPLE_CMPWrapper += GL_GREATER;	//  4: PLRenderer::Compare::Greater
	m_cPLE_CMPWrapper += GL_NOTEQUAL;	//  5: PLRenderer::Compare::NotEqual
	m_cPLE_CMPWrapper += GL_GEQUAL;		//  6: PLRenderer::Compare::GreaterEqual
	m_cPLE_CMPWrapper += GL_ALWAYS;		//  7: PLRenderer::Compare::Always

	// Stencil operations
	m_cPLE_SOPWrapper.Resize(PLRenderer::StencilOp::Number, false, false);
	m_cPLE_SOPWrapper += GL_KEEP;		//  0: PLRenderer::StencilOp::Keep
	m_cPLE_SOPWrapper += GL_ZERO;		//  1: PLRenderer::StencilOp::Zero
	m_cPLE_SOPWrapper += GL_REPLACE;	//  2: PLRenderer::StencilOp::Replace
	m_cPLE_SOPWrapper += GL_INCR;		//  3: PLRenderer::StencilOp::Incr
	m_cPLE_SOPWrapper += GL_DECR;		//  4: PLRenderer::StencilOp::Decr
	m_cPLE_SOPWrapper += GL_INCR_WRAP;	//  5: PLRenderer::StencilOp::IncrWrap
	m_cPLE_SOPWrapper += GL_DECR_WRAP;	//  6: PLRenderer::StencilOp::DecrWrap
	m_cPLE_SOPWrapper += GL_INVERT;		//  7: PLRenderer::StencilOp::Invert

	// Primitive types
	m_cPLE_PTWrapper.Resize(PLRenderer::Primitive::Number, false, false);
	m_cPLE_PTWrapper += GL_POINTS;			//  0: PLRenderer::Primitive::PointList
	m_cPLE_PTWrapper += GL_LINES;			//  1: PLRenderer::Primitive::LineList
	m_cPLE_PTWrapper += GL_LINE_STRIP;		//  2: PLRenderer::Primitive::LineStrip
	m_cPLE_PTWrapper += GL_TRIANGLES;		//  3: PLRenderer::Primitive::TriangleList
	m_cPLE_PTWrapper += GL_TRIANGLE_STRIP;	//  4: PLRenderer::Primitive::TriangleStrip
	m_cPLE_PTWrapper += GL_TRIANGLE_FAN;	//  5: PLRenderer::Primitive::TriangleFan

	// Texture-addressing modes
	m_cPLE_TAWrapper.Resize(PLRenderer::TextureAddressing::Number, false, false);
	m_cPLE_TAWrapper += GL_CLAMP_TO_EDGE;		//  0: PLRenderer::TextureAddressing::Clamp

	// [TODO]
	m_cPLE_TAWrapper += 0;		//  1: PLRenderer::TextureAddressing::Border
//	m_cPLE_TAWrapper += GL_CLAMP_TO_BORDER;		//  1: PLRenderer::TextureAddressing::Border

	m_cPLE_TAWrapper += GL_REPEAT;			//  2: PLRenderer::TextureAddressing::Wrap
	m_cPLE_TAWrapper += GL_MIRRORED_REPEAT;	//  3: PLRenderer::TextureAddressing::Mirror

	// Texture buffer pixel formats (internal)
	m_cPLE_TPFWrapper.Resize(PLRenderer::TextureBuffer::NumOfPixelFormats, false, false);
	m_cPLE_TPFWrapper += GL_LUMINANCE;			//  0: PLRenderer::TextureBuffer::L8
	m_cPLE_TPFWrapper += GL_LUMINANCE;			//  1: PLRenderer::TextureBuffer::L16			- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_ALPHA;				//  2: PLRenderer::TextureBuffer::A8
	m_cPLE_TPFWrapper += GL_LUMINANCE_ALPHA;	//  3: PLRenderer::TextureBuffer::L4A4			- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_LUMINANCE_ALPHA;	//  4: PLRenderer::TextureBuffer::L8A8
	m_cPLE_TPFWrapper += GL_DEPTH_COMPONENT;	//  5: PLRenderer::TextureBuffer::D16
	m_cPLE_TPFWrapper += GL_DEPTH_COMPONENT;	//  6: PLRenderer::TextureBuffer::D24
	m_cPLE_TPFWrapper += GL_DEPTH_COMPONENT;	//  7: PLRenderer::TextureBuffer::D32
	m_cPLE_TPFWrapper += GL_RGB;				//  8: PLRenderer::TextureBuffer::R3G3B2		- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_RGB;				//  9: PLRenderer::TextureBuffer::R5G6B5		- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_RGBA;				// 10: PLRenderer::TextureBuffer::R5G5B5A1		- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_RGBA;				// 11: PLRenderer::TextureBuffer::R4G4B4A4		- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_RGB;				// 12: PLRenderer::TextureBuffer::R8G8B8
	m_cPLE_TPFWrapper += GL_RGBA;				// 13: PLRenderer::TextureBuffer::R8G8B8A8
	m_cPLE_TPFWrapper += GL_RGBA;				// 14: PLRenderer::TextureBuffer::R10G10B10A2	- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_RGBA;				// 15: PLRenderer::TextureBuffer::R16G16B16A16	- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_RGB;				// 16: PLRenderer::TextureBuffer::DXT1			- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_RGBA;				// 17: PLRenderer::TextureBuffer::DXT3			- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_RGBA;				// 18: PLRenderer::TextureBuffer::DXT5			- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_LUMINANCE;			// 19: PLRenderer::TextureBuffer::LATC1			- not supported by OpenGL ES 2.0
	m_cPLE_TPFWrapper += GL_LUMINANCE_ALPHA;	// 20: PLRenderer::TextureBuffer::LATC2			- not supported by OpenGL ES 2.0

	// [TODO]
	// Float pixel format
}

/**
*  @brief
*    Setup the renderer capabilities
*/
void Renderer::SetupCapabilities()
{
	m_sCapabilities.nMaxColorRenderTargets			= 1;
	m_sCapabilities.nMaxTextureUnits				= 8;
	m_sCapabilities.nMaxAnisotropy					= 16;
	m_sCapabilities.nMaxTessellationFactor			= 1;
	m_sCapabilities.nMaxTextureBufferSize			= 4096;
	m_sCapabilities.bTextureBufferRectangle			= true;
	m_sCapabilities.nMaxRectangleTextureBufferSize	= 4096;
	m_sCapabilities.bTextureBuffer3D				= true;
	m_sCapabilities.nMax3DTextureBufferSize			= 512;
	m_sCapabilities.bTextureBufferCube				= true;
	m_sCapabilities.nMaxCubeTextureBufferSize		= 2048;
	m_sCapabilities.bStencilWrap					= false;
	m_sCapabilities.bTwoSidedStencils				= true;
	m_sCapabilities.bDepthBoundsTest				= false;
	m_sCapabilities.bPointSprite					= false;
	m_sCapabilities.bPointParameters				= false;
	m_sCapabilities.bOcclusionQuery					= false;
	m_sCapabilities.bVertexBufferSecondaryColor		= false;

	// Show renderer capabilities
	ShowRendererCapabilities();
}

/**
*  @brief
*    Sampler state filter mode helper function
*/
GLuint Renderer::GetCombinedMinMipFilter(uint32 nStage)
{
	// Has the current texture buffer any mipmaps?
	if (m_ppCurrentTextureBuffer[nStage] && !m_ppCurrentTextureBuffer[nStage]->GetNumOfMipmaps()) {
		// Nope, we can't use mipmappig. Check internal sampler state...
		switch (m_ppnInternalSamplerState[nStage][PLRenderer::Sampler::MinFilter]) {
			case PLRenderer::TextureFiltering::Anisotropic:
			case PLRenderer::TextureFiltering::Linear:
				// Linear min
				return GL_LINEAR;

			case PLRenderer::TextureFiltering::Point:
			case PLRenderer::TextureFiltering::None:
				// Nearest min
				return GL_NEAREST;
		}
	} else {
		// Check internal sampler state
		switch (m_ppnInternalSamplerState[nStage][PLRenderer::Sampler::MinFilter]) {
			case PLRenderer::TextureFiltering::Anisotropic:
			case PLRenderer::TextureFiltering::Linear:
				switch (m_ppnInternalSamplerState[nStage][PLRenderer::Sampler::MipFilter]) {
					case PLRenderer::TextureFiltering::Anisotropic:
					case PLRenderer::TextureFiltering::Linear:
						// Linear min, linear mip
						return GL_LINEAR_MIPMAP_LINEAR;

					case PLRenderer::TextureFiltering::Point:
						// Linear min, point mip
						return GL_LINEAR_MIPMAP_NEAREST;

					case PLRenderer::TextureFiltering::None:
						// Linear min, no mip
						return GL_LINEAR;
				}
				break;

			case PLRenderer::TextureFiltering::Point:
			case PLRenderer::TextureFiltering::None:
				switch (m_ppnInternalSamplerState[nStage][PLRenderer::Sampler::MipFilter]) {
					case PLRenderer::TextureFiltering::Anisotropic:
					case PLRenderer::TextureFiltering::Linear:
						// Nearest min, linear mip
						return GL_NEAREST_MIPMAP_LINEAR;

					case PLRenderer::TextureFiltering::Point:
						// Nearest min, point mip
						return GL_NEAREST_MIPMAP_NEAREST;

					case PLRenderer::TextureFiltering::None:
						// Nearest min, no mip
						return GL_NEAREST;
				}
				break;
		}
	}

	// Fallback, should never get here!
	return GL_NEAREST;
}


//[-------------------------------------------------------]
//[ Public virtual PLRenderer::Renderer functions         ]
//[-------------------------------------------------------]
String Renderer::GetAPI(uint32 *pnVersion) const
{
	// Get version
	if (pnVersion) {
		// Set version to 0 as default
		*pnVersion = 0;

		// Get version (for example '2.1.3' -> we are only interessted in '2.1' because
		// the third number is not really normed)
		String sVersion = (const char*)glGetString(GL_VERSION);
		if (sVersion.GetLength()) {
			// Major
			int nIndex = sVersion.IndexOf('.');
			if (nIndex > -1) {
				// Delete the dot
				sVersion.Delete(nIndex, 1);

				// Minor
				nIndex = sVersion.IndexOf('.');
				if (nIndex > -1) {
					// Get the version string
					String sMajorMinorVersion = sVersion.GetSubstring(0, nIndex);

					// Version string to version number
					*pnVersion = sMajorMinorVersion.GetInt();
				}
			}
		}
	}

	// Get API
	static const String sString = "OpenGL ES 2.0";
	return sString;
}

String Renderer::GetVendor() const
{
	return (const char*)glGetString(GL_VENDOR);
}

String Renderer::GetDefaultShaderLanguage() const
{
	return ShaderLanguageGLSL::GLSL;
}

PLRenderer::ShaderLanguage *Renderer::GetShaderLanguage(const String &sShaderLanguage)
{
	// Only the build in GLSL shader language is supported
	return (!sShaderLanguage.GetLength() || sShaderLanguage == ShaderLanguageGLSL::GLSL) ? m_pShaderLanguageGLSL : nullptr;
}

PLRenderer::FixedFunctions *Renderer::GetFixedFunctions() const
{
	// We're in luck - it's not supported by OpenGL ES 2.0
	return nullptr;
}

PLRenderer::FontManager &Renderer::GetFontManager() const
{
	return *m_pFontManager;
}


//[-------------------------------------------------------]
//[ Create surface/resource                               ]
//[-------------------------------------------------------]
PLRenderer::SurfaceWindow *Renderer::CreateSurfaceWindow(PLRenderer::SurfaceWindowHandler &cHandler, handle nWindow, const PLRenderer::DisplayMode &sDisplayMode, bool bFullscreen)
{
	// Is the surface window handler valid?
	if (cHandler.GetRenderer() != this)
		return nullptr; // Error!

	// Create and register renderer surface
	PLRenderer::SurfaceWindow *pRendererSurface = new SurfaceWindow(cHandler, nWindow, bFullscreen);
	m_lstSurfaces.Add(pRendererSurface);

	// Return created renderer surface
	return pRendererSurface;
}

PLRenderer::SurfaceTextureBuffer *Renderer::CreateSurfaceTextureBuffer2D(const Vector2i &vSize, PLRenderer::TextureBuffer::EPixelFormat nFormat, uint32 nFlags, uint8 nMaxColorTargets)
{
	// [TODO] Implement render to texture
	/*
	// Check maximum render targets and dimension
	if (nMaxColorTargets && nMaxColorTargets <= m_sCapabilities.nMaxColorRenderTargets && vSize.x && vSize.y &&
		vSize.x <= m_sCapabilities.nMaxTextureBufferSize && vSize.y <= m_sCapabilities.nMaxTextureBufferSize &&
		Math::IsPowerOfTwo(vSize.x) && Math::IsPowerOfTwo(vSize.y)) {
		// Create and register renderer surface
		PLRenderer::TextureBuffer *pTextureBuffer = new TextureBuffer2D(*this, vSize, nFormat, PLRenderer::TextureBuffer::RenderTarget);
		PLRenderer::SurfaceTextureBuffer *pRendererSurface = new SurfaceTextureBuffer(*this, *pTextureBuffer, nFlags, nMaxColorTargets);
		m_lstSurfaces.Add(pRendererSurface);

		// Return created renderer surface
		return pRendererSurface;
	} else {
		// Error!
		return nullptr;
	}
//	*/
	// Error!
	return nullptr;
}

PLRenderer::SurfaceTextureBuffer *Renderer::CreateSurfaceTextureBufferRectangle(const Vector2i &vSize, PLRenderer::TextureBuffer::EPixelFormat nFormat, uint32 nFlags, uint8 nMaxColorTargets)
{
	// [TODO] Implement render to texture
	/*
	// Check maximum render targets and dimension
	if (nMaxColorTargets && nMaxColorTargets <= m_sCapabilities.nMaxColorRenderTargets && vSize.x && vSize.y &&
		vSize.x <= m_sCapabilities.nMaxRectangleTextureBufferSize && vSize.y <= m_sCapabilities.nMaxRectangleTextureBufferSize) {
		// Create and register renderer surface
		PLRenderer::TextureBuffer *pTextureBuffer = new TextureBufferRectangle(*this, vSize, nFormat, PLRenderer::TextureBuffer::RenderTarget);
		PLRenderer::SurfaceTextureBuffer *pRendererSurface = new SurfaceTextureBuffer(*this, *pTextureBuffer, nFlags, nMaxColorTargets);
		m_lstSurfaces.Add(pRendererSurface);

		// Return created renderer surface
		return pRendererSurface;
	} else {
		// Error!
		return nullptr;
	}
	*/
	// Error!
	return nullptr;
}

PLRenderer::SurfaceTextureBuffer *Renderer::CreateSurfaceTextureBufferCube(uint16 nSize, PLRenderer::TextureBuffer::EPixelFormat nFormat, uint32 nFlags)
{
	// [TODO] Implement render to texture
	/*
	// Valid dimension?
	if (nSize > m_sCapabilities.nMaxCubeTextureBufferSize || nSize < 1 || !Math::IsPowerOfTwo(nSize))
		return nullptr; // Error!

	// Create and register renderer surface
	PLRenderer::TextureBuffer *pTextureBuffer = new TextureBufferCube(*this, nSize, nFormat, PLRenderer::TextureBuffer::RenderTarget);
	PLRenderer::SurfaceTextureBuffer *pRendererSurface = new SurfaceTextureBuffer(*this, *pTextureBuffer, nFlags, 1);
	m_lstSurfaces.Add(pRendererSurface);

	// Return created renderer surface
	return pRendererSurface;
	*/
	// Error!
	return nullptr;
}

PLRenderer::TextureBuffer1D *Renderer::CreateTextureBuffer1D(Image &cImage, PLRenderer::TextureBuffer::EPixelFormat nInternalFormat, uint32 nFlags)
{
	// 1D textures are not supported by OpenGL ES 2.0
	return nullptr;
}

PLRenderer::TextureBuffer2D *Renderer::CreateTextureBuffer2D(Image &cImage, PLRenderer::TextureBuffer::EPixelFormat nInternalFormat, uint32 nFlags)
{
	// Check texture buffer
	if (!CheckTextureBuffer2D(cImage, nInternalFormat))
		return nullptr; // Error!

	// Create the OpenGL ES 2.0 2D texture buffer
	return new TextureBuffer2D(*this, cImage, nInternalFormat, nFlags, false);
}

PLRenderer::TextureBuffer *Renderer::CreateTextureBufferRectangle(Image &cImage, PLRenderer::TextureBuffer::EPixelFormat nInternalFormat, uint32 nFlags)
{
	// Check texture buffer
	if (!m_sCapabilities.bTextureBufferRectangle || !CheckTextureBufferRectangle(cImage, nInternalFormat))
		return nullptr; // Error!

	// Create the OpenGL ES 2.0 2D texture buffer - OpenGL ES 2.0 has no special "rectangle"/"non-power-of-two" textures. GL_TEXTURE_2D
	// can also be used with non-power-of-two texture data, but non-power-of-two textures have restrictions on the allowed texture wrap
	// modes and filters. The texture wrap mode must be GL_CLAMP_TO_EDGE and the minification filter must be GL_NEAREST or GL_LINEAR.
	return new TextureBuffer2D(*this, cImage, nInternalFormat, nFlags, true);
}

PLRenderer::TextureBuffer3D *Renderer::CreateTextureBuffer3D(Image &cImage, PLRenderer::TextureBuffer::EPixelFormat nInternalFormat, uint32 nFlags)
{
	// Check texture buffer
	if (!CheckTextureBuffer3D(cImage, nInternalFormat))
		return nullptr; // Error!

	// Create the null 3D texture buffer
	return new TextureBuffer3D(*this, cImage, nInternalFormat, nFlags);
}

PLRenderer::TextureBufferCube *Renderer::CreateTextureBufferCube(Image &cImage, PLRenderer::TextureBuffer::EPixelFormat nInternalFormat, uint32 nFlags)
{
	// Check texture buffer
	if (!CheckTextureBufferCube(cImage, nInternalFormat))
		return nullptr; // Error!

	// Create the null cube texture buffer
	return new TextureBufferCube(*this, cImage, nInternalFormat, nFlags);
}

PLRenderer::IndexBuffer *Renderer::CreateIndexBuffer()
{
	// Create the null index buffer
	return new IndexBuffer(*this);
}

PLRenderer::VertexBuffer *Renderer::CreateVertexBuffer()
{
	// Create the null vertex buffer
	return new VertexBuffer(*this);
}

PLRenderer::OcclusionQuery *Renderer::CreateOcclusionQuery()
{
	// OpenGL ES 2.0 has no support for occlusion queries
	return nullptr;
}


//[-------------------------------------------------------]
//[ States                                                ]
//[-------------------------------------------------------]
bool Renderer::SetRenderState(PLRenderer::RenderState::Enum nState, uint32 nValue)
{
	// Check if the state is a valid render state member
	if (nState >= PLRenderer::RenderState::Number)
		return false; // Error!

	// Check if this render state is already set to this value
	if (m_nRenderState[nState] != nValue) {
		// Set the render state
		m_nRenderState[nState] = nValue;
		m_sStatistics.nRenderStateChanges++;

		// Set render state - because there are that many render states, split the states binary tree like
		if (nState < PLRenderer::RenderState::PointSize) {
			if (nState < PLRenderer::RenderState::BlendEnable) {
				switch (nState) {
				// Modes
					case PLRenderer::RenderState::FillMode:
						// Not supported by OpenGL ES 2.0
						break;

					case PLRenderer::RenderState::CullMode:
						if (nValue == PLRenderer::Cull::None) {
							glDisable(GL_CULL_FACE);
						} else {
							// Invert cull mode?
							bool bAPISwapY = false;
							if (m_cCurrentSurface.GetSurface())
								bAPISwapY = m_cCurrentSurface.GetSurface()->IsAPISwapY();
							if (!bAPISwapY &&  GetRenderState(PLRenderer::RenderState::InvCullMode) ||
								 bAPISwapY && !GetRenderState(PLRenderer::RenderState::InvCullMode)) {
								if (nValue == PLRenderer::Cull::CW)
									nValue = PLRenderer::Cull::CCW;
								else
									nValue = PLRenderer::Cull::CW;
							}

							// Set the state
							glEnable(GL_CULL_FACE);
							const uint32 &nAPIValue = m_cPLE_CULLWrapper[nValue];
							if (&nAPIValue != &Array<uint32>::Null) {
								glFrontFace(nAPIValue);
							} else {
								// Error, invalid value!
								return false;
							}
						}
						break;

				// Z buffer
					case PLRenderer::RenderState::ZEnable:
						if (nValue == 0)
							glDisable(GL_DEPTH_TEST);
						else if (nValue == 1)
							glEnable(GL_DEPTH_TEST);
						else
							return false; // Error, invalid value!
						break;

					case PLRenderer::RenderState::ZWriteEnable:
						if (nValue == 0)
							glDepthMask(false);
						else if (nValue == 1)
							glDepthMask(true);
						else
							return false; // Error, invalid value!
						break;

					case PLRenderer::RenderState::ZFunc:
					{
						const uint32 &nAPIValue = m_cPLE_CMPWrapper[nValue];
						if (&nAPIValue != &Array<uint32>::Null) {
							glDepthFunc(nAPIValue);
						} else {
							// Error, invalid value!
							return false;
						}
						break;
					}

					case PLRenderer::RenderState::ZBias:
						// Not supported
						break;

					case PLRenderer::RenderState::SlopeScaleDepthBias:
						if (!nValue && !GetRenderState(PLRenderer::RenderState::DepthBias)) {
							glDisable(GL_POLYGON_OFFSET_FILL);
							// The bias is not applied to any line and point primitive within D3D, so, do not use this for OpenGL :)
	//						glDisable(GL_POLYGON_OFFSET_POINT);
	//						glDisable(GL_POLYGON_OFFSET_LINE);
						} else {
							glEnable(GL_POLYGON_OFFSET_FILL);
							// The bias is not applied to any line and point primitive within D3D, so, do not use this for OpenGL :)
	//						glEnable(GL_POLYGON_OFFSET_POINT);
	//						glEnable(GL_POLYGON_OFFSET_LINE);
							glPolygonOffset(Tools::UInt32ToFloat(nValue), Tools::UInt32ToFloat(GetRenderState(PLRenderer::RenderState::DepthBias)));
						}
						break;

					case PLRenderer::RenderState::DepthBias:
						if (!nValue && !GetRenderState(PLRenderer::RenderState::SlopeScaleDepthBias)) {
							glDisable(GL_POLYGON_OFFSET_FILL);
							// The bias is not applied to any line and point primitive within D3D, so, do not use this for OpenGL :)
	//						glDisable(GL_POLYGON_OFFSET_POINT);
	//						glDisable(GL_POLYGON_OFFSET_LINE);
						} else {
							glEnable(GL_POLYGON_OFFSET_FILL);
							// The bias is not applied to any line and point primitive within D3D, so, do not use this for OpenGL :)
	//						glEnable(GL_POLYGON_OFFSET_POINT);
	//						glEnable(GL_POLYGON_OFFSET_LINE);
							glPolygonOffset(Tools::UInt32ToFloat(GetRenderState(PLRenderer::RenderState::SlopeScaleDepthBias)), Tools::UInt32ToFloat(nValue));
						}
						break;
				}
			} else {
				switch (nState) {
				// Blend
					case PLRenderer::RenderState::BlendEnable:
						if (nValue == 0)
							glDisable(GL_BLEND);
						else if (nValue == 1)
							glEnable(GL_BLEND);
						else
							return false; // Error, invalid value!
						break;

					case PLRenderer::RenderState::SrcBlendFunc:
					{
						const uint32 &nAPIValue1 = m_cPLE_BLENDWrapper[nValue];
						const uint32 &nAPIValue2 = m_cPLE_BLENDWrapper[GetRenderState(PLRenderer::RenderState::DstBlendFunc)];
						if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null)) {
							glBlendFunc(nAPIValue1, nAPIValue2);
						} else {
							// Error, invalid value!
							return false;
						}
						break;
					}

					case PLRenderer::RenderState::DstBlendFunc:
					{
						const uint32 &nAPIValue1 = m_cPLE_BLENDWrapper[nValue];
						const uint32 &nAPIValue2 = m_cPLE_BLENDWrapper[GetRenderState(PLRenderer::RenderState::SrcBlendFunc)];
						if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null)) {
							glBlendFunc(nAPIValue2, nAPIValue1);
						} else {
							// Error, invalid value!
							return false;
						}
						break;
					}

				// Stencil
					case PLRenderer::RenderState::StencilEnable:
						if (nValue == 0)
							glDisable(GL_STENCIL_TEST);
						else if (nValue == 1)
							glEnable(GL_STENCIL_TEST);
						else {
							// Error, invalid value!
							return false;
						}
						break;

					case PLRenderer::RenderState::StencilFunc:
						if (GetRenderState(PLRenderer::RenderState::TwoSidedStencilMode)) {
							// Two sided
							const uint32 &nAPIValue1 = m_cPLE_CMPWrapper[nValue];
							const uint32 &nAPIValue2 = m_cPLE_CMPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilFunc)];
							if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null))
								glStencilFuncSeparate(nAPIValue2, nAPIValue1, GetRenderState(PLRenderer::RenderState::StencilRef), GetRenderState(PLRenderer::RenderState::StencilMask));
							else {
								// Error, invalid value!
								return false;
							}
						} else {
							// No two sided
							const uint32 &nAPIValue = m_cPLE_CMPWrapper[nValue];
							if (&nAPIValue != &Array<uint32>::Null)
								glStencilFuncSeparate(nAPIValue, nAPIValue, GetRenderState(PLRenderer::RenderState::StencilRef), GetRenderState(PLRenderer::RenderState::StencilMask));
							else {
								// Error, invalid value!
								return false;
							}
						}
						break;

					case PLRenderer::RenderState::StencilRef:
						if (GetRenderState(PLRenderer::RenderState::TwoSidedStencilMode)) {
							// Two sided
							const uint32 &nAPIValue1 = m_cPLE_CMPWrapper[GetRenderState(PLRenderer::RenderState::StencilFunc)];
							const uint32 &nAPIValue2 = m_cPLE_CMPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilFunc)];
							if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null))
								glStencilFuncSeparate(nAPIValue2, nAPIValue1, nValue, GetRenderState(PLRenderer::RenderState::StencilMask));
							else {
								// Error, invalid value!
								return false;
							}
						} else {
							// No two sided
							const uint32 &nAPIValue = m_cPLE_CMPWrapper[GetRenderState(PLRenderer::RenderState::StencilFunc)];
							if (&nAPIValue != &Array<uint32>::Null)
								glStencilFuncSeparate(nAPIValue, nAPIValue, nValue, GetRenderState(PLRenderer::RenderState::StencilMask));
							else {
								// Error, invalid value!
								return false;
							}
						}
						break;

					case PLRenderer::RenderState::StencilMask:
						if (GetRenderState(PLRenderer::RenderState::TwoSidedStencilMode)) {
							// Two sided
							const uint32 &nAPIValue1 = m_cPLE_CMPWrapper[GetRenderState(PLRenderer::RenderState::StencilFunc)];
							const uint32 &nAPIValue2 = m_cPLE_CMPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilFunc)];
							if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null))
								glStencilFuncSeparate(nAPIValue2, nAPIValue1, GetRenderState(PLRenderer::RenderState::StencilRef), nValue);
							else {
								// Error, invalid value!
								return false;
							}
						} else {
							// No two sided
							const uint32 &nAPIValue = m_cPLE_CMPWrapper[GetRenderState(PLRenderer::RenderState::StencilFunc)];
							if (&nAPIValue != &Array<uint32>::Null)
								glStencilFuncSeparate(nAPIValue, nAPIValue, GetRenderState(PLRenderer::RenderState::StencilRef), nValue);
							else {
								// Error, invalid value!
								return false;
							}
						}
						break;

					case PLRenderer::RenderState::StencilFail:
					{
						const uint32 &nAPIValue1 = m_cPLE_SOPWrapper[nValue];
						const uint32 &nAPIValue2 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::StencilZFail)];
						const uint32 &nAPIValue3 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::StencilPass)];
						if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null) && (&nAPIValue3 != &Array<uint32>::Null)) {
							if (GetRenderState(PLRenderer::RenderState::TwoSidedStencilMode))
								glStencilOpSeparate(GL_BACK, nAPIValue1, nAPIValue2, nAPIValue3);
							else
								glStencilOpSeparate(GL_FRONT_AND_BACK, nAPIValue1, nAPIValue2, nAPIValue3);
						} else {
							// Error, invalid value!
							return false;
						}
					}
					break;

					case PLRenderer::RenderState::StencilZFail:
					{
						const uint32 &nAPIValue1 = m_cPLE_SOPWrapper[nValue];
						const uint32 &nAPIValue2 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::StencilFail)];
						const uint32 &nAPIValue3 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::StencilPass)];
						if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null) && (&nAPIValue3 != &Array<uint32>::Null)) {
							if (GetRenderState(PLRenderer::RenderState::TwoSidedStencilMode))
								glStencilOpSeparate(GL_BACK, nAPIValue2, nAPIValue1, nAPIValue3);
							else
								glStencilOpSeparate(GL_FRONT_AND_BACK, nAPIValue2, nAPIValue1, nAPIValue3);
						} else {
							// Error, invalid value!
							return false;
						}
					}
					break;

					case PLRenderer::RenderState::StencilPass:
					{
						const uint32 &nAPIValue1 = m_cPLE_SOPWrapper[nValue];
						const uint32 &nAPIValue2 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::StencilFail)];
						const uint32 &nAPIValue3 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::StencilZFail)];
						if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null) && (&nAPIValue3 != &Array<uint32>::Null)) {
							if (GetRenderState(PLRenderer::RenderState::TwoSidedStencilMode))
								glStencilOpSeparate(GL_BACK, nAPIValue2, nAPIValue3, nAPIValue1);
							else
								glStencilOpSeparate(GL_FRONT_AND_BACK, nAPIValue2, nAPIValue3, nAPIValue1);
						} else {
							// Error, invalid value!
							return false;
						}
					}
					break;

					case PLRenderer::RenderState::TwoSidedStencilMode:
						{ // No two sided stencil enable/disable required, but set the correct settings for sure
							const uint32 &nAPIValue1 = m_cPLE_CMPWrapper[GetRenderState(PLRenderer::RenderState::StencilFunc)];
							const uint32 &nAPIValue2 = m_cPLE_CMPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilFunc)];
							if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null))
								glStencilFuncSeparate(nAPIValue2, nAPIValue1, GetRenderState(PLRenderer::RenderState::StencilRef), nValue);
							else {
								// Error, invalid value!
								return false;
							}
						}
						{ // Back
							const uint32 &nAPIValue1 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::StencilFail)];
							const uint32 &nAPIValue2 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::StencilZFail)];
							const uint32 &nAPIValue3 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::StencilPass)];
							if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null) && (&nAPIValue3 != &Array<uint32>::Null))
								glStencilOpSeparate(GL_BACK, nAPIValue1, nAPIValue2, nAPIValue3);
							else {
								return false; // Error, invalid value!
							}
						}
						{ // Front and back
							const uint32 &nAPIValue1 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilFail)];
							const uint32 &nAPIValue2 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilZFail)];
							const uint32 &nAPIValue3 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilPass)];
							if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null) && (&nAPIValue3 != &Array<uint32>::Null))
								glStencilOpSeparate(GL_FRONT, nAPIValue1, nAPIValue2, nAPIValue3);
							else {
								// Error, invalid value!
								return false;
							}
						}
						break;

					case PLRenderer::RenderState::CCWStencilFunc:
						if (GetRenderState(PLRenderer::RenderState::TwoSidedStencilMode)) {
							const uint32 &nAPIValue1 = m_cPLE_CMPWrapper[nValue];
							const uint32 &nAPIValue2 = m_cPLE_CMPWrapper[GetRenderState(PLRenderer::RenderState::StencilFunc)];
							if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null))
								glStencilFuncSeparate(nAPIValue1, nAPIValue2, GetRenderState(PLRenderer::RenderState::StencilRef), GetRenderState(PLRenderer::RenderState::StencilMask));
							else {
								// Error, invalid value!
								return false;
							}
						}
						break;

					case PLRenderer::RenderState::CCWStencilFail:
						if (GetRenderState(PLRenderer::RenderState::TwoSidedStencilMode)) {
							const uint32 &nAPIValue1 = m_cPLE_SOPWrapper[nValue];
							const uint32 &nAPIValue2 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilZFail)];
							const uint32 &nAPIValue3 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilPass)];
							if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null) && (&nAPIValue3 != &Array<uint32>::Null))
								glStencilOpSeparate(GL_FRONT, nAPIValue1, nAPIValue2, nAPIValue3);
							else {
								// Error, invalid value!
								return false;
							}
						}
						break;

					case PLRenderer::RenderState::CCWStencilZFail:
						if (GetRenderState(PLRenderer::RenderState::TwoSidedStencilMode)) {
							const uint32 &nAPIValue1 = m_cPLE_SOPWrapper[nValue];
							const uint32 &nAPIValue2 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilFail)];
							const uint32 &nAPIValue3 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilPass)];
							if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null) && (&nAPIValue3 != &Array<uint32>::Null))
								glStencilOpSeparate(GL_FRONT, nAPIValue2, nAPIValue1, nAPIValue3);
							else {
								// Error, invalid value!
								return false;
							}
						}
						break;

					case PLRenderer::RenderState::CCWStencilPass:
						if (GetRenderState(PLRenderer::RenderState::TwoSidedStencilMode)) {
							const uint32 &nAPIValue1 = m_cPLE_SOPWrapper[nValue];
							const uint32 &nAPIValue2 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilFail)];
							const uint32 &nAPIValue3 = m_cPLE_SOPWrapper[GetRenderState(PLRenderer::RenderState::CCWStencilZFail)];
							if ((&nAPIValue1 != &Array<uint32>::Null) && (&nAPIValue2 != &Array<uint32>::Null) && (&nAPIValue3 != &Array<uint32>::Null))
								glStencilOpSeparate(GL_FRONT, nAPIValue2, nAPIValue3, nAPIValue1);
							else {
								// Error, invalid value!
								return false;
							}
						}
						break;
				}
			}
		} else {
			switch (nState) {
			// Point and line
				case PLRenderer::RenderState::PointSize:
					// Not supported by OpenGL ES 2.0, must be done within a vertex shader by writing to the gl_PointSize output attribute
					break;

				case PLRenderer::RenderState::PointScaleEnable:
					// Not supported by OpenGL ES 2.0
					break;

				case PLRenderer::RenderState::PointSizeMin:
					// Not supported by OpenGL ES 2.0
					break;

				case PLRenderer::RenderState::PointSizeMax:
					// Not supported by OpenGL ES 2.0
					break;

				case PLRenderer::RenderState::PointScaleA:
					// Not supported by OpenGL ES 2.0
					break;

				case PLRenderer::RenderState::PointScaleB:
					// Not supported by OpenGL ES 2.0
					break;

				case PLRenderer::RenderState::PointScaleC:
					// Not supported by OpenGL ES 2.0
					break;

				case PLRenderer::RenderState::LineWidth:
					glLineWidth(Tools::UInt32ToFloat(nValue));
					break;

			// Tessellation
				case PLRenderer::RenderState::TessellationFactor:
					// Not supported by OpenGL ES 2.0
					break;

				case PLRenderer::RenderState::TessellationMode:
					// Not supported by OpenGL ES 2.0
					break;

			// Misc
				case PLRenderer::RenderState::PointSpriteEnable:
					// In OpenGL ES 2.0 replaced by the gl_PointCoord variable defined in the OpenGL ES shading language specification
					break;

				case PLRenderer::RenderState::DitherEnable:
					if (nValue == 0)
						glDisable(GL_DITHER);
					else if (nValue == 1)
						glEnable(GL_DITHER);
					else {
						// Error, invalid value!
						return false;
					}
					break;

				case PLRenderer::RenderState::ScissorTestEnable:
					if (nValue == 0)
						glDisable(GL_SCISSOR_TEST);
					else if (nValue == 1)
						glEnable(GL_SCISSOR_TEST);
					else {
						// Error, invalid value!
						return false;
					}
					break;

				case PLRenderer::RenderState::MultisampleEnable:
					// Not supported by OpenGL ES 2.0
					break;

				case PLRenderer::RenderState::InvCullMode:
					if (GetRenderState(PLRenderer::RenderState::CullMode) != PLRenderer::Cull::None) {
						// Invert cull mode?
						bool bAPISwapY = false;
						if (m_cCurrentSurface.GetSurface())
							bAPISwapY = m_cCurrentSurface.GetSurface()->IsAPISwapY();
						if (!bAPISwapY && nValue || bAPISwapY && !nValue) { // Invert current active cull mode
							switch (GetRenderState(PLRenderer::RenderState::CullMode)) {
								case PLRenderer::Cull::CW:
									nValue = PLRenderer::Cull::CCW;
									break;

								case PLRenderer::Cull::CCW:
									nValue = PLRenderer::Cull::CW;
									break;
							}
						} else { // Use current cull mode
							switch (GetRenderState(PLRenderer::RenderState::CullMode)) {
								case PLRenderer::Cull::CW:
									nValue = PLRenderer::Cull::CW;
									break;

								case PLRenderer::Cull::CCW:
									nValue = PLRenderer::Cull::CCW;
									break;
							}
						}

						// Set the state
						const uint32 &nAPIValue = m_cPLE_CULLWrapper[nValue];
						if (&nAPIValue != &Array<uint32>::Null) {
							glFrontFace(nAPIValue);
						} else {
							// Error, invalid value!
							return false;
						}
					}
					break;

				case PLRenderer::RenderState::FixedFillMode:
					// Not supported by OpenGL ES 2.0
					break;

				default:
					return false; // Error, invalid render state!
			}
		}
	}

	// All went fine
	return true;
}

bool Renderer::SetSamplerState(uint32 nStage, PLRenderer::Sampler::Enum nState, uint32 nValue)
{
	// Check if the stage is correct and check if the state is a valid sampler member
	if (nStage >= m_sCapabilities.nMaxTextureUnits || nState >= PLRenderer::Sampler::Number)
		return false; // Error!

	// Set the sampler state
	m_ppnSamplerState[nStage][nState] = nValue;

	{ // Handle rectangle texture restrictions
		PLRenderer::TextureBuffer *pTextureBuffer = m_ppCurrentTextureBuffer[nStage];
		if (pTextureBuffer && pTextureBuffer->GetType() == PLRenderer::Resource::TypeTextureBuffer2D && ((TextureBuffer2D*)pTextureBuffer)->IsRectangleTexture()) {
			// ... just set this fixed settings, and let it go...
			glActiveTexture(GL_TEXTURE0 + nStage);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (m_ppnSamplerState[nStage][PLRenderer::Sampler::MinFilter] == PLRenderer::TextureFiltering::None) ? GL_NEAREST : GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (m_ppnSamplerState[nStage][PLRenderer::Sampler::MagFilter] == PLRenderer::TextureFiltering::None) ? GL_NEAREST : GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

			// Done
			return true;
		}
	}

	// Check if this sampler state is already set to this value and whether currently a correct
	// texture buffer is used
	// [TODO]
	if (m_ppnInternalSamplerState[nStage][nState] != nValue) {
//	if (m_ppnInternalSamplerState[nStage][nState] != nValue && m_nTextureBufferTypes[nStage]) {
		// Check whether mipmapping is allowed, if not, set a proper internal texture filter mode
		// for correct rendering...
		if (nState == PLRenderer::Sampler::MipFilter) {
			PLRenderer::TextureBuffer *pTextureBuffer = m_ppCurrentTextureBuffer[nStage];
			if (pTextureBuffer && !(pTextureBuffer->GetFlags() & PLRenderer::TextureBuffer::Mipmaps)) {
				nValue = PLRenderer::TextureFiltering::None;

				// Check if this sampler state is already set to this value
				if (m_ppnInternalSamplerState[nStage][nState] == nValue)
					return true; // Nothing to do here :)
			}
		}

		// Set the sampler state
		m_ppnInternalSamplerState[nStage][nState] = nValue;
		// [TODO]
		const uint32 nType = GL_TEXTURE_2D;
		//const uint32 nType = m_nTextureBufferTypes[nStage];
		m_sStatistics.nSamplerStateChanges++;

		// Activate the correct OpenGL ES texture unit
		glActiveTexture(GL_TEXTURE0 + nStage);

		// Set sampler state
		switch (nState) {
		// Address modes
			case PLRenderer::Sampler::AddressU:
			{
				const uint32 &nAPIValue = m_cPLE_TAWrapper[nValue];
				if (&nAPIValue != &Array<uint32>::Null) {
					glTexParameteri(nType, GL_TEXTURE_WRAP_S, nAPIValue);
				} else {
					// Error, invalid value!
					return false;
				}
				break;
			}

			case PLRenderer::Sampler::AddressV:
			{
				const uint32 &nAPIValue = m_cPLE_TAWrapper[nValue];
				if (&nAPIValue != &Array<uint32>::Null) {
					glTexParameteri(nType, GL_TEXTURE_WRAP_T, nAPIValue);
				} else {
					// Error, invalid value!
					return false;
				}
				break;
			}

			case PLRenderer::Sampler::AddressW:
			{
				const uint32 &nAPIValue = m_cPLE_TAWrapper[nValue];
				if (&nAPIValue != &Array<uint32>::Null) {
					// [TODO] GL_OES_texture3D
					// glTexParameteri(nType, GL_TEXTURE_WRAP_R_OES, nAPIValue);
				} else {
					// Error, invalid value!
					return false;
				}
				break;
			}

		// Filter
			case PLRenderer::Sampler::MinFilter:
				// Combine with existing mip filter
				glTexParameteri(nType, GL_TEXTURE_MIN_FILTER, GetCombinedMinMipFilter(nStage));
				break;

			case PLRenderer::Sampler::MagFilter:
				switch (nValue) {
					case PLRenderer::TextureFiltering::Anisotropic: // GL treats linear and aniso the same
					case PLRenderer::TextureFiltering::Linear:
						glTexParameteri(nType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					break;

					case PLRenderer::TextureFiltering::Point:
					case PLRenderer::TextureFiltering::None:
						glTexParameteri(nType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					break;
				}
				break;

			case PLRenderer::Sampler::MipFilter:
				// Combine with existing min filter
				glTexParameteri(nType, GL_TEXTURE_MIN_FILTER, GetCombinedMinMipFilter(nStage));
				break;

		// Misc
			case PLRenderer::Sampler::MipmapLODBias:
				// Not supported by OpenGL ES 2.0
				break;

			case PLRenderer::Sampler::MaxMapLevel:
				// Not supported by OpenGL ES 2.0
				break;

			case PLRenderer::Sampler::MaxAnisotropy:
				if (m_sCapabilities.nMaxAnisotropy) {
					if (nValue > m_sCapabilities.nMaxAnisotropy)
						nValue = m_sCapabilities.nMaxAnisotropy;
					// [TODO] GL_EXT_texture_filter_anisotropic
					// glTexParameteri(nType, GL_TEXTURE_MAX_ANISOTROPY_EXT, nValue);
					break;
				}
				break;

			default:
				return false; // Invalid sampler state!
		}
	}

	// All went fine
	return true;
}


//[-------------------------------------------------------]
//[ Misc                                                  ]
//[-------------------------------------------------------]
bool Renderer::BeginScene()
{
	// Done
	return true;
}

bool Renderer::EndScene()
{
	// Done
	return true;
}

bool Renderer::SetViewport(const PLMath::Rectangle *pRectangle, float fMinZ, float fMaxZ)
{
	// Call base implementation
	PLRenderer::RendererBackend::SetViewport(pRectangle);

	// OpenGL assumes LOWER-left corner of the viewport rectangle, in pixels
	// and TOP-left corner given - so fit it :)
	GLint nY = (GLint)m_cViewportRect.GetY();
	if (m_cCurrentSurface.GetSurface())
		nY = m_cCurrentSurface.GetSurface()->GetSize().y-GLint(m_cViewportRect.vMax.y);

	// Set viewport
	glViewport(GLint(m_cViewportRect.GetX()), nY, GLint(m_cViewportRect.GetWidth()), (GLint)m_cViewportRect.GetHeight());

	// Set depth range
	glDepthRangef(fMinZ, fMaxZ);

	// Done
	return true;
}

bool Renderer::SetScissorRect(const PLMath::Rectangle *pRectangle)
{
	// Call base implementation
	PLRenderer::RendererBackend::SetScissorRect(pRectangle);

	// OpenGL assumes LOWER-left corner of the viewport rectangle, in pixels
	// and TOP-left corner given - so fit it :)
	GLint nY = (GLint)m_cScissorRect.GetY();
	if (m_cCurrentSurface.GetSurface())
		nY = m_cCurrentSurface.GetSurface()->GetSize().y-GLint(m_cScissorRect.vMax.y);

	// Set scissor rectangle
	glScissor(GLint(m_cScissorRect.GetX()), nY, GLint(m_cScissorRect.GetWidth()), (GLint)m_cScissorRect.GetHeight());

	// Done
	return true;
}

bool Renderer::GetDepthBounds(float &fZMin, float &fZMax) const
{
	// Get data
	fZMin = m_fDepthBoundsZMin;
	fZMax = m_fDepthBoundsZMax;

	// Done
	return true;
}

bool Renderer::SetDepthBounds(float fZMin, float fZMax)
{
	// Set data
	m_fDepthBoundsZMin = fZMin;
	m_fDepthBoundsZMax = fZMax;

	// Not available in OpenGL ES 2.0

	// Done
	return true;
}

void Renderer::GetColorMask(bool &bRed, bool &bGreen, bool &bBlue, bool &bAlpha) const
{
	// Get color mask
	bRed   = m_bColorMask[0];
	bGreen = m_bColorMask[1];
	bBlue  = m_bColorMask[2];
	bAlpha = m_bColorMask[3];
}

bool Renderer::SetColorMask(bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
	// Set color mask
	m_bColorMask[0] = bRed;
	m_bColorMask[1] = bGreen;
	m_bColorMask[2] = bBlue;
	m_bColorMask[3] = bAlpha;
	glColorMask(bRed, bGreen, bBlue, bAlpha);

	// Done
	return true;
}

bool Renderer::Clear(uint32 nFlags, const Color4 &cColor, float fZ, uint32 nStencil)
{
	// Get API flags
	uint32 nFlagsAPI = 0;
	if (nFlags & PLRenderer::Clear::Color)
		nFlagsAPI |= GL_COLOR_BUFFER_BIT;
	if (nFlags & PLRenderer::Clear::ZBuffer)
		nFlagsAPI |= GL_DEPTH_BUFFER_BIT;
	if (nFlags & PLRenderer::Clear::Stencil)
		nFlagsAPI |= GL_STENCIL_BUFFER_BIT;

	// Are API flags set?
	if (!nFlagsAPI)
		return false; // Error!

	// Set clear settings
	if (nFlags & PLRenderer::Clear::Color)
		glClearColor(cColor.r, cColor.g, cColor.b, cColor.a);
	uint32 nZWriteEnableT = 0;
	if (nFlags & PLRenderer::Clear::ZBuffer) {
		nZWriteEnableT = GetRenderState(PLRenderer::RenderState::ZWriteEnable);
		SetRenderState(PLRenderer::RenderState::ZWriteEnable, true);
		glClearDepthf(fZ);
	}
	if (nFlags & PLRenderer::Clear::Stencil)
		glClearStencil(nStencil);

	// Set color mask to default because OpenGL is using the color mask also for clearing (while DirectX does not :/)
	if (nFlags & PLRenderer::Clear::Color)
		glColorMask(true, true, true, true);

	// Clear
	glClear(nFlagsAPI);

	// Reset to current color mask
	if (nFlags & PLRenderer::Clear::Color) 
		glColorMask(m_bColorMask[0], m_bColorMask[1], m_bColorMask[2], m_bColorMask[3]);

	// Reset z write enable
	if (nFlags & PLRenderer::Clear::ZBuffer)
		SetRenderState(PLRenderer::RenderState::ZWriteEnable, nZWriteEnableT);

	// Done
	return true;
}


//[-------------------------------------------------------]
//[ Get/set current resources                             ]
//[-------------------------------------------------------]
bool Renderer::SetRenderTarget(PLRenderer::Surface *pSurface, uint8 nFace)
{
	// Check parameter
	if (!m_lstSurfaces.IsElement(pSurface))
		return false; // Error!

	// Check face index
	if (pSurface->GetType() == PLRenderer::Surface::TextureBuffer) {
		PLRenderer::SurfaceTextureBuffer *pSurfaceTextureBuffer = (PLRenderer::SurfaceTextureBuffer*)pSurface;
		if (pSurfaceTextureBuffer->GetTextureBuffer()) {
			if (pSurfaceTextureBuffer->GetTextureBuffer()->GetType() == PLRenderer::Resource::TypeTextureBufferCube) {
				if (nFace > 5) return false; // Error!
			} else {
				if (nFace > 0) return false; // Error!
			}
		} else return false; // Error!?!
	} else {
		if (nFace > 0) return false; // Error!
	}

	if (m_cCurrentSurface.GetSurface()) UnmakeSurfaceCurrent(*m_cCurrentSurface.GetSurface());
	m_cCurrentSurface.SetSurface(pSurface);

	// Make the surface to the current render target
	bool bError = MakeSurfaceCurrent(*pSurface, nFace);

	// Setup viewport and scissor rectangle
	SetViewport();
	SetScissorRect();

	// Done
	return bError;
}

bool Renderer::SetColorRenderTarget(PLRenderer::TextureBuffer *pTextureBuffer, uint8 nColorIndex, uint8 nFace)
{
/*	// Check parameter
	if (!m_lstSurfaces.IsElement(pSurface)) return true;
	if (m_cCurrentSurface.GetSurface()) m_cCurrentSurface.GetSurface()->UnmakeCurrent();
	m_cCurrentSurface.SetSurface(pSurface);

	// Make the surface to the current render target
	bool bError = pSurface->MakeCurrent(nFace);

	// Done
	return bError;
	*/
	// [TODO] Implement
	return false; // Error!
}

bool Renderer::MakeScreenshot(PLGraphics::Image &cImage)
{
	// [TODO] Render To Texture
	// In case the current surface is a texture, we need to 'finish' the current rendering process
//	PLRenderer::Surface *pSurface = m_cCurrentSurface.GetSurface();
//	if (pSurface && pSurface->GetType() == PLRenderer::Surface::TextureBuffer)
//		((SurfaceTextureBuffer*)pSurface)->Finish();

	// Get viewport data
	GLint nViewPort[4];
	glGetIntegerv(GL_VIEWPORT, nViewPort);

	// Create image buffer
	cImage.Clear();
	ImageBuffer *pImageBuffer = cImage.CreatePart()->CreateMipmap();
	pImageBuffer->CreateImage(DataByte, ColorRGBA, Vector3i(nViewPort[2], nViewPort[3], 1));

	// Get image data
	glReadPixels(nViewPort[0], nViewPort[1], nViewPort[2], nViewPort[3], GL_RGBA, GL_UNSIGNED_BYTE, pImageBuffer->GetData());

	// We need to flip the y axis because the image center of OpenGL is
	// lower left but the image center of our image class is upper left
	ImageEffects::FlipYAxis().Apply(*pImageBuffer);

	// Done
	return true;
}

bool Renderer::SetTextureBuffer(int nStage, PLRenderer::TextureBuffer *pTextureBuffer)
{
	// Set all available texture stages to this setting?
	if (nStage < 0) {
		for (uint32 i=0; i<m_sCapabilities.nMaxTextureUnits; i++)
			SetTextureBuffer(i, pTextureBuffer);
	} else {
		// Check if the stage is correct
		if (nStage >= m_sCapabilities.nMaxTextureUnits) return false; // Error!

		// Is this texture buffer already set?
		if (m_ppCurrentTextureBuffer[nStage] == pTextureBuffer) return false; // Error!

		// Make this texture buffer to the renderers current one
		PLRenderer::TextureBuffer *pT = m_ppCurrentTextureBuffer[nStage];
		m_ppCurrentTextureBuffer[nStage] = pTextureBuffer;

		// Should an texture buffer be set?
		if (pTextureBuffer) {
			// Yes, make it current
			m_sStatistics.nTextureBufferBinds++;
			if (!MakeTextureBufferCurrent(*pTextureBuffer, nStage)) {
				m_ppCurrentTextureBuffer[nStage] = pT;

				// Error!
				return false;
			}
		}
	}

	// Done
	return true;
}

bool Renderer::SetIndexBuffer(PLRenderer::IndexBuffer *pIndexBuffer)
{
	// Is this index buffer already set?
	if (m_pCurrentIndexBuffer == pIndexBuffer)
		return false; // Error!

	// Make this index buffer to the renderers current one
	m_pCurrentIndexBuffer = pIndexBuffer;

	// Should an index buffer be set?
	if (pIndexBuffer) {
		// Yes, make it current
		if (!((IndexBuffer*)pIndexBuffer)->MakeCurrent()) {
			// Now, no index buffer is set...
			m_pCurrentIndexBuffer = nullptr;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			// Error!
			return false;
		}
	} else {
		// No, deactivate index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// Done
	return true;
}

bool Renderer::SetProgram(PLRenderer::Program *pProgram)
{
	// Is the new program the same one as the current one?
	PLRenderer::Program *pCurrentProgram = (PLRenderer::Program*)m_cProgramHandler.GetResource();
	if (pCurrentProgram != pProgram) {
		// Was there a previous program? (must be GLSL because that's the only supported shader language in here :D)
		if (pCurrentProgram)
			((ProgramGLSL*)pCurrentProgram)->UnmakeCurrent();

		// Update the program resource handler
		m_cProgramHandler.SetResource(pProgram);

		// Make the new program to the current one (must be GLSL because that's the only supported shader language in here :D)
		if (pProgram)
			return ((ProgramGLSL*)pProgram)->MakeCurrent();
	}

	// Done
	return true;
}


//[-------------------------------------------------------]
//[ Draw                                                  ]
//[-------------------------------------------------------]
bool Renderer::DrawPrimitives(PLRenderer::Primitive::Enum nType, uint32 nStartIndex, uint32 nNumVertices)
{
	// Draw something?
	if (!nNumVertices)
		return true; // Done

	// Get number of primitives
	uint32 nPrimitiveCount;
	switch (nType) {
		case PLRenderer::Primitive::PointList:	   nPrimitiveCount = nNumVertices;   break;
		case PLRenderer::Primitive::LineList:	   nPrimitiveCount = nNumVertices-1; break;
		case PLRenderer::Primitive::LineStrip:	   nPrimitiveCount = nNumVertices-1; break;
		case PLRenderer::Primitive::TriangleList:  nPrimitiveCount = nNumVertices/3; break;
		case PLRenderer::Primitive::TriangleStrip: nPrimitiveCount = nNumVertices-2; break;
		case PLRenderer::Primitive::TriangleFan:   nPrimitiveCount = nNumVertices-2; break;
		default:								   return false; // Error!
	}

	// Update statistics
	m_sStatistics.nDrawPrimitivCalls++;
	m_sStatistics.nVertices  += nNumVertices;
	m_sStatistics.nTriangles += nPrimitiveCount;

	// Get API primitive type
	const uint32 &nAPIValue = m_cPLE_PTWrapper[nType];
	if (&nAPIValue != &Array<uint32>::Null) {
		glDrawArrays(nAPIValue, nStartIndex, nNumVertices);

		// Done
		return true;
	} else {
		// Error, invalid value!
		return false;
	}
}

bool Renderer::DrawIndexedPrimitives(PLRenderer::Primitive::Enum nType, uint32 nMinIndex, uint32 nMaxIndex, uint32 nStartIndex, uint32 nNumVertices)
{
	// Index and vertex buffer correct?
	if (!m_pCurrentIndexBuffer)
		return false; // Error!

	// Draw something?
	if (!nNumVertices)
		return true; // Done

	// Check parameters
	if (nStartIndex+nNumVertices > m_pCurrentIndexBuffer->GetNumOfElements() || nMinIndex > nMaxIndex)
		return false; // Definitely NOT good...

	// Get API dependent type
	uint32 nTypeSize;
	uint32 nTypeAPI = m_pCurrentIndexBuffer->GetElementType();
	if (nTypeAPI == PLRenderer::IndexBuffer::UShort) {
		nTypeSize = sizeof(uint16);
		nTypeAPI = GL_UNSIGNED_SHORT;
	} else if (nTypeAPI == PLRenderer::IndexBuffer::UByte) {
		nTypeSize = sizeof(uint8);
		nTypeAPI = GL_UNSIGNED_BYTE;
	} else {
		// UInt is not supported by OpenGL ES 2.0

		// Error!
		return false;
	}

	// Get number of primitives
	uint32 nPrimitiveCount;
	switch (nType) {
		case PLRenderer::Primitive::PointList:	   nPrimitiveCount = nNumVertices;   break;
		case PLRenderer::Primitive::LineList:	   nPrimitiveCount = nNumVertices-2; break;
		case PLRenderer::Primitive::LineStrip:	   nPrimitiveCount = nNumVertices-2; break;
		case PLRenderer::Primitive::TriangleList:  nPrimitiveCount = nNumVertices/3; break;
		case PLRenderer::Primitive::TriangleStrip: nPrimitiveCount = nNumVertices-2; break;
		case PLRenderer::Primitive::TriangleFan:   nPrimitiveCount = nNumVertices-2; break;
		default:								   return false; // Error!
	}

	// Update statistics
	m_sStatistics.nDrawPrimitivCalls++;
	m_sStatistics.nVertices  += nNumVertices;
	m_sStatistics.nTriangles += nPrimitiveCount;

	// Get API primitive type
	const uint32 &nAPIValue = m_cPLE_PTWrapper[nType];
	if (&nAPIValue != &Array<uint32>::Null) {
		// Draw primitive
		glDrawElements(nAPIValue, nNumVertices, nTypeAPI, (char*)((IndexBuffer*)m_pCurrentIndexBuffer)->GetDynamicData()+(nStartIndex*nTypeSize));
	} else {
		// Error, invalid value!
		return false;
	}

	// Done
	return true;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // PLRendererOpenGLES
