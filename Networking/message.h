#pragma once

#include "Core.h"

namespace net
{
	template <typename T>
	struct MessageHeader
	{
		T id{};
		uint32_t size = 0;
	};

	template <typename T>
	class Message
	{
	public:
		MessageHeader<T> header{};
		std::vector<uint8_t> body;

		size_t Size() const
		{
			return body.size(); //sizeof(MessageHeader<T>) + body.size();
		}

		friend std::ostream& operator << (std::ostream& os, const Message<T>& msg)
		{
			os << "ID: " << int(msg.header.id) << "SIZE: " << msg.header.size;
			return os;
		}

		// Stack behaviour
		template <typename DataType>
		friend Message<T>& operator << (Message<T>& msg, const DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex!");

			size_t i = msg.body.size();
			msg.body.resize(msg.body.size() + sizeof(DataType));
			std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

			msg.header.size = msg.Size();
			return msg;
		}

		template <typename DataType>
		friend Message<T>& operator >> (Message<T>& msg, DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex!");

			size_t i = msg.body.size() - sizeof(DataType);
			std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
			msg.body.resize(i);

			msg.header.size = msg.Size();
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

		friend std::ostream& operator << (std::ostream& os, const OwnedMessage<T>& msg)
		{
			os << msg.msg;
			return os;
		}
	};
}
