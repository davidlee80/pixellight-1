/*********************************************************\
 *  File: Application.cpp                                *
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
#include <PLGeneral/Log/Log.h>
#include <PLGeneral/System/System.h>
#include <PLCore/Tools/Localization.h>
#include <PLGui/Gui/Base/Keys.h>
#include <PLGui/Widgets/Widget.h>
#include <PLScene/Scene/SPScene.h>
#include <PLScene/Scene/SceneContainer.h>
#include <PLPhysics/Body.h>
#include <PLPhysics/World.h>
#include <PLPhysics/ContactInformation.h>
#include <PLPhysics/SceneNodes/SCPhysicsWorld.h>
#include <PLPhysics/SceneNodeModifiers/SNMPhysicsBodyBox.h>
#include "Application.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
using namespace PLGeneral;
using namespace PLMath;
using namespace PLGui;
using namespace PLRenderer;
using namespace PLScene;
using namespace PLPhysics;


//[-------------------------------------------------------]
//[ Public functions                                      ]
//[-------------------------------------------------------]
/**
*  @brief
*    Constructor
*/
Application::Application() : BasicSceneApplication(),
	EventHandlerKeyDown(&Application::NotifyKeyDown, this),
	EventHandlerKeyUp  (&Application::NotifyKeyUp,   this),
	EventHandlerContact(&Application::NotifyContact, this),
	m_pLine(nullptr),
	m_pFallingBox(nullptr),
	m_bApplyForce(false),
	m_bTorqueForce(false)
{
	// Set application name and title
	SetName("PLDemoPhysicsBasic");
	SetTitle(PLT("PL basic physics demo"));
	SetAppDataSubdir(System::GetInstance()->GetDataDirName("PixelLight"));

	// Add command line argument for choosing the physics API
	m_cCommandLine.AddArgument("PhysicsAPI", "Physics API", "PLPhysicsNewton::World", false);
}

/**
*  @brief
*    Destructor
*/
Application::~Application()
{
}

/**
*  @brief
*    Returns the name of the used physics API
*/
String Application::GetPhysicsAPI() const
{
	return m_sPhysicsAPI;
}


//[-------------------------------------------------------]
//[ Private functions                                     ]
//[-------------------------------------------------------]
/**
*  @brief
*    Updates the text of the mass text node
*/
void Application::UpdateMassTextNode()
{
	// Get the root scene
	SceneContainer *pRootContainer = GetRootScene();
	if (pRootContainer) {
		// Get the scene info text container
		SceneContainer *pSceneContainer = static_cast<SceneContainer*>(pRootContainer->Get("InfoText"));
		if (pSceneContainer) {
			// Get the mass text scene node
			SceneNode *pInfoTextNode = pSceneContainer->Get("Mass");
			if (pInfoTextNode) {
				// Get the scene container
				pSceneContainer = static_cast<SceneContainer*>(pRootContainer->Get("Scene"));
				if (pSceneContainer) {
					// Get the current mass of the small falling box
					SceneNode *pSceneNode = pSceneContainer->Get("SmallBox");
					if (pSceneNode) {
						const SceneNodeModifier *pModifier = pSceneNode->GetModifier("PLPhysics::SNMPhysicsBodyBox");
						if (pModifier)
							pInfoTextNode->SetAttribute("Text", PLT("m=Change mass of small falling box (current: ") + pModifier->GetAttribute("Mass")->GetString() + ')');
					}
				}
			}
		}
	}
}

/**
*  @brief
*    Function where the user has to choose the desired physics API
*/
bool Application::ChoosePhysicsAPI()
{
	// Because I'am pessimistic I espect by default that the user has canceled the choice *sigh*
	bool bResult = false; // Error by default

	// Are there multiple physics API's available?
	List<const PLCore::Class*> lstClasses;
	PLCore::ClassManager::GetInstance()->GetClasses(lstClasses, "PLPhysics::World", PLCore::Recursive, PLCore::NoBase, PLCore::NoAbstract);
	if (lstClasses.GetNumOfElements()) {
		if (lstClasses.GetNumOfElements() == 1) {
			// Get the class name of the physics API
			m_sPhysicsAPI = lstClasses[0]->GetClassName();

			// An API was 'chosen'...
			bResult = true; // Done
		} else {
			// [TODO] PLGui: Selection dialog
			m_sPhysicsAPI = "PLPhysicsNewton::World";
			bResult = true; // Done
			/*
			// Show your choose dialog
			ChoosePhysicsAPIGui *pDialog = new ChoosePhysicsAPIGui();
			if (PLGui::GuiManager::GetInstance()->GetSystemGui()->ShowDialogModal(*pDialog, nullptr) && m_sPhysicsAPI != pDialog->GetName()) {
				// A new API was chosen...
				m_sPhysicsAPI = pDialog->GetName();
				bResult = true; // Done
			}
			pDialog->Destroy();
			*/
		}

		// Was an API chosen?
		if (bResult) {
			// Destroy the old root scene
			SceneContainer *pRootScene = GetRootScene();
			if (pRootScene) {
				// Setup scene surface painter
				SurfacePainter *pPainter = GetPainter();
				if (pPainter && pPainter->IsInstanceOf("PLScene::SPScene")) {
					SPScene *pSPScene = static_cast<SPScene*>(pPainter);
					pSPScene->SetRootContainer(nullptr);
					pSPScene->SetSceneContainer(nullptr);
					pSPScene->SetCamera(nullptr);
				}

				// Destroy old root scene
				SetRootScene(nullptr);
				delete pRootScene;
			}

			// Reset variables
			m_pLine        = nullptr;
			m_pFallingBox  = nullptr;
			m_bApplyForce  = false;
			m_bTorqueForce = false;

			// Create new root scene
			OnCreateRootScene();
		}
	} else {
		PL_LOG(Error, "No physics API available")
	}

	// Done
	return bResult;
}

/**
*  @brief
*    Returns the PL physics body of the given scene node
*/
Body *Application::GetPhysicsBody(SceneNode &cSceneNode) const
{
	// Get the PL physics body scene node modifier of the given scene node
	const SNMPhysicsBodyBox *pModifier = static_cast<SNMPhysicsBodyBox*>(cSceneNode.GetModifier("PLPhysics::SNMPhysicsBodyBox"));

	// Return the PL physics body this modifier is using
	return pModifier ? pModifier->GetBody() : nullptr;
}

/**
*  @brief
*    Called when a key is pressed down
*/
void Application::NotifyKeyDown(uint32 nKey, uint32 nModifiers)
{
	switch (nKey) {
		// Check whether the escape key was pressed
		case PLGUIKEY_ESCAPE:
			// Shut down application
			Exit(0);
			break;

		// Choose another physics API?
		// [TODO] PLGui: Selection dialog
//		case PLGUIKEY_C:
//			ChoosePhysicsAPI();
//			break;

		// Reset the small falling physics box?
		case PLGUIKEY_R:
			// Check your pointer to the falling box
			if (m_pFallingBox) {
				// Reset the position and rotation
				m_pFallingBox->GetTransform().SetPosition(Vector3(1.5f, 4.0f, 7.2f));
				m_pFallingBox->GetTransform().SetRotation(Quaternion::Identity);

				// Set some special PL physics body properties
				Body *pBody = GetPhysicsBody(*m_pFallingBox);
				if (pBody) {
					// Reset the linear and angular velocity
					pBody->SetLinearVelocity(Vector3::Zero);
					pBody->SetAngularVelocity(Vector3::Zero);

					// Unfreeze the physics box
					pBody->SetFrozen(false);
				}
			}
			break;

		// Apply a force to the small falling physics box?
		case PLGUIKEY_F:
			m_bApplyForce = true;

			// Update the line scene node
			if (m_pLine) {
				m_pLine->SetActive(true);
				m_pLine->SetAttribute("StartPosition", "1.5 4.0 7.7");
			}
			break;

		// Apply a torque to the small falling physics box?
		case PLGUIKEY_T:
			m_bTorqueForce = true;
			break;

		// Change mass of the small falling physics box?
		case PLGUIKEY_M:
			// Check your pointer to the falling box
			if (m_pFallingBox) {
				SceneNodeModifier *pModifier = m_pFallingBox->GetModifier("PLPhysics::SNMPhysicsBodyBox");
				if (pModifier) {
					// Get the current mass
					const float fValue = pModifier->GetAttribute("Mass")->GetFloat();

					// Set the new mass
					if (fValue == 1.0f)
						pModifier->SetAttribute("Mass", "10.0");
					else if (fValue == 10.0f)
						pModifier->SetAttribute("Mass", "100.0");
					else if (fValue == 100.0f)
						pModifier->SetAttribute("Mass", "0.01");
					else if (fValue == 0.01f)
						pModifier->SetAttribute("Mass", "1.0");

					// Update the mass text node
					UpdateMassTextNode();
				}
			}
			break;
	}
}

/**
*  @brief
*    Called when a key is released
*/
void Application::NotifyKeyUp(uint32 nKey, uint32 nModifiers)
{
	switch (nKey) {
		// Apply a force to the small falling physics box?
		case PLGUIKEY_F:
			m_bApplyForce = false;

			// Update the line scene node
			if (m_pLine)
				m_pLine->SetActive(false);
			break;

		// Apply a torque to the small falling physics box?
		case PLGUIKEY_T:
			m_bTorqueForce = false;
			break;
	}
}

/**
*  @brief
*    Called when a contact between two bodies was detected by the physics
*/
void Application::NotifyContact(ContactInformation &cContactInformation)
{
	// First PING!
	SceneNode *pSceneNode = GetScene()->Get("ContactText_FirstBody");
	if (pSceneNode) {
		Vector3 vPosition;
		cContactInformation.GetFirstBody().GetPosition(vPosition);
		pSceneNode->GetTransform().SetPosition(vPosition);
		pSceneNode->SetAttribute("Text", "PING!");
		pSceneNode->SetActive(true);
		SceneNodeModifier *pSceneNodeModifier = pSceneNode->GetModifier("PLScene::SNMDeactivationOnTimeout");
		if (pSceneNodeModifier)
			pSceneNodeModifier->SetAttribute("Timeout", "0.5");
	}

	// Second PONG!
	pSceneNode = GetScene()->Get("ContactText_SecondBody");
	if (pSceneNode) {
		Vector3 vPosition;
		cContactInformation.GetSecondBody().GetPosition(vPosition);
		pSceneNode->GetTransform().SetPosition(vPosition);
		pSceneNode->SetAttribute("Text", "PONG!");
		pSceneNode->SetActive(true);
		SceneNodeModifier *pSceneNodeModifier = pSceneNode->GetModifier("PLScene::SNMDeactivationOnTimeout");
		if (pSceneNodeModifier)
			pSceneNodeModifier->SetAttribute("Timeout", "0.5");
	}
}


//[-------------------------------------------------------]
//[ Private virtual PLGui::GuiApplication functions       ]
//[-------------------------------------------------------]
void Application::OnInit()
{
	// Valid default physics API given?
	String sClassName = m_cCommandLine.GetValue("PhysicsAPI");
	if (sClassName.GetLength()) {
		const PLCore::Class *pClass = PLCore::ClassManager::GetInstance()->GetClass(sClassName);
		if (pClass && pClass->IsDerivedFrom("PLPhysics::World")) {
			m_sPhysicsAPI = sClassName;

			// Create new root scene
			OnCreateRootScene();
		}
	}

	// Choose physics API
	if (!m_sPhysicsAPI.GetLength() && !ChoosePhysicsAPI()) {
		// Error!
		Exit(1);
	}
}

void Application::OnCreateMainWindow()
{
	// Call base implementation
	BasicSceneApplication::OnCreateMainWindow();

	// Connect event handler
	Widget *pWidget = GetMainWindow();
	if (pWidget) {
		pWidget->EventKeyDown.Connect(&EventHandlerKeyDown);
		pWidget->EventKeyUp.  Connect(&EventHandlerKeyUp);
		// [TODO] Linux: Currently we need to listen to the content widget key events as well ("focus follows mouse"-topic)
		if (pWidget->GetContentWidget() != pWidget) {
			pWidget->GetContentWidget()->EventKeyDown.Connect(&EventHandlerKeyDown);
			pWidget->GetContentWidget()->EventKeyUp.  Connect(&EventHandlerKeyUp);
		}
	}
}


//[-------------------------------------------------------]
//[ Private virtual PLRenderer::RenderApplication functions ]
//[-------------------------------------------------------]
bool Application::OnUpdate()
{
	// One important word at the beginning: DON'T COPYCAT THIS!
	// The following is 'just' a simple demonstration how the scene graph 'can' be used. It's
	// definitely not good to update your scene nodes in the way you can see within this function.
	// Its quite to intricate, inflexible and not performant.

	// Call base implementation
	if (BasicSceneApplication::OnUpdate()) {
		// Check your pointer to the falling box
		if (m_pFallingBox && (m_bApplyForce || m_bTorqueForce)) {
			Body *pBody = GetPhysicsBody(*m_pFallingBox);
			if (pBody && pBody->GetWorld().IsSimulationActive()) {
				// Apply force?
				if (m_bApplyForce) {
					const Vector3 &vPos = m_pFallingBox->GetTransform().GetPosition();
					Vector3 vDir = Vector3(1.5f, 4.0f, 7.7f) - vPos;

					// Add a force (f=m*a) to the start position of the box
					pBody->AddForce(vDir*pBody->GetMass());

					// Update the line scene node
					if (m_pLine)
						m_pLine->SetAttribute("EndPosition", vPos.ToString());
				}

				// Apply torque?
				if (m_bTorqueForce) {
					// Add a torque
					pBody->AddTorque(Vector3(0.1f, 0.0f, 0.1f)*pBody->GetMass());
				}
			}
		}

		// Done
		return true;
	} else {
		// Not updated
		return false;
	}
}


//[-------------------------------------------------------]
//[ Private virtual PLEngine::BasicSceneApplication functions ]
//[-------------------------------------------------------]
void Application::OnCreateScene(SceneContainer &cContainer)
{
	// Set no scene container as default
	SetScene(nullptr);

	// Physics API given?
	if (m_sPhysicsAPI.GetLength()) {
		// Create a scene container with our 'concrete physics scene', the physics simulation is not active by default
		SceneNode *pSceneContainerNode = cContainer.Create("PLPhysics::SCPhysicsWorld", "Scene", "PhysicsAPI=\"" + m_sPhysicsAPI + "\" SimulationActive=\"0\"");
		if (pSceneContainerNode && pSceneContainerNode->IsInstanceOf("PLScene::SceneContainer")) {
			SceneContainer *pSceneContainer = static_cast<SceneContainer*>(pSceneContainerNode);

			// Protect this important container!
			pSceneContainer->SetProtected(true);

			// Populate the scene container
			// Create a camera
			SceneNode *pCamera = pSceneContainer->Create("PLScene::SNCamera", "FixedCamera", "Position=\"0.0 5.0 15.0\" Rotation=\"30.0 180.0 0.0\"");
			if (pCamera && pCamera->IsInstanceOf("PLScene::SNCamera")) {
				// Make this to our main scene camera
				SetCamera(reinterpret_cast<SNCamera*>(pCamera));
			}

			// Create an static physics scene node with the 'floor' mesh
			SceneNode *pSceneNode = pSceneContainer->Create("PLScene::SNMesh", "Floor", "Position=\"0.0 -1.08 5.0\" Scale=\"10.0 0.1 10.0\" Mesh=\"Default\"");
			// Add Newton physics body modifier to the floor scene node
			if (pSceneNode)
				pSceneNode->AddModifier("PLPhysics::SNMPhysicsBodyBox");

			// Create the small falling physics box scene node
			m_pFallingBox = pSceneContainer->Create("PLScene::SNMesh", "SmallBox", "Position=\"1.5 4.0 7.2\" Scale=\"0.5 0.5 0.5\" Mesh=\"Default\"");
			// Add Newton physics body modifier to the floor scene node
			if (m_pFallingBox)
				m_pFallingBox->AddModifier("PLPhysics::SNMPhysicsBodyBox", "Mass=\"1.0\" Flags=\"InitUnfrozen\"");

			// Create a big static physics box scene node
			pSceneNode = pSceneContainer->Create("PLScene::SNMesh", "BigBox", "Position=\"1.5 0.0 5.5\" Scale=\"1.5 1.5 1.5\" Mesh=\"Default\"");
			// Add Newton physics body modifier to the floor scene node
			if (pSceneNode)
				pSceneNode->AddModifier("PLPhysics::SNMPhysicsBodyBox");

			// Create a static physics box scene node
			pSceneNode = pSceneContainer->Create("PLScene::SNMesh", "Box", "Position=\"0.3 0.0 7.5\" Scale=\"1.0 1.0 1.0\" Mesh=\"Default\"");
			// Add Newton physics body modifier to the floor scene node
			if (pSceneNode)
				pSceneNode->AddModifier("PLPhysics::SNMPhysicsBodyBox");

			// Create a 'line'
			m_pLine = pSceneContainer->Create("PLScene::SNLine", "Line");

			// Setup scene surface painter
			SurfacePainter *pPainter = GetPainter();
			if (pPainter && pPainter->IsInstanceOf("PLScene::SPScene")) {
				SPScene *pSPScene = static_cast<SPScene*>(pPainter);
				pSPScene->SetRootContainer(cContainer.GetContainer());
				pSPScene->SetSceneContainer(pSceneContainer);
			}

			// Create contact information text scene nodes
			pSceneNode = pSceneContainer->Create("PLScene::SNText2D", "ContactText_FirstBody");
			if (pSceneNode)
				pSceneNode->AddModifier("PLScene::SNMDeactivationOnTimeout", "Timeout=\"0.5\"");
			pSceneNode = pSceneContainer->Create("PLScene::SNText2D", "ContactText_SecondBody");
			if (pSceneNode)
				pSceneNode->AddModifier("PLScene::SNMDeactivationOnTimeout", "Timeout=\"0.5\"");

			// Within the parent container...
			SceneContainer *pContainer = cContainer.GetContainer();
			if (pContainer) {
				// Add our information text scene nodes
				SceneNode *pInfoText = pContainer->Create("PLScene::SceneContainer", "InfoText", "Flags=\"NoCulling\"");
				if (pInfoText && pInfoText->IsInstanceOf("PLScene::SceneContainer")) {
					SceneContainer *pInfoTextContainer = static_cast<SceneContainer*>(pInfoText);
					
					// The name of the used physics API
					pInfoTextContainer->Create("PLScene::SNText2D", "PhysicsAPI", "Position=\"0.01 0.01\" Flags=\"No3DPosition|NoCenter\" Text=\"" + PLT("Physics API: ") + GetPhysicsAPI() + '\"');

					// Draw keys information
					pInfoTextContainer->Create("PLScene::SNText2D", "Keys",      "Position=\"0.01 0.04\" Flags=\"No3DPosition|NoCenter\" Text=\"" + PLT("Keys:")                                 + '\"');
					// [TODO] PLGui: Selection dialog
				//	pInfoTextContainer->Create("PLScene::SNText2D", "Choose",    "Position=\"0.06 0.06\" Flags=\"No3DPosition|NoCenter\" Text=\"" + PLT("c=Choose another physics API")          + '\"');
					pInfoTextContainer->Create("PLScene::SNText2D", "Reset",     "Position=\"0.06 0.08\" Flags=\"No3DPosition|NoCenter\" Text=\"" + PLT("r=Reset small falling box")             + '\"');
					pInfoTextContainer->Create("PLScene::SNText2D", "AddForce",  "Position=\"0.06 0.10\" Flags=\"No3DPosition|NoCenter\" Text=\"" + PLT("f=Add force to the small falling box")  + '\"');
					pInfoTextContainer->Create("PLScene::SNText2D", "AddTorque", "Position=\"0.06 0.12\" Flags=\"No3DPosition|NoCenter\" Text=\"" + PLT("t=Add torque to the small falling box") + '\"');
					pInfoTextContainer->Create("PLScene::SNText2D", "Mass",      "Position=\"0.06 0.14\" Flags=\"No3DPosition|NoCenter\"");
					UpdateMassTextNode();
				}
			}

			// Our scene is complete, now activate the physics simulation - let's rock'n'roll!
			pSceneContainer->SetAttribute("SimulationActive", "1");

			/*
			// This is an example how to disable contact generation between two physics bodies. We disable 'collision detection'
			// between 'Box' and 'SmallBox':
			// First, get the 'Box' scene node
			pSceneNode = pSceneContainer->Get("Box");
			if (pSceneNode) {
				// Get the physics body scene node modifier of the 'Box' scene node
				SNMPhysicsBodyBox *pBodyModifier = static_cast<SNMPhysicsBodyBox*>(pSceneNode->GetModifier("PLPhysics::SNMPhysicsBodyBox"));
				if (pBodyModifier) {
					// Get the physics body of 'Box'
					Body *pBoxBody = pBodyModifier->GetBody();
					// Second, get the 'SmallBox' scene node
					pSceneNode = pBoxBody ? pSceneContainer->Get("SmallBox") : nullptr;
					if (pSceneNode) {
						// Get the physics body scene node modifier of the 'SmallBox' scene node
						pBodyModifier = static_cast<SNMPhysicsBodyBox*>(pSceneNode->GetModifier("PLPhysics::SNMPhysicsBodyBox"));
						if (pBodyModifier) {
							// Get the physics body of 'SmallBox'
							Body *pSmallBoxBody = pBodyModifier->GetBody();
							if (pSmallBoxBody) {
								// ... and FINALLY set the desired body pair flag :)
								pBoxBody->GetWorld()->SetBodyPairFlags(pBoxBody, pSmallBoxBody, World::Ignore);
							}
						}
					}
				}
			}
			//*/
			/*
			// Instead of using the body pair flags to disable the contact generation between certain types of
			// bodies, you can also 'group' together this bodies and disable collision between this groups:
			pSceneNode = pSceneContainer->Get("Box");
			if (pSceneNode) {
				// Get the physics body scene node modifier of the 'Box' scene node
				SNMPhysicsBodyBox *pBodyModifier = static_cast<SNMPhysicsBodyBox*>(pSceneNode->GetModifier("PLPhysics::SNMPhysicsBodyBox"));
				if (pBodyModifier) {
					// Get the physics body of 'Box'
					Body *pBoxBody = pBodyModifier->GetBody();
					// Second, get the 'SmallBox' scene node
					pSceneNode = pBoxBody ? pSceneContainer->Get("SmallBox") : nullptr;
					if (pSceneNode) {
						// Get the physics body scene node modifier of the 'SmallBox' scene node
						pBodyModifier = static_cast<SNMPhysicsBodyBox*>(pSceneNode->GetModifier("PLPhysics::SNMPhysicsBodyBox"));
						if (pBodyModifier) {
							// Get the physics body of 'SmallBox'
							Body *pSmallBoxBody = pBodyModifier->GetBody();
							if (pSmallBoxBody) {
								// Put the 'Box' into group 1
								pBoxBody->SetCollisionGroup(1);
								// Put the 'SmallBox' into group 0 (the body is already within this group by default :)
								pSmallBoxBody->SetCollisionGroup(0);
								// ... and FINALLY set the desired collision group active state :)
								pBoxBody->GetWorld()->SetGroupCollision(pBoxBody->GetCollisionGroup(), pSmallBoxBody->GetCollisionGroup(), false);
							}
						}
					}
				}
			}
			//*/
			/*
			// It's also possible to deactivate the collision detection for a body...
			pSceneNode = pSceneContainer->Get("Box");
			if (pSceneNode) {
				// Get the physics body scene node modifier of the 'Box' scene node
				SNMPhysicsBodyBox *pBodyModifier = static_cast<SNMPhysicsBodyBox*>(pSceneNode->GetModifier("PLPhysics::SNMPhysicsBodyBox"));
				if (pBodyModifier) {
					// Get the physics body of 'Box'
					Body *pBoxBody = pBodyModifier->GetBody();
					// Get the 'SmallBox' scene node
					if (pBoxBody) {
						// ... and disable collision detection
						pBoxBody->SetCollisionActive(false);
					}
				}
			}
			//*/

			// Connect contact event handler
			if (pSceneContainer->IsInstanceOf("PLPhysics::SCPhysicsWorld")) {
				// Get the physics world
				World *pPhysicsWorld = static_cast<SCPhysicsWorld*>(pSceneContainer)->GetWorld();
				if (pPhysicsWorld)
					pPhysicsWorld->EventContact.Connect(&EventHandlerContact);
			}

			// Set scene container
			SetScene(pSceneContainer);
		}
	}
}
