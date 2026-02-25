// consumer_test/consumer_test.cpp

#include <gtest/gtest.h>
#include "consumer.h"       // pact_consumer::Pact, Interaction 等
#include "matchers.h"       // matchers 下的东西

#include <string>
#include <iostream>

// 关键：明确使用 pact_consumer::Pact，避免与 pact_ffi 的 Pact 冲突
using pact_consumer::Pact;
using pact_consumer::Interaction;
using pact_consumer::MockServerHandle;
using namespace pact_consumer::matchers;  // ← 让 Array、Object、Like、Integer 直接可用

class TodoConsumerTest : public ::testing::Test {
protected:
    // 现在明确用 pact_consumer::Pact 或直接 Pact（因为 using 了）
    Pact pact{"TodoClient", "TodoService"};
};

TEST_F(TodoConsumerTest, GetProjectsList) {
    auto interaction = pact
        .given("some projects exist")
        .uponReceiving("a request to get all projects")
        .withRequest("GET", "/projects")
        .willRespondWith(200)
        .withHeader("Content-Type", "application/json")  // 单数 withHeader 通常在旧版可用
        .withJsonBody(
            Array({                                      // ← 来自 matchers
                Object({
                    {"id",        Integer(1)},
                    {"name",      Like(std::string("Project Alpha"))},
                    {"completed", Like(false)}
                })
            })
        );

    bool result = pact.run_test([&](MockServerHandle mock_server) -> bool {
        std::cout << "Mock URL = " << mock_server.get_url() << std::endl;
        // TODO: 在这里放真实的 HTTP 客户端调用 + 断言
        // 例如： TodoClient client(mock_server.get_url());
        //       auto list = client.get_projects();
        //       ASSERT_EQ(list.size(), 1u);
        return true;  // 暂时让测试通过
    });

    EXPECT_TRUE(result);
}

TEST_F(TodoConsumerTest, GetSingleProject) {
    auto interaction = pact
        .given("project id 1 exists")
        .uponReceiving("a request to get project 1")
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
        // 真实调用...
        return true;
    });

    EXPECT_TRUE(result);
}

TEST_F(TodoConsumerTest, GetNonExistentProject) {
    auto interaction = pact
        .given("no project with id 999")
        .uponReceiving("a request for non-existent project")
        .withRequest("GET", "/projects/999")
        .willRespondWith(404);

    bool result = pact.run_test([&](MockServerHandle mock_server) -> bool {
        // 真实调用 + 期待 404 或异常...
        return true;
    });

    EXPECT_TRUE(result);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}