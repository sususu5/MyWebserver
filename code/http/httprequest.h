#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>
#include <mysql/mysql.h>
#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"

class HttpRequest {
public:
    enum PARSE_STATE {REQUEST_LINE, HEADERS, BODY, FINISH};

    HttpRequest() {Init();}
    ~HttpRequest() = default;

    void Init();
    bool parse(Buffer& buff);

    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;

private:
    bool ParseRequestLine_(const std::string& line);    // handle the request line
    void ParseHeader_(const std::string& line);         // handle the header
    void ParseBody_(const std::string& line);           // handle the body

    void ParsePath_();                                  // parse the path
    void ParsePost_();                                  // parse the post content
    void ParseFromUrlencoded_();                        // parse the form content

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);

    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConvertHex(char ch);  // convert hex to decimal
};

#endif