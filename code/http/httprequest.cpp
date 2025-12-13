#include "httprequest.h"
using namespace std;

// Webpage path
const unordered_set<string> HttpRequest::DEFAULT_HTML{"/index",   "/register", "/login",
                                                      "/welcome", "/video",    "/picture"};

// Login page and register page
const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG{{"/login.html", 1}, {"/register.html", 0}};

void HttpRequest::Init() {
    state_ = REQUEST_LINE;
    method_ = path_ = version_ = body_ = "";
    header_.clear();
    post_.clear();
}

// Use state machine to parse the request
bool HttpRequest::parse(Buffer& buff) {
    const char END[] = "\r\n";
    // If the buffer is empty, return false
    if (buff.readableBytes() == 0) {
        return false;
    };
    while (buff.readableBytes() && state_ != FINISH) {
        // Reads a string ends with '\r\n' for each loop
        const char* lineEnd = search(buff.Peek(), buff.BeginWriteConst(), END, END + 2);
        string line(buff.Peek(), lineEnd);
        switch (state_) {
            case REQUEST_LINE:
                // If the request line is not parsed successfully, return false
                if (!ParseRequestLine_(line)) {
                    return false;
                }
                ParsePath_();
                break;
            case HEADERS:
                ParseHeader_(line);
                if (buff.readableBytes() <= 2) {
                    state_ = FINISH;
                }
                break;
            case BODY:
                ParseBody_(line);
                break;
            default:
                break;
        }
        if (lineEnd == buff.BeginWrite()) {
            buff.RetrieveAll();
            break;
        }
        // Skip the '\r\n' characters
        buff.RetrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

bool HttpRequest::ParseRequestLine_(const string& line) {
    regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch Match;
    if (regex_match(line, Match, pattern)) {
        method_ = Match[1];
        path_ = Match[2];
        version_ = Match[3];
        state_ = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::ParsePath_() {
    if (path_ == "/") {
        path_ = "/index.html";
    } else {
        if (DEFAULT_HTML.find(path_) != DEFAULT_HTML.end()) {
            path_ += ".html";
        }
    }
}

void HttpRequest::ParseHeader_(const string& line) {
    regex pattern("^([^:]*): ?(.*)$");
    smatch Match;
    if (regex_match(line, Match, pattern)) {
        header_[Match[1]] = Match[2];
    } else {
        //  The header is empty, which means the body is coming
        state_ = BODY;
    }
}

void HttpRequest::ParseBody_(const string& line) {
    body_ = line;
    ParsePost_();
    // The state is set to FINISH, which means the parsing is complete
    state_ = FINISH;
    LOG_DEBUG("Body: %s, len: %d", line.c_str(), line.size());
}

int HttpRequest::ConvertHex(char ch) {
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return ch;
}

// The POST request contains data of username and password
void HttpRequest::ParsePost_() {
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlencoded_();
        if (DEFAULT_HTML_TAG.count(path_)) {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag: %d", tag);
            if (tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                if (UserVerify(post_["username"], post_["password"], isLogin)) {
                    path_ = "/welcome.html";
                } else {
                    path_ = "/error.html";
                }
            }
        }
    }
}

void HttpRequest::ParseFromUrlencoded_() {
    if (body_.size() == 0) return;
    string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;
    for (; i < n; i++) {
        char ch = body_[i];
        switch (ch) {
            case '=':
                key = body_.substr(j, i - j);
                j = i + 1;
                break;
            case '+':
                body_[i] = ' ';
                break;
            case '%':
                num = ConvertHex(body_[i + 1]) * 16 + ConvertHex(body_[i + 2]);
                body_[i + 2] = num % 10 + '0';
                body_[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                value = body_.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
    }
    assert(j <= i);
    if (post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

bool HttpRequest::UserVerify(const string& name, const string& pwd, bool isLogin) {
    if (name == "" || pwd == "") return false;
    LOG_INFO("Verify name: %s pwd: %s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SqlConnRAII(&sql, SqlConnPool::Instance());
    assert(sql);

    bool flag = false;
    unsigned int j = 0;
    char order[256] = {0};
    MYSQL_FIELD* fields = nullptr;
    MYSQL_RES* res = nullptr;

    if (!isLogin) {
        flag = true;
    }
    // Search for the user and password in the database
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s", order);

    if (mysql_query(sql, order)) {
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        string password(row[1]);
        // If the user exists and the password is correct, set the flag to true
        if (isLogin) {
            if (pwd == password) {
                flag = true;
            } else {
                flag = false;
                LOG_INFO("pwd error!");
            }
        } else {
            flag = false;
            LOG_INFO("user used!");
        }
    }
    mysql_free_result(res);

    if (!isLogin && flag == true) {
        LOG_DEBUG("register!");
        bzero(order, 256);
        snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s', '%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG("%s", order);
        if (mysql_query(sql, order)) {
            LOG_DEBUG("Insert error!");
            flag = false;
        }
        flag = true;
    }
    LOG_DEBUG("UserVerify success!");
    return flag;
}

string HttpRequest::path() const { return path_; }

string& HttpRequest::path() { return path_; }

string HttpRequest::method() const { return method_; }

string HttpRequest::version() const { return version_; }

string HttpRequest::GetPost(const string& key) const {
    assert(key != "");
    if (post_.count(key) == 1) return post_.find(key)->second;
    return "";
}

string HttpRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    if (post_.count(key) == 1) return post_.find(key)->second;
    return "";
}

bool HttpRequest::IsKeepAlive() const {
    return header_.count("Connection") == 1 && header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
}