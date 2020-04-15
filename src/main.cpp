#include <CLI/CLI.hpp>

#include <restinio/all.hpp>

#include <exception>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <thread>

using router_t = restinio::router::express_router_t<>;

const char *
content_type_by_file_extention( const restinio::string_view_t & ext )
{
    // Incomplete list of mime types from here:
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Complete_list_of_MIME_types
    if(ext == "aac" ) return "audio/aac";
    if(ext == "abw" ) return "application/x-abiword";
    if(ext == "arc" ) return "application/octet-stream";
    if(ext == "avi" ) return "video/x-msvideo";
    if(ext == "azw" ) return "application/vnd.amazon.ebook";
    if(ext == "bin" ) return "application/octet-stream";
    if(ext == "bz" ) return "application/x-bzip";
    if(ext == "bz2" ) return "application/x-bzip2";
    if(ext == "csh" ) return "application/x-csh";
    if(ext == "css" ) return "text/css";
    if(ext == "csv" ) return "text/csv";
    if(ext == "doc" ) return "application/msword";
    if(ext == "docx" ) return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    if(ext == "eot" ) return "application/vnd.ms-fontobject";
    if(ext == "epub" ) return "application/epub+zip";
    if(ext == "gif" ) return "image/gif";
    if(ext == "htm" || ext == "html" ) return "text/html";
    if(ext == "ico" ) return "image/x-icon";
    if(ext == "ics" ) return "text/calendar";
    if(ext == "jar" ) return "application/java-archive";
    if(ext == "jpeg" || ext == "jpg" ) return "image/jpeg";
    if(ext == "js" ) return "application/javascript";
    if(ext == "json" ) return "application/json";
    if(ext == "mid" || ext == "midi" ) return "audio/midi";
    if(ext == "mpeg" ) return "video/mpeg";
    if(ext == "mpkg" ) return "application/vnd.apple.installer+xml";
    if(ext == "odp" ) return "application/vnd.oasis.opendocument.presentation";
    if(ext == "ods" ) return "application/vnd.oasis.opendocument.spreadsheet";
    if(ext == "odt" ) return "application/vnd.oasis.opendocument.text";
    if(ext == "oga" ) return "audio/ogg";
    if(ext == "ogv" ) return "video/ogg";
    if(ext == "ogx" ) return "application/ogg";
    if(ext == "otf" ) return "font/otf";
    if(ext == "png" ) return "image/png";
    if(ext == "pdf" ) return "application/pdf";
    if(ext == "ppt" ) return "application/vnd.ms-powerpoint";
    if(ext == "pptx" ) return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    if(ext == "rar" ) return "archive	application/x-rar-compressed";
    if(ext == "rtf" ) return "application/rtf";
    if(ext == "sh" ) return "application/x-sh";
    if(ext == "svg" ) return "image/svg+xml";
    if(ext == "swf" ) return "application/x-shockwave-flash";
    if(ext == "tar" ) return "application/x-tar";
    if(ext == "tif" || ext == "tiff" ) return "image/tiff";
    if(ext == "ts" ) return "application/typescript";
    if(ext == "ttf" ) return "font/ttf";
    if(ext == "vsd" ) return "application/vnd.visio";
    if(ext == "wav" ) return "audio/x-wav";
    if(ext == "weba" ) return "audio/webm";
    if(ext == "webm" ) return "video/webm";
    if(ext == "webp" ) return "image/webp";
    if(ext == "woff" ) return "font/woff";
    if(ext == "woff2" ) return "font/woff2";
    if(ext == "xhtml" ) return "application/xhtml+xml";
    if(ext == "xls" ) return "application/vnd.ms-excel";
    if(ext == "xlsx" ) return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    if(ext == "xml" ) return "application/xml";
    if(ext == "xul" ) return "application/vnd.mozilla.xul+xml";
    if(ext == "zip" ) return "archive	application/zip";
    if(ext == "3gp" ) return "video/3gpp";
    if(ext == "3g2" ) return "video/3gpp2";
    if(ext == "7z" ) return "application/x-7z-compressed";

    return "application/text";
}

auto createDatabase(std::string const& libraryPath)
{
    std::vector<std::filesystem::path> result;
    for(auto const& p: std::filesystem::recursive_directory_iterator(libraryPath))
    {
        if (p.is_regular_file())
        {
            result.push_back(p.path());
        }
    }

    return result;
}

std::mutex mt;
std::filesystem::path imageOfTheDay;

auto create_request_handler()
{
    auto router = std::make_unique<router_t>();

    router->http_get("/api/wallpaper/get", [](restinio::request_handle_t req, auto)
    {
        try
        {
            std::filesystem::path chosenImage;
            {
                std::lock_guard lg{mt};
                chosenImage = imageOfTheDay;
            }

            auto sf = restinio::sendfile(chosenImage);
            auto modified_at =
                    restinio::make_date_field_value(sf.meta().last_modified_at());

            auto expires_at =
                    restinio::make_date_field_value(
                            std::chrono::system_clock::now() +
                            std::chrono::hours(24 * 7));

            return
                    req->create_response()
                            .append_header(
                                    restinio::http_field::server,
                                    "Wallpaper service")
                            .append_header_date_field()
                            .append_header(
                                    restinio::http_field::last_modified,
                                    std::move(modified_at))
                            .append_header(
                                    restinio::http_field::expires,
                                    std::move(expires_at))
                            .append_header(
                                    restinio::http_field::content_type,
                                    content_type_by_file_extention(chosenImage.extension().string()))
                            .set_body(std::move(sf))
                            .done();
        }
        catch (std::exception const& e)
        {
            return req->create_response( restinio::status_not_found() )
                            .append_header_date_field()
                            .connection_close()
                            .done();
        }
    });

    return router;
}

int main(int argc, char* argv[])
{
    try
    {
        CLI::App app("Wallpaper service");

        std::string imageLibraryPath;
        app.add_option("--library-path", imageLibraryPath, "Image library absolute path")->required();

        int ServicePort = 8080;
        app.add_option("--port", ServicePort, "Service port")->required();

        CLI11_PARSE(app, argc, argv);

        // Initialize library
        auto const imageLibrary = createDatabase(imageLibraryPath);

        if (imageLibrary.empty())
        {
            throw std::invalid_argument("There are no files in provided library");
        }

        std::random_device rd;
        std::mt19937 gen{rd()};
        std::uniform_int_distribution<uint32_t> dis(0, imageLibrary.size() - 1);

        imageOfTheDay = imageLibrary.at(dis(gen));

        std::thread updatePapersThread([&imageLibrary, &dis, &gen]()
            {
                while(true)
                {
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(std::chrono::duration(24h));

                    {
                        std::lock_guard lg{mt};
                        imageOfTheDay = imageLibrary.at(dis(gen));
                    }
                }
            });
        updatePapersThread.detach();

        // Initialize service
        using traits_t = restinio::traits_t<restinio::asio_timer_manager_t, restinio::single_threaded_ostream_logger_t, router_t>;
        restinio::run(
            restinio::on_this_thread<traits_t>().port(ServicePort).address("localhost").
            request_handler(create_request_handler()));
    }
    catch (std::exception const& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }

    return 0;
}
