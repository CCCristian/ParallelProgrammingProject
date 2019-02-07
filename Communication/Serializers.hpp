#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <stack>
#include <map>
#include "Traits.hpp"
#include "ScopeGuard.hpp"
#include "Buffer.hpp"


namespace Communication
{
	class SerializedData;
}

template<typename Type> struct Serializer
{
	static void serialize(Communication::SerializedData& data, const Type& val)
	{
		static_assert(!std::is_same<Type, Type>::value, "Class specialization needed.");
	}
	static void deserialize(const Communication::SerializedData& data, Type& val)
	{
		static_assert(!std::is_same<Type, Type>::value, "Class specialization needed.");
	}
};


namespace Communication
{
	// Primary template
	template<typename Type, GeneralType generalType = getGeneralType<Type>()>
	struct BasicSerializer
	{
		static Buffer serialize(const Type& value)
		{
			static_assert(!std::is_same<Type, Type>::value, "Class specialization needed.");
		}
		static Type deserialize(const Buffer& buffer)
		{
			static_assert(!std::is_same<Type, Type>::value, "Class specialization needed.");
		}
	};

	template<typename Type, GeneralType generalType = getGeneralType<Type>()>
	struct SerializerSelector
	{
		static Buffer serialize(const Type& value)
		{
			if constexpr (getGeneralType<Type>() != GeneralType::CustomType)
				return BasicSerializer<Type>::serialize(value);
			else
			{
				SerializedData data;
				Serializer<Type> serializer;
				serializer.serialize(data, value);
				return data;
			}
		}
		static Type deserialize(const Buffer& buffer)
		{
			if constexpr (getGeneralType<Type>() != GeneralType::CustomType)
				return BasicSerializer<Type>::deserialize(buffer);
			else
			{
				SerializedData data(buffer);
				Serializer<Type> serializer;
				Type object;
				serializer.deserialize(data, object);
				return object;
			}
		}
	};


	// ----------------------------------------------------------------------------
	// ----------------------------------------------------------------------------
	// ----------------------------------------------------------------------------


	class SerializedData
	{
		std::map<std::string, Buffer> parts;		// Map with key = name and value = buffer

	public:
		//
		// Serializers :
		//
		// Default template class
		SerializedData() = default;
		SerializedData(const Buffer& buffer)
		{
			assert(buffer.getType() == Buffer::BufferType::Custom, "Eroare la deserializare - nu se deserializeaza un tip custom.");
			std::vector<Buffer> buffers = Buffer::unpackBuffer(buffer);
			assert(buffers.size() % 2 == 0, "Eroare la deserializare - tipul custom nu este serializat corect.");
			for (size_t i = 0; i < buffers.size(); i += 2)
			{
				Buffer& nameBuffer = buffers[i];
				Buffer& data = buffers[i + 1];

				assert(nameBuffer.getType() == Buffer::BufferType::String, "Nu se poate deserializa - cheile nu sunt stringuri.");
				std::string name = static_cast<const char *>(nameBuffer.getData());
				parts.emplace(name, std::move(data));
			}
		}

		template<class Type> void add(const std::string& name, const Type& object)
		{
			assert(parts.count(name) == 0, "Exista deja un element cu cheia \"", name, "\".");
			parts.emplace(name, SerializerSelector<remove_reference_and_const<Type>::type>::serialize(object));
		}
		template<class Type> bool extract(const std::string& name, Type& object)
		{
			auto it = _get<remove_reference_and_const<Type>::type>(name, object);
			if (it == parts.end())
				return false;
			parts.erase(it);
			return true;
		}
		template<class Type> bool peek(const std::string& name, Type& object) const
		{
			auto it = _get<remove_reference_and_const<Type>::type>(name, object);
			if (it == parts.end())
				return false;
			return true;
		}
		operator Buffer()
		{
			std::vector<const Buffer *> buffers;
			std::stack<std::unique_ptr<Buffer>> names;
			for (auto& pair : parts)
			{
				names.push(std::unique_ptr<Buffer>(new Buffer(pair.first)));
				buffers.push_back(names.top().get());
				buffers.push_back(&pair.second);
			}
			return Buffer::packBuffers(buffers);
		}

		void addBuffer(const std::string& name, Buffer&& buffer)
		{
			parts.emplace(name, std::move(buffer));
		}
		bool removeBuffer(const std::string& name, Buffer& buffer)
		{
			auto it = parts.find(name);
			if (it == parts.end())
				return false;

			buffer = std::move(it->second);
			parts.erase(it);
			return true;
		}

	private:
		template<class Type> decltype(parts)::const_iterator _get(const std::string& name, Type& object) const
		{
			auto it = parts.find(name);
			if (it == parts.end())
				return parts.end();

			object = SerializerSelector<Type>::deserialize(it->second);
			return it;
		}
	};


	// ----------------------------------------------------------------------------
	// ----------------------------------------------------------------------------
	// ----------------------------------------------------------------------------

	
	// Buffer self-serialize specialization
	template<> struct BasicSerializer<Buffer, GeneralType::CustomType>
	{
		static Buffer serialize(const Buffer& value) noexcept
		{
			return Buffer(value);
		}
		static Buffer deserialize(const Buffer& buffer)
		{
			return Buffer(buffer);
		}
	};

	// Fundamental type specialization
	template<typename Type> struct BasicSerializer<Type, GeneralType::FundamentalType>
	{
		static Buffer serialize(const Type& value) noexcept
		{
			return Buffer(value);
		}
		static Type deserialize(const Buffer& buffer)
		{
			assert(buffer.getType() == Buffer::TypeEnumFromTypeName<Type>::value, "Eroare la deserializare - tipul de deserializat nu e acelasi cu cel din buffer.");
			return *reinterpret_cast<const Type *>(buffer.getData());
		}
	};

	// StringType specialization
	template<typename CharType> struct BasicSerializer<std::basic_string<CharType>, GeneralType::StringType>
	{
		using StringType = std::basic_string<CharType>;

		static Buffer serialize(const StringType& value)
		{
			return Buffer(value);
		}
		static StringType deserialize(const Buffer& buffer)
		{
			assert(buffer.getType() == Buffer::TypeEnumFromTypeName<std::basic_string<CharType>>::value, "Eroare la deserializare - tipul de deserializat nu e acelasi cu cel din buffer.");
			return StringType(static_cast<const CharType *>(buffer.getData()));
		}
	};

	// Pair type specialization
	template<typename Type1, typename Type2> struct BasicSerializer<std::pair<Type1, Type2>, GeneralType::CustomImplementedType>
	{
		static Buffer serialize(const std::pair<Type1, Type2>& value) noexcept
		{
			Buffer buffer1 = SerializerSelector<Type1>::serialize(value.first);
			Buffer buffer2 = SerializerSelector<Type2>::serialize(value.second);
			return Buffer::packBuffers({ &buffer1, &buffer2 });
		}
		static std::pair<Type1, Type2> deserialize(const Buffer& buffer)
		{
			assert(buffer.getType() == Buffer::BufferType::Custom, "Eroare la deserializare - tipul de deserializat nu e custom.");
			std::vector<Buffer> parts = Buffer::unpackBuffer(buffer);
			assert(parts.size() == 2, "Eroare la deserializare - nu s-a deserializat o pereche.");
			assert(parts[0].getType() == Buffer::TypeEnumFromTypeName<Type1>::value, "Eroare la deserializare - primul tip din pereche nu coincide cu cel din buffer.");
			assert(parts[1].getType() == Buffer::TypeEnumFromTypeName<Type2>::value, "Eroare la deserializare - al doilea tip din pereche nu coincide cu cel din buffer.");
			return std::pair<Type1, Type2>(SerializerSelector<Type1>::deserialize(parts[0]), SerializerSelector<Type2>::deserialize(parts[1]));
		}
	};

	// Vector specialization
	template<typename Type> struct BasicSerializer<std::vector<Type>, GeneralType::CustomImplementedType>
	{
		static Buffer serialize(const std::vector<Type>& value) noexcept
		{
			std::vector<const Buffer *> elementBuffers;
			elementBuffers.push_back(new Buffer(BasicSerializer<size_t>::serialize(value.size())));
			ScopeGuard deleteTempBuffers([&elementBuffers]
			{
				for (const Buffer* buf : elementBuffers)
					delete buf;
			});

			for (auto& elem : value)
				elementBuffers.push_back(new Buffer(SerializerSelector<Type>::serialize(elem)));
			Buffer result = Buffer::packBuffers(elementBuffers, Buffer::BufferType::Vector);

			return result;
		}
		static std::vector<Type> deserialize(const Buffer& buffer)
		{
			assert(buffer.getType() == Buffer::BufferType::Vector, "Eroare la deserializare - tipul de deserializat nu e vector.");
			std::vector<Buffer> parts = Buffer::unpackBuffer(buffer);
			assert(parts.size() >= 1, "Eroare la deserializare - vectorul nu este serializat corect.");
			assert(parts[0].getType() == Buffer::BufferType::Size_T, "Eroare la deserializare - vectorul nu este serializat corect.");
			decltype(parts.size()) size = BasicSerializer<size_t>::deserialize(parts[0]);
			assert(parts.size() - 1 == size, "Eroare la deserializare - vectorul nu este serializat corect.");

			std::vector<Type> result;
			result.reserve(size);
			for (auto i = 1; i < 1 + size; i++)
				result.push_back(SerializerSelector<Type>::deserialize(parts[i]));

			return result;
		}
	};

	// Map specialization
	//template<typename Type1, typename Type2> struct BasicSerializer<std::map<Type1, Type2>, GeneralType::CustomImplementedType>
	//{
	//	static Buffer serialize(const std::map<Type1, Type2>& value) noexcept
	//	{
	//		std::vector<std::unique_ptr<Buffer>> buffers;
	//		for (auto& pair : val)
	//		{
	//			buffers.push_back(std::unique_ptr(new Buffer(Serializer)));
	//		}
	//		//assert(buffer != nullptr, "Eroare la alocare memorie de");
	//		//if (buffer != nullptr)
	//		//{
	//		//	std::cerr << __FUNCTION__ " : Eroare la alocare memorie\n";
	//		//	std::cerr << "Fisierul: " __FILE__ "\n";
	//		//	std::cerr << "Linia: " << __LINE__ << '\n';
	//		//	abort();
	//		//	return Buffer();	// Just in case the above abort gets replaced
	//		//}
	//
	//		*buffer = val;
	//		return Buffer(std::move(buffer), sizeof(Type), Buffer::TypeEnumFromTypeName<Type>::value);
	//	}
	//	static std::map<Type1, Type2> deserialize(const Buffer& buffer)
	//	{
	//		assert(buffer.getType() == Buffer::BufferType::Map, "Eroare la deserializare - tipul de deserializat nu e acelasi cu cel din buffer");
	//		std::vector<const Buffer *> buffers;
	//		std::stack<std::unique_ptr<Buffer>> names;
	//		for (auto& pair : parts)
	//		{
	//			names.push(std::unique_ptr<Buffer>(new Buffer(Serializer<std::string>::serialize(pair.first))));
	//			buffers.push_back(names.top().get());
	//			buffers.push_back(&pair.second);
	//		}
	//		return Buffer::packBuffers(buffers);
	//	}
	//};
}
