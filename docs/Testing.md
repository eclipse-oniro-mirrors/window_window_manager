# Testing

- Test files use `*_test.cpp` suffix, named after the class under test.

### Test structure pattern

```cpp
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MyFeatureTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MyFeatureTest::SetUpTestCase() {}
void MyFeatureTest::TearDownTestCase() {}
void MyFeatureTest::SetUp() {}
void MyFeatureTest::TearDown() {}

HWTEST_F(MyFeatureTest, TestSomething, TestSize.Level1)
{
    EXPECT_EQ(result, expected);
    ASSERT_NE(ptr, nullptr);
}
```

### Running a single test binary

Each `ohos_unittest` target in `test/unittest/BUILD.gn` produces its own binary.
Target names follow the pattern `<module>_<test_name>`:
| Module | Build target |
|--------|-------------|
| wm | `wm:test` |
| wmserver | `wmserver:test` |
| dm | `dm:test` |
| dmserver | `dmserver:test` |
| dm_lite | `dm_lite:test` |
| window_scene | `window_scene:test` |
| snapshot | `snapshot:test` |
| extension/window_extension | `extension/window_extension:test` |