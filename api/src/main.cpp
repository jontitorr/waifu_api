#include <iostream>
#include <net/http.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif

using net::HttpConnection;
using net::HttpMethod;
using net::HttpRequest;
using net::HttpResponse;
using net::HttpServer;
using net::HttpStatus;
using net::Result;
using net::ServerHttpRequest;
using net::SocketAddr;
using net::Uri;

std::optional<std::string> get_env(std::string_view name)
{
#ifdef _WIN32
    const auto size = GetEnvironmentVariable(name.data(), nullptr, 0);

    if (size == 0) {
        return std::nullopt;
    }

    std::string value(size, '\0');
    GetEnvironmentVariable(name.data(), value.data(), size);
    return value;
#else
    const auto* value = std::getenv(name.data());

    if (value == nullptr) {
        return std::nullopt;
    }

    return value;
#endif
}

struct MongoDbAtlasApi {
    static Result<MongoDbAtlasApi> create(std::string_view api_key,
        std::string_view cluster_name, std::string_view collection_name,
        std::string_view database_name, std::string_view url) // NOLINT
    {
        auto connection = HttpConnection::connect(url);

        if (!connection) {
            return tl::make_unexpected(connection.error());
        }

        return MongoDbAtlasApi {
            api_key,
            cluster_name,
            collection_name,
            database_name,
            Uri::parse(url).path,
            std::move(*connection),
        };
    }

    [[nodiscard]] Result<HttpResponse> get_waifu_like_name(
        std::string_view name) const
    {
        const auto payload = R"({
            "dataSource": ")"
            + m_cluster_name + R"(",
            "database": ")"
            + std::string(m_database_name) + R"(",
            "collection": ")"
            + std::string(m_collection_name) + R"(",
            "pipeline": [
                {
                    "$match": {
                        "$expr": {
                            "$gt": [
                                {
                                    "$indexOfCP": [
                                        {
                                            "$toLower": "$name"
                                        },
                                        ")"
            + std::string(name) + R"("
                                    ]
                                },
                                -1
                            ]
                        }
                    }
                },
                {
                    "$project": {
                        "_id": 0
                    }
                }
            ]
        })";

        const HttpRequest req {
            HttpMethod::Post,
            m_path + "/action/aggregate",
            payload,
            {
                { "Content-Type", "application/json" },
                { "api-key", m_api_key },
            },
        };

        return m_connection.request(req);
    }

private:
    MongoDbAtlasApi(std::string_view api_key, // NOLINT
        std::string_view cluster_name, std::string_view collection_name,
        std::string_view database_name, std::string_view path,
        HttpConnection connection)
        : m_api_key { api_key }
        , m_cluster_name { cluster_name }
        , m_collection_name { collection_name }
        , m_database_name { database_name }
        , m_path { path }
        , m_connection { std::move(connection) }
    {
    }

    std::string m_api_key;
    std::string m_cluster_name;
    std::string m_collection_name;
    std::string m_database_name;
    std::string m_path;
    HttpConnection m_connection;
};

int main()
{
    const auto api_key = get_env("MONGO_DB_API_KEY");
    const auto cluster_name = get_env("MONGO_DB_CLUSTER_NAME");
    const auto collection_name = get_env("MONGO_DB_COLLECTION_NAME");
    const auto database_name = get_env("MONGO_DB_DATABASE_NAME");
    const auto url = get_env("MONGO_DB_URL");

    if (!api_key || api_key->empty()) {
        std::cerr << "MONGO_DB_API_KEY is not set or is empty" << '\n';
        return 1;
    }

    if (!cluster_name || cluster_name->empty()) {
        std::cerr << "MONGO_DB_CLUSTER_NAME is not set or is empty" << '\n';
        return 1;
    }

    const size_t max_name_length { 64 };

    if (!collection_name || collection_name->empty()) {
        std::cerr << "MONGO_DB_COLLECTION_NAME is not set or is empty" << '\n';
        return 1;
    }

    if (collection_name->length() > max_name_length) {
        std::cerr
            << "MONGO_DB_COLLECTION_NAME must be between 1 and 64 characters"
            << '\n';
        return 1;
    }

    if (!database_name || database_name->empty()) {
        std::cerr << "MONGO_DB_DATABASE_NAME is not set or is empty" << '\n';
        return 1;
    }

    if (database_name->length() > max_name_length) {
        std::cerr
            << "MONGO_DB_DATABASE_NAME must be between 1 and 64 characters"
            << '\n';
        return 1;
    }

    if (!url || url->empty()) {
        std::cerr << "MONGO_DB_URL is not set or is empty" << '\n';
        return 1;
    }

    const auto api = MongoDbAtlasApi::create(
        *api_key, *cluster_name, *collection_name, *database_name, *url);

    if (!api) {
        std::cerr << "Could not create MongoDbAtlasApi: "
                  << api.error().message() << '\n';
        return 1;
    }

    auto server = HttpServer::bind(*SocketAddr::parse("0.0.0.0:8080"));

    if (!server) {
        std::cerr << "Could not bind to port 8080: " << server.error().message()
                  << '\n';
        return 1;
    }

    server->add_route(HttpMethod::Get, "/", [](const ServerHttpRequest& req) {
        const auto connection
            = req.headers.find("Connection") != req.headers.end()
            ? req.headers.at("Connection")
            : "close";

        return HttpResponse {
            HttpStatus::Ok,
            "OK",
            "<!DOCTYPE html><html><head><title>Waifu "
            "API</title></head><body><h1>Waifu API</h1><p>Get a random waifu "
            "by sending a GET request to /waifu</p></body></html>",
            {
                { "Content-Type", "text/html" },
                { "Connection", connection },
            },
        };
    });

    server->add_route(
        HttpMethod::Get, "/waifu/{name}", [&api](const ServerHttpRequest& req) {
            HttpResponse res;
            res.status_code = HttpStatus::InternalServerError;
            res.status_message = "Internal Server Error";
            res.body = "Failed to fetch waifu";
            res.headers = {
                { "Content-Type", "application/json" },
            };

            const auto& name = req.params.at("name");
            auto waifu = api->get_waifu_like_name(name);

            if (!waifu || waifu->status_code != HttpStatus::Ok) {
                return res;
            }

            const auto key_begin = waifu->body.find("\"documents\"");

            if (key_begin == std::string::npos) {
                return res;
            }

            waifu->body.replace(key_begin, 11, "\"results\"");

            res.status_code = HttpStatus::Ok;
            res.status_message = "OK";
            res.body = waifu->body;
            return res;
        });

    if (const auto res = server->run(); !res) {
        std::cerr << "Error while running server: " << res.error().message()
                  << '\n';
        return 1;
    }
}
