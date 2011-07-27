/*********************************************************\
 *  File: TypeVector4.inl                                *
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


#ifndef __PLMATH_TYPE_VECTOR4_INL__
#define __PLMATH_TYPE_VECTOR4_INL__
#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include <PLCore/Base/Type/Type.h>


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace PLCore {


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Type wrapper for 'PLMath::Vector4'
*/
template <>
class Type<PLMath::Vector4> {


	//[-------------------------------------------------------]
	//[ Public static type information                        ]
	//[-------------------------------------------------------]
	public:
		// Type
		typedef PLMath::Vector4 _Type;			/**< Real type */
		typedef PLMath::Vector4 _StorageType;	/**< Storage type, for this type identical to the real type */

		// Type ID
		static const int TypeID = 1002;

		// Get type name
		static String GetTypeName()
		{
			return "vector4";
		}

		// Convert var to PLMath::Vector4
		static PLMath::Vector4 ConvertFromVar(const DynVar *pVar)
		{
			const float fValue = pVar->GetFloat();
			return PLMath::Vector4(fValue, fValue, fValue, fValue);
		}

		// Convert PLMath::Vector4 to bool
		static bool ConvertToBool(const PLMath::Vector4 &vVector)
		{
			return vVector.IsNull();
		}

		// Convert bool to PLMath::Vector4
		static PLMath::Vector4 ConvertFromBool(bool bValue)
		{
			const float fValue = bValue ? 1.0f : 0.0f;
			return PLMath::Vector4(fValue, fValue, fValue, fValue);
		}

		// Convert PLMath::Vector4 to int
		static int ConvertToInt(const PLMath::Vector4 &vVector)
		{
			return vVector.IsNull();
		}

		// Convert int to PLMath::Vector4
		static PLMath::Vector4 ConvertFromInt(int nValue)
		{
			return PLMath::Vector4(static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue));
		}

		// Convert PLMath::Vector4 to int8
		static int8 ConvertToInt8(const PLMath::Vector4 &vVector)
		{
			return static_cast<int8>(ConvertToInt(vVector));
		}

		// Convert int8 to PLMath::Vector4
		static PLMath::Vector4 ConvertFromInt8(int8 nValue)
		{
			return PLMath::Vector4(static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue));
		}

		// Convert PLMath::Vector4 to int16
		static int16 ConvertToInt16(const PLMath::Vector4 &vVector)
		{
			return static_cast<int16>(ConvertToInt(vVector));
		}

		// Convert int16 to PLMath::Vector4
		static PLMath::Vector4 ConvertFromInt16(int16 nValue)
		{
			return PLMath::Vector4(static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue));
		}

		// Convert PLMath::Vector4 to int32
		static int32 ConvertToInt32(const PLMath::Vector4 &vVector)
		{
			return static_cast<int32>(ConvertToInt(vVector));
		}

		// Convert int32 to PLMath::Vector4
		static PLMath::Vector4 ConvertFromInt32(int32 nValue)
		{
			return PLMath::Vector4(static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue));
		}

		// Convert PLMath::Vector4 to int64
		static int64 ConvertToInt64(const PLMath::Vector4 &vVector)
		{
			return static_cast<int64>(ConvertToInt(vVector));
		}

		// Convert int64 to PLMath::Vector4
		static PLMath::Vector4 ConvertFromInt64(int64 nValue)
		{
			return PLMath::Vector4(static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue));
		}

		// Convert PLMath::Vector4 to uint8
		static uint8 ConvertToUInt8(const PLMath::Vector4 &vVector)
		{
			return static_cast<uint8>(ConvertToInt(vVector));
		}

		// Convert uint8 to PLMath::Vector4
		static PLMath::Vector4 ConvertFromUInt8(uint8 nValue)
		{
			return PLMath::Vector4(static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue));
		}

		// Convert PLMath::Vector4 to uint16
		static uint16 ConvertToUInt16(const PLMath::Vector4 &vVector)
		{
			return static_cast<uint16>(ConvertToInt(vVector));
		}

		// Convert uint16 to PLMath::Vector4
		static PLMath::Vector4 ConvertFromUInt16(uint16 nValue)
		{
			return PLMath::Vector4(static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue));
		}

		// Convert PLMath::Vector4 to uint32
		static uint32 ConvertToUInt32(const PLMath::Vector4 &vVector)
		{
			return static_cast<uint32>(ConvertToInt(vVector));
		}

		// Convert uint32 to PLMath::Vector4
		static PLMath::Vector4 ConvertFromUInt32(uint32 nValue)
		{
			return PLMath::Vector4(static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue));
		}

		// Convert PLMath::Vector4 to uint64
		static uint64 ConvertToUInt64(const PLMath::Vector4 &vVector)
		{
			return static_cast<uint64>(ConvertToInt(vVector));
		}

		// Convert uint64 to PLMath::Vector4
		static PLMath::Vector4 ConvertFromUInt64(uint64 nValue)
		{
			return PLMath::Vector4(static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue));
		}

		// Convert PLMath::Vector4 to uint_ptr
		static uint_ptr ConvertToUIntPtr(const PLMath::Vector4 &vVector)
		{
			return static_cast<uint_ptr>(ConvertToInt(vVector));
		}

		// Convert uint_ptr to PLMath::Vector4
		static PLMath::Vector4 ConvertFromUIntPtr(uint_ptr nValue)
		{
			return PLMath::Vector4(static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue), static_cast<float>(nValue));
		}

		// Convert PLMath::Vector4 to float
		static float ConvertToFloat(const PLMath::Vector4 &vVector)
		{
			return static_cast<float>(ConvertToInt(vVector));
		}

		// Convert float to PLMath::Vector4
		static PLMath::Vector4 ConvertFromFloat(float fValue)
		{
			return PLMath::Vector4(fValue, fValue, fValue, fValue);
		}

		// Convert PLMath::Vector4 to double
		static double ConvertToDouble(const PLMath::Vector4 &vVector)
		{
			return static_cast<double>(ConvertToInt(vVector));
		}

		// Convert double to PLMath::Vector4
		static PLMath::Vector4 ConvertFromDouble(double dValue)
		{
			return PLMath::Vector4(static_cast<float>(dValue), static_cast<float>(dValue), static_cast<float>(dValue), static_cast<float>(dValue));
		}

		// Convert PLMath::Vector4 to string
		static String ConvertToString(const PLMath::Vector4 &vVector)
		{
			return vVector.ToString();
		}

		// Convert string to PLMath::Vector4
		static PLMath::Vector4 ConvertFromString(const String &sString)
		{
			PLMath::Vector4 vVector;
			vVector.FromString(sString);
			return vVector;
		}

		// Convert real to storage type (reference is enough in here)
		static PLMath::Vector4 &ConvertRealToStorage(PLMath::Vector4 &vValue)
		{
			return vValue;
		}

		// Convert storage to real type (reference is enough in here)
		static PLMath::Vector4 &ConvertStorageToReal(PLMath::Vector4 &vValue)
		{
			return vValue;
		}


};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // PLCore


#endif // __PLMATH_TYPE_VECTOR4_INL__