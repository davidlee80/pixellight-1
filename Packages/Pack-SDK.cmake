#*********************************************************#
#*  File: Pack-SDK.cmake                                 *
#*
#*  Copyright (C) 2002-2011 The PixelLight Team (http://www.pixellight.org/)
#*
#*  This file is part of PixelLight.
#*
#*  PixelLight is free software: you can redistribute it and/or modify
#*  it under the terms of the GNU Lesser General Public License as published by
#*  the Free Software Foundation, either version 3 of the License, or
#*  (at your option) any later version.
#*
#*  PixelLight is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#*  GNU Lesser General Public License for more details.
#*
#*  You should have received a copy of the GNU Lesser General Public License
#*  along with PixelLight. If not, see <http://www.gnu.org/licenses/>.
#*********************************************************#


##################################################
## Prerequisites
##################################################

# Create copyright file used within the PixelLight SDK installer
configure_file("${CMAKE_SOURCE_DIR}/Packages/Files/copyright.txt.in" "${CMAKE_SOURCE_DIR}/Packages/Files/copyright.txt")
configure_file("${CMAKE_SOURCE_DIR}/Packages/Files/readme.txt.in" "${CMAKE_SOURCE_DIR}/Packages/Files/readme.txt")
configure_file("${CMAKE_SOURCE_DIR}/Packages/Files/welcome.txt.in" "${CMAKE_SOURCE_DIR}/Packages/Files/welcome.txt")

##################################################
## CPack configuration
##################################################
if(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
	##################################################
	## Package specific options
	##################################################
	set(CMAKE_MODULE_PATH		${CMAKE_SOURCE_DIR}/Packages/SDK)
	set(CPACK_PL_PACKAGE_PATH	${CMAKE_SOURCE_DIR}/Packages)
	if(MSVC10)
		include(${CMAKE_SOURCE_DIR}/External/Base/VC2010/Package.cmake  ${CMAKETOOLS_DIR}/CMakeTools_Packs.cmake)
	endif()

	##################################################
	## Includes
	##################################################
	include(${CMAKETOOLS_DIR}/CMakeTools_Packs.cmake)

	##################################################
	## Package information
	##################################################
	set(CPACK_PACKAGE_NAME					"PixelLight-SDK")
	set(CPACK_PACKAGE_VENDOR				"PixelLight") # No spaces allowed!
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY	"PixelLight Software Developer Kit - http://www.pixellight.org/")
	set(CPACK_PACKAGE_VERSION_MAJOR			"${CMAKETOOLS_PROJECT_VERSION_MAJOR}")
	set(CPACK_PACKAGE_VERSION_MINOR			"${CMAKETOOLS_PROJECT_VERSION_MINOR}")
	set(CPACK_PACKAGE_VERSION_PATCH			"${CMAKETOOLS_PROJECT_VERSION_PATCH}")
	set(CPACK_PACKAGE_VERSION_RELEASE		"0") # The last one must be a number - too, because we're using something like 'R1', we just don't mention the last number in here...
	set(CPACK_PACKAGE_LEGAL					"Copyright (C) 2002-2011 by The PixelLight Team")
	set(CPACK_RESOURCE_FILE_LICENSE			"${CMAKE_SOURCE_DIR}/Packages/Files/copyright.txt")
	set(CPACK_RESOURCE_FILE_README			"${CMAKE_SOURCE_DIR}/Packages/Files/readme.txt")
	set(CPACK_RESOURCE_FILE_WELCOME			"${CMAKE_SOURCE_DIR}/Packages/Files/welcome.txt")
	set(CPACK_PACKAGE_DESCRIPTION_FILE		"${CMAKE_SOURCE_DIR}/Packages/Files/readme.txt")
	set(CPACK_PACKAGE_ICON					"${CMAKE_SOURCE_DIR}/Packages/Files/pl_logo.bmp")
	set(CPACK_NSIS_DISPLAY_NAME				"PixelLight SDK")
	set(CPACK_NSIS_INSTALLED_ICON_NAME		"bin\\\\InstallD.exe")
	set(CPACK_NSIS_HELP_LINK				"http://www.pixellight.org")
	set(CPACK_NSIS_URL_INFO_ABOUT			"http://www.pixellight.org")
	set(CPACK_NSIS_CONTACT					"contact@pixellight.org")
	if(WIN32)
		make_windows_path(CPACK_PACKAGE_ICON)
	endif()

	##################################################
	## Debian package information
	##################################################
	set(CPACK_DEBIAN_PACKAGE_NAME			"pixellight")
	set(CPACK_DEBIAN_PACKAGE_VERSION		"${CMAKETOOLS_PROJECT_VERSION_MAJOR}.${CMAKETOOLS_PROJECT_VERSION_MINOR}.${CMAKETOOLS_PROJECT_VERSION_PATCH}-${CMAKETOOLS_PROJECT_VERSION_RELEASE}")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE	"all")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS		"libzip1 (>= 0.9-3), libncurses5 (>= 5.7+20090803-2ubuntu3), libpcre3 (>= 7.8-3build1), libjpeg62 (>= 6b-15ubuntu1), libpng12-0 (>= 1.2.42-1ubuntu2.1), libxcursor1 (>= 1:1.1.10-1), libxext6 (>= 2:1.1.1-2), libdbus-1-3 (>= 1.2.16-2ubuntu4), libfreetype6 (>= 2.3.11-1ubuntu2.2), libopenal1 (>= 1:1.12.854-0ubuntu1~lucid1), libogg0 (>= 1.1.4~dfsg-2), libvorbisfile3 (>= 1.2.3-3ubuntu1), libxxf86vm1 (>= 1:1.1.0-2), libglu1-mesa (>= 7.7.1-1ubuntu3)")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER		"contact@pixellight.org")
	set(CPACK_DEBIAN_PACKAGE_DESCRIPTION	${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
	set(CPACK_DEBIAN_PACKAGE_SECTION		"devel")
	set(CPACK_DEBIAN_PACKAGE_PRIORITY		"optional")
#	set(CPACK_DEBIAN_PACKAGE_RECOMMENDS 	"")
#	set(CPACK_DEBIAN_PACKAGE_SUGGESTS	 	"")
	set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA	"")

	##################################################
	## Package name
	##################################################
	# Use name convention "PixelLight-MAJOR.MINOR.PATCH-RELEASE-SYSTEM-Package"
	set(CPACK_PACKAGE_FILE_NAME "PixelLight-${CMAKETOOLS_PROJECT_VERSION_MAJOR}.${CMAKETOOLS_PROJECT_VERSION_MINOR}.${CMAKETOOLS_PROJECT_VERSION_PATCH}-${CMAKETOOLS_PROJECT_VERSION_RELEASE}-${CMAKETOOLS_TARGET_SYSTEM}-SDK")

	##################################################
	## Start menu icons
	##################################################
	# Add icons
	set(CPACK_NSIS_CREATE_ICONS " 
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\SDK-Browser.lnk\\\" \\\"$INSTDIR\\\\SDKBrowser.chm\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Website.lnk\\\" \\\"http://www.pixellight.org/\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\ReleaseNotes.lnk\\\" \\\"$INSTDIR\\\\ReleaseNotes.txt\\\"
		CreateDirectory \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\Samples.sln.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Samples.sln\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\05NetworkChat.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\05NetworkChat${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\45ScriptApplication.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\45ScriptApplication${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\50RendererTriangle.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\50RendererTriangle${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\51RendererTexturing.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\51RendererTexturing${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\52RendererMesh.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\52RendererMesh${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\53RendererMultiView.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\53RendererMultiView${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\54RendererRTT.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\54RendererRTT${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\60Scene.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\60Scene${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\61Sound.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\61Sound${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\62Physics.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\62Physics${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\63PhysicsRagdoll.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\63PhysicsRagdoll${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\64Picking.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\64Picking${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\65PostProcess.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\65PostProcess${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\66IngameGUI.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\66IngameGUI${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\67Game2D.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\67Game2D${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\70SPARK.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\70SPARK${PROJECT_SUFFIX}.exe\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Samples\\\\71libRocket.lnk\\\" \\\"$INSTDIR\\\\Samples\\\\Bin\\\\x86\\\\71libRocket${PROJECT_SUFFIX}.exe\\\"
		CreateDirectory \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tools\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tools\\\\PLViewer.lnk\\\" \\\"$INSTDIR\\\\Tools\\\\x86\\\\PLViewer${PROJECT_SUFFIX}.exe\\\"
		CreateDirectory \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation\\\\PixelLightFeatures.pdf.lnk\\\" \\\"$INSTDIR\\\\Docs\\\\PixelLightFeatures.pdf\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation\\\\PixelLightBase.pdf.lnk\\\" \\\"$INSTDIR\\\\Docs\\\\PixelLightBase.pdf\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation\\\\PixelLightAPI.chm.lnk\\\" \\\"$INSTDIR\\\\Docs\\\\PixelLightAPI.chm\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation\\\\PixelLightCompositing.pdf.lnk\\\" \\\"$INSTDIR\\\\Docs\\\\PixelLightCompositing.pdf\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation\\\\PixelLightConventions.pdf.lnk\\\" \\\"$INSTDIR\\\\Docs\\\\PixelLightConventions.pdf\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation\\\\PixelLightBuild.pdf.lnk\\\" \\\"$INSTDIR\\\\Docs\\\\PixelLightBuild.pdf\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation\\\\PixelLightScript.pdf.lnk\\\" \\\"$INSTDIR\\\\Docs\\\\PixelLightScript.pdf\\\"
		CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation\\\\PixelLight3dsMaxSceneExport.pdf.lnk\\\" \\\"$INSTDIR\\\\Docs\\\\PixelLight3dsMaxSceneExport.pdf\\\"
	")

	# Delete icons
	set(CPACK_NSIS_DELETE_ICONS " 
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\SDK-Browser.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Website.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\ReleaseNotes.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\Samples.sln.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\05NetworkChat.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\45ScriptApplication.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\50RendererTriangle.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\51RendererTexturing.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\52RendererMesh.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\53RendererMultiView.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\54RendererRTT.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\60Scene.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\61Sound.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\62Physics.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\63PhysicsRagdoll.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\64Picking.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\65PostProcess.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\66IngameGUI.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\67Game2D.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\70SPARK.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\\71libRocket.lnk\\\"
		RMDir  \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Samples\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tools\\\\PLViewer.lnk\\\"
		RMDir  \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tools\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation\\\\PixelLightFeatures.pdf.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation\\\\PixelLightBase.pdf.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation\\\\PixelLightAPI.chm.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation\\\\PixelLightCompositing.pdf.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation\\\\PixelLightConventions.pdf.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation\\\\PixelLightBuild.pdf.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation\\\\PixelLightScript.pdf.lnk\\\"
		Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation\\\\PixelLight3dsMaxSceneExport.pdf.lnk\\\"
		RMDir  \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation\\\"
	")

	##################################################
	## Extra install commands
	##################################################
	# Registry key
	if(CMAKETOOLS_CONFIG_SUFFIX)
		set(regkey "\\\"Software\\\\${CPACK_PACKAGE_VENDOR}\\\\${CPACK_PACKAGE_NAME}-${CMAKETOOLS_CONFIG_SUFFIX}\\\"")
	else()
		set(regkey "\\\"Software\\\\${CPACK_PACKAGE_VENDOR}\\\\${CPACK_PACKAGE_NAME}\\\"")
	endif()

	# Install
	set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
		; Install registry key to the PixelLight SDK
		WriteRegStr HKLM ${regkey} \\\"Runtime\\\" \\\"$INSTDIR\\\\Runtime\\\\x86\\\"
		; Install PATH environment variable to the PixelLight SDK
		Push \\\"PATH\\\"
		Push \\\"A\\\"
		Push \\\"HKCU\\\"
		Push \\\"$INSTDIR\\\\Runtime\\\\x86\\\"
		Call EnvVarUpdate
	")

	# Un-Install
	set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
		; Un-Install registry key to the PixelLight SDK
		DeleteRegKey HKLM ${regkey}
		; Un-Install PATH environment variable to the PixelLight SDK
		Push \\\"PATH\\\"
		Push \\\"R\\\"
		Push \\\"HKCU\\\"
		Push \\\"$INSTDIR\\\\Runtime\\\\x86\\\"
		Call un.EnvVarUpdate
	")

	##################################################
	## Install files
	##################################################
	set(CPACK_INSTALL_CMAKE_PROJECTS		"${CMAKE_BINARY_DIR};PixelLight;SDK;/;${CMAKE_BINARY_DIR};PixelLight;Runtime;/;${CMAKE_BINARY_DIR}/Docs;PixelLight;Docs;/")
	set(CPACK_PACKAGE_INSTALL_DIRECTORY		"PixelLight-SDK")
	set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY	"PixelLight-SDK")

	##################################################
	## Set generator
	##################################################
	set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackConfig-PL_SDK.cmake")
	if(WIN32)
		set(CPACK_GENERATOR		"NSIS")
		set(CPACK_OUTPUT_FILE	"${CPACK_PACKAGE_FILE_NAME}.exe")
	elseif(LINUX)
#		set(CPACK_GENERATOR		"TGZ")
#		set(CPACK_OUTPUT_FILE	"${CPACK_PACKAGE_FILE_NAME}.tar.gz")
		set(CPACK_GENERATOR		"DEB")
		set(CPACK_OUTPUT_FILE	"${CPACK_PACKAGE_FILE_NAME}.deb")
	endif()

	##################################################
	## CPack
	##################################################
	# We have to check whether or not CPack was already included, if this is not done CMake will give us warnings about multiple inclusion
	# (we include CPack in "Pack-Docs.cmake", "Pack-Runtime.cmake" and "Pack-SDK.cmake")
	if(NOT CPack_CMake_INCLUDED)
		include(CPack)
	endif()
endif()

##################################################
## Package target
##################################################
add_custom_target(
	Pack-SDK
	COMMAND ${CPACK_COMMAND} -C ${CMAKE_BUILD_TYPE} --config ${CMAKE_BINARY_DIR}/CPackConfig-PL_SDK.cmake
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_target_properties(Pack-SDK PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)

##################################################
## Dependencies
##################################################
add_dependencies(Pack-SDK		PixelLight Docs)
if(WIN32)
	add_dependencies(Pack-SDK	External-VC2010)
endif()
add_dependencies(Pack			Pack-SDK)

##################################################
## Post-Build
##################################################
add_custom_command(TARGET Pack-SDK
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PL_BIN_DIR}/Packages"
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${CPACK_OUTPUT_FILE} "${PL_BIN_DIR}/Packages"
)

