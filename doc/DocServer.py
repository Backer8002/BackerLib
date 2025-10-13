import http.server as server

def main() -> None:
    server_address: tuple[str, int] = "127.0.0.1", 8080
    http_server: server.ThreadingHTTPServer = server.ThreadingHTTPServer(server_address, server.SimpleHTTPRequestHandler)
    http_server.serve_forever()


if __name__ == '__main__':
    main()
