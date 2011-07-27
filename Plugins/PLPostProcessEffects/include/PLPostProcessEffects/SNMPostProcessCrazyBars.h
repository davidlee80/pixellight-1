/*********************************************************\
 *  File: SNMPostProcessCrazyBars.h                      *
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


#ifndef __PLPOSTPROCESSEFFECTS_CRAZYBARS_H__
#define __PLPOSTPROCESSEFFECTS_CRAZYBARS_H__
#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include <PLCompositing/SceneNodeModifiers/SNMPostProcess.h>


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace PLPostProcessEffects {


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Crazy bars camera scene node post process modifier class
*
*  @remarks
*    Crazy bar.
*/
class SNMPostProcessCrazyBars : public PLCompositing::SNMPostProcess {


	//[-------------------------------------------------------]
	//[ RTTI interface                                        ]
	//[-------------------------------------------------------]
	pl_class(pl_rtti_export, SNMPostProcessCrazyBars, "PLPostProcessEffects", PLCompositing::SNMPostProcess, "")
		// Attributes
		pl_attribute(ColorScaleY,	float,			1.0f,								ReadWrite,	DirectValue,	"Color scale Y (0.002 for a rainbow :) )",	"")
			// Overloaded SNMPostProcess attributes
		pl_attribute(Filename,		PLCore::String,	"Data/PostProcesses/CrazyBars.pp",	ReadWrite,	ModifyAttr,		"Filename of the post process to use",		"Type='PostProcess'")
		// Constructors
		pl_constructor_1(ParameterConstructor,	PLScene::SceneNode&,	"Parameter constructor",	"")
		// Slots
		pl_slot_0(OnSceneNodeUpdate,	"Slot for SceneNode::EventUpdate",	"")
	pl_class_end


	//[-------------------------------------------------------]
	//[ Public functions                                      ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] cSceneNode
		*    Owner scene node
		*/
		SNMPostProcessCrazyBars(PLScene::SceneNode &cSceneNode);

		/**
		*  @brief
		*    Destructor
		*/
		virtual ~SNMPostProcessCrazyBars();


	//[-------------------------------------------------------]
	//[ Private virtual PLScene::SceneNodeModifier functions  ]
	//[-------------------------------------------------------]
	private:
		virtual void OnActivate(bool bActivate);


	//[-------------------------------------------------------]
	//[ Private functions                                     ]
	//[-------------------------------------------------------]
	private:
		/**
		*  @brief
		*    Called when the scene node needs to be updated
		*/
		void OnSceneNodeUpdate();


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		// Private data
		float m_fTimer;


	//[-------------------------------------------------------]
	//[ Public virtual PLCompositing::SNMPostProcess functions ]
	//[-------------------------------------------------------]
	public:
		virtual void SetParameters();


};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // PLPostProcessEffects


#endif // __PLPOSTPROCESSEFFECTS_CRAZYBARS_H__