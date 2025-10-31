# ICB Client Test Suite

## Overview

This test suite includes unit tests and integration tests for the ICB client.

## Unit Tests

Run all unit tests:
```bash
cd build
cmake ..
make
ctest
```

### Test Categories

1. **StringsTest** - Tests safe string functions (`safe_strncpy`, `safe_strncat`, `quoteify`, etc.)
2. **SafeAtoiTest** - Tests safe integer parsing with overflow protection
3. **StrlistTest** - Tests string list operations (linking, unlinking, searching)

## Integration Tests

Integration tests connect to a real ICB server to test chat commands.

### Server Configuration

The integration tests are configured to connect to:
- **Server**: `default.icb.net`
- **Port**: `7326`
- **Test Group**: `cicb_test`

These values are hardcoded in the test. To use a different server, modify `icb_integration_test.cpp`.

### Setup

1. Enable integration tests:
```bash
cd build
cmake -DBUILD_INTEGRATION_TESTS=ON ..
make
```

2. Run integration tests:
```bash
ctest -R ICBIntegrationTest --output-on-failure
```

### Available Tests

All tests automatically connect to `default.icb.net:7326` and attempt to login to group `cicb_test`:

- **ConnectToServer** - Tests basic TCP connection
- **LoginToServer** - Tests ICB login protocol
- **WhoCommand** - Tests /who command
- **VersionCommand** - Tests /version command
- **MOTDCommand** - Tests /motd command
- **SendPublicMessage** - Tests public group messages
- **ChangeGroup** - Tests group switching
- **CommandAbbreviations** - Tests multiple commands
- **PingPong** - Tests ping/pong handling
- **InvalidCommand** - Tests error handling
- **EmptyMessage** - Tests empty message handling
- **LongMessage** - Tests message length limits
- **PacketFormat** - Tests ICB packet structure
- **RapidCommands** - Tests multiple rapid commands
- **TestGroupExists** - Verifies test group availability
- **PersonalMessageStructure** - Tests personal message format
- **Reconnect** - Tests reconnection after disconnect
- **BeepCommand** - Tests beep command

**Note**: Tests will automatically skip if the server is unavailable or if login fails (e.g., if group `cicb_test` doesn't exist yet).
- **SendOpenMessage** - Tests public messages
- **PersonalMessage** - Tests private messages
- **ChangeGroup** - Tests group switching
- **ChangeNickname** - Tests nickname changes
- **VersionCommand** - Tests /version command
- **BeepCommand** - Tests beep command
- **MOTDCommand** - Tests MOTD retrieval
- **CommandAbbreviations** - Tests command abbreviations (w, h, q, etc.)

## Mock Server Tests

Mock server tests use an in-memory mock ICB server for testing without a real server.

Run mock server tests:
```bash
ctest -R ServerMockTest --output-on-failure
```

## Writing New Tests

### Adding Unit Tests

Add test cases to existing test files or create new ones:

```cpp
TEST(MyTestSuite, MyTestCase) {
    // Your test code
    EXPECT_EQ(expected, actual);
}
```

### Adding Integration Tests

1. Add test case to `icb_integration_test.cpp`
2. Use the `ICBIntegrationTest` fixture
3. Check for server availability:
```cpp
if (!getenv("ICB_TEST_SERVER")) {
    GTEST_SKIP() << "ICB_TEST_SERVER not set";
}
```

## Test Server Requirements

For integration tests, you need:
- A running ICB server accessible via network
- A test account/nickname
- Permission to join test groups
- Network connectivity

## Continuous Integration

Tests can be run in CI/CD pipelines. Integration tests will be skipped if `ICB_TEST_SERVER` is not set.

Example CI configuration:
```yaml
- name: Run unit tests
  run: |
    cd build
    cmake ..
    make
    ctest --output-on-failure

- name: Run integration tests
  if: env.ICB_TEST_SERVER != ''
  run: |
    cd build
    cmake -DBUILD_INTEGRATION_TESTS=ON ..
    make
    ctest -R IntegrationTest --output-on-failure
```

