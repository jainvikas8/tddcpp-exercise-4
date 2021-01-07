#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::StartsWith;
using ::testing::ContainsRegex;

#include "IDisplay.h"
#include "ds1820.h"
#include "temperature_sensor.h"

class Mock_Display : public Display::IDisplay 
{
  public:
    MOCK_METHOD1(display, int (const char* str));
};

class Mock_tests : public ::testing::Test 
{
  protected:
    Mock_tests()          = default;
    virtual ~Mock_tests() = default;

    Sensor::Ds1820 sensor{};
    StrictMock<Mock_Display> fake_display{};
};

TEST_F(Mock_tests, test_initialisation)
{
  Temperature_sensor test_obj{ fake_display, sensor };

  EXPECT_CALL(fake_display, display(_)).Times(2);
  auto status = test_obj.initialize();
  
  ASSERT_EQ(status, Temperature_sensor::status::ok);
}

TEST_F(Mock_tests, test_init_display_1st_failure)
{
  Temperature_sensor test_obj{ fake_display, sensor };

  EXPECT_CALL(fake_display, display(_)).WillOnce(Return(-1));

  auto status = test_obj.initialize();
  ASSERT_EQ(status, Temperature_sensor::status::display_failure);
}

TEST_F(Mock_tests, test_init_display_2nd_failure)
{
  ::testing::InSequence strict_sequence; 

  Temperature_sensor test_obj{ fake_display, sensor };

  EXPECT_CALL(fake_display, display(_)).WillOnce(Return(0));
  EXPECT_CALL(fake_display, display(_)).WillOnce(Return(-1));

  auto status = test_obj.initialize();
  ASSERT_EQ(status, Temperature_sensor::status::display_failure);
}

TEST_F(Mock_tests, test_run_display_no_failure)
{
  Temperature_sensor test_obj{ fake_display, sensor };

  EXPECT_CALL(fake_display, display(_)).Times(3).WillRepeatedly(Return(0));

  test_obj.initialize();
  auto status = test_obj.run();
  ASSERT_EQ(status, Temperature_sensor::status::ok);
}

TEST_F(Mock_tests, test_init_display_3nd_failure)
{
  ::testing::InSequence strict_sequence; 

  Temperature_sensor test_obj{ fake_display, sensor };
  
  EXPECT_CALL(fake_display, display(_)).WillOnce(Return(0));
  EXPECT_CALL(fake_display, display(_)).WillOnce(Return(0));
  EXPECT_CALL(fake_display, display(_)).WillOnce(Return(-1));

  test_obj.initialize();
  auto status = test_obj.run();
  ASSERT_EQ(status, Temperature_sensor::status::display_failure);
}

TEST_F(Mock_tests, test_init_display_param_check)
{
  ::testing::InSequence strict_sequence; 

  Temperature_sensor test_obj{ fake_display, sensor };

  EXPECT_CALL(fake_display, display(StartsWith("Family code: 0x"))).WillOnce(Return(0));
  EXPECT_CALL(fake_display, display(StartsWith("Serial Number:"))).WillOnce(Return(0));
  EXPECT_CALL(fake_display, display(ContainsRegex("..\\...C"))).WillOnce(Return(0));

  test_obj.initialize();
  auto status = test_obj.run();
  ASSERT_EQ(status, Temperature_sensor::status::ok);
}
