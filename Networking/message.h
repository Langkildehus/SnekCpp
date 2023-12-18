#pragma once

#include "Core.h"

namespace net
{
	// Header struct for all messages
	template <typename T>
	struct MessageHeader
	{
		T id{};
		uint32_t bodySize = 0;
	};

	template <typename T>
	class Message
	{
	public:
		// All messages have a header and a body vector of bytes (body can be empty)
		MessageHeader<T> header{};
		std::vector<uint8_t> body;

		size_t GetBodySize() const
		{
			return body.size();
		}

		// Operator overload bitshift left for output streams
		friend std::ostream& operator << (std::ostream& os, const Message<T>& msg)
		{
			os << "ID: " << int(msg.header.id) << "SIZE: " << msg.header.bodySize;
			return os;
		}

		// Operator overload bitshift left into msg to allow stack behaviour
		template <typename DataType>
		friend Message<T>& operator << (Message<T>& msg, const DataType& data)
		{
			// Check if data is too complex
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex!");

			// Resize body for the extra needed bytes and copy bytes into body
			size_t i = msg.body.size();
			msg.body.resize(msg.body.size() + sizeof(DataType));
			std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

			// Update body's size
			msg.header.bodySize = (uint32_t)msg.GetBodySize();
			return msg;
		}

		// Operator overload bitshift right from msg to allow stack behaviour
		template <typename DataType>
		friend Message<T>& operator >> (Message<T>& msg, DataType& data)
		{
			// Check if data is too complex
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex!");

			// Copy bytes into variable, afterwards delete them from the body
			size_t i = msg.body.size() - sizeof(DataType);
			std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
			msg.body.resize(i);

			// Update body's size
			msg.header.bodySize = (uint32_t)msg.GetBodySize();
			return msg;
		}
	};

	// Forward declare connection class
	template <typename T>
	class Connection;

	template <typename T>
	struct OwnedMessage
	{
		std::shared_ptr<Connection<T>> remote = nullptr;
		Message<T> msg;

		// Operator overload bitshift left for output streams
		friend std::ostream& operator << (std::ostream& os, const OwnedMessage<T>& msg)
		{
			os << msg.msg;
			return os;
		}
	};
}
