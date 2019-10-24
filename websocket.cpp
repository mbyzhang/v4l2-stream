#include "websocket.h"
#include <set>

using websocketpp::log::alevel;
using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

BroadcastServer::BroadcastServer(event fn)
{
    m_server.set_access_channels(alevel::all);
    m_server.clear_access_channels(alevel::frame_payload);
    m_server.clear_access_channels(alevel::frame_header);
    m_server.set_reuse_addr(true);
    
    m_server.init_asio();

    m_server.set_open_handler(bind(&BroadcastServer::on_open, this, ::_1, fn));
    m_server.set_close_handler(bind(&BroadcastServer::on_close, this, ::_1, fn));
    m_server.set_message_handler(bind(&BroadcastServer::on_message, this, ::_1, ::_2));
}

void BroadcastServer::broadcast(void *data, size_t len)
{
    for (auto it : m_connections)
    {
        m_server.send(it, data, len, websocketpp::frame::opcode::binary);
    }
}

void BroadcastServer::run(uint16_t port)
{
    m_server.listen(port);
    m_server.start_accept();
    m_server.run();
}
void BroadcastServer::on_message(connection_hdl hdl, server::message_ptr msg)
{
}

void BroadcastServer::on_open(connection_hdl hdl, event fn)
{
    m_connections.insert(hdl);

    if (m_connections.size() == 1)
    {
        fn(BroadcastServerEvent::FirstOpen);
    }
}

void BroadcastServer::on_close(connection_hdl hdl, event fn)
{
    m_connections.erase(hdl);

    if (m_connections.size() == 0)
    {
        fn(BroadcastServerEvent::LastClose);
    }
}