#include <gtest/gtest.h>
#include <cstdio>
#include "database/database_manager.hpp"

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = std::make_unique<DatabaseManager>("test_db.db");
    }

    void TearDown() override {
        db.reset();
        std::remove("test_db.db");
    }

    std::unique_ptr<DatabaseManager> db;
};

TEST_F(DatabaseTest, CreateAndAuthenticateUser) {
    auto player = db->createUser("daria", "password123");
    ASSERT_NE(player, nullptr);
    EXPECT_EQ(player->get_login(), "daria");
    EXPECT_EQ(player->get_points(), 1000);

    auto auth = db->authenticate("daria", "password123");
    ASSERT_NE(auth, nullptr);
    EXPECT_EQ(auth->get_login(), "daria");
    EXPECT_EQ(auth->get_points(), 1000);
}

TEST_F(DatabaseTest, CreateDuplicateUserFails) {
    auto p1 = db->createUser("duplicate", "pass");
    ASSERT_NE(p1, nullptr);

    auto p2 = db->createUser("duplicate", "other");
    EXPECT_EQ(p2, nullptr);
}

TEST_F(DatabaseTest, AuthenticateWrongPasswordFails) {
    db->createUser("user", "correct");
    auto auth = db->authenticate("user", "wrong");
    EXPECT_EQ(auth, nullptr);
}

TEST_F(DatabaseTest, AuthenticateNonExistentUser) {
    auto auth = db->authenticate("no_such_user", "pass");
    EXPECT_EQ(auth, nullptr);
}

TEST_F(DatabaseTest, UpdateRatings) {
    db->createUser("p1", "pass");
    db->createUser("p2", "pass");

    EXPECT_TRUE(db->updateRatings(1, 1050, 2, 950));

    auto p1 = db->authenticate("p1", "pass");
    EXPECT_EQ(p1->get_points(), 1050);
    auto p2 = db->authenticate("p2", "pass");
    EXPECT_EQ(p2->get_points(), 950);
}

TEST_F(DatabaseTest, UpdateRatingsWithNonExistentPlayers) {
    EXPECT_FALSE(db->updateRatings(999, 1100, 1000, 900));
}

TEST_F(DatabaseTest, UpdateRatingsRollbackOnFailure) {
    db->createUser("p1", "pass");
    db->createUser("p2", "pass");

    EXPECT_FALSE(db->updateRatings(1, 1100, 999, 900));

    auto p1 = db->authenticate("p1", "pass");
    EXPECT_EQ(p1->get_points(), 1000);
    auto p2 = db->authenticate("p2", "pass");
    EXPECT_EQ(p2->get_points(), 1000);
}

TEST_F(DatabaseTest, HandleEmptyCredentials) {
    auto p1 = db->createUser("", "pass");
    EXPECT_EQ(p1, nullptr);

    auto p2 = db->createUser("user", "");
    EXPECT_EQ(p2, nullptr);
}

TEST_F(DatabaseTest, HandleLongStrings) {
    std::string long_login(256, 'a');
    auto p = db->createUser(long_login, "pass");
    EXPECT_NE(p, nullptr);
    auto auth = db->authenticate(long_login, "pass");
    ASSERT_NE(auth, nullptr);
    EXPECT_EQ(auth->get_login(), long_login);
}
