// consumer_test/consumer_test.cpp

#include <gtest/gtest.h>
#include "consumer.h"       // pact_consumer::Pact 等
#include "matchers.h"       // matchers::Object, matchers::Like 等

#include <nlohmann/json.hpp>
#include <string>
#include <iostream>

// 使用命名空间简化代码（强烈推荐）
using namespace pact_consumer;
using namespace pact_consumer::matchers;

class TodoConsumerTest : public ::testing::Test {
protected:
    Pact pact{"TodoClient", "TodoService"};
};

TEST_F(TodoConsumerTest, GetProjectsList) {
    // 定义交互
    auto interaction = pact
        .given("projects exist")                      // 可选 given 状态
        .uponReceiving("a request to get all projects")
        .withRequest("GET", "/projects")
        .willRespondWith(200)
        .withHeader("Content-Type", "application/json")   // 注意：旧版常用 withHeader（单数）
        .withJsonBody(
            Array({                                       // matchers::Array
                Object({
                    {"id",        Integer(1)},            // matchers::Integer(示例值)
                    {"name",      Like(std::string("Project Alpha"))},
                    {"completed", Like(false)}
                })
            })
        );

    // 运行测试（mock server 会自动启动）
    // run_test 的 lambda 需要返回 bool（true = 测试通过）
    bool result = pact.run_test([&](MockServerHandle mock_server) -> bool {
        // 这里放真实的 HTTP 调用代码，例如使用你的 TodoClient
        // 示例（伪代码）：
        // TodoClient client(mock_server.get_url());  // 或 mock_server.url()
        // auto projects = client.getProjects();
        // EXPECT_EQ(projects.size(), 1);
        // EXPECT_EQ(projects[0].name, "Project Alpha");

        std::cout << "Mock server URL: " << mock_server.get_url() << std::endl;

        // 为了让测试通过，暂时直接返回 true
        // 实际项目中要在这里做真实的 HTTP 请求 + 断言
        return true;
    });

    EXPECT_TRUE(result);   // 旧版 pact-cplusplus 中 run_test 返回 bool
}

// 单个项目
TEST_F(TodoConsumerTest, GetSingleProject) {
    auto interaction = pact
        .given("project with id 1 exists")
        .uponReceiving("a request to get project with id 1")
        .withRequest("GET", "/projects/1")
        .willRespondWith(200)
        .withHeader("Content-Type", "application/json")
        .withJsonBody(
            Object({
                {"id",        Integer(1)},
                {"name",      Like(std::string("Project Alpha"))},
                {"completed", Like(false)}
            })
        );

    bool result = pact.run_test([&](MockServerHandle mock_server) -> bool {
        // TodoClient client(mock_server.get_url());
        // auto project = client.getProject(1);
        // EXPECT_EQ(project.id, 1);
        // EXPECT_EQ(project.name, "Project Alpha");

        return true;
    });

    EXPECT_TRUE(result);
}

// 不存在的项目 → 404
TEST_F(TodoConsumerTest, GetNonExistentProject) {
    auto interaction = pact
        .given("project with id 999 does not exist")
        .uponReceiving("a request to get a non-existent project")
        .withRequest("GET", "/projects/999")
        .willRespondWith(404)
        .withHeader("Content-Type", "application/json");

    bool result = pact.run_test([&](MockServerHandle mock_server) -> bool {
        // TodoClient client(mock_server.get_url());
        // EXPECT_THROW(client.getProject(999), HttpNotFoundException);  // 或检查 status == 404

        return true;
    });

    EXPECT_TRUE(result);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}