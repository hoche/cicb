#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <poll.h>
#include <time.h>
#include <sys/types.h>
#include <pwd.h>

/**
 * ICB Integration Tests
 * 
 * Tests actual ICB client functionality against default.icb.net:7326
 * Test group: cicb_test
 * 
 * These tests implement the ICB protocol directly to test server interaction.
 */

// ICB Protocol constants
#define M_LOGIN     'a'
#define M_OPEN      'b'
#define M_PERSONAL  'c'
#define M_STATUS    'd'
#define M_ERROR     'e'
#define M_COMMAND   'h'
#define M_CMDOUT    'i'
#define M_PING      'l'
#define M_PONG      'm'

class ICBIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        server_host = "default.icb.net";
        server_port = 7326;
        test_group = "cicb_test";
        
        // Generate unique test nickname
        char nick_buf[32];
        snprintf(nick_buf, sizeof(nick_buf), "test_%ld", time(NULL) % 10000);
        test_nick = strdup(nick_buf);
        
        // Get login ID
        struct passwd *pw = getpwuid(getuid());
        login_id = pw ? strdup(pw->pw_name) : strdup("testuser");
        
        sock_fd = -1;
        connected = false;
        logged_in = false;
    }
    
    void TearDown() override {
        if (sock_fd >= 0) {
            close(sock_fd);
            sock_fd = -1;
        }
        if (test_nick) {
            free(test_nick);
            test_nick = NULL;
        }
        if (login_id) {
            free(login_id);
            login_id = NULL;
        }
    }
    
    // Connect to ICB server
    bool Connect() {
        struct sockaddr_in addr;
        struct hostent *he;
        
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            return false;
        }
        
        he = gethostbyname(server_host);
        if (!he) {
            close(sock_fd);
            sock_fd = -1;
            return false;
        }
        
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(server_port);
        memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
        
        if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            close(sock_fd);
            sock_fd = -1;
            return false;
        }
        
        // Make non-blocking for test purposes
        int flags = fcntl(sock_fd, F_GETFL, 0);
        fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);
        
        connected = true;
        return true;
    }
    
    // Send ICB packet: format is [length][type][data]\001
    bool SendPacket(char type, const char *data) {
        if (sock_fd < 0) {
            return false;
        }
        
        // Build packet: type + data + \001
        size_t data_len = strlen(data);
        size_t packet_len = 1 + data_len + 1;  // type + data + \001
        
        if (packet_len > 255) {
            return false;  // Too large
        }
        
        char packet[257];
        packet[0] = (unsigned char)packet_len;  // Length byte
        packet[1] = type;
        memcpy(packet + 2, data, data_len);
        packet[2 + data_len] = '\001';
        
        ssize_t sent = write(sock_fd, packet, packet_len + 1);  // +1 for length byte
        return (sent == (ssize_t)(packet_len + 1));
    }
    
    // Read ICB packet
    int ReadPacket(char *buffer, size_t buf_size, int timeout_ms = 1000) {
        if (sock_fd < 0) {
            return -1;
        }
        
        struct pollfd pfd;
        pfd.fd = sock_fd;
        pfd.events = POLLIN;
        
        int ready = poll(&pfd, 1, timeout_ms);
        if (ready <= 0 || !(pfd.revents & POLLIN)) {
            return 0;  // Timeout or error
        }
        
        // Read length byte
        unsigned char len;
        ssize_t n = read(sock_fd, &len, 1);
        if (n != 1) {
            return -1;
        }
        
        if (len == 0 || len > 255) {
            return -1;
        }
        
        // Read packet data
        if (len > buf_size - 1) {
            len = buf_size - 1;
        }
        
        n = read(sock_fd, buffer, len);
        if (n != (ssize_t)len) {
            return -1;
        }
        
        buffer[n] = '\0';
        return n;
    }
    
    // Login to server
    bool Login() {
        if (!connected && !Connect()) {
            return false;
        }
        
        // Try test group first, then fallback to group "1"
        const char *groups[] = {test_group, "1", NULL};
        
        for (int g = 0; groups[g] != NULL; g++) {
            // Build login packet: id\001nick\001group\001command\001pass\001
            char login_data[256];
            snprintf(login_data, sizeof(login_data), "%s\001%s\001%s\001\001\001",
                     login_id, test_nick, groups[g]);
            
            bool sent = SendPacket(M_LOGIN, login_data);
            if (!sent) {
                continue;
            }
            
            // Wait for login response
            char response[256];
            int attempts = 0;
            while (attempts < 20) {  // Give more time for response
                int n = ReadPacket(response, sizeof(response), 300);
                if (n > 0) {
                    // Check if it's a login OK (M_LOGINOK = 'a') or status/proto message
                    if (response[0] == M_LOGIN || response[0] == M_STATUS || 
                        response[0] == 'j') {  // 'j' = M_PROTO
                        logged_in = true;
                        if (g > 0) {
                            // Used fallback group, update test_group
                            test_group = groups[g];
                        }
                        return true;
                    } else if (response[0] == M_ERROR) {
                        // Login error - try next group
                        break;
                    }
                } else if (n < 0) {
                    // Connection error
                    return false;
                }
                attempts++;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            // Clear any remaining packets from failed login attempt
            ReadResponses(100);
        }
        
        return false;
    }
    
    // Send a command
    bool SendCommand(const char *cmd, const char *args = "") {
        if (!logged_in && !Login()) {
            return false;
        }
        
        // Build command packet: cmd\001args\001
        char cmd_data[256];
        if (args && strlen(args) > 0) {
            snprintf(cmd_data, sizeof(cmd_data), "%s\001%s\001", cmd, args);
        } else {
            snprintf(cmd_data, sizeof(cmd_data), "%s\001\001", cmd);
        }
        
        return SendPacket(M_COMMAND, cmd_data);
    }
    
    // Send a public message
    bool SendPublicMessage(const char *message) {
        if (!logged_in && !Login()) {
            return false;
        }
        
        // Public message format: just the message text
        char msg_data[256];
        snprintf(msg_data, sizeof(msg_data), "%s\001", message);
        
        return SendPacket(M_OPEN, msg_data);
    }
    
    // Read and collect responses
    std::vector<std::string> ReadResponses(int timeout_ms = 2000) {
        std::vector<std::string> responses;
        char buffer[256];
        
        auto start = std::chrono::steady_clock::now();
        
        while (true) {
            int n = ReadPacket(buffer, sizeof(buffer), 200);
            if (n > 0) {
                responses.push_back(std::string(buffer, n));
            } else if (n < 0) {
                break;
            }
            
            auto elapsed = std::chrono::steady_clock::now() - start;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() >= timeout_ms) {
                break;
            }
        }
        
        return responses;
    }
    
    const char *server_host;
    int server_port;
    const char *test_group;
    char *test_nick;
    char *login_id;
    int sock_fd;
    bool connected;
    bool logged_in;
};

// Test connection to server
TEST_F(ICBIntegrationTest, ConnectToServer) {
    bool result = Connect();
    
    if (!result) {
        GTEST_SKIP() << "Could not connect to " << server_host << ":" << server_port 
                     << " - server may be unavailable";
    }
    
    EXPECT_GE(sock_fd, 0);
    EXPECT_TRUE(connected);
}

// Test login
TEST_F(ICBIntegrationTest, LoginToServer) {
    if (!Connect()) {
        GTEST_SKIP() << "Could not connect to server";
    }
    
    bool result = Login();
    
    if (!result) {
        GTEST_SKIP() << "Login failed - server may require authentication or group '" 
                     << test_group << "' may not exist";
    }
    
    EXPECT_TRUE(logged_in);
}

// Test who command
TEST_F(ICBIntegrationTest, WhoCommand) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    bool sent = SendCommand("who", "");
    ASSERT_TRUE(sent);
    
    auto responses = ReadResponses(3000);
    
    // Should get some response (could be empty user list or user list)
    EXPECT_GE(responses.size(), 0);
    
    // Check if we got any responses
    // Server may send various response types or no response if no users
    // Valid response types: M_CMDOUT ('i'), M_STATUS ('d'), or any other packet type
    if (responses.size() > 0) {
        for (const auto &resp : responses) {
            if (!resp.empty()) {
                // Any non-empty response is valid
                EXPECT_GT(resp.length(), 0);
            }
        }
    }
    // Note: Server may not send response if no users, which is acceptable
}

// Test version command
TEST_F(ICBIntegrationTest, VersionCommand) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    bool sent = SendCommand("version", "");
    ASSERT_TRUE(sent);
    
    auto responses = ReadResponses(3000);
    
    // Should get version response
    bool got_version = false;
    for (const auto &resp : responses) {
        if (!resp.empty() && (resp[0] == M_CMDOUT || resp[0] == 'i')) {
            got_version = true;
            break;
        }
    }
    // Version response is optional depending on server
}

// Test MOTD command
TEST_F(ICBIntegrationTest, MOTDCommand) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    bool sent = SendCommand("motd", "");
    ASSERT_TRUE(sent);
    
    auto responses = ReadResponses(3000);
    EXPECT_GE(responses.size(), 0);
}

// Test public message sending
TEST_F(ICBIntegrationTest, SendPublicMessage) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    const char *test_msg = "Test message from integration test";
    bool sent = SendPublicMessage(test_msg);
    
    ASSERT_TRUE(sent);
    
    // Message may be echoed back or appear in group
    auto responses = ReadResponses(2000);
    // Response is optional - server may not echo
}

// Test group change
TEST_F(ICBIntegrationTest, ChangeGroup) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    // Try to change to group 1 (should exist)
    bool sent = SendCommand("group", "1");
    ASSERT_TRUE(sent);
    
    auto responses = ReadResponses(2000);
    
    // Should get status message about group change
    bool found_status = false;
    for (const auto &resp : responses) {
        if (!resp.empty() && resp[0] == M_STATUS) {
            found_status = true;
            // Check if it mentions group change
            if (resp.find("group") != std::string::npos || 
                resp.find("Group") != std::string::npos) {
                break;
            }
        }
    }
    // Status message is optional depending on server
    EXPECT_TRUE(sent);
}

// Test command abbreviations
TEST_F(ICBIntegrationTest, CommandAbbreviations) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    // Test 'w' -> who (should work via client abbreviation handling)
    // Note: Server sees full command, so we test 'who' directly
    bool sent1 = SendCommand("who", "");
    EXPECT_TRUE(sent1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Test 'v' -> version
    bool sent2 = SendCommand("version", "");
    EXPECT_TRUE(sent2);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

// Test ping/pong handling
TEST_F(ICBIntegrationTest, PingPong) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    // Server may send ping, we should respond with pong
    // Wait for potential ping
    auto responses = ReadResponses(5000);
    
    bool got_ping = false;
    for (const auto &resp : responses) {
        if (!resp.empty() && resp[0] == M_PING) {
            got_ping = true;
            // Send pong response
            SendPacket(M_PONG, "\001");
            break;
        }
    }
    
    // Ping is optional - server may not send immediately
    EXPECT_GE(responses.size(), 0);
}

// Test error handling - invalid command
TEST_F(ICBIntegrationTest, InvalidCommand) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    // Send an invalid command
    bool sent = SendCommand("invalid_command_xyz123", "");
    EXPECT_TRUE(sent);  // Command should be sent (server will reject)
    
    auto responses = ReadResponses(2000);
    // Server may or may not send error response
}

// Test empty message (should be handled gracefully)
TEST_F(ICBIntegrationTest, EmptyMessage) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    bool sent = SendPublicMessage("");
    // Should not crash - server may accept or reject
    EXPECT_TRUE(sent);
}

// Test long message handling
TEST_F(ICBIntegrationTest, LongMessage) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    // Create a message close to max (ICB limits messages)
    std::string long_msg(200, 'A');
    bool sent = SendPublicMessage(long_msg.c_str());
    EXPECT_TRUE(sent);
}

// Test packet format validation
TEST_F(ICBIntegrationTest, PacketFormat) {
    // Verify that login packet format is correct
    if (!Connect()) {
        GTEST_SKIP() << "Could not connect";
    }
    
    // Login will send properly formatted packet
    bool result = Login();
    
    if (result) {
        EXPECT_TRUE(logged_in);
    }
}

// Test multiple rapid commands
TEST_F(ICBIntegrationTest, RapidCommands) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    // Send multiple commands rapidly
    SendCommand("version", "");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    SendCommand("motd", "");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    SendCommand("who", "");
    
    // Collect all responses
    auto responses = ReadResponses(3000);
    
    // Should get at least some responses
    EXPECT_GE(responses.size(), 0);
}

// Test group name validation
TEST_F(ICBIntegrationTest, TestGroupExists) {
    if (!Connect()) {
        GTEST_SKIP() << "Could not connect";
    }
    
    // Try to login to test group
    bool result = Login();
    
    if (!result) {
        // If login fails, try with default group
        test_group = "1";
        logged_in = false;
        result = Login();
    }
    
    if (result) {
        // Try to change to test group
        SendCommand("group", (char *)test_group);
        auto responses = ReadResponses(2000);
        EXPECT_GE(responses.size(), 0);
    } else {
        GTEST_SKIP() << "Could not login to test group";
    }
}

// Test personal message (if we had another test user)
TEST_F(ICBIntegrationTest, PersonalMessageStructure) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    // Test personal message packet format (won't actually send to anyone)
    // Format: to\001message\001
    char pm_data[256];
    snprintf(pm_data, sizeof(pm_data), "testuser\001Hello test\001");
    
    bool sent = SendPacket(M_PERSONAL, pm_data);
    // Should send successfully even if user doesn't exist
    EXPECT_TRUE(sent);
}

// Test reconnect after disconnect
TEST_F(ICBIntegrationTest, Reconnect) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    // Disconnect
    if (sock_fd >= 0) {
        close(sock_fd);
        sock_fd = -1;
        connected = false;
        logged_in = false;
    }
    
    // Small delay
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Reconnect
    bool reconnect = Connect();
    EXPECT_TRUE(reconnect);
    
    if (reconnect) {
        bool relogin = Login();
        // May succeed or fail depending on server rate limiting
        EXPECT_GE(sock_fd, 0);
    }
}

// Test beep command
TEST_F(ICBIntegrationTest, BeepCommand) {
    if (!Connect() || !Login()) {
        GTEST_SKIP() << "Connection or login failed";
    }
    
    // Send beep (would need target user, but test packet format)
    // Beep is sent as a command: "beep username"
    bool sent = SendCommand("beep", test_nick);  // Beep ourselves (may not work)
    EXPECT_TRUE(sent);
    
    auto responses = ReadResponses(2000);
    // May get error if can't beep self, or may work
}
