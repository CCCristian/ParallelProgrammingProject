#pragma once

#include <string>
#include <type_traits>

namespace Communication
{
	template<typename T> struct remove_reference_and_const
	{
		using type = std::remove_const_t<typename std::remove_reference<T>::type>;
	};
	enum class GeneralType
	{
		FundamentalType,
		StringType,
		CustomImplementedType,
		CustomType
	};

	template<typename T> constexpr bool is_fundamental_type()
	{
		if constexpr (std::is_same<T, bool>::value
			|| std::is_same<T, int>::value
			|| std::is_same<T, size_t>::value
			|| std::is_same<T, float>::value
			|| std::is_same<T, double>::value
			|| std::is_same<T, char>::value
			|| std::is_same<T, wchar_t>::value)
			return true;
		else
			return false;
	}

	template<typename T> struct is_serialization_implemented
	{
		static const bool value = false;
	};
	template<typename T> struct is_serialization_implemented<std::vector<T>>
	{
		static const bool value = true;
	};
	template<typename T1, typename T2> struct is_serialization_implemented<std::pair<T1, T2>>
	{
		static const bool value = true;
	};
	template<typename T1, typename T2> struct is_serialization_implemented<std::map<T1, T2>>
	{
		static const bool value = true;
	};

	template<typename T> struct is_string_type
	{
		static const bool value = false;
	};
	template<typename T> struct is_string_type<std::basic_string<T>>
	{
		static const bool value = true;
	};

	//template<typename T> constexpr GeneralType getGeneralType()
	//{
	//	if constexpr (is_fundamental_type<std::remove_reference<T>::type>::value)
	//		return GeneralType::FundamentalType;
	//	else if constexpr (is_string_type<std::remove_reference<T>::type>::value)
	//		return GeneralType::StringType;
	//	else
	//		return GeneralType::CustomType;
	//}
	template<typename T> constexpr GeneralType getGeneralType()
	{
		if constexpr (is_fundamental_type<remove_reference_and_const<T>::type>())
			return GeneralType::FundamentalType;
		else if constexpr (is_string_type<remove_reference_and_const<T>::type>::value)
			return GeneralType::StringType;
		else if constexpr (is_serialization_implemented<remove_reference_and_const<T>::type>::value)
			return GeneralType::CustomImplementedType;
		else
			return GeneralType::CustomType;
	}
}
