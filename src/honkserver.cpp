#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <uWS/uWS.h>

#define TRACE_DEBUG BOOST_LOG_TRIVIAL(debug)
#define TRACE_INFO BOOST_LOG_TRIVIAL(info)
#define TRACE_WARNING BOOST_LOG_TRIVIAL(warning)
#define TRACE_ERROR BOOST_LOG_TRIVIAL(error)

// =============================================================================
/// Types, serialization.
// =============================================================================
enum class client_msg_t : unsigned char {
    INIT       = 1,
    HONK_START = 2,
    HONK_END   = 3
};

template<typename Stream>
Stream&  operator<<( Stream&  os, const client_msg_t &  t ) {
    return os << static_cast<unsigned char>( t );
}

std::string  to_string( const client_msg_t &  t ) {
    return std::string( 1, static_cast<unsigned char>( t ) );
}

enum class server_msg_t : unsigned char {
    ERR        = 1,
    HONK_START = 2,
    HONK_END   = 3
};

std::string  to_string( const server_msg_t &  t ) {
    return std::string( 1, static_cast<unsigned char>( t ) );
}

struct init_data {
    std::string  hostname;
    std::string  port;
    std::string  player_name;
    std::string  player_id;
};

std::vector<std::string>  split( const char *  data, size_t  size )
{
    std::vector<std::string> out;
    const char *  p = data;
    do {
        out.push_back( std::string( p ) );
        p += out.back().size() + 1;
    } while (p - data < size);
    return out;
}

bool  check_init_params( const std::string &  msg, init_data &  data )
{
    // Skip msg id.
    std::vector<std::string> args = split( msg.data(), msg.size() );
    if ( args.size() < 4 ) {
        TRACE_ERROR << "Fewer args than required: " << args.size();
        TRACE_ERROR << "Message: " << msg;
        return false;
    }
    data.hostname    = args[1];
    data.port        = args[2];
    data.player_name = args[3];
    data.player_id   = args[4];
    return true;
}
// End types/serialization

std::string get_group_key( const init_data &  data )
{
    return data.hostname + ":" + data.port;
}

typedef uWS::Hub                    hub_t;
typedef uWS::WebSocket<uWS::SERVER> ws_t;
typedef uWS::Group<uWS::SERVER>     group_t;
typedef uWS::OpCode                 code_t;

// Make a group, setting all applicable listeners.
group_t *  make_group( hub_t &  h )
{
    group_t *  g = h.createGroup<uWS::SERVER>();
    g->listen(uWS::TRANSFERS);
    g->onTransfer([](ws_t *ws) {
        TRACE_INFO << "group.onTransfer()";
        // Might alert friends here.
    });
    g->onMessage( [ g ]( ws_t *  ws, char *  message, size_t  length,
                         code_t code ) {
        TRACE_INFO << "group.onMessage()";
        if ( length < 1 )
        {
            // TODO: reject
        }
        auto  t = static_cast<client_msg_t>( message[0] );
        init_data *  data =
            static_cast<init_data *>( ws->getUserData() );
        std::string res;
        switch(t) {
        case client_msg_t::HONK_START:
            res.append( to_string( server_msg_t::HONK_START ) );
            break;
        case (client_msg_t::HONK_END):
            res.append( to_string( server_msg_t::HONK_END ) );
            break;
        default:
            TRACE_ERROR << "Client message type not recognized: " << t;
            ws->close();
            return;
        }
        // Send to others in the group.
        res.push_back( '\0' );
        res.append( data->player_id );
        g->broadcast( res.data(), res.size(), uWS::OpCode::TEXT );
    });

    g->onDisconnection( []( ws_t *  ws, int  code, char *  message,
                            size_t  length) {
        TRACE_INFO << "group.onDisconnection()";
        // Might alert friends here.
    });
    return g;
}

int main()
{
    // Hub receives all websocket requests.
    uWS::Hub  h;

    std::map<std::string, group_t *>  groups;

    // Server connection listener.
    h.onConnection( []( ws_t *  ws, uWS::HttpRequest  req ) {
        // TODO: Trace some stats, add connect time and IP to user data.
        TRACE_INFO << "hub.onConnection()";
        /*
        TRACE_DEBUG << "Headers:";
        for (uWS::Header *h = req.headers; *++h; ) {
            TRACE_DEBUG << std::string( h->key, h->keyLength ) << ": "
                        << std::string( h->value, h->valueLength );
        }
        */
    });

    // This gets properly called for each message.
    // echo
    h.onMessage( [ &groups, &h ]( ws_t *ws, char *message,
                                  size_t length, uWS::OpCode opCode ) {
        TRACE_INFO << "hub.onMessage()";
        TRACE_DEBUG << "Length:  <" << length << ">";
        if ( length < 1 ) {
            // TODO: reject
        }
        auto  t = static_cast<client_msg_t>( message[0] );
        switch( t ) {
        case client_msg_t::INIT: {
            init_data *  data = new init_data();
            if ( !check_init_params( std::string( message, length ), *data ) )
            {
                TRACE_ERROR << "Invalid message";
                delete data;
                ws->terminate();
                return;
            }
            ws->setUserData( static_cast<void *>( data ) );
            std::string  k = get_group_key( *data );
            group_t *  group;
            auto  result = groups.find( k );
            if ( result == groups.end() ) {
                group = make_group( h );
                groups[k] = group;
            } else {
                group = result->second;
            }
            ws->transfer( group );
            } break;
        default:
            TRACE_ERROR << "Message type not recognized: "
                        << static_cast<unsigned char>( t );
            ws->terminate();
        }
    });

    h.onDisconnection( []( ws_t *ws, int code,
                           char *message, size_t length ) {
        TRACE_INFO << "hub.onDisconnection()";
    });

    h.onHttpRequest( []( uWS::HttpResponse *res, uWS::HttpRequest req,
                         char *data, size_t length,
                         size_t remainingBytes ) {
        TRACE_DEBUG << "hub.onHttpRequest()";
        res->end("", 0);
    });

    unsigned int  port = 3333;
    if ( !h.listen( port ) ) {
        TRACE_ERROR << "Cannot listen on " << port;
        return 1;
    }
    h.run();
    // TODO: Stats socket/endpoint.
}
