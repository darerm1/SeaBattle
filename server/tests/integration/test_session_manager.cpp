#include <gtest/gtest.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <future>
#include "core/session_manager.hpp"
#include "core/player.hpp"
#include "utils/thread_pool.hpp"
#include "database/database_manager.hpp"

class SessionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        pool = std::make_unique<ThreadPool>(4);
        db = std::make_unique<DatabaseManager>("test_seabattle.db");
        manager = std::make_unique<SessionManager>(*pool, *db);

        p1 = std::make_shared<Player>(1, "Alice", "");
        p2 = std::make_shared<Player>(2, "Bob", "");
        p3 = std::make_shared<Player>(3, "Charlie", "");
        p1->set_points(1000);
        p2->set_points(1100);
        p3->set_points(1050);
    }

    void TearDown() override {
        std::remove("test_seabattle.db");
    }

    template<typename Func>
    bool wait_async(Func&& async_call) {
        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();

        async_call([promise](bool result) {
            promise->set_value(result);
        });

        if (future.wait_for(std::chrono::seconds(1)) == std::future_status::ready) {
            return future.get();
        }
        return false;
    }

    void place_all_ships(SessionManager& manager, int game_id, std::shared_ptr<Player> player) {
        auto place = [&](int len, int x, int y, bool hor) {
            return wait_async([&](auto cb) {
                manager.place_ship_async(player->get_id(), game_id, len, x, y, hor, cb);
            });
        };

        EXPECT_TRUE(place(4, 0, 0, true));
        EXPECT_TRUE(place(3, 5, 0, true));
        EXPECT_TRUE(place(3, 0, 2, true));
        EXPECT_TRUE(place(2, 4, 2, true));
        EXPECT_TRUE(place(2, 7, 2, true));
        EXPECT_TRUE(place(2, 0, 4, true));
        EXPECT_TRUE(place(1, 3, 4, true));
        EXPECT_TRUE(place(1, 5, 4, true));
        EXPECT_TRUE(place(1, 7, 4, true));
        EXPECT_TRUE(place(1, 9, 4, true));
    }

    std::shared_ptr<Player> p1, p2, p3;
    std::unique_ptr<ThreadPool> pool;
    std::unique_ptr<DatabaseManager> db;
    std::unique_ptr<SessionManager> manager;
};

TEST_F(SessionManagerTest, AddToQueueCreatesGameWhenTwoPlayers) {
    manager->add_to_queue_async(p1);
    manager->add_to_queue_async(p2);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    int game_id1 = manager->get_player_game(p1->get_id());
    int game_id2 = manager->get_player_game(p2->get_id());
    EXPECT_NE(game_id1, -1);
    EXPECT_EQ(game_id1, game_id2);
}

TEST_F(SessionManagerTest, PlaceShipAndMakeMove) {
    manager->add_to_queue_async(p1);
    manager->add_to_queue_async(p2);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    int game_id = manager->get_player_game(p1->get_id());

    place_all_ships(*manager, game_id, p1);
    place_all_ships(*manager, game_id, p2);

    manager->set_player_ready(p1->get_id(), game_id);
    manager->set_player_ready(p2->get_id(), game_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto promise = std::make_shared<std::promise<ShotResult>>();
    auto future = promise->get_future();

    manager->make_move_async(p1->get_id(), game_id, 0, 0, [promise](ShotResult res) {
        promise->set_value(res);
    });

    ShotResult result = future.get();
    EXPECT_EQ(result, ShotResult::HIT);
    EXPECT_FALSE(manager->get_session(game_id)->game_is_over());
}

TEST_F(SessionManagerTest, TimeoutAfterMissThenNoMove) {
    manager->add_to_queue_async(p1);
    manager->add_to_queue_async(p2);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    int game_id = manager->get_player_game(p1->get_id());
    int p1_initial = p1->get_points();
    int p2_initial = p2->get_points();

    place_all_ships(*manager, game_id, p1);
    place_all_ships(*manager, game_id, p2);

    manager->set_player_ready(p1->get_id(), game_id);
    manager->set_player_ready(p2->get_id(), game_id);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto session = manager->get_session(game_id);
    ASSERT_NE(session, nullptr);
    EXPECT_TRUE(session->game_is_started());

    wait_async([&](auto cb) {
        manager->make_move_async(p1->get_id(), game_id, 1, 1, [cb](ShotResult res) {
            cb(res == ShotResult::MISS);
        });
    });

    std::this_thread::sleep_for(std::chrono::seconds(4));
    manager->check_timeouts();
    
    session = manager->get_session(game_id);
    EXPECT_NE(session, nullptr);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    manager->check_timeouts();

    session = manager->get_session(game_id);
    EXPECT_EQ(session, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    int diff = std::abs(p1_initial - p2_initial) * 10 / 100;
    EXPECT_EQ(p1->get_points(), p1_initial + diff);
    EXPECT_EQ(p2->get_points(), p2_initial - diff);
}
