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

TEST_F(QueueTest, PushAndPopFront) {
    Queue q;
    q.push_back(p1);
    q.push_back(p2);
    EXPECT_EQ(q.size(), 2);
    
    auto front = q.pop_front();
    EXPECT_EQ(front->get_id(), 1);
    EXPECT_EQ(q.size(), 1);
    
    front = q.pop_front();
    EXPECT_EQ(front->get_id(), 2);
    EXPECT_TRUE(q.is_empty());
}

TEST_F(QueueTest, Remove) {
    Queue q;
    q.push_back(p1);
    q.push_back(p2);
    q.push_back(p3);
    
    q.remove(2);
    EXPECT_EQ(q.size(), 2);
    
    auto front = q.pop_front();
    EXPECT_EQ(front->get_id(), 1);
    front = q.pop_front();
    EXPECT_EQ(front->get_id(), 3);
}

TEST_F(QueueTest, FindOpponentExactMatch) {
    Queue q;
    q.push_back(p1); // 1000
    q.push_back(p2); // 1100
    q.push_back(p3); // 1050
    
    auto opponent = q.find_opponent(p2);
    ASSERT_NE(opponent, nullptr);
    EXPECT_EQ(opponent->get_id(), 3);
    EXPECT_EQ(q.size(), 2);
}

TEST_F(QueueTest, FindOpponentWhenQueueContainsOnlyOne) {
    Queue q;
    q.push_back(p1);
    
    auto opponent = q.find_opponent(p2);
    ASSERT_NE(opponent, nullptr);
    EXPECT_EQ(opponent->get_id(), 1);
    EXPECT_EQ(q.size(), 0);
}

TEST_F(QueueTest, FindOpponentEmptyQueue) {
    Queue q;
    auto opponent = q.find_opponent(p1);
    EXPECT_EQ(opponent, nullptr);
}

TEST_F(QueueTest, FindOpponentWithSameRating) {
    Queue q;
    auto p5 = std::make_shared<Player>(5, "Eve", "");
    p5->set_points(1100);
    q.push_back(p5); //1100
    q.push_back(p2); // 1100
    
    auto opponent = q.find_opponent(p2);
    ASSERT_NE(opponent, nullptr);
    EXPECT_EQ(opponent->get_id(), 5);
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
    
    auto opponent = q.find_opponent(p1); // 1000
    ASSERT_NE(opponent, nullptr);
    EXPECT_EQ(opponent->get_id(), 7); // 950
}
