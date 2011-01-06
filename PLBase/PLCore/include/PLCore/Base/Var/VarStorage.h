/*********************************************************\
 *  File: VarStorage.h                                   *
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


#ifndef __PLCORE_VAR_STORAGE_H__
#define __PLCORE_VAR_STORAGE_H__
#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "PLCore/PLCore.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace PLCore {


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
class Object;


//[-------------------------------------------------------]
//[ Helper classes                                        ]
//[-------------------------------------------------------]
/**
*  @brief
*    Storage type: Store a direct value of the specific type
*/
class StorageDirectValue
{
	public:
		// Save storage type
		typedef StorageDirectValue StorageType;
		typedef StorageDirectValue BaseStorageType;
};	

/**
*  @brief
*    Storage type: Use getter and setter methods to store a value
*/
class StorageGetSet
{
	public:
		// Save storage type
		typedef StorageGetSet StorageType;
		typedef StorageGetSet BaseStorageType;
};

/**
*  @brief
*    Storage type: Use an already defined attribute to store a value
*/
class StorageModifyAttr
{
	public:
		// Save storage type
		typedef StorageModifyAttr StorageType;
		typedef StorageModifyAttr BaseStorageType;
};

/**
*  @brief
*    Check Storage type to determine, whether the default value shall be set or not
*/
template <typename STORAGE>
class CheckStorage {
	public:
		// Set default value?
		enum {
			InitDefault = 1
		};
};

template <>
class CheckStorage<StorageGetSet> {
	public:
		// Set default value?
		enum {
			InitDefault = 0
		};
};


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Storage type of a variable
*
*  @remarks
*    Base template declaration
*/
template <typename T, typename STORAGE, typename STORAGETYPE>
class VarStorage {
};


/**
*  @brief
*    Storage type of a variable
*
*  @remarks
*    Implementation for direct value storage
*/
template <typename T, typename STORAGE>
class VarStorage<T, STORAGE, StorageDirectValue> {


	//[-------------------------------------------------------]
	//[ Public data types                                     ]
	//[-------------------------------------------------------]
	public:
		// Storage type
		typedef typename Type<T>::_Type _Type;


	//[-------------------------------------------------------]
	//[ Public functions                                      ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] DefaultValue
		*    Default value
		*/
		VarStorage(const _Type &DefaultValue) :
			m_DefaultValue(DefaultValue),
			m_Value(DefaultValue)
		{
		}

		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] DefaultValue
		*    Default value
		*  @param[in] pObject
		*    Pointer to object holding the attribute
		*/
		VarStorage(const _Type &DefaultValue, Object *pObject) :
			m_DefaultValue(DefaultValue),
			m_Value(DefaultValue)
		{
		}

		/**
		*  @brief
		*    Get default value
		*
		*  @return
		*    Default value
		*/
		inline _Type GetDefault() const
		{
			return m_DefaultValue;
		}


	//[-------------------------------------------------------]
	//[ Protected functions                                   ]
	//[-------------------------------------------------------]
	protected:
		/**
		*  @brief
		*    Get stored value
		*
		*  @return
		*    Value
		*/
		inline _Type StorageGet() const
		{
			return m_Value;
		}

		/**
		*  @brief
		*    Set stored value
		*
		*  @param[in] Value
		*    Value
		*/
		inline void StorageSet(const _Type &Value)
		{
			m_Value = Value;
		}


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		_Type m_DefaultValue;	/**< Default value */
		_Type m_Value;			/**< Stored value */


};


/**
*  @brief
*    Storage type of a variable
*
*  @remarks
*    Implementation for get/set value storage
*
*  @remarks
*    Base template declaration
*/
template <typename T, typename STORAGE>
class VarStorage<T, STORAGE, StorageGetSet> {


	//[-------------------------------------------------------]
	//[ Public data types                                     ]
	//[-------------------------------------------------------]
	public:
		// Storage type
		typedef typename Type<T>::_Type _Type;


	//[-------------------------------------------------------]
	//[ Public functions                                      ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] DefaultValue
		*    Default value of var
		*/
		VarStorage(const _Type &DefaultValue) :
			m_DefaultValue(DefaultValue),
			m_pObject(nullptr)
		{
			// We do NOT set the default value here, because this constructor is called inside
			// the constructor of the object holding the attribute. If we set a value here, it
			// would cause the set/get-methods to be called and then we end up calling a method
			// of a not-fully-initialized-yet object. That should be avoided, so you have to
			// initialize you external values by yourself.
			//StorageSet(DefaultValue);
		}

		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] DefaultValue
		*    Default value
		*  @param[in] pObject
		*    Pointer to object holding the attribute
		*/
		VarStorage(const _Type &DefaultValue, Object *pObject) :
			m_DefaultValue(DefaultValue),
			m_pObject(pObject)
		{
			// We do NOT set the default value here (see above)
			//StorageSet(DefaultValue);
		}

		/**
		*  @brief
		*    Get default value
		*
		*  @return
		*    Default value
		*/
		inline _Type GetDefault() const
		{
			return m_DefaultValue;
		}


	//[-------------------------------------------------------]
	//[ Protected functions                                   ]
	//[-------------------------------------------------------]
	protected:
		/**
		*  @brief
		*    Get stored value
		*
		*  @return
		*    Value
		*/
		inline _Type StorageGet() const
		{
			return STORAGE::Get(m_pObject);
		}

		/**
		*  @brief
		*    Set stored value
		*
		*  @param[in] Value
		*    Value
		*/
		inline void StorageSet(const _Type &Value)
		{
			STORAGE::Set(m_pObject, Value);
		}


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		_Type	m_DefaultValue;	/**< Default value */
		Object *m_pObject;		/**< Pointer to object */


};


/**
*  @brief
*    Storage type of a variable
*
*  @remarks
*    Implementation for get/set value storage
*
*  @remarks
*    Base template declaration
*/
template <typename T, typename STORAGE>
class VarStorage<T, STORAGE, StorageModifyAttr> {


	//[-------------------------------------------------------]
	//[ Public data types                                     ]
	//[-------------------------------------------------------]
	public:
		// Storage type
		typedef typename Type<T>::_Type _Type;


	//[-------------------------------------------------------]
	//[ Public functions                                      ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] DefaultValue
		*    Default value of var
		*/
		VarStorage(const _Type &DefaultValue) :
			m_DefaultValue(DefaultValue),
			m_pObject(nullptr)
		{
			// Set default value only, if the base storage is not GetSet (see above for explanation)
			bool bInitDefault = (CheckStorage<typename STORAGE::BaseStorageType>::InitDefault == 1);
			if (bInitDefault) {
				// Set default value
				StorageSet(DefaultValue);
			}
		}

		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] DefaultValue
		*    Default value
		*  @param[in] pObject
		*    Pointer to object holding the attribute
		*/
		VarStorage(const _Type &DefaultValue, Object *pObject) :
			m_DefaultValue(DefaultValue),
			m_pObject(pObject)
		{
			// Set default value only, if the base storage is not GetSet (see above for explanation)
			bool bInitDefault = (CheckStorage<typename STORAGE::BaseStorageType>::InitDefault == 1);
			if (bInitDefault) {
				// Set default value
				StorageSet(DefaultValue);
			}
		}

		/**
		*  @brief
		*    Get default value
		*
		*  @return
		*    Default value
		*/
		inline _Type GetDefault() const
		{
			return m_DefaultValue;
		}


	//[-------------------------------------------------------]
	//[ Protected functions                                   ]
	//[-------------------------------------------------------]
	protected:
		/**
		*  @brief
		*    Get stored value
		*
		*  @return
		*    Value
		*/
		inline _Type StorageGet() const
		{
			return STORAGE::Get(m_pObject);
		}

		/**
		*  @brief
		*    Set stored value
		*
		*  @param[in] Value
		*    Value
		*/
		inline void StorageSet(const _Type &Value)
		{
			STORAGE::Set(m_pObject, Value);
		}


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		_Type	m_DefaultValue;	/**< Default value */
		Object *m_pObject;		/**< Pointer to object */


};


/**
*  @brief
*    Helper class to choose a storage type
*
*  @remarks
*    Base template declaration
*/
template <typename STORAGE, typename CLASS_GS, typename CLASS_MOD>
class StorageChooser {
};


/**
*  @brief
*    Helper class to choose a storage type
*
*  @remarks
*    Implementation for direct value storage
*/
template <typename CLASS_GS, typename CLASS_MOD>
class StorageChooser<StorageDirectValue, CLASS_GS, CLASS_MOD> {
	public:
		// Choose direct-value storage
		typedef StorageDirectValue Storage;
};

/**
*  @brief
*    Helper class to choose a storage type
*
*  @remarks
*    Implementation for get/set-method storage
*/
template <typename CLASS_GS, typename CLASS_MOD>
class StorageChooser<StorageGetSet, CLASS_GS, CLASS_MOD> {
	public:
		// Choose get/set storage
		typedef CLASS_GS Storage;
};

/**
*  @brief
*    Helper class to choose a storage type
*
*  @remarks
*    Implementation for modified-attribute storage
*/
template <typename CLASS_GS, typename CLASS_MOD>
class StorageChooser<StorageModifyAttr, CLASS_GS, CLASS_MOD> {
	public:
		// Choose modify storage
		typedef CLASS_MOD Storage;
};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // PLCore


#endif // __PLCORE_VAR_STORAGE_H__
