#include <gtest/gtest.h>
#include <memory>
#include "network/commands/base_command.hpp"

class MockCommand : public BaseCommand {
public:
    std::vector<std::vector<std::string>> received_args;
    std::vector<CommandContext> received_contexts;

    void handle(const std::vector<std::string>& args, CommandContext context) override {
        received_args.push_back(args);
        received_contexts.push_back(context);
        if (next_) next_->handle(args, context);
    }
};

TEST(ProtocolTest, CommandParsingAndChaining) {
    auto mock1 = std::make_shared<MockCommand>();
    auto mock2 = std::make_shared<MockCommand>();
    auto mock3 = std::make_shared<MockCommand>();
    mock1->setNext(mock2);
    mock2->setNext(mock3);

    std::vector<std::string> args = {"login", "alice", "pass"};

    CommandContext ctx;
    ctx.send_response = [](const std::string&) { };
    ctx.on_login_success = [](std::shared_ptr<Player>) { };
    ctx.close_connection = []() {};

    mock1->handle(args, ctx);

    EXPECT_EQ(mock1->received_args.size(), 1);
    EXPECT_EQ(mock2->received_args.size(), 1);
    EXPECT_EQ(mock3->received_args.size(), 1);
    EXPECT_EQ(mock1->received_args[0], args);
    EXPECT_EQ(mock2->received_args[0], args);
    EXPECT_EQ(mock3->received_args[0], args);
}

TEST(ProtocolTest, CommandSplitting) {
    auto mock = std::make_shared<MockCommand>();

    std::string input = "login alice pass\nplay\nmove 3 4\n";
    std::istringstream iss(input);
    std::string line;
    CommandContext ctx;
    ctx.send_response = [](const std::string&) { };
    ctx.on_login_success = [](std::shared_ptr<Player>) { };
    ctx.close_connection = []() {};

    while (std::getline(iss, line)) {
        std::istringstream line_stream(line);
        std::vector<std::string> args;
        std::string arg;
        while (line_stream >> arg) {
            args.push_back(arg);
        }
        mock->handle(args, ctx);
    }

    EXPECT_EQ(mock->received_args.size(), 3);
    EXPECT_EQ(mock->received_args[0], (std::vector<std::string>{"login", "alice", "pass"}));
    EXPECT_EQ(mock->received_args[1], (std::vector<std::string>{"play"}));
    EXPECT_EQ(mock->received_args[2], (std::vector<std::string>{"move", "3", "4"}));
}
