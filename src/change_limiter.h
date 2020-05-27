#pragma once
#include <Arduino.h>

/**
 *  Change Limiter
 *
 *  maxFalling restricts the delta if delta is smaller than zero, maxRising
 *  restricts the delta if the delta is bigger than zero.
 *  *
 *  Example:
 *      maxFalling = -10, maxRising = 100, value = -34, target = 130
 *  will yield:
 *      -34, 66, 130
 */
class ChangeLimiter
{
public:
    void begin()
    {
        enabled_ = true;
        max_falling_ = 0;
        max_rising_ = 0;
        period_ = 0;
        prev_time_ = 0;
        value_ = 0;
        target_ = 0;
    }

    void set_limit(int maxChange)
    {
        set_limit(maxChange, maxChange);
    }
    void set_limit(int maxFalling, int maxRising)
    {
        max_falling_ = -abs(maxFalling);
        max_rising_ = abs(maxRising);
    }

    void set_period(unsigned int period)
    {
        period_ = period;
    }

    int value()
    {
        return value_;
    }
    void set_value(int value)
    {
        value_ = value;
    }

    bool enabled()
    {
        return enabled_;
    }
    void set_enabled(bool enabled)
    {
        enabled_ = enabled;
    }

    bool target_reached()
    {
        return value_ == target_;
    }
    void set_target(int target)
    {
        target_ = target;
    }

    int output()
    {
        return output(millis());
    }
    int output(unsigned long ms)
    {
        if (enabled_)
        {
            // take the in current time for the first iteration
            if (prev_time_ == 0)
            {
                prev_time_ = ms;
            }
            // is it time to update the value?
            if ((period_ == 0) || ((ms - prev_time_) >= period_))
            {
                prev_time_ = ms;

                int delta = target_ - value_;
                if (delta > 0 && delta > max_rising_)
                {
                    value_ += max_rising_;
                }
                else if (delta < 0 && delta < max_falling_)
                {
                    value_ += max_falling_;
                }
                else
                {
                    value_ += delta;
                }
            }
        }
        else
        {
            value_ = target_;
        }
        return value_;
    }

    bool enabled_;
    int max_falling_;
    int max_rising_;
    unsigned int period_;
    unsigned long prev_time_;
    int value_;
    int target_;
};

/**
 *  Absolute Change Limiter
 *
 *  Works like the normal change limiter, but the maxFalling limit restricts the
 *  change of the value when going towards zero. The maxRising limit restricts
 *  the change of the value towards the target value. This absolute change
 *  limiter will go down to zero and then to the target value if target and
 *  current value have different signs.
 *
 *  Example:
 *      maxFalling = 10, maxRising = 100, value = -34, target = 130
 *  will yield:
 *      -34, -24, -14, -4, 0, 100, 130
 */
class AbsChangeLimiter : public ChangeLimiter
{
public:
    void set_limit(int maxFalling, int maxRising)
    {
        max_falling_ = abs(maxFalling);
        max_rising_ = abs(maxRising);
    }

    int output(unsigned long ms)
    {
        if (enabled_)
        {
            // take the in current time for the first iteration
            if (prev_time_ == 0)
            {
                prev_time_ = ms;
            }
            // is it time to update the value?
            if ((period_ == 0) || ((ms - prev_time_) >= period_))
            {
                prev_time_ = ms;

                bool differentSigns = (sign(target_) * sign(value_) == -1);
                if (differentSigns)
                {
                    // in this case the value must make a zero pass first and
                    // is treated as falling
                    if (abs(value_) <= max_falling_)
                    {
                        value_ = 0;
                    }
                    else
                    {
                        value_ -= sign(value_) * max_falling_;
                    }
                }
                else
                {
                    // in this case we can use normal delta logic
                    int delta = abs(target_) - abs(value_);
                    int dir = sign(value_) | sign(target_);
                    if (delta > 0 && delta > max_rising_)
                    {
                        value_ += dir * max_rising_;
                    }
                    else if ((delta < 0) && (abs(delta) > max_falling_))
                    {
                        value_ -= dir * max_falling_;
                    }
                    else
                    {
                        value_ += dir * delta;
                    }
                }
            }
        }
        else
        {
            value_ = target_;
        }
        return value_;
    }
};
