/*********************************************************\
 *  File: SNBomb.h                                       *
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


#ifndef __PLSAMPLE_67_SNBOMB_H__
#define __PLSAMPLE_67_SNBOMB_H__
#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include <PLSound/SceneNodes/SNSound.h>


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Bomb scene node
*/
class SNBomb : public PLSound::SNSound {


	//[-------------------------------------------------------]
	//[ RTTI interface                                        ]
	//[-------------------------------------------------------]
	pl_class(pl_rtti_export, SNBomb, "", PLSound::SNSound, "Bomb scene node")
		// Attributes
		pl_attribute(Killed,	bool,					false,							ReadWrite,	DirectValue,	"Killed?",																								"")
			// Overwritten PLSound::SNSound attributes
		pl_attribute(Sound,		PLCore::String,			"Data/Sounds/Explosion.ogg",	ReadWrite,	GetSet,			"Filename of the sound which should be played (full path, supported file formats are API dependent)",	"Ext='mp3 ogg wav mid midi it mod s3m xm'")
			// Overwritten PLScene::SceneNode attributes
		pl_attribute(Flags,		pl_flag_type(EFlags),	NoCulling,						ReadWrite,	GetSet,			"Flags",																								"")
		// Constructors
		pl_constructor_0(DefaultConstructor,	"Default constructor",	"")
		// Slots
		pl_slot_0(OnUpdate,	"Called when the scene node needs to be updated",	"")
	pl_class_end


	//[-------------------------------------------------------]
	//[ Public functions                                      ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Default constructor
		*/
		SNBomb();

		/**
		*  @brief
		*    Destructor
		*/
		virtual ~SNBomb();

		/**
		*  @brief
		*    Returns the current frame
		*
		*  @return
		*    The current frame
		*/
		char GetFrame() const;


	//[-------------------------------------------------------]
	//[ Private functions                                     ]
	//[-------------------------------------------------------]
	private:
		/**
		*  @brief
		*    Called when the scene node needs to be updated
		*/
		void OnUpdate();


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		float m_fTimer;
		char  m_nFrame;
		float m_fFrame;
		bool  m_bAnimationDirection;


	//[-------------------------------------------------------]
	//[ Private virtual PLScene::SceneNode functions          ]
	//[-------------------------------------------------------]
	private:
		virtual void OnActivate(bool bActivate) override;


};


#endif // __PLSAMPLE_67_SNBOMB_H__
