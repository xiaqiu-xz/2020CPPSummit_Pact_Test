// provider_test/provider_test.cpp
// Provider 拉取契约文件，验证自己的实现是否满足所有期望
#include <pact-cpp/provider.hpp>
#include <gtest/gtest.h>
// Provider State 处理器
// 在验证每个 interaction 前，设置必要的数据库/内存状态
void setup_provider_state(const std::string& state) {
    if (state == "project with id 1 exists") {
        // 在测试数据库中创建 id=1 的项目
        // db.insert({1, "Project Alpha", false});
    } else if (state == "project with id 999 does not exist") {
        // 确保 id=999 的项目不存在
        // db.delete_if_exists(999);
    }
}
TEST(TodoProviderTest, VerifyConsumerContracts) {
    pact_provider::ProviderOptions options;
    options.provider_name = "TodoService";
    options.provider_url = "http://localhost:8080";  // 真实运行的 Provider 地址
    // 从本地文件读取契约（开发阶段）
    options.pact_files = {"./pacts/TodoClient-TodoService.json"};
    // 或从 Pact Broker 读取（CI/CD 阶段）
    // options.broker_url    = "http://pact-broker:9292";
    // options.broker_token  = "your-token";
    // 注册 Provider State 处理器
    options.state_change_handler = setup_provider_state;
    // 回放所有 interactions，验证 Provider 的真实响应
    auto result = pact_provider::verify(options);
    EXPECT_TRUE(result.is_ok()) << "Provider 验证失败，以下 interactions 不符合契约:\n"
                                << result.get_error();
}