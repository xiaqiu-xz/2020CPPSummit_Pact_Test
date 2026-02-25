// consumer_test/consumer_test.cpp
#include <gtest/gtest.h>
#include <pact-cpp/consumer.hpp>
#include "../src/todo_client.hpp"
// ─────────────────────────────────────────────────
// 测试套件：TodoClient 作为消费者
// ─────────────────────────────────────────────────
class TodoConsumerTest : public ::testing::Test {
protected:
    // 创建 Pact 实例：声明 Consumer 和 Provider 名称
    pact_consumer::Pact pact{
        "TodoClient",  // 消费者名称
        "TodoService"  // 提供者名称
    };
};
// ─────────────────────────────────────────────────
// 测试1：获取项目列表
// ─────────────────────────────────────────────────
TEST_F(TodoConsumerTest, GetProjectsList) {
    // 1⃣ 定义交互期望（Consumer 的期望写入契约）
    auto interaction = pact.uponReceiving("a request to get all projects")
                           .withRequest("GET", "/projects")
                           .withHeader("Accept", "application/json")
                           // ── 定义期望的响应 ──
                           .willRespondWith(200)
                           .withHeader("Content-Type", "application/json")
                           .withBody(R"([
            {
                "id":        { "pact:matcher:type": "integer", "value": 1 },
                "name":      { "pact:matcher:type": "type",    "value": "Project Alpha" },
                "completed": { "pact:matcher:type": "type",    "value": false }
            }
        ])");
    // 注意：使用 matcher 而非固定值
    // integer matcher → 任何整数都能匹配
    // type matcher    → 相同类型的任意值都能匹配
    // 这样 Provider 改变具体值不会破坏契约
    // 2⃣ 运行测试（Pact 启动 Mock Server 来响应请求）
    auto result = pact.run_test([](auto mock_server) {
        TodoClient client;
        client.serverUrl = mock_server->get_url();
        // 调用真实的客户端代码
        auto projects = client.getProjects();
        // 断言客户端行为正确
        EXPECT_FALSE(projects.empty());
        EXPECT_EQ(projects[0].id, 1);
        EXPECT_FALSE(projects[0].name.empty());
        return true;  // 返回 true 表示测试通过
    });
    // 3⃣ 验证所有期望都被满足
    EXPECT_TRUE(result.is_ok()) << "Pact 验证失败: " << result.get_error();
    // ✓ 测试成功后，Pact 自动将契约写入
    //    ./pacts/TodoClient-TodoService.json
}
// ─────────────────────────────────────────────────
// 测试2：获取单个项目
// ─────────────────────────────────────────────────
TEST_F(TodoConsumerTest, GetSingleProject) {
    auto interaction =
        pact.uponReceiving("a request to get project with id 1")
            .given("project with id 1 exists")  // Provider State：告诉 Provider 需要哪种前置状态
            .withRequest("GET", "/projects/1")
            .willRespondWith(200)
            .withBody(R"({
            "id":        1,
            "name":      "Project Alpha",
            "completed": false
        })");
    auto result = pact.run_test([](auto mock_server) {
        TodoClient client;
        client.serverUrl = mock_server->get_url();
        auto project = client.getProject(1);
        EXPECT_EQ(project.id, 1);
        EXPECT_EQ(project.name, "Project Alpha");
        EXPECT_FALSE(project.completed);
        return true;
    });
    EXPECT_TRUE(result.is_ok());
}
// ─────────────────────────────────────────────────
// 测试3：项目不存在时的 404 处理
// ─────────────────────────────────────────────────
TEST_F(TodoConsumerTest, GetNonExistentProject) {
    auto interaction = pact.uponReceiving("a request to get a non-existent project")
                           .given("project with id 999 does not exist")
                           .withRequest("GET", "/projects/999")
                           .willRespondWith(404)
                           .withBody(R"({"error": "Project not found"})");
    auto result = pact.run_test([](auto mock_server) {
        TodoClient client;
        client.serverUrl = mock_server->get_url();
        // 验证客户端正确处理 404
        EXPECT_THROW(client.getProject(999), std::runtime_error);
        return true;
    });
    EXPECT_TRUE(result.is_ok());
}
// ─────────────────────────────────────────────────
// 测试4：创建项目
// ─────────────────────────────────────────────────
TEST_F(TodoConsumerTest, CreateProject) {
    auto interaction = pact.uponReceiving("a request to create a new project")
                           .withRequest("POST", "/projects")
                           .withHeader("Content-Type", "application/json")
                           .withBody(R"({
            "name":      { "pact:matcher:type": "type", "value": "New Project" },
            "completed": false
        })")
                           .willRespondWith(201);
    auto result = pact.run_test([](auto mock_server) {
        TodoClient client;
        client.serverUrl = mock_server->get_url();
        bool created = client.createProject("New Project");
        EXPECT_TRUE(created);
        return true;
    });
    EXPECT_TRUE(result.is_ok());
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}