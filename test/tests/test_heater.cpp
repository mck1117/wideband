#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test_helpers.h"

#include "heater_control.h"

namespace
{

    constexpr float kHeaterPreheatTime = 5.0f;
    constexpr float kHeaterWarmupTimeout = 10.0f;

    constexpr float kTargetTempC = 780.0f;
    constexpr float kTargetEsr = 300.0f;

    constexpr float kOverTempC = over(kTargetTempC + 100.0f);
    constexpr float kUnderTempC = under(kTargetTempC - 100.0f);

    constexpr float kGoodVoltage = 12.0f;

    constexpr float kOffVoltage = 8.0f;
    constexpr float kOnVoltage = 9.5f;

    constexpr float kVoltageStabDelaySec = 0.5f;
    constexpr float kClosedLoopFaultDelaySec = 0.5f;

    struct MockHeater : public HeaterControllerBase
    {
        MockHeater() : HeaterControllerBase(0, kHeaterPreheatTime, kHeaterWarmupTimeout) { }

        MOCK_METHOD(void, SetDuty, (float), (const, override));
    };

    void configureDut(MockHeater& dut)
    {
        Timer::setMockTime(0);
        dut.Configure(kTargetTempC, kTargetEsr);
    }

}


// ====================
// GetVoltageForState
// ====================

TEST(GetVoltageForState, Preheat_ConstantVoltage)
{
    MockHeater dut;

    // Shouldn't depend upon sensor ESR
    EXPECT_EQ(2.0f, dut.GetVoltageForState(HeaterState::Preheat, 0));
    EXPECT_EQ(2.0f, dut.GetVoltageForState(HeaterState::Preheat, 300));
    EXPECT_EQ(2.0f, dut.GetVoltageForState(HeaterState::Preheat, 1000));
}

TEST(GetVoltageForState, WarmupRamp_MonotonicIncreaseAndClamp)
{
    constexpr float rampClampVoltage = 12.0f;

    MockHeater dut;

    float last = dut.GetVoltageForState(HeaterState::WarmupRamp, 300);
    EXPECT_GT(last, 0.0f);

    for (int i = 0; i < 1000; i++) {
        float next = dut.GetVoltageForState(HeaterState::WarmupRamp, 300);
        EXPECT_GE(next, last);

        // issue: voltage for state is not clamped in GetVoltageForState, instead it is clamped in Update.
        // To maintain clear separation of concerns (for maintainability reasons),
        // all voltage-related logic should be in GetVoltageForState,
        // and Update should just call SetDuty with the final duty.
        EXPECT_LE(next, rampClampVoltage + 0.05f);
        last = next;
    }

    EXPECT_NEAR(rampClampVoltage, last, 0.05f);
}

TEST(GetVoltageForState, ClosedLoop_DirectionalityAroundTargetEsr)
{
    MockHeater dut;
    configureDut(dut);

    // At target -> zero output but with 7.5v offset
    EXPECT_EQ(dut.GetVoltageForState(HeaterState::ClosedLoop, kTargetEsr), 7.5f);

    // Below target -> more voltage
    EXPECT_GT(dut.GetVoltageForState(HeaterState::ClosedLoop, kTargetEsr + 100), 7.5f);

    // Above target -> less voltage
    EXPECT_LT(dut.GetVoltageForState(HeaterState::ClosedLoop, kTargetEsr - 100), 7.5f);
}

TEST(GetVoltageForState, ClosedLoop_MaxVoltageClamp)
{
    MockHeater dut;
    configureDut(dut);

    // Huge ESR error will command high voltage
    // issue: voltage for state is not clamped to max value (12.0f) in GetVoltageForState
    EXPECT_GT(dut.GetVoltageForState(HeaterState::ClosedLoop, 10000.0f), 12.0f);
}

TEST(GetVoltageForState, Stopped_ZeroVoltage)
{
    MockHeater dut;

    // Should be zero regardless of sensor ESR
    EXPECT_EQ(0.0f, dut.GetVoltageForState(HeaterState::Stopped, 0));
    EXPECT_EQ(0.0f, dut.GetVoltageForState(HeaterState::Stopped, 300));
    EXPECT_EQ(0.0f, dut.GetVoltageForState(HeaterState::Stopped, 1000));
}


// ================
// State: Preheat
// ================

TEST(HeaterStateMachine, PreheatToWarmup_ByTimeout_WhenAllowed)
{
    MockHeater dut;
    configureDut(dut);

    // start in preheat
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));

    // Preheat timer not expired, stay in preheat
    Timer::setMockTime(toUs(under(kHeaterPreheatTime)));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));

    // Timer expired, transition to warmup ramp
    Timer::setMockTime(toUs(over(kHeaterPreheatTime)));
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));
}

TEST(HeaterStateMachine, PreheatToWarmup_ByTemperature_WhenAllowed)
{
    MockHeater dut;
    configureDut(dut);

    // Preheat for a little while
    for (size_t i = 0; i < 10; i++)
    {
        // Temperature below target, stay in preheat
        EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));
    }
    // Temperature at target, stay in preheat (boundary condition)
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC - 30.0f));

    // Temperature over target, transition to warmup ramp
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC - 29.0f));
    
    // issue: test that ramp voltage is reset when transitioning from preheat to warmup ramp

    // TODO: HEATER_FAST_HEATING_THRESHOLD_T test
}

TEST(HeaterStateMachine, PreheatStays_WhenNotAllowed)
{
    MockHeater dut;
    configureDut(dut);

    // starts in preheat
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::NotAllowed, kGoodVoltage, kUnderTempC));

    // timer almost expired but still should stay in preheat
    Timer::setMockTime(toUs(under(kHeaterPreheatTime)));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::NotAllowed, kGoodVoltage, kUnderTempC));

    // timer expired but still should stay in preheat
    Timer::setMockTime(toUs(2*kHeaterPreheatTime));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::NotAllowed, kGoodVoltage, kUnderTempC));

    // even if temperature is on target, should stay in preheat
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::NotAllowed, kGoodVoltage, kTargetTempC));
}

TEST(HeaterStateMachine, PreheatToWarmup_ByVoltageStabilization_WhenUnknown)
{
    MockHeater dut;
    configureDut(dut);

    // voltage below off voltage, stay in preheat, reset stabilization and preheat timers
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, under(kOffVoltage), kUnderTempC));

    // voltage above OnVoltage, but stabilization time not elapsed, stay in preheat, reset preheat timer
    Timer::advanceMockTime(toUs(under(kVoltageStabDelaySec)));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, over(kOnVoltage), kUnderTempC));

    // voltage above OnVoltage and stabilization time elapsed, transition to the actual preheat handler
    Timer::advanceMockTime(toUs(2*kValueDelta));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, over(kOnVoltage), kUnderTempC));

    // preheat timer elapsed, transition to warmup ramp
    Timer::advanceMockTime(toUs(over(kHeaterPreheatTime)));
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, over(kOnVoltage), kUnderTempC));
}

TEST(HeaterStateMachine, PreheatToWarmup_ByTemperature_WhenUnknown)
{
    MockHeater dut;
    configureDut(dut);

    // voltage below off voltage, stay in preheat, reset stabilization and preheat timers
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, under(kOffVoltage), kUnderTempC));

    // voltage above OnVoltage, but stabilization time not elapsed, stay in preheat, reset preheat timer
    Timer::advanceMockTime(toUs(under(kVoltageStabDelaySec)));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, over(kOnVoltage), kUnderTempC));

    // voltage above OnVoltage and stabilization time elapsed,
    // transition to the actual preheat handler, and to WarmupRamp
    Timer::advanceMockTime(toUs(2*kValueDelta));
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, over(kOnVoltage), kTargetTempC));
}

TEST(HeaterStateMachine, PreheatToWarmupByOnVoltage_AfterLowVoltageDip_WhenUnknown)
{
    MockHeater dut;
    configureDut(dut);

    // voltage below off voltage, stay in preheat, reset preheat timer
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, under(kOffVoltage), kUnderTempC));

    // half of preheat timeout expired, voltage is still below on voltage, so stay in preheat
    Timer::advanceMockTime(toUs(kHeaterPreheatTime / 2));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, under(kOnVoltage), kUnderTempC));
    
    // voltage dipped below off voltage, reset preheat timer
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, under(kOffVoltage), kUnderTempC));
    
    // rest of original voltage stability delay expired, but timer was reset by voltage dip, so stay in preheat
    Timer::advanceMockTime(toUs(over(kHeaterPreheatTime/2)));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, over(kOnVoltage), kUnderTempC));

    // preheat delay expired, transition to warmup ramp
    Timer::advanceMockTime(toUs(kHeaterPreheatTime/2));
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Unknown, over(kOnVoltage), kUnderTempC));
}

TEST(HeaterStateMachine, Preheat_WhenAllowedAndBelowOffVoltage)
{
    MockHeater dut;
    configureDut(dut);

    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));

    // Preheat timer expired, but voltage is below off voltage
    Timer::setMockTime(toUs(over(kHeaterPreheatTime)));
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));
}


// ===================
// State: WarmupRamp
// ===================

TEST(HeaterStateMachine, WarmupStays_WhenAllowed_AndNoExitCondition)
{
    MockHeater dut;
    configureDut(dut);

    // loop is largely unnecessary
    for (size_t i = 0; i < 10; i++)
    {
        EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));
    }
}

TEST(HeaterStateMachine, WarmupToClosedLoop_ByTemperatureReached_WhenAllowed)
{
    MockHeater dut;
    configureDut(dut);

    // start in warmup ramp
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));

    // transition to closed loop when temperature reaches target
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));
}

TEST(HeaterStateMachine, WarmupToStoppedAfterTimeout_WhenAllowed)
{
    MockHeater dut;
    configureDut(dut);
    
    // start in warmup ramp
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));

    // just before timeout, stay in warmup ramp
    Timer::setMockTime(toUs(under(kHeaterWarmupTimeout)));
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));

    // after timeout, transition to stopped
    Timer::setMockTime(toUs(over(kHeaterWarmupTimeout)));
    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));
}

TEST(HeaterStateMachine, WarmupToPreheat_WhenNotAllowed)
{
    MockHeater dut;
    configureDut(dut);

    // transition to preheat immediately when NotAllowed
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::NotAllowed, kGoodVoltage, kUnderTempC));

    // even if temperature is on target, transition to preheat
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::NotAllowed, kGoodVoltage, kTargetTempC));

    // even if timer is expired, transition to preheat
    Timer::setMockTime(toUs(over(kHeaterWarmupTimeout)));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::NotAllowed, kGoodVoltage, kUnderTempC));
}

TEST(HeaterStateMachine, Warmup_IfNoExitCondition_AndUnknown)
{
    MockHeater dut;
    configureDut(dut);

    // voltage above on voltage, stabilization time elapsed
    Timer::setMockTime(toUs(over(kVoltageStabDelaySec)));
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Unknown, over(kOnVoltage), kUnderTempC));

    // issue: Voltage momentarily drops below OnVoltage because heater turned on
    // this resets the stability timer and drops us back to the preheat state, even though it is a transient spike
    // while it may be the indication of weak wiring or bad connection, we at least should report the condition
    // especially since this is only in the Unknown state. Allowed state does not check for that at all.
    // There should be two modifications to fix this issue:
    // 1. Introduce hysteresis by checking OffVoltage for dropping back to preheat, and OnVoltage for transitioning to warmup ramp
    // 2. Use a timer not only for transitioning from preheat to warmup ramp, but also for dropping back to preheat
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Unknown, under(kOnVoltage), kUnderTempC));

    // Also check ClosedLoop state
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Unknown, under(kOnVoltage), kUnderTempC));
}


// ====================
// State: ClosedLoop
// ====================

TEST(HeaterStateMachine, ClosedLoop_StaysClosedLoopAtNominalTemperature)
{
    MockHeater dut;
    configureDut(dut);

    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));
    Timer::setMockTime(toUs(10.0f));
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));
}

TEST(HeaterStateMachine, ClosedLoop_ToStoppedByOverheat)
{
    MockHeater dut;
    configureDut(dut);

    // start in closed loop at target temperature
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));

    // temperature above overheat threshold, should not trip immediately
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kOverTempC));

    // and also should not trip if fault delay time has not yet elapsed
    Timer::setMockTime(toUs(under(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kOverTempC));

    // but should trip if fault delay time has elapsed
    Timer::setMockTime(toUs(over(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kOverTempC));
}

TEST(HeaterStateMachine, ClosedLoop_ToStoppedByUnderheat)
{
    MockHeater dut;
    configureDut(dut);

    // start in closed loop
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));

    // temperature below underheat threshold, should not trip immediately
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));

    // and also should not trip if fault delay time has not yet elapsed
    Timer::setMockTime(toUs(under(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));

    // but should trip if fault delay time has elapsed
    Timer::setMockTime(toUs(over(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));
}

TEST(HeaterStateMachine, ClosedLoop_NoUnderheatFaultBeforeStabilization)
{
    MockHeater dut;
    configureDut(dut);

    // transition to closed loop
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));

    // do not allow underheat fault until stabilization time has passed, to account for transient conditions during closed loop startup

    // issue: underheat_timer is not reset (either during transition to closed loop, or initially by Configure())
    // so it is started on undefined value and may immediately expire when we transition to closed loop
    // as we may go from ramp max of up to 12V to closed loop voltage of 7.5V, producing a transient temperature drop until pid control catches up,
    // which would cause an immediate transition to stopped state
    Timer::setMockTime(toUs(under(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));

    // after stabilization time, underheat fault should be allowed to trip
    Timer::setMockTime(toUs(over(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));
}

TEST(HeaterStateMachine, ClosedLoop_NoOverheatFaultBeforeStabilization)
{
    MockHeater dut;
    configureDut(dut);

    // transition to closed loop
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));

    // do not allow overheat fault until stabilization time has passed, to account for transient conditions during closed loop startup
    // issue: same as underheat case, but possible overshoot from ramp
    Timer::setMockTime(toUs(under(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kOverTempC));

    // after stabilization time, overheat fault should be allowed to trip
    Timer::setMockTime(toUs(over(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kOverTempC));
}

TEST(HeaterStateMachine, ClosedLoop_RecoveryToNormalTempResetsOverheatTimer)
{
    MockHeater dut;
    configureDut(dut);

    // start in closed loop at target temperature, timers should be reset at this point
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));

    // temperature exceeds overheat threshold, should not trip immediately
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kOverTempC));

    // wait almost the fault delay time, then recover back to normal temperature
    Timer::setMockTime(toUs(under(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));

    // go over fault delay time, should not trip because timer should have been reset
    Timer::setMockTime(toUs(over(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kOverTempC));
}

TEST(HeaterStateMachine, ClosedLoop_RecoveryToNormalTempResetsUnderheatTimer)
{
    MockHeater dut;
    configureDut(dut);

    // start in closed loop at target temperature, timers should be reset at this point
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));

    // temperature drops below underheat threshold, should not trip immediately
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));

    // wait almost the fault delay time, then recover back to normal temperature
    Timer::setMockTime(toUs(under(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));

    // go over fault delay time, should not trip because timer should have been reset
    Timer::setMockTime(toUs(over(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));
}

TEST(HeaterStateMachine, ClosedLoopToPreheat_WhenNotAllowed)
{
    MockHeater dut;
    configureDut(dut);

    // go from closed loop to preheat immediately when not allowed
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::NotAllowed, kGoodVoltage, kTargetTempC));

    // test that over/underheat faults are not triggered, because state transitions before those checks

    // start in closed loop at target temperature, timers should be reset at this point
    Timer::setMockTime(0);
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));

    // start overheat condition
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::NotAllowed, kGoodVoltage, kOverTempC));
    Timer::setMockTime(toUs(over(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::NotAllowed, kGoodVoltage, kOverTempC));


    // start in closed loop at target temperature, timers should be reset at this point
    Timer::setMockTime(0);
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));

    // start underheat condition
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, kGoodVoltage, kUnderTempC));
    Timer::setMockTime(toUs(over(kClosedLoopFaultDelaySec)));
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::NotAllowed, kGoodVoltage, kUnderTempC));
}

// Issue: Need to repeat all of the tests for the Unknown case as well, to check for any unintended interactions with the voltage stabilization logic.
// It will be better to move the Unknown handinling logic into a separate function to make it easier to test in isolation, and to avoid unintended interactions with the main state machine logic.
// Because rest of the state machine logic already has two distinct paths for allowed/not allowed

// ====================
// State: Stopped
// ====================

TEST(HeaterStateMachine, Stopped_WhenAllowed)
{
    MockHeater dut;
    configureDut(dut);

    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::Stopped, HeaterAllow::Allowed, kGoodVoltage, kTargetTempC));
}

TEST(HeaterStateMachine, StoppedToPreheat_WhenNotAllowed)
{
    MockHeater dut;
    configureDut(dut);

    // issue: once in stopped state, but heater allow isn't Allowed, state immediately transitions back to Preheat
    // Is it intended behavior?
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Stopped, HeaterAllow::NotAllowed, kGoodVoltage, kTargetTempC));
}

TEST(HeaterStateMachine, StoppedToPreheat_WhenUnknown)
{
    MockHeater dut;
    configureDut(dut);

    // issue: same here
    EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Stopped, HeaterAllow::Unknown, kGoodVoltage, kTargetTempC));
}
