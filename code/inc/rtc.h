
typedef struct {
    uint8_t RTC_Sec;     /* Second value - [0,59] */
    uint8_t RTC_Min;     /* Minute value - [0,59] */
    uint8_t RTC_Hour;    /* Hour value - [0,23] */
    uint8_t RTC_Mday;    /* Day of the month value - [1,31] */
        uint8_t RTC_Wday;    /* Day of week value - [0,6] */
        uint16_t RTC_Yday;    /* Day of year value - [1,365] */
    uint8_t RTC_Mon;     /* Month value - [1,12] */
    uint16_t RTC_Year;    /* Year value - [0,4095] */
} RTCTime;

DWORD get_fattime (void);
