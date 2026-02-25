// consumer_test/consumer_test.cpp
//
// 头文件路径说明（基于 pact-cplusplus 真实目录结构）：
//   consumer/include/consumer.h   → #include "consumer.h"
//   consumer/include/matchers.h   → #include "matchers.h"
// 注意：没有 pact-cpp/ 子目录，后缀是 .h 不是 .hpp

#include <gtest/gtest.h>
#include "consumer.h"    // ✅ 正确（原来错误地写成 <pact-cpp/consumer.hpp>）
#include "matchers.h"
#include <nlohmann/json.hpp>
#include <string>

// ─────────────────────────────────────────────────────────────
// 参考官方示例：
//   consumer/test/src/consumer_test.cpp
//   consumer/test/src/todo.cpp
// ─────────────────────────────────────────────────────────────

class TodoConsumerTest : public ::testing::Test {
protected:
    pact_consumer::Pact pact{
        "TodoClient",   // Consumer 名称
        "TodoService"   // Provider 名称
    };
};

// 测试：GET /projects 返回项目列表
TEST_F(TodoConsumerTest, GetProjectsList) {

    auto interaction = pact
        .uponReceiving("a request to get all projects")
        .withRequest("GET", "/projects")
        .willRespondWith(200)
        .withHeader("Content-Type", "application/json")
        .withJsonBody(
            pact_consumer::Array({
                pact_consumer::Object({
                    {"id",        pact_consumer::Integer(1)},
                    {"name",      pact_consumer::Like(std::string("Project Alpha"))},
                    {"completed", pact_consumer::Like(false)}
                })
            })
        );

    auto result = pact.run_test([](auto mock_server) -> bool {
        // 在此处调用你的 HTTP 客户端，指向 mock_server->get_url()
        // 例如：
        //   TodoClient client(mock_server->get_url());
        //   auto projects = client.getProjects();
        //   EXPECT_EQ(projects.size(), 1);
        return true;
    });

    EXPECT_EQ(result, pact_consumer::PactTestResult::Ok);
}

// 测试：GET /projects/1 返回单个项目
TEST_F(TodoConsumerTest, GetSingleProject) {

    auto interaction = pact
        .uponReceiving("a request to get project with id 1")
        .given("project with id 1 exists")
        .withRequest("GET", "/projects/1")
        .willRespondWith(200)
        .withJsonBody(
            pact_consumer::Object({
                {"id",        pact_consumer::Integer(1)},
                {"name",      pact_consumer::Like(std::string("Project Alpha"))},
                {"completed", pact_consumer::Like(false)}
            })
        );

    auto result = pact.run_test([](auto mock_server) -> bool {
        return true;
    });

    EXPECT_EQ(result, pact_consumer::PactTestResult::Ok);
}

// 测试：项目不存在返回 404
TEST_F(TodoConsumerTest, GetNonExistentProject) {

    auto interaction = pact
        .uponReceiving("a request to get a non-existent project")
        .given("project with id 999 does not exist")
        .withRequest("GET", "/projects/999")
        .willRespondWith(404);

    auto result = pact.run_test([](auto mock_server) -> bool {
        return true;
    });

    EXPECT_EQ(result, pact_consumer::PactTestResult::Ok);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}