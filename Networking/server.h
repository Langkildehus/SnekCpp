#pragma once

#include "Core.h"
#include "queue.h"
#include "message.h"
#include "connection.h"

namespace net
{
	template <typename T>
	class ServerInterface
	{
	public:
		ServerInterface(uint16_t port)
			: m_acceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{

		}

		virtual ~ServerInterface()
		{
			Stop();
		}

		bool Start()
		{
			try
			{
				// Start asynchronous listen for clients
				WaitForClientConnection();

				// Start asio context thread
				m_contextThread = std::thread([this]() { m_context.run(); });
			}
			catch (std::exception& e)
			{
				std::cerr << "[SERVER] Exception: " << e.what() << "\n";
				return false;
			}

			std::cout << "[SERVER] Started!\n";
			return true;
		}

		void Stop()
		{
			// Stop context
			m_context.stop();

			// Wait for thread
			if (m_contextThread.joinable())
				m_contextThread.join();

			std::cout << "[SERVER] Stopped!\n";
		}

		// ASIO
		void WaitForClientConnection()
		{
			// Asynchronous waiting for new clients
			m_acceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					if (ec)
					{
						std::cout << "[SERVER] New connection error: " << ec.message() << "\n";
					}
					else
					{
						std::cout << "[SERVER] New connection: " << socket.remote_endpoint() << "\n";

						// Create pointer to new connection
						std::shared_ptr<Connection<T>> conn =
							std::make_shared<Connection<T>>(
								Connection<T>::owner::server,
								m_context,
								std::move(socket),
								m_qMessagesIn
							);

						// OVERRIDE - check if client is allowed
						if (OnClientConnect(conn))
						{
							// Save connection and allow connection
							m_deqConnections.push_back(std::move(conn));
							m_deqConnections.back()->ConnectToClient(this, IDCounter++);

							std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection approved\n";
						}
						else
						{
							std::cout << "[-----] Connection denied!\n";
						}
					}

					// Wait for new clients
					WaitForClientConnection();
				});
		}

		// Send message to one client
		void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
		{
			// Make sure client is connected
			if (client && client->IsConnected())
			{
				client->Send(msg);
			}
			else
			{
				// If client is not connected, call OnClientDisconnect
				OnClientDisconnect(client);

				// Reset client and delete it from queue
				client.reset();
				m_deqConnections.erase(
					std::remove(
						m_deqConnections.begin(),
						m_deqConnections.end(),
						client
					),
					m_deqConnections.end()
				);
			}
		}

		// Messages all clients - can ignore one client
		void MessageAllClients(const Message<T>& msg, std::shared_ptr<Connection<T>> ignoreClient = nullptr)
		{
			bool invalidClientExists = false;

			for (std::shared_ptr<Connection<T>>& client : m_deqConnections)
			{
				// Check if client is connected
				if (client && client->IsConnected())
				{
					// Only send message to client if not ignored
					if (client != ignoreClient)
						client->Send(msg);
				}
				else
				{
					// OVERRIDE - call OnClientDisconnect
					OnClientDisconnect(client);

					// Since client is no longer connected, delete it
					client.reset();

					// At least one client has disconnected
					invalidClientExists = true;
				}
			}

			// Remove all invalid clients, if any exists
			if (invalidClientExists)
			{
				m_deqConnections.erase(
					std::remove(
						m_deqConnections.begin(),
						m_deqConnections.end(),
						nullptr
					),
					m_deqConnections.end()
				);
			}
		}

		void Update(size_t maxMessages = -1, bool wait = false)
		{
			// If enabled, wait until new messages arrive
			if (wait)
				m_qMessagesIn.Wait();

			// Update a certain amount of messages, size_t is unsigned meaning default is: -1 -> very big
			size_t messageCount = 0;
			while (messageCount < maxMessages && !m_qMessagesIn.IsEmpty())
			{
				// Get first message from queue
				OwnedMessage<T> msg = m_qMessagesIn.PopFront();

				// OVERRIDE - Call OnMessage
				OnMessage(msg.remote, msg.msg);

				messageCount++;
			}
		}

		// OVERRIDE
		virtual void OnClientValidated(std::shared_ptr<Connection<T>> client) {}

	protected:
		// OVERRIDE
		virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client) { return false; }
		virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client) {}
		virtual void OnMessage(std::shared_ptr<Connection<T>> client, Message<T>& msg) {}

		// Thread safe queue of owned messages
		Queue<OwnedMessage<T>> m_qMessagesIn;

		// Queue of connection pointers
		std::deque<std::shared_ptr<Connection<T>>> m_deqConnections;

		// Asio context & acceptor
		asio::io_context m_context;
		asio::ip::tcp::acceptor m_acceptor;

		// Asio context thread
		std::thread m_contextThread;

		// Client IDCounter
		uint32_t IDCounter = 10000;
	};
}
