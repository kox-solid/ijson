
#ifndef CONNECT_H
#define CONNECT_H

#include "server.h"
#include "utils.h"
#include "json.h"


#define HTTP_START 0
#define HTTP_HEADER 1
#define HTTP_READ_BODY 2
#define HTTP_REQUEST_COMPLETED 3

#define STATUS_NET 21
#define STATUS_WAIT_JOB 22
#define STATUS_WAIT_RESPONSE 23
#define STATUS_WAIT_RESULT 24


class HttpSender {
private:
    Connect *conn;
public:
    HttpSender() {conn=NULL;};
    void set_connect(Connect *n_conn) {this->conn = n_conn;};
    HttpSender *status(const char *status);
    HttpSender *header(const char *key, ISlice &value);
    void done(ISlice &body);
    void done(int error);
    void done();
};


class Connect {
private:
    char _socket_status;  //  1 - read, 2 - write, -1 - closed
    int _link;
public:
    int fd;
    bool keep_alive;
    HttpSender send;
    Buffer send_buffer;
    Loop *loop;
    CoreServer *server;

    Connect(CoreServer *server, int fd) {
        _socket_status = 1;
        this->server = server;
        this->fd = fd;
        _link = 0;
        loop = server->loops[0];
        send.set_connect(this);

        http_step = HTTP_START;
        status = STATUS_NET;
        client = NULL;
    };
    ~Connect() {
        fd = 0;
        send.set_connect(NULL);
    };

    void write_mode(bool active);
    void read_mode(bool active);

    void close() {_socket_status = -1;};
    inline bool is_closed() {return _socket_status == -1;};

    int get_link() { return _link; }
    void link() { _link++; };
    void unlink();

    void on_recv(char *buf, int size);
    void on_send();
    int raw_send(const void *buf, uint size);

private:
    int http_step;
    int content_length;
    int http_version;  // 10, 11
    Buffer buffer;

    Buffer path;
    Buffer name;
public:
    int status;
    Buffer body;
    Buffer id;
    bool fail_on_disconnect;
    bool noid;
    Connect *client;
    JData jdata;

    int read_method(Slice &line);
    void read_header(Slice &data);
    void send_details();
    void send_help();
    void rpc_add();

    void header_completed();
    void gen_id();
};

#endif  /* CONNECT_H */