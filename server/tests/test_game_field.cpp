#include <gtest/gtest.h>
#include "core/game_field.hpp"

TEST(GameFieldTest, PlaceShipValid) {
    GameField field;
    EXPECT_TRUE(field.set_ship(4, 0, 0, true));
    EXPECT_TRUE(field.set_ship(3, 5, 0, true));
    EXPECT_TRUE(field.set_ship(3, 0, 2, true));
    EXPECT_TRUE(field.set_ship(2, 4, 2, true));
    EXPECT_TRUE(field.set_ship(2, 7, 2, true));
    EXPECT_TRUE(field.set_ship(2, 0, 4, true));
    EXPECT_TRUE(field.set_ship(1, 3, 4, true));
    EXPECT_TRUE(field.set_ship(1, 5, 4, true));
    EXPECT_TRUE(field.set_ship(1, 7, 4, true));
    EXPECT_TRUE(field.set_ship(1, 9, 4, true));
    
    EXPECT_FALSE(field.set_ship(1, 0, 6, true));
}

TEST(GameFieldTest, PlaceShipOutOfBounds) {
    GameField field;
    EXPECT_FALSE(field.set_ship(4, 8, 0, true));
    EXPECT_FALSE(field.set_ship(4, 0, 8, false));
    EXPECT_FALSE(field.set_ship(1, -1, 0, true));
    EXPECT_FALSE(field.set_ship(1, 0, -1, true));
}

TEST(GameFieldTest, PlaceShipOverlap) {
    GameField field;
    EXPECT_TRUE(field.set_ship(4, 0, 0, true));

    EXPECT_FALSE(field.set_ship(1, 0, 0, true));
    EXPECT_FALSE(field.set_ship(2, 1, 0, true));
    EXPECT_FALSE(field.set_ship(2, 4, 0, true));
}

TEST(GameFieldTest, ShotResults) {
    GameField field;
    EXPECT_TRUE(field.set_ship(1, 0, 0, true));

    EXPECT_EQ(field.check_shot(0, 0), ShotResult::HIT);
    EXPECT_EQ(field.check_shot(0, 0), ShotResult::RESHOT);

    EXPECT_EQ(field.check_shot(1, 0), ShotResult::MISS);
    EXPECT_EQ(field.check_shot(1, 0), ShotResult::RESHOT);

    EXPECT_EQ(field.check_shot(2, 0), ShotResult::MISS);
    EXPECT_EQ(field.check_shot(2, 0), ShotResult::RESHOT);

    EXPECT_EQ(field.check_shot(-1, 0), ShotResult::INVALID);
    EXPECT_EQ(field.check_shot(0, -1), ShotResult::INVALID);
    EXPECT_EQ(field.check_shot(10, 0), ShotResult::INVALID);
}

TEST(GameFieldTest, GameOver) {
    GameField field;
    EXPECT_TRUE(field.set_ship(1, 0, 0, true));
    EXPECT_TRUE(field.set_ship(1, 2, 0, true));
    EXPECT_FALSE(field.game_is_over());

    EXPECT_EQ(field.check_shot(0, 0), ShotResult::HIT);
    EXPECT_FALSE(field.game_is_over());

    EXPECT_EQ(field.check_shot(2, 0), ShotResult::HIT);
    EXPECT_TRUE(field.game_is_over());
}

TEST(GameFieldTest, ClearField) {
    GameField field;
    EXPECT_TRUE(field.set_ship(4, 0, 0, true));
    EXPECT_TRUE(field.set_ship(3, 5, 0, true));
    field.clear_field(); 

    EXPECT_TRUE(field.set_ship(4, 0, 0, true));
    EXPECT_TRUE(field.set_ship(3, 5, 0, true));
}

TEST(GameFieldTest, ShipLimit) {
    GameField field;
    for (int i = 0; i < 4; ++i) {
        EXPECT_TRUE(field.set_ship(1, i*2, 0, true));
    }
    EXPECT_FALSE(field.set_ship(1, 8, 0, true));

    GameField field2;
    EXPECT_TRUE(field2.set_ship(2, 0, 0, true));
    EXPECT_TRUE(field2.set_ship(2, 3, 0, true));
    EXPECT_TRUE(field2.set_ship(2, 6, 0, true));
    EXPECT_FALSE(field2.set_ship(2, 0, 2, true));
}

TEST(GameFieldTest, BorderCreation) {
    GameField field;
    EXPECT_TRUE(field.set_ship(2, 0, 0, true));

    EXPECT_FALSE(field.set_ship(1, 0, 1, true));
    EXPECT_FALSE(field.set_ship(1, 1, 1, true));
    EXPECT_FALSE(field.set_ship(1, 2, 0, true));

    EXPECT_FALSE(field.set_ship(1, 2, 1, true));
}
