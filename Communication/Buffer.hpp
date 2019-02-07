#pragma once

#include "Error.hpp"
#include "Traits.hpp"

namespace Communication
{
	class Buffer
	{
	public:
		enum class BufferType
		{
			Bool,
			Int,
			Size_T,
			Float,
			Double,
			Char,
			WideChar,
			String,
			WideString,
			Vector,
			Pair,
			Map,
			Custom
		};

	private:
		size_t dataSize = 0;
		size_t size = 0;
		BufferType type = BufferType::Custom;
		std::unique_ptr<void, void(*)(void *)> buf;
		static constexpr size_t headerSize = sizeof(BufferType) + sizeof(dataSize);

	public:
		Buffer()
			: dataSize(0)
			, size(0)
			, type(BufferType::Custom)
			, buf(nullptr, nullptr) {}

		/** Buffer constructed from byte array (deserialization), takes ownership. */
		Buffer(void*&& bytes)
			: dataSize(*reinterpret_cast<decltype(dataSize) *>((static_cast<char *>(bytes) + sizeof(BufferType))))
			, size(headerSize + dataSize)
			, type(*static_cast<BufferType *>(bytes))
			, buf(bytes, [](void *buf) { std::free(buf); }) {}

		/** Constructs a buffer directly from a fundamental type value. */
		template<typename T> Buffer(T value)
			: dataSize(sizeof(T))
			, size(headerSize + dataSize)
			, type(TypeEnumFromTypeName<T>::value)
			, buf(std::malloc(size), [] (void *buf) { std::free(buf); })
		{
			static_assert(getGeneralType<T>() == GeneralType::FundamentalType, "Fundamental type required for this overload.");
			assert(buf != nullptr, "Eroare la alocare memorie de ", size, " bytes.");

			*static_cast<BufferType *>(buf.get()) = type;
			*reinterpret_cast<decltype(dataSize) *>(static_cast<char *>(buf.get()) + sizeof(BufferType)) = dataSize;
			*reinterpret_cast<T *>(static_cast<char *>(buf.get()) + headerSize) = value;
		}

		/** Constructs a buffer directly from a string. */
		template<typename CharType> Buffer(const std::basic_string<CharType>& string)
			: dataSize(sizeof(CharType) * (string.length() + 1))
			, size(headerSize + dataSize)
			, type(TypeEnumFromTypeName<std::basic_string<CharType>>::value)
			, buf(std::malloc(size), [] (void *buf) { std::free(buf); })
		{
			assert(buf != nullptr, "Eroare la alocare memorie de ", size, " bytes.");

			*static_cast<BufferType *>(buf.get()) = type;
			*reinterpret_cast<decltype(dataSize) *>(static_cast<char *>(buf.get()) + sizeof(BufferType)) = dataSize;
			std::memcpy(static_cast<char *>(buf.get()) + headerSize, string.c_str(), dataSize);
		}

		/** Disabling buffer construction from pointers. */
		template<typename T> Buffer(T* ptr)
		{
			static_assert(!std::is_same<T, T>::value, "Pointer types not allowed.");
		}

	private:
		/** Buffer constructed from byte array (deserialization), does not take ownership. */
		Buffer(const void*& bytes)
			: dataSize(*reinterpret_cast<const decltype(dataSize) *>((static_cast<const char *>(bytes) + sizeof(BufferType))))
			, size(headerSize + dataSize)
			, type(*static_cast<const BufferType *>(bytes))
			, buf(std::malloc(size), [](void *buf) { std::free(buf); })
		{
			std::memcpy(buf.get(), bytes, size);
		}

		///** Buffer owns the byte array, byte array parameter does not include header, create it. */
		//Buffer(void*&& buffer, decltype(dataSize) dataSize, decltype(type) type):
		//	dataSize(dataSize),
		//	size(headerSize + dataSize),
		//	type(type),
		//	buf(std::malloc(size), [](void *buf) { std::free(buf); })
		//{
		//	ScopeGuard freeBuffer([buffer] { std::free(buffer); });

		//	*static_cast<BufferType *>(buf.get()) = type;
		//	void* dataStartAddress = static_cast<char *>(buf.get()) + headerSize;
		//	std::memcpy(dataStartAddress, buffer, dataSize);
		//}
		///** Buffer owns the byte array, byte array parameter includes header, use it. */
		//Buffer(void*&& buffer):
		//	dataSize(size - headerSize),
		//	size(size),
		//	type(*static_cast<BufferType *>(buffer)),
		//	buf(buffer, [](void *buf) { std::free(buf); })
		//{
		//	buffer = nullptr;
		//}
		///** Buffer does not own the byte array, byte array parameter includes header, use it. */
		////Buffer(const void*& buffer, decltype(dataSize) size):
		////	dataSize(size - headerSize),
		////	size(size),
		////	type(*static_cast<const BufferType *>(buffer)),
		////	buf(std::malloc(size), [](void *buf) { std::free(buf); })
		////{
		////	std::memcpy(buf.get(), buffer, size);
		////}

	public:
		//Buffer(const Buffer&) = delete;
		Buffer(const Buffer& other):
			dataSize(other.dataSize),
			size(other.size),
			type(other.type),
			buf(std::malloc(size), [](void *buf) { std::free(buf); })
		{
			void* dataStartAddress = static_cast<char *>(buf.get()) + headerSize;
			std::memcpy(dataStartAddress, other, dataSize);
		}
		void operator =(const Buffer&) = delete;

		Buffer(Buffer&& other) noexcept:
			dataSize(other.dataSize),
			size(other.size),
			type(other.type),
			buf(std::move(other.buf))
		{
			other.size = 0;
			other.dataSize = 0;
		}
		//void operator =(Buffer&& other) = delete;
		void operator =(Buffer&& other) noexcept
		{
			if (&other == this)
				return;
			buf = std::move(other.buf);
			size = other.size;
			dataSize = other.dataSize;
			type = other.type;
			other.size = 0;
			other.dataSize = 0;
		}

		operator const void*() const
		{
			return buf.get();
		}
		const void* getData() const
		{
			return static_cast<char *>(buf.get()) + headerSize;
		}
		size_t getSize() const
		{
			return size;
		}
		BufferType getType() const
		{
			return type;
		}

	private:
		template<typename T>				struct _TypeEnumFromTypeName { static const BufferType value = BufferType::Custom; };
		template<>							struct _TypeEnumFromTypeName<bool> { static const BufferType value = BufferType::Bool; };
		template<>							struct _TypeEnumFromTypeName<int> { static const BufferType value = BufferType::Int; };
		template<>							struct _TypeEnumFromTypeName<size_t> { static const BufferType value = BufferType::Size_T; };
		template<>							struct _TypeEnumFromTypeName<float> { static const BufferType value = BufferType::Float; };
		template<>							struct _TypeEnumFromTypeName<double> { static const BufferType value = BufferType::Double; };
		template<>							struct _TypeEnumFromTypeName<char> { static const BufferType value = BufferType::Char; };
		template<>							struct _TypeEnumFromTypeName<wchar_t> { static const BufferType value = BufferType::WideChar; };
		template<>							struct _TypeEnumFromTypeName<std::string> { static const BufferType value = BufferType::String; };
		template<>							struct _TypeEnumFromTypeName<std::wstring> { static const BufferType value = BufferType::WideString; };
		template<typename T>				struct _TypeEnumFromTypeName<std::vector<T>> { static const BufferType value = BufferType::Vector; };
		template<typename T, typename Y>	struct _TypeEnumFromTypeName<std::pair<T, Y>> { static const BufferType value = BufferType::Pair; };
		template<typename T, typename Y>	struct _TypeEnumFromTypeName<std::map<T, Y>> { static const BufferType value = BufferType::Map; };

	public:
		template<typename T>				struct TypeEnumFromTypeName { static const BufferType value = _TypeEnumFromTypeName<remove_reference_and_const<T>::type>::value; };

	private:
		//template<Type type, GeneralType generalType = getGeneralType(type)> static Buffer getBufferFromBytes(const void* bytes)

	public:
		/** Concatenates the buffers into a single, larger, one and destroys the initial buffers. */
		static Buffer packBuffers(const std::vector<const Buffer *>& buffers, const BufferType type = BufferType::Custom)
		{
			size_t totalSize = headerSize;
			for (const Buffer* buffer : buffers)
				totalSize += buffer->size;
			void *mergedBuffer = std::malloc(totalSize);
			assert(mergedBuffer != nullptr, "Eroare la alocare memorie de ", totalSize, " bytes.");

			*static_cast<BufferType *>(mergedBuffer) = type;
			*reinterpret_cast<decltype(dataSize) *>(static_cast<char *>(mergedBuffer) + sizeof(BufferType)) = totalSize - headerSize;
			size_t offset = headerSize;
			for (const Buffer* buffer : buffers)
			{
				std::memcpy(static_cast<char *>(mergedBuffer) + offset, *buffer, buffer->size);
				offset += buffer->size;
			}
			return Buffer(std::move(mergedBuffer));
		}

		/** Splits a large buffer into its components and destroys the initial buffer. */
		static std::vector<Buffer> unpackBuffer(const Buffer& mergedBuffer)
		{
			std::vector<Buffer> result;
			for (size_t offset = headerSize; offset < mergedBuffer.size; )
			{
				const void* startBuffer = static_cast<const char *>(static_cast<const void *>(mergedBuffer)) + offset;
				Buffer buffer(startBuffer);
				offset += buffer.getSize();
				result.push_back(std::move(buffer));
			}
			return result;
		}
	};
}
