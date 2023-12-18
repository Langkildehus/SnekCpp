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
			: m_acceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) { }

		virtual ~ServerInterface()
		{
			Stop();
		}

		bool Start()
		{
			try
			{
				WaitForClientConnection();
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
			m_context.stop();

			if (m_contextThread.joinable())
				m_contextThread.join();

			std::cout << "[SERVER] Stopped!\n";
		}

		// ASIO
		void WaitForClientConnection()
		{
			m_acceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					if (!ec)
					{
						std::cout << "[SERVER] New connection: " << socket.remote_endpoint() << "\n";

						std::shared_ptr<Connection<T>> conn =
							std::make_shared<Connection<T>>(
								Connection<T>::owner::server,
								m_context,
								std::move(socket),
								m_qMessagesIn
							);

						if (OnClientConnect(conn))
						{
							m_deqConnections.push_back(std::move(conn));

							m_deqConnections.back()->ConnectToClient(this, nIDCounter++);

							std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection approved\n";
						}
						else
							std::cout << "[-----] Connection denied!\n";
					}
					else
						std::cout << "[SERVER] New connection error: " << ec.message() << "\n";

					WaitForClientConnection();
				});
		}

		void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
		{
			if (client && client->IsConnected())
				client->Send(msg);
			else
			{
				OnClientDisconnect(client);
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

		void MessageAllClients(const Message<T>& msg,
			std::shared_ptr<Connection<T>> ignoreClient = nullptr)
		{
			bool invalidClientExists = false;

			for (auto& client : m_deqConnections)
			{
				if (client && client->IsConnected())
				{
					if (client != ignoreClient)
						client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					invalidClientExists = true;
				}
			}

			if (invalidClientExists)
				m_deqConnections.erase(
					std::remove(
						m_deqConnections.begin(),
						m_deqConnections.end(),
						nullptr
					),
					m_deqConnections.end()
				);
		}

		void Update(size_t maxMessages = -1, bool wait = false)
		{
			if (wait)
				m_qMessagesIn.Wait();

			size_t messageCount = 0;
			while (messageCount < maxMessages && !m_qMessagesIn.IsEmpty())
			{
				auto msg = m_qMessagesIn.PopFront();

				OnMessage(msg.remote, msg.msg);

				messageCount++;
			}
		}

		virtual void OnClientValidated(std::shared_ptr<Connection<T>> client) { }

	protected:
		virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client)
		{
			return false;
		}

		virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client) { }

		virtual void OnMessage(std::shared_ptr<Connection<T>> client, Message<T>& msg) { }

		Queue<OwnedMessage<T>> m_qMessagesIn;
		std::deque<std::shared_ptr<Connection<T>>> m_deqConnections;
		
		asio::io_context m_context;
		std::thread m_contextThread;
		asio::ip::tcp::acceptor m_acceptor;

		uint32_t nIDCounter = 10000;
	};
}
