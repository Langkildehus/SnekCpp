#pragma once

#include "Core.h"
#include "message.h"
#include "queue.h"
#include "connection.h"

namespace net
{
	template <typename T>
	class ClientInterface
	{
	public:
		ClientInterface() { }

		virtual ~ClientInterface()
		{
			Disconnect();
		}

		bool Connect(const std::string& host, const uint16_t port)
		{
			try
			{
				asio::ip::tcp::resolver resolver(m_context);
				asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

				m_connection = std::make_unique<Connection<T>>(
					Connection<T>::owner::client,
					m_context,
					asio::ip::tcp::socket(m_context),
					m_qMessagesIn
				);

				m_connection->ConnectToServer(endpoints);

				contextThread = std::thread([this]() { m_context.run(); });
			}
			catch (std::exception& e)
			{
				std::cerr << "Client Exception: " << e.what() << "\n";
				return false;
			}

			return true;
		}

		void Disconnect()
		{
			if (IsConnected())
				m_connection->Disconnect();

			m_context.stop();
			if (contextThread.joinable())
				contextThread.join();

			m_connection.release();
		}

		bool IsConnected() const
		{
			if (m_connection)
				return m_connection->IsConnected();

			return false;
		}

		void Send(const Message<T>& msg)
		{
			if (IsConnected())
				m_connection->Send(msg);
		}

		Queue<OwnedMessage<T>>& Incoming()
		{
			return m_qMessagesIn;
		}

	protected:
		asio::io_context m_context;

		std::thread contextThread;

		std::unique_ptr<Connection<T>> m_connection;

	private:
		Queue<OwnedMessage<T>> m_qMessagesIn;
	};
}
