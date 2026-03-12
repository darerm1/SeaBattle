#include <gtest/gtest.h>
#include "core/queue.hpp"
#include "core/player.hpp"

class QueueTest : public ::testing::Test {
protected:
    void SetUp() override {
        p1 = std::make_shared<Player>(1, "Alice", "");
        p2 = std::make_shared<Player>(2, "Bob", "");
        p3 = std::make_shared<Player>(3, "Charlie", "");
        p4 = std::make_shared<Player>(4, "David", "");
        
        p1->set_points(1000);
        p2->set_points(1100);
        p3->set_points(1050);
        p4->set_points(2000);
    }

    std::shared_ptr<Player> p1, p2, p3, p4;
};

TEST_F(QueueTest, Remove) {
    Queue q;
    q.push_back(p1);
    q.push_back(p2);
    q.push_back(p3);
    
    q.remove(2);
    EXPECT_EQ(q.size(), 2);
    
    auto pair = q.make_game_pair();
    ASSERT_NE(pair.first, nullptr);
    ASSERT_NE(pair.second, nullptr);
    int id1 = pair.first->get_id();
    int id2 = pair.second->get_id();
    EXPECT_TRUE((id1 == 1 && id2 == 3) || (id1 == 3 && id2 == 1));
    
    EXPECT_EQ(q.size(), 0);
}

TEST_F(QueueTest, FindOpponentExactMatch) {
    Queue q;
    q.push_back(p1); // 1000
    q.push_back(p2); // 1100
    q.push_back(p3); // 1050
    
    auto pair = q.make_game_pair();
    ASSERT_NE(pair.first, nullptr);
    ASSERT_NE(pair.second, nullptr);
    EXPECT_EQ(pair.first->get_id(), 1);
    EXPECT_EQ(pair.second->get_id(), 3);
    
    EXPECT_EQ(q.size(), 1);
    
    auto pair2 = q.make_game_pair();
    EXPECT_EQ(pair2.first, nullptr);
    EXPECT_EQ(pair2.second, nullptr);
    EXPECT_EQ(q.size(), 1);
}

TEST_F(QueueTest, FindOpponentWhenQueueContainsOnlyOne) {
    Queue q;
    q.push_back(p1);
    
    auto pair = q.make_game_pair();
    EXPECT_EQ(pair.first, nullptr);
    EXPECT_EQ(pair.second, nullptr);
    EXPECT_EQ(q.size(), 1);
}

TEST_F(QueueTest, FindOpponentEmptyQueue) {
    Queue q;
    auto pair = q.make_game_pair();
    EXPECT_EQ(pair.first, nullptr);
    EXPECT_EQ(pair.second, nullptr);
}

TEST_F(QueueTest, FindOpponentWithSameRating) {
    Queue q;
    auto p5 = std::make_shared<Player>(5, "Eve", "");
    p5->set_points(1100);
    q.push_back(p5); //1100
    q.push_back(p2); // 1100
    
    auto pair = q.make_game_pair();
    ASSERT_NE(pair.first, nullptr);
    ASSERT_NE(pair.second, nullptr);
    int id1 = pair.first->get_id();
    int id2 = pair.second->get_id();
    EXPECT_TRUE((id1 == 5 && id2 == 2) || (id1 == 2 && id2 == 5));
    EXPECT_EQ(q.size(), 0);
}

TEST_F(QueueTest, FindOpponentWithMultipleCandidates) {
    Queue q;
    auto p6 = std::make_shared<Player>(6, "Frank", "");
    p6->set_points(900);
    auto p7 = std::make_shared<Player>(7, "Grace", "");
    p7->set_points(950);
    q.push_back(p6);
    q.push_back(p7);
    q.push_back(p1); // 1000
    q.push_back(p2); // 1100
    
    auto pair = q.make_game_pair();
    ASSERT_NE(pair.first, nullptr);
    ASSERT_NE(pair.second, nullptr);
    EXPECT_EQ(pair.first->get_id(), 6);
    EXPECT_EQ(pair.second->get_id(), 7);
    
    auto pair2 = q.make_game_pair();
    ASSERT_NE(pair2.first, nullptr);
    ASSERT_NE(pair2.second, nullptr);
    EXPECT_EQ(pair2.first->get_id(), 1);
    EXPECT_EQ(pair2.second->get_id(), 2);
}
