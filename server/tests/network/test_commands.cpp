#include <gtest/gtest.h>
#include <memory>
#include <future>
#include "network/commands/login_command.hpp"
#include "network/commands/sign_up_command.hpp"
#include "network/commands/place_ship_command.hpp"
#include "network/commands/move_command.hpp"
#include "network/commands/clear_field_command.hpp"
#include "network/commands/ready_command.hpp"
#include "network/commands/play_command.hpp"
#include "network/commands/disconnect_command.hpp"
#include "core/session_manager.hpp"
#include "auth/auth_manager.hpp"
#include "database/database_manager.hpp"
#include "utils/thread_pool.hpp"

class CommandTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_pool = std::make_shared<ThreadPool>(1);
        action_pool = std::make_shared<ThreadPool>(1);
        db = std::make_unique<DatabaseManager>("cmd_test.db");
        auth = std::make_unique<AuthManager>(*db, *db_pool);
        session = std::make_unique<SessionManager>(*action_pool, *db);
    }

    void TearDown() override {
        session.reset();
        auth.reset();
        db.reset();
        action_pool.reset();
        db_pool.reset();
        std::remove("cmd_test.db");
    }

    bool wait_bool_async(std::function<void(std::function<void(bool)>)> async_func) {
        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();
        async_func([promise](bool ok) { promise->set_value(ok); });
        if (future.wait_for(std::chrono::seconds(2)) == std::future_status::ready)
            return future.get();
        return false;
    }

    void wait_void_async(std::function<void(std::function<void()>)> async_func) {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();
        async_func([promise]() { promise->set_value(); });
        future.wait_for(std::chrono::seconds(2));
    }

    std::shared_ptr<ThreadPool> db_pool;
    std::shared_ptr<ThreadPool> action_pool;
    std::unique_ptr<DatabaseManager> db;
    std::unique_ptr<AuthManager> auth;
    std::unique_ptr<SessionManager> session;
};

TEST_F(CommandTest, LoginCommandSuccess) {
    db->createUser("alice", "secret");

    bool login_success = false;
    std::string last_response;
    CommandContext ctx;
    ctx.send_response = [&](const std::string& msg) { last_response = msg; };
    ctx.on_login_success = [&](std::shared_ptr<Player> p) { login_success = true; };

    auto cmd = std::make_shared<LoginCommand>(*auth);
    std::vector<std::string> args = {"login", "alice", "secret"};
    cmd->handle(args, ctx);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(login_success);
    EXPECT_EQ(last_response, "Logging successful");
}

TEST_F(CommandTest, LoginCommandWrongPassword) {
    db->createUser("bob", "pass");

    bool login_success = false;
    std::string last_response;
    CommandContext ctx;
    ctx.send_response = [&](const std::string& msg) { last_response = msg; };
    ctx.on_login_success = [&](std::shared_ptr<Player> p) { login_success = true; };

    auto cmd = std::make_shared<LoginCommand>(*auth);
    std::vector<std::string> args = {"login", "bob", "wrong"};
    cmd->handle(args, ctx);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(login_success);
    EXPECT_EQ(last_response, "Login failed. Invalid details.");
}

TEST_F(CommandTest, LoginCommandMissingArgs) {
    auto cmd = std::make_shared<LoginCommand>(*auth);
    std::vector<std::string> args = {"login"};
    std::string response;
    CommandContext ctx;
    ctx.send_response = [&](const std::string& msg) { response = msg; };

    cmd->handle(args, ctx);
    EXPECT_EQ(response, "Missing login or password, try again");
}

TEST_F(CommandTest, SignUpCommandSuccess) {
    bool registered = false;
    std::string response;
    CommandContext ctx;
    ctx.send_response = [&](const std::string& msg) { response = msg; };
    ctx.on_login_success = [&](std::shared_ptr<Player> p) { registered = true; };

    auto cmd = std::make_shared<SignUpCommand>(*auth);
    std::vector<std::string> args = {"signup", "charlie", "pass"};
    cmd->handle(args, ctx);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(registered);
    EXPECT_EQ(response, "New user registered");
}

TEST_F(CommandTest, SignUpCommandDuplicateLogin) {
    db->createUser("duplicate", "pass");

    bool registered = false;
    std::string response;
    CommandContext ctx;
    ctx.send_response = [&](const std::string& msg) { response = msg; };
    ctx.on_login_success = [&](std::shared_ptr<Player> p) { registered = true; };

    auto cmd = std::make_shared<SignUpCommand>(*auth);
    std::vector<std::string> args = {"signup", "duplicate", "pass"};
    cmd->handle(args, ctx);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(registered);
    EXPECT_EQ(response, "Reqistration failed");
}

TEST_F(CommandTest, PlaceShipCommandValid) {
    db->createUser("player", "pass");
    auto player = db->authenticate("player", "pass");
    ASSERT_NE(player, nullptr);

    db->createUser("player2", "pass");
    auto player2 = db->authenticate("player2", "pass");
    session->add_to_queue_async(player);
    session->add_to_queue_async(player2);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    int game_id = session->get_player_game(player->get_id());
    ASSERT_NE(game_id, -1);

    bool ship_placed = false;
    std::string response;
    CommandContext ctx;
    ctx.current_player = player;
    ctx.send_response = [&](const std::string& msg) { response = msg; };

    auto cmd = std::make_shared<PlaceShipCommand>(*session);
    std::vector<std::string> args = {"place", "4", "0", "0", "1"};
    cmd->handle(args, ctx);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(response, "Ship successfully placed");
}

TEST_F(CommandTest, PlaceShipCommandNoPlayer) {
    CommandContext ctx;
    std::string response;
    ctx.send_response = [&](const std::string& msg) { response = msg; };
    auto cmd = std::make_shared<PlaceShipCommand>(*session);
    std::vector<std::string> args = {"place", "4", "0", "0", "1"};
    cmd->handle(args, ctx);
    EXPECT_EQ(response, "ERROR: Not authorized");
}

TEST_F(CommandTest, MoveCommandSuccess) {
    db->createUser("move1", "pass");
    db->createUser("move2", "pass");
    auto p1 = db->authenticate("move1", "pass");
    auto p2 = db->authenticate("move2", "pass");
    session->add_to_queue_async(p1);
    session->add_to_queue_async(p2);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    int game_id = session->get_player_game(p1->get_id());
    ASSERT_NE(game_id, -1);

    session->place_ship_async(p1->get_id(), game_id, 4, 0, 0, true, [](bool){});
    session->place_ship_async(p1->get_id(), game_id, 3, 5, 0, true, [](bool){});
    session->place_ship_async(p1->get_id(), game_id, 3, 0, 2, true, [](bool){});
    session->place_ship_async(p1->get_id(), game_id, 2, 4, 2, true, [](bool){});
    session->place_ship_async(p1->get_id(), game_id, 2, 7, 2, true, [](bool){});
    session->place_ship_async(p1->get_id(), game_id, 2, 0, 4, true, [](bool){});
    session->place_ship_async(p1->get_id(), game_id, 1, 3, 4, true, [](bool){});
    session->place_ship_async(p1->get_id(), game_id, 1, 5, 4, true, [](bool){});
    session->place_ship_async(p1->get_id(), game_id, 1, 7, 4, true, [](bool){});
    session->place_ship_async(p1->get_id(), game_id, 1, 9, 4, true, [](bool){});

    session->place_ship_async(p2->get_id(), game_id, 4, 5, 5, true, [](bool){});
    session->place_ship_async(p2->get_id(), game_id, 3, 0, 0, true, [](bool){});
    session->place_ship_async(p2->get_id(), game_id, 3, 7, 0, true, [](bool){});
    session->place_ship_async(p2->get_id(), game_id, 2, 0, 2, true, [](bool){});
    session->place_ship_async(p2->get_id(), game_id, 2, 3, 2, true, [](bool){});
    session->place_ship_async(p2->get_id(), game_id, 2, 6, 2, true, [](bool){});
    session->place_ship_async(p2->get_id(), game_id, 1, 0, 4, true, [](bool){});
    session->place_ship_async(p2->get_id(), game_id, 1, 2, 4, true, [](bool){});
    session->place_ship_async(p2->get_id(), game_id, 1, 9, 2, true, [](bool){});
    session->place_ship_async(p2->get_id(), game_id, 1, 0, 6, true, [](bool){});
    session->set_player_ready(p1->get_id(), game_id);
    session->set_player_ready(p2->get_id(), game_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    bool move_done = false;
    std::string response;
    CommandContext ctx;
    ctx.current_player = p1;
    ctx.send_response = [&](const std::string& msg) { response = msg; };

    auto cmd = std::make_shared<MoveCommand>(*session);
    std::vector<std::string> args = {"move", "5", "5"};
    cmd->handle(args, ctx);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(response, "Move result: 0");
}
