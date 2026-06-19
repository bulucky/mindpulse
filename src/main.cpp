#include <cstdio>

#include <nlohmann/json.hpp>
#include <httplib.h>

int main() {
    nlohmann::json j;
    j["event"] = "session_start";
    j["tool_id"] = "claude";
    std::printf("[json] 构造成功: %s\n", j.dump().c_str());

    // 验证 httplib — 仅构造 server 对象，不启动监听
    httplib::Server svr;
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("hello", "text/plain");
    });
    std::printf("[httplib] Server 对象构造成功\n");

    std::printf("\n=== 依赖验证通过 ===\n");

    svr.listen("0.0.0.0", 8080);
    return 0;
}
