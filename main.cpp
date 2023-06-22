#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include <iostream>
#include <string>
#include <string_view>

std::string makeGoToHtml(const std::string_view to) {
    std::string result;
    result = "<p>Go to <a href=\"";
    result += to;
    result += "\">";
    result += to;
    result += "</a>.</p>";
    return result;
}

std::string makeRedirectToHtml(const std::string_view to) {
    std::string result;
    result = "<p>Should have redirected to <a href=\"";
    result += to;
    result += "\">";
    result += to;
    result += "</a>.</p>";
    return result;
}

void onUriSchemeRequest(WebKitURISchemeRequest *wkRequest, gpointer userData) {
    static constexpr const std::string_view indexPath{"/"};
    static constexpr const std::string_view redirectInternalFromPath{"/r/internal"};
    static constexpr const std::string_view redirectInternalToPath{"/r/internal/ok"};
    static constexpr const std::string_view redirectExternalFromPath{"/r/external"};
    static constexpr const std::string_view redirectExternaToPath{"https://duckduckgo.com/"};

    std::string_view path{webkit_uri_scheme_request_get_path(wkRequest)};
    std::string resBody;
    std::string destination;
    int status{200};
    std::string statusReason{"OK"};
    std::string contentType{"text/html"};

    if (path == indexPath) {
        resBody = makeGoToHtml(redirectInternalFromPath);
        resBody += makeGoToHtml(redirectExternalFromPath);
    } else if (path == redirectInternalToPath) {
        resBody = "<p>Success</p>";
    } else if (path == redirectInternalFromPath) {
        status = 302;
        statusReason = "Found";
        destination = redirectInternalToPath;
        resBody = makeRedirectToHtml(redirectInternalToPath);
    } else if (path == redirectExternalFromPath) {
        status = 302;
        statusReason = "Found";
        destination = redirectExternaToPath;
        resBody = makeRedirectToHtml(redirectExternaToPath);
    } else {
        status = 404;
        statusReason = "Not Found";
        resBody = "<p>Not found</p>";
    }

    auto* stream{g_memory_input_stream_new_from_data(resBody.c_str(), resBody.size(), nullptr)};
    auto* wkResponse{webkit_uri_scheme_response_new(stream, resBody.size())};
    g_object_unref(stream);

    webkit_uri_scheme_response_set_status(wkResponse, status, statusReason.c_str());
    webkit_uri_scheme_response_set_content_type(wkResponse, contentType.c_str());

    if (!destination.empty()) {
        auto* wkHeaders{soup_message_headers_new(SOUP_MESSAGE_HEADERS_RESPONSE)};
        soup_message_headers_append(wkHeaders, "Location", destination.c_str());
        webkit_uri_scheme_response_set_http_headers(wkResponse, wkHeaders);
        //soup_message_headers_unref(wkHeaders);
    }

    webkit_uri_scheme_request_finish_with_response(wkRequest, wkResponse);
    g_object_unref(wkResponse);
}

void onActivate(GtkApplication* gtkApp, gpointer userData) {
    auto* gtkWindow{gtk_application_window_new(gtkApp)};
    auto* wkWebView{WEBKIT_WEB_VIEW(webkit_web_view_new())};
    auto* wkContext{webkit_web_context_get_default()};

    webkit_web_context_register_uri_scheme(wkContext, "app", onUriSchemeRequest, nullptr, nullptr);

    auto* settings{webkit_web_view_get_settings(wkWebView)};
    webkit_settings_set_enable_write_console_messages_to_stdout(settings, 1);
    webkit_settings_set_enable_developer_extras(settings, 1);

    gtk_window_set_child(GTK_WINDOW(gtkWindow), GTK_WIDGET(wkWebView));
    gtk_window_present(GTK_WINDOW(gtkWindow));

    webkit_web_view_load_uri(wkWebView, "app:///");
}

int main(int argc, char** argv) {
    auto* gtkApp{gtk_application_new(nullptr, G_APPLICATION_DEFAULT_FLAGS)};
    g_signal_connect(gtkApp, "activate", G_CALLBACK(onActivate), nullptr);
    auto status{g_application_run(G_APPLICATION(gtkApp), argc, argv)};
    g_object_unref(gtkApp);
    return status;
}
