#pragma once
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <iostream>

#pragma comment(lib, "winhttp.lib")

namespace HTTPHelper {

//USING WINDOWS
std::string HttpGet(const std::string& url)
{
    std::string result;

    // Convert URL to wide string
    std::wstring wurl(url.begin(), url.end());

    // Crack the URL into components
    URL_COMPONENTSW components;
    ZeroMemory(&components, sizeof(components));

    wchar_t host[256] = {};
    wchar_t path[2048] = {};

    components.dwStructSize = sizeof(components);
    components.lpszHostName = host;
    components.dwHostNameLength = _countof(host);
    components.lpszUrlPath = path;
    components.dwUrlPathLength = _countof(path);

    if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &components)) {
        std::cerr << "[HttpGet] WinHttpCrackUrl failed, error: " << GetLastError() << std::endl;
        return "";
    }
    // Open WinHTTP session
    HINTERNET hSession = WinHttpOpen(
        L"LoLClient/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (!hSession) {
        std::cerr << "[HttpGet] WinHttpOpen failed, error: " << GetLastError() << std::endl;
        return "";
    }

    // Connect to server
    HINTERNET hConnect = WinHttpConnect(hSession, host, components.nPort, 0);
    if (!hConnect) {
        std::cerr << "[HttpGet] WinHttpConnect failed, error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Open request
    DWORD flags = WINHTTP_FLAG_SECURE;
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        path,
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags
    );

    if (!hRequest) {
        std::cerr << "[HttpGet] WinHttpOpenRequest failed, error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Disable certificate validation for Riot localhost
    DWORD secureFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                        SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
                        SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                        SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

    if (!WinHttpSetOption(hRequest,
        WINHTTP_OPTION_SECURITY_FLAGS,
        &secureFlags,
        sizeof(secureFlags)))
    {
        std::cerr << "[HttpGet] WinHttpSetOption failed, error: " << GetLastError() << std::endl;
    }

    // Send request
    if (!WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0))
    {
        std::cerr << "[HttpGet] WinHttpSendRequest failed, error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Receive response
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        std::cerr << "[HttpGet] WinHttpReceiveResponse failed, error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Read response
    DWORD bytesAvailable = 0;
    while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0)
    {
        std::string buffer(bytesAvailable, 0);
        DWORD bytesRead = 0;
        if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
            result.append(buffer.data(), bytesRead);
        } else {
            std::cerr << "[HttpGet] WinHttpReadData failed, error: " << GetLastError() << std::endl;
            break;
        }
    }

    std::cout << "[HttpGet] Response size: " << result.size() << std::endl;

    // Cleanup
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return result;
}

} // namespace HTTPHelper