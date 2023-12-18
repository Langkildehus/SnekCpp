#pragma once

#include "Core.h"
#include "queue.h"
#include "message.h"

namespace net
{
	template <typename T>
	class ServerInterface;

	template <typename T>
	class Connection : public std::enable_shared_from_this<Connection<T>>
	{
	public:
		enum class owner
		{
			server,
			client
		};

		Connection(owner parent, asio::io_context& context, asio::ip::tcp::socket socket, Queue<OwnedMessage<T>>& qIn)
			: m_ownerType(parent), m_context(context), m_socket(std::move(socket)), m_qMessagesIn(qIn)
		{
			if (m_ownerType == owner::server)
			{
				m_handshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

				m_handshakeCheck = Scramble(m_handshakeOut);
			}
		}
		
		virtual ~Connection() {}

		uint32_t GetID() const
		{
			return id;
		}

		void ConnectToClient(net::ServerInterface<T>* server, uint32_t uid = 0)
		{
			if (m_ownerType == owner::server)
			{
				if (m_socket.is_open())
				{
					id = uid;

					WriteValidation();
					ReadValidation(server);
				}
			}
		}

		void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
		{
			if (m_ownerType == owner::client)
			{
				asio::async_connect(m_socket, endpoints,
					[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
					{
						if (!ec)
							ReadValidation();
					}
				);
			}
		}

		void Disconnect()
		{
			if (IsConnected())
				asio::post(m_context,
					[this]()
					{
						m_socket.close();
					}
				);
		}

		bool IsConnected() const
		{
			return m_socket.is_open();
		}

	public:
		void Send(const Message<T>& msg)
		{
			asio::post(m_context,
				[this, msg]()
				{
					bool writingMessage = !m_qMessagesOut.IsEmpty();
					m_qMessagesOut.PushBack(msg);
					if (!writingMessage)
						WriteHeader();
				}
			);
		}

	private:
		// ASIO
		void ReadHeader()
		{
			asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_msgTemporaryIn.header.size > 0)
						{
							m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
							ReadBody();
						}
						else
						{
							AddToIncomingMessageQueue();
						}
					}
					else
					{
						std::cout << "[" << id << "] Read header failed!\n";
						std::cout << ec.message() << "\n";
						m_socket.close();
					}
				}
			);
		}

		// ASIO
		void ReadBody()
		{
			asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
						AddToIncomingMessageQueue();
					else
					{
						std::cout << "[" << id << "] Read body failed!\n";
						std::cout << ec.message() << "\n";
						m_socket.close();
					}
				}
			);
		}

		// ASIO
		void WriteHeader()
		{
			asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.Front().header, sizeof(MessageHeader<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_qMessagesOut.Front().body.size() > 0)
							WriteBody();
						else
						{
							m_qMessagesOut.PopFront();
							if (!m_qMessagesOut.IsEmpty())
								WriteHeader();
						}
					}
					else
					{
						std::cout << "[" << id << "] Write header failed!\n";
						std::cout << ec.message() << "\n";
						m_socket.close();
					}
				}
			);
		}

		// ASIO
		void WriteBody()
		{
			asio::async_write(m_socket, asio::buffer(m_qMessagesOut.Front().body.data(), m_qMessagesOut.Front().body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						m_qMessagesOut.PopFront();

						if (!m_qMessagesOut.IsEmpty())
							WriteHeader();
					}
					else
					{
						std::cout << "[" << id << "] Write body failed!\n";
						std::cout << ec.message() << "\n";
						m_socket.close();
					}
				}
			);
		}

		void AddToIncomingMessageQueue()
		{
			if (m_ownerType == owner::server)
				m_qMessagesIn.PushBack({ this->shared_from_this(), m_msgTemporaryIn });
			else
				m_qMessagesIn.PushBack({ nullptr, m_msgTemporaryIn });

			ReadHeader();
		}

		uint64_t Scramble(uint64_t input)
		{
			uint64_t out = input ^ 0xDEADBEEFC0DECAFE;
			out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
			return out ^ 0xC0DEFACE12345678;
		}

		// ASIO
		void WriteValidation()
		{
			asio::async_write(m_socket, asio::buffer(&m_handshakeOut, sizeof(uint64_t)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_ownerType == owner::client)
							ReadHeader();
					}
					else
						m_socket.close();
				}
			);
		}

		// ASIO
		void ReadValidation(net::ServerInterface<T>* server = nullptr)
		{
			asio::async_read(m_socket, asio::buffer(&m_handshakeIn, sizeof(uint64_t)),
				[this, server](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_ownerType == owner::server)
						{
							if (m_handshakeIn == m_handshakeCheck)
							{
								std::cout << "Client validated\n";
								server->OnClientValidated(this->shared_from_this());

								ReadHeader();
							}
							else
							{
								std::cout << "Client disconnected (Failed validation)\n";
								m_socket.close();
							}
						}
						else
						{
							m_handshakeOut = Scramble(m_handshakeIn);

							WriteValidation();
						}
					}
					else
					{
						std::cout << "Client disconnected (ReadValidation)\n";
						m_socket.close();
					}
				}
			);
		}

	protected:
		asio::ip::tcp::socket m_socket;
		asio::io_context& m_context;

		Queue<Message<T>> m_qMessagesOut;
		Queue<OwnedMessage<T>>& m_qMessagesIn;
		Message<T> m_msgTemporaryIn;

		owner m_ownerType = owner::server;
		uint32_t id = 0;

		uint64_t m_handshakeOut = 0;
		uint64_t m_handshakeIn = 0;
		uint64_t m_handshakeCheck = 0;
	};
}
