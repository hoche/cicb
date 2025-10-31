#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>

extern "C" {
    // Forward declare types and functions
    typedef struct strlist {
        struct strlist *next, *prev;
        char str[1];
    } STRLIST;
    
    STRLIST *strmakenode(int len);
    void strlinkhead(STRLIST *s, STRLIST **head, STRLIST **tail);
    void strlinktail(STRLIST *s, STRLIST **head, STRLIST **tail);
    void strlinkafter(STRLIST *s, STRLIST *i, STRLIST **head, STRLIST **tail);
    void strlinkbefore(STRLIST *s, STRLIST *i, STRLIST **head, STRLIST **tail);
    void strunlink(STRLIST *s, STRLIST **head, STRLIST **tail);
    STRLIST *strgetnode(char *str, STRLIST *head, int caseindep);
}

class StrlistTest : public ::testing::Test {
protected:
    void SetUp() override {
        head = NULL;
        tail = NULL;
    }
    
    void TearDown() override {
        // Clean up any remaining nodes
        while (head != NULL) {
            STRLIST *next = head->next;
            free(head);
            head = next;
        }
        head = NULL;
        tail = NULL;
    }
    
    STRLIST *head;
    STRLIST *tail;
};

TEST_F(StrlistTest, MakeNode) {
    STRLIST *node = strmakenode(10);
    
    ASSERT_NE(nullptr, node);
    EXPECT_EQ(nullptr, node->next);
    EXPECT_EQ(nullptr, node->prev);
    EXPECT_STREQ("", node->str);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, MakeNodeZeroSize) {
    STRLIST *node = strmakenode(0);
    
    ASSERT_NE(nullptr, node);
    EXPECT_STREQ("", node->str);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, MakeNodeLargeSize) {
    STRLIST *node = strmakenode(100);
    
    ASSERT_NE(nullptr, node);
    // Verify we can write to the buffer
    strcpy(node->str, "test");
    EXPECT_STREQ("test", node->str);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, LinkHeadToEmptyList) {
    STRLIST *node = strmakenode(10);
    strcpy(node->str, "first");
    
    strlinkhead(node, &head, &tail);
    
    EXPECT_EQ(node, head);
    EXPECT_EQ(node, tail);
    EXPECT_EQ(nullptr, node->next);
    EXPECT_EQ(nullptr, node->prev);
    
    // Don't free here - TearDown will handle it
}

TEST_F(StrlistTest, LinkTailToEmptyList) {
    STRLIST *node = strmakenode(10);
    strcpy(node->str, "first");
    
    strlinktail(node, &head, &tail);
    
    EXPECT_EQ(node, head);
    EXPECT_EQ(node, tail);
    EXPECT_EQ(nullptr, node->next);
    EXPECT_EQ(nullptr, node->prev);
    
    // Don't free here - TearDown will handle it
}

TEST_F(StrlistTest, LinkMultipleNodesHead) {
    STRLIST *node1 = strmakenode(10);
    STRLIST *node2 = strmakenode(10);
    STRLIST *node3 = strmakenode(10);
    
    strcpy(node1->str, "first");
    strcpy(node2->str, "second");
    strcpy(node3->str, "third");
    
    strlinkhead(node1, &head, &tail);
    strlinkhead(node2, &head, &tail);
    strlinkhead(node3, &head, &tail);
    
    EXPECT_EQ(node3, head);
    EXPECT_EQ(node1, tail);
    EXPECT_EQ(node2, node3->next);
    EXPECT_EQ(node1, node2->next);
    EXPECT_EQ(nullptr, node1->next);
    EXPECT_EQ(nullptr, node3->prev);
    EXPECT_EQ(node3, node2->prev);
    EXPECT_EQ(node2, node1->prev);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, LinkMultipleNodesTail) {
    STRLIST *node1 = strmakenode(10);
    STRLIST *node2 = strmakenode(10);
    STRLIST *node3 = strmakenode(10);
    
    strcpy(node1->str, "first");
    strcpy(node2->str, "second");
    strcpy(node3->str, "third");
    
    strlinktail(node1, &head, &tail);
    strlinktail(node2, &head, &tail);
    strlinktail(node3, &head, &tail);
    
    EXPECT_EQ(node1, head);
    EXPECT_EQ(node3, tail);
    EXPECT_EQ(node2, node1->next);
    EXPECT_EQ(node3, node2->next);
    EXPECT_EQ(nullptr, node3->next);
    EXPECT_EQ(nullptr, node1->prev);
    EXPECT_EQ(node1, node2->prev);
    EXPECT_EQ(node2, node3->prev);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, LinkAfter) {
    STRLIST *node1 = strmakenode(10);
    STRLIST *node2 = strmakenode(10);
    STRLIST *node3 = strmakenode(10);
    
    strlinktail(node1, &head, &tail);
    strlinktail(node2, &head, &tail);
    
    // Insert node3 after node1
    strlinkafter(node3, node1, &head, &tail);
    
    EXPECT_EQ(node1, head);
    EXPECT_EQ(node2, tail);
    EXPECT_EQ(node3, node1->next);
    EXPECT_EQ(node2, node3->next);
    EXPECT_EQ(node1, node3->prev);
    EXPECT_EQ(node3, node2->prev);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, LinkAfterLast) {
    STRLIST *node1 = strmakenode(10);
    STRLIST *node2 = strmakenode(10);
    
    strlinktail(node1, &head, &tail);
    
    // Insert after tail
    strlinkafter(node2, node1, &head, &tail);
    
    EXPECT_EQ(node1, head);
    EXPECT_EQ(node2, tail);
    EXPECT_EQ(node2, node1->next);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, LinkBefore) {
    STRLIST *node1 = strmakenode(10);
    STRLIST *node2 = strmakenode(10);
    STRLIST *node3 = strmakenode(10);
    
    strlinktail(node1, &head, &tail);
    strlinktail(node2, &head, &tail);
    
    // Insert node3 before node2
    strlinkbefore(node3, node2, &head, &tail);
    
    EXPECT_EQ(node1, head);
    EXPECT_EQ(node2, tail);
    EXPECT_EQ(node3, node1->next);
    EXPECT_EQ(node2, node3->next);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, UnlinkHead) {
    STRLIST *node1 = strmakenode(10);
    STRLIST *node2 = strmakenode(10);
    
    strlinktail(node1, &head, &tail);
    strlinktail(node2, &head, &tail);
    
    strunlink(node1, &head, &tail);
    
    EXPECT_EQ(node2, head);
    EXPECT_EQ(node2, tail);
    EXPECT_EQ(nullptr, node2->prev);
    EXPECT_EQ(nullptr, node2->next);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, UnlinkTail) {
    STRLIST *node1 = strmakenode(10);
    STRLIST *node2 = strmakenode(10);
    
    strlinktail(node1, &head, &tail);
    strlinktail(node2, &head, &tail);
    
    strunlink(node2, &head, &tail);
    
    EXPECT_EQ(node1, head);
    EXPECT_EQ(node1, tail);
    EXPECT_EQ(nullptr, node1->prev);
    EXPECT_EQ(nullptr, node1->next);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, UnlinkMiddle) {
    STRLIST *node1 = strmakenode(10);
    STRLIST *node2 = strmakenode(10);
    STRLIST *node3 = strmakenode(10);
    
    strlinktail(node1, &head, &tail);
    strlinktail(node2, &head, &tail);
    strlinktail(node3, &head, &tail);
    
    strunlink(node2, &head, &tail);
    
    EXPECT_EQ(node1, head);
    EXPECT_EQ(node3, tail);
    EXPECT_EQ(node3, node1->next);
    EXPECT_EQ(node1, node3->prev);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, UnlinkSingleNode) {
    STRLIST *node = strmakenode(10);
    
    strlinktail(node, &head, &tail);
    strunlink(node, &head, &tail);
    
    EXPECT_EQ(nullptr, head);
    EXPECT_EQ(nullptr, tail);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, GetNodeExactMatch) {
    STRLIST *node1 = strmakenode(10);
    STRLIST *node2 = strmakenode(10);
    STRLIST *node3 = strmakenode(10);
    
    strcpy(node1->str, "first");
    strcpy(node2->str, "second");
    strcpy(node3->str, "third");
    
    strlinktail(node1, &head, &tail);
    strlinktail(node2, &head, &tail);
    strlinktail(node3, &head, &tail);
    
    char search[] = "second";
    STRLIST *found = strgetnode(search, head, 0);
    
    EXPECT_EQ(node2, found);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, GetNodeCaseIndependent) {
    STRLIST *node1 = strmakenode(10);
    STRLIST *node2 = strmakenode(10);
    
    strcpy(node1->str, "Hello");
    strcpy(node2->str, "World");
    
    strlinktail(node1, &head, &tail);
    strlinktail(node2, &head, &tail);
    
    char search[] = "hello";
    STRLIST *found = strgetnode(search, head, 1);  // case independent
    
    EXPECT_EQ(node1, found);
    
    // TearDown will free all nodes
}

TEST_F(StrlistTest, GetNodeNotFound) {
    STRLIST *node = strmakenode(10);
    strcpy(node->str, "test");
    
    strlinktail(node, &head, &tail);
    
    char search[] = "notfound";
    STRLIST *found = strgetnode(search, head, 0);
    
    EXPECT_EQ(nullptr, found);
    
    // TearDown will free all nodes
}

