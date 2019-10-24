#include <set>
#include <functional>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

enum BroadcastServerEvent {FirstOpen, LastClose};

class BroadcastServer {
public:
    typedef websocketpp::server<websocketpp::config::asio> server;
    typedef std::function<void(BroadcastServerEvent)> event;

    BroadcastServer(event fn);

    void broadcast(void *data, size_t len);
    
    void run(uint16_t port);
private:
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);

    void on_open(websocketpp::connection_hdl hdl, event fn);

    void on_close(websocketpp::connection_hdl hdl, event fn);

    typedef std::set<websocketpp::connection_hdl,std::owner_less<websocketpp::connection_hdl>> con_list;

    server m_server;
    con_list m_connections;
};
