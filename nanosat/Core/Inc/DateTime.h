/*
 * DateTime.h
 *
 *  Created on: Mar 25, 2025
 *      Author: 97254
 */

#ifndef INC_DATETIME_H_
#define INC_DATETIME_H_

#include "rtc.h"

typedef struct {
    uint8_t year;   // Last two digits of the year
    uint8_t month;  // 1-12
    uint8_t day;    // 1-31
    uint8_t weekday;// 1-7 (Monday-Sunday)
    uint8_t hours;  // 0-23
    uint8_t minutes;// 0-59
    uint8_t seconds;// 0-59
} DateTime;

void RTC_Init(void);

void RTC_ReadDateTime(DateTime *dt);

void RTC_SetDateTime(DateTime *dt);


void RTC_SetDate(DateTime *dt);

void RTC_SetTime(DateTime *dt);


void RTC_PrintDateTime(const DateTime *dt);
void RTC_DateTimeToString(char *buffer,  const DateTime *dt);

void parse_timestamp(uint32_t timestamp, DateTime *dt);

uint32_t datetime_to_timestamp(DateTime const *dt);

uint32_t get_timestamp();



#endif /* INC_DATETIME_H_ */
