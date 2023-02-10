//
// Created by zr on 23-2-10.
//

#ifndef EV_TIMESTAMP_H
#define EV_TIMESTAMP_H
#include <ctime>
#include <string>
#include <vector>

namespace ev
{
    class Timestamp
    {
    public:
        typedef enum TimeUnit
        {
            second,
            minute,
            hour,
            day,
            month,
            year
        } TimeUnit;

        Timestamp();
        explicit Timestamp(int64_t time);

        void addTime(int value, Timestamp::TimeUnit unit);
        void addMicroSeconds(int64_t ms);
        [[nodiscard]] std::string toFormattedString() const;
        [[nodiscard]] bool isValid() const;

        static Timestamp now();
        static Timestamp fromUnix(time_t time);
        [[nodiscard]] time_t toUnix() const;

        const static int microSecondsPerSecond = 1000 * 1000;

    private:
        int64_t microSecondsSinceEpoch;
    };

    bool _isvalid_tm_field(time_t value, Timestamp::TimeUnit unit);

}

#endif //EV_TIMESTAMP_H
