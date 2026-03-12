#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "core/session_manager.hpp"
#include "core/player.hpp"

class SessionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        p1 = std::make_shared<Player>(1, "Alice", "");
        p2 = std::make_shared<Player>(2, "Bob", "");
        p3 = std::make_shared<Player>(3, "Charlie", "");
        p1->set_points(1000);
        p2->set_points(1100);
        p3->set_points(1050);
    }

    void place_all_ships(SessionManager& manager, int game_id, int player_id) {
        EXPECT_TRUE(manager.place_ship(player_id, game_id, 4, 0, 0, true));
        EXPECT_TRUE(manager.place_ship(player_id, game_id, 3, 5, 0, true));
        EXPECT_TRUE(manager.place_ship(player_id, game_id, 3, 0, 2, true));
        EXPECT_TRUE(manager.place_ship(player_id, game_id, 2, 4, 2, true));
        EXPECT_TRUE(manager.place_ship(player_id, game_id, 2, 7, 2, true));
        EXPECT_TRUE(manager.place_ship(player_id, game_id, 2, 0, 4, true));
        EXPECT_TRUE(manager.place_ship(player_id, game_id, 1, 3, 4, true));
        EXPECT_TRUE(manager.place_ship(player_id, game_id, 1, 5, 4, true));
        EXPECT_TRUE(manager.place_ship(player_id, game_id, 1, 7, 4, true));
        EXPECT_TRUE(manager.place_ship(player_id, game_id, 1, 9, 4, true));
    }

    std::shared_ptr<Player> p1, p2, p3;
    SessionManager manager;
};

TEST_F(SessionManagerTest, AddToQueueCreatesGameWhenTwoPlayers) {
    manager.add_to_queue(p1);
    manager.add_to_queue(p2);
    
    int game_id1 = manager.get_player_game(1);
    int game_id2 = manager.get_player_game(2);
    EXPECT_NE(game_id1, -1);
    EXPECT_EQ(game_id1, game_id2);
}

TEST_F(SessionManagerTest, AddToQueueWithThreePlayersMatchesFirstTwo) {
    manager.add_to_queue(p1);
    manager.add_to_queue(p2);
    manager.add_to_queue(p3);
    
    EXPECT_NE(manager.get_player_game(1), -1);
    EXPECT_NE(manager.get_player_game(2), -1);
    EXPECT_EQ(manager.get_player_game(3), -1);
}

TEST_F(SessionManagerTest, PlayerDisconnectedFromQueue) {
    manager.add_to_queue(p1);
    manager.add_to_queue(p2);
    manager.player_disconnected(1);
    
    EXPECT_EQ(manager.get_player_game(1), -1);
    EXPECT_EQ(manager.get_player_game(2), -1);
}

TEST_F(SessionManagerTest, PlayerDisconnectedDuringGame) {
    manager.add_to_queue(p1);
    manager.add_to_queue(p2);
    int game_id = manager.get_player_game(1);
    ASSERT_NE(game_id, -1);
    
    manager.player_disconnected(1);
    
    EXPECT_EQ(manager.get_player_game(1), -1);
    EXPECT_EQ(manager.get_player_game(2), -1);
    EXPECT_EQ(manager.get_session(game_id), nullptr);
}

TEST_F(SessionManagerTest, PlaceShipAndMakeMove) {
    manager.add_to_queue(p1);
    manager.add_to_queue(p2);
    int game_id = manager.get_player_game(1);
    
    place_all_ships(manager, game_id, 1);
    place_all_ships(manager, game_id, 2);
    
    manager.set_player_ready(1, game_id);
    manager.set_player_ready(2, game_id);
    
    ShotResult result = manager.make_move(1, game_id, 0, 0);
    EXPECT_EQ(result, ShotResult::HIT);
    EXPECT_FALSE(manager.get_session(game_id)->game_is_over());
}

TEST_F(SessionManagerTest, CheckTimeoutsDoesNotCrash) {
    manager.add_to_queue(p1);
    manager.add_to_queue(p2);
    int game_id = manager.get_player_game(1);
    
    place_all_ships(manager, game_id, 1);
    place_all_ships(manager, game_id, 2);

    manager.set_player_ready(1, game_id);
    manager.set_player_ready(2, game_id);
    
    manager.check_timeouts();
    std::shared_ptr<Session> session = manager.get_session(game_id);
    EXPECT_NE(session, nullptr);
}

TEST_F(SessionManagerTest, ClearField) {
    manager.add_to_queue(p1);
    manager.add_to_queue(p2);
    int game_id = manager.get_player_game(1);
    
    EXPECT_TRUE(manager.place_ship(1, game_id, 4, 0, 0, true));
    manager.clear_field(1, game_id);
    EXPECT_TRUE(manager.place_ship(1, game_id, 4, 0, 0, true));
}

TEST_F(SessionManagerTest, TimeoutAfterMissThenNoMove) {
    manager.add_to_queue(p1);
    manager.add_to_queue(p2);
    int game_id = manager.get_player_game(1);
    int p1_initial = p1->get_points(); // 1000
    int p2_initial = p2->get_points(); // 1100

    place_all_ships(manager, game_id, 1);
    place_all_ships(manager, game_id, 2);

    manager.set_player_ready(1, game_id);
    manager.set_player_ready(2, game_id);

    auto session = manager.get_session(game_id);
    EXPECT_TRUE(session->game_is_started());

    EXPECT_EQ(session->get_current_turn(), 1);
    ShotResult result = manager.make_move(1, game_id, 1, 1);
    EXPECT_EQ(result, ShotResult::MISS);

    std::this_thread::sleep_for(std::chrono::seconds(4));

    manager.check_timeouts();
    session = manager.get_session(game_id);
    EXPECT_FALSE(session->game_is_over());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    manager.check_timeouts();

    session = manager.get_session(game_id);
    EXPECT_EQ(session, nullptr);
    
    int diff = std::abs(p1_initial - p2_initial) * 10 / 100; // 10

    EXPECT_EQ(p1->get_points(), p1_initial + diff);
    EXPECT_EQ(p2->get_points(), p2_initial - diff);
}
