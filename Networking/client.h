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
		virtual ~ClientInterface()
		{
			Disconnect();
		}

		// Resolves host and attempts to connect
		bool Connect(const std::string& host, const uint16_t port)
		{
			try
			{
				// Resolve host - allows for doman names as well as ips
				asio::ip::tcp::resolver resolver(m_context);
				asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

				// Create connection pointer
				m_connection = std::make_unique<Connection<T>>(
					Connection<T>::owner::client,
					m_context,
					asio::ip::tcp::socket(m_context),
					m_qMessagesIn
				);

				// Connect to server
				m_connection->ConnectToServer(endpoints);

				// Create the context thread for asio
				contextThread = std::thread([this]() { m_context.run(); });
			}
			catch (std::exception& e)
			{
				std::cerr << "Client Exception: " << e.what() << "\n";

				// Connection failed
				return false;
			}

			// Connection successful
			return true;
		}

		void Disconnect()
		{
			if (IsConnected())
				m_connection->Disconnect();

			// Stop asio context & wait for thread termination
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
			if (!IsConnected())
				return;

			m_connection->Send(msg);
		}

		// Get reference to the incoming messages queue
		Queue<OwnedMessage<T>>& Incoming()
		{
			return m_qMessagesIn;
		}

	protected:
		// Asio context
		asio::io_context m_context;
		// Asio thread
		std::thread contextThread;

		// Pointer to connection
		std::unique_ptr<Connection<T>> m_connection;

	private:
		// incoming messages queue
		Queue<OwnedMessage<T>> m_qMessagesIn;
	};
}
