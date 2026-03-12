#include <gtest/gtest.h>
#include "core/session.hpp"
#include "core/player.hpp"

class SessionTest : public ::testing::Test {
protected:
    void SetUp() override {
        p1 = std::make_shared<Player>(1, "Alice", "");
        p2 = std::make_shared<Player>(2, "Bob", "");
        p1->set_points(1000);
        p2->set_points(1200);
        session = std::make_shared<Session>(1, p1, p2);
    }

        void place_all_ships_for_player(int player_id) {
        EXPECT_TRUE(session->place_ship(player_id, 4, 0, 0, true));
        EXPECT_TRUE(session->place_ship(player_id, 3, 5, 0, true));
        EXPECT_TRUE(session->place_ship(player_id, 3, 0, 2, true));
        EXPECT_TRUE(session->place_ship(player_id, 2, 4, 2, true));
        EXPECT_TRUE(session->place_ship(player_id, 2, 7, 2, true));
        EXPECT_TRUE(session->place_ship(player_id, 2, 0, 4, true));
        EXPECT_TRUE(session->place_ship(player_id, 1, 3, 4, true));
        EXPECT_TRUE(session->place_ship(player_id, 1, 5, 4, true));
        EXPECT_TRUE(session->place_ship(player_id, 1, 7, 4, true));
        EXPECT_TRUE(session->place_ship(player_id, 1, 9, 4, true));
    }

    void kill_all_opponents_ships(int player_id) {
        EXPECT_EQ(session->make_move(player_id, 0, 0), ShotResult::HIT);
        EXPECT_EQ(session->make_move(player_id, 1, 0), ShotResult::HIT);
        EXPECT_EQ(session->make_move(player_id, 2, 0), ShotResult::HIT);
        EXPECT_EQ(session->make_move(player_id, 3, 0), ShotResult::HIT);
        
        EXPECT_EQ(session->make_move(player_id, 5, 0), ShotResult::HIT);
        EXPECT_EQ(session->make_move(player_id, 6, 0), ShotResult::HIT);
        EXPECT_EQ(session->make_move(player_id, 7, 0), ShotResult::HIT);
        
        EXPECT_EQ(session->make_move(player_id, 0, 2), ShotResult::HIT);
        EXPECT_EQ(session->make_move(player_id, 1, 2), ShotResult::HIT);
        EXPECT_EQ(session->make_move(player_id, 2, 2), ShotResult::HIT);
        
        EXPECT_EQ(session->make_move(player_id, 4, 2), ShotResult::HIT);
        EXPECT_EQ(session->make_move(player_id, 5, 2), ShotResult::HIT);
        
        EXPECT_EQ(session->make_move(player_id, 7, 2), ShotResult::HIT);
        EXPECT_EQ(session->make_move(player_id, 8, 2), ShotResult::HIT);
        
        EXPECT_EQ(session->make_move(player_id, 0, 4), ShotResult::HIT);
        EXPECT_EQ(session->make_move(player_id, 1, 4), ShotResult::HIT);

        EXPECT_EQ(session->make_move(player_id, 3, 4), ShotResult::HIT);
        
        EXPECT_EQ(session->make_move(player_id, 5, 4), ShotResult::HIT);
        
        EXPECT_EQ(session->make_move(player_id, 7, 4), ShotResult::HIT);
        
        EXPECT_EQ(session->make_move(player_id, 9, 4), ShotResult::GAME_OVER);
    }

    std::shared_ptr<Player> p1, p2;
    std::shared_ptr<Session> session;
};

TEST_F(SessionTest, FirstTurnDeterminedByLowerRating) {
    EXPECT_EQ(session->get_current_turn(), 1);
}

TEST_F(SessionTest, PlaceShipOnlyInSetup) {
    place_all_ships_for_player(1);
    place_all_ships_for_player(2);
    EXPECT_TRUE(session->set_player_ready(1));
    EXPECT_TRUE(session->set_player_ready(2));
    
    EXPECT_FALSE(session->place_ship(1, 3, 2, 2, true));
}

TEST_F(SessionTest, PlayersReadyStartsGame) {
    place_all_ships_for_player(1);
    place_all_ships_for_player(2);
    EXPECT_TRUE(session->set_player_ready(1));
    EXPECT_TRUE(session->set_player_ready(2));
    
    EXPECT_TRUE(session->game_is_started());
}

TEST_F(SessionTest, CannotMakeMoveBeforeGameStart) {
    ShotResult result = session->make_move(1, 0, 0);
    EXPECT_EQ(result, ShotResult::INVALID);
}

TEST_F(SessionTest, MakeMoveInvalidTurn) {
    place_all_ships_for_player(1);
    place_all_ships_for_player(2);
    EXPECT_TRUE(session->set_player_ready(1));
    EXPECT_TRUE(session->set_player_ready(2));
    
    ShotResult result = session->make_move(2, 0, 0);
    EXPECT_EQ(result, ShotResult::INVALID);
}

TEST_F(SessionTest, MakeMoveValidTurn) {
    place_all_ships_for_player(1);
    place_all_ships_for_player(2);
    EXPECT_TRUE(session->set_player_ready(1));
    EXPECT_TRUE(session->set_player_ready(2));
    
    ShotResult result = session->make_move(1, 5, 0);
    EXPECT_EQ(result, ShotResult::HIT);
}

TEST_F(SessionTest, TurnSwitchesAfterMiss) {
    place_all_ships_for_player(1);
    place_all_ships_for_player(2);
    EXPECT_TRUE(session->set_player_ready(1));
    EXPECT_TRUE(session->set_player_ready(2));
    
    ShotResult result = session->make_move(1, 1, 1);
    EXPECT_EQ(result, ShotResult::MISS);
    EXPECT_EQ(session->get_current_turn(), 2);
}

TEST_F(SessionTest, TurnDoesNotSwitchAfterHit) {
    place_all_ships_for_player(1);
    place_all_ships_for_player(2);
    EXPECT_TRUE(session->set_player_ready(1));
    EXPECT_TRUE(session->set_player_ready(2));
    
    ShotResult result = session->make_move(1, 5, 0);
    EXPECT_EQ(result, ShotResult::HIT);
    EXPECT_EQ(session->get_current_turn(), 1);
}

TEST_F(SessionTest, GameOverWhenAllShipsSunk) {
    place_all_ships_for_player(1);
    place_all_ships_for_player(2);
    EXPECT_TRUE(session->set_player_ready(1));
    EXPECT_TRUE(session->set_player_ready(2));
    
    kill_all_opponents_ships(1);
    EXPECT_TRUE(session->game_is_over());
    EXPECT_EQ(session->get_winner_id(), 1);
}

TEST_F(SessionTest, RatingUpdateAfterWin) {
    int p1_initial = p1->get_points();
    int p2_initial = p2->get_points();
    
    place_all_ships_for_player(1);
    place_all_ships_for_player(2);
    EXPECT_TRUE(session->set_player_ready(1));
    EXPECT_TRUE(session->set_player_ready(2));
    kill_all_opponents_ships(1);
    EXPECT_TRUE(session->game_is_over());
    EXPECT_EQ(session->get_winner_id(), 1);

    session->end_game();
    
    int diff = std::abs(p1_initial - p2_initial) * 10 / 100;
    EXPECT_EQ(p1->get_points(), p1_initial + diff);
    EXPECT_EQ(p2->get_points(), std::max(p2_initial - diff, 0));
}

TEST_F(SessionTest, CheckTimeoutReturnsFalseWhenGameNotInProgress) {
    EXPECT_FALSE(session->check_timeout());
}

TEST_F(SessionTest, TimeoutDoesNotTriggerImmediately) {
    place_all_ships_for_player(1);
    place_all_ships_for_player(2);

    EXPECT_TRUE(session->set_player_ready(1));
    EXPECT_TRUE(session->set_player_ready(2));
    EXPECT_FALSE(session->check_timeout());
}

TEST_F(SessionTest, ClearFieldOnlyInSetup) {
    place_all_ships_for_player(1);
    place_all_ships_for_player(2);

    EXPECT_TRUE(session->clear_field(1));
    
    place_all_ships_for_player(1);
    
    session->set_player_ready(1);
    session->set_player_ready(2);
    
    EXPECT_FALSE(session->clear_field(1));
    
    ShotResult result = session->make_move(1, 0, 0);
    EXPECT_EQ(result, ShotResult::HIT);
}

TEST_F(SessionTest, ClearFieldInvalidPlayer) {
    session->clear_field(999);
}
