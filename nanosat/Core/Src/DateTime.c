/*
 * DateTime.c
 *
 *  Created on: Mar 25, 2025
 *      Author: 97254
 */

#define _GNU_SOURCE
#include "DateTime.h"

#include <stdio.h>
#include <stdint.h>
#include <time.h>

static uint8_t bcdToDec(uint8_t bcd);
static uint8_t decToBcd(uint8_t dec);

void RTC_ReadDateTime(DateTime *dt) {

	if(dt == NULL){
		return;
	}

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);

    dt->year = bcdToDec(sDate.Year);
    dt->month = bcdToDec(sDate.Month);
    dt->day = bcdToDec(sDate.Date);
    dt->weekday = bcdToDec(sDate.WeekDay);

    dt->hours = bcdToDec(sTime.Hours);
    dt->minutes = bcdToDec(sTime.Minutes);
    dt->seconds = bcdToDec(sTime.Seconds);
}

void RTC_SetDateTime(DateTime *dt){

	if(dt == NULL){
		return;
	}
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sDate.Year = decToBcd(dt->year);
    sDate.Month = decToBcd(dt->month);
    sDate.Date = decToBcd(dt->day);
    sDate.WeekDay = decToBcd(dt->weekday);

    sTime.Hours = decToBcd(dt->hours);
    sTime.Minutes = decToBcd(dt->minutes);
    sTime.Seconds = decToBcd(dt->seconds);

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
}

void RTC_SetDate(DateTime *dt)
{
	if(dt == NULL){
		return;
	}

	DateTime current;

	RTC_ReadDateTime(&current);

	dt->hours = current.hours;
	dt->minutes = current.minutes;
	dt->seconds = current.seconds;

	RTC_SetDateTime(dt);
}

void RTC_SetTime(DateTime *dt)
{
	if(dt == NULL){
		return;
	}

	DateTime current;

	RTC_ReadDateTime(&current);

    dt->year = current.year;
    dt->month = current.month;
    dt->day = current.day;
    dt->weekday = current.weekday;

	RTC_SetDateTime(dt);
}

void RTC_PrintDateTime(const DateTime *dt) {
    static const char *weekdays[] = {"UNK", "MON", "TUE", "WED", "THU", "FRI", "SAT","SUN"};
    const char *weekdayStr = (dt->weekday >= 1 && dt->weekday <= 7) ? weekdays[dt->weekday] : weekdays[0];

    printf("%02d/%02d/%02d-%s-%02d:%02d:%02d\r\n", dt->year, dt->month, dt->day, weekdayStr, dt->hours, dt->minutes, dt->seconds);

}

void RTC_DateTimeToString(char *buffer,  const DateTime *dt) {
    static const char *weekdays[] = {"UNK", "MON", "TUE", "WED", "THU", "FRI", "SAT","SUN"};
    const char *weekdayStr = (dt->weekday >= 1 && dt->weekday <= 7) ? weekdays[dt->weekday] : weekdays[0];

    sprintf(buffer, "%02d-%02d-%02d_%s_%02d:%02d:%02d",
            dt->year, dt->month, dt->day, weekdayStr,
            dt->hours, dt->minutes, dt->seconds);
}

void parse_timestamp(uint32_t timestamp, DateTime *dt)
{
    // Convert timestamp to struct tm (standard C time structure)
    time_t raw_time = (time_t) timestamp;
    struct tm *time_info = gmtime(&raw_time); // Convert to UTC


    // Fill DateTime struct
    dt->year    = (uint8_t)(time_info->tm_year % 100); // Last two digits of the year
    dt->month   = (uint8_t)(time_info->tm_mon + 1);    // tm_mon is 0-based
    dt->day     = (uint8_t)(time_info->tm_mday);
    dt->hours   = (uint8_t)(time_info->tm_hour);
    dt->minutes = (uint8_t)(time_info->tm_min);
    dt->seconds = (uint8_t)(time_info->tm_sec);

    // Convert tm_wday (0 = Sunday, 6 = Saturday) to (1 = Monday, 7 = Sunday)
    dt->weekday = (time_info->tm_wday == 0) ? 7 : time_info->tm_wday;
}

uint32_t datetime_to_timestamp(DateTime const *dt) {
    struct tm time_info = {0};

    // Fill the tm structure (year offset from 1900, month is 0-based)
    time_info.tm_year = dt->year + 100;  // Convert 2-digit year to full year (2000+)
    time_info.tm_mon  = dt->month - 1;   // Convert 1-12 to 0-11
    time_info.tm_mday = dt->day;
    time_info.tm_hour = dt->hours;
    time_info.tm_min  = dt->minutes;
    time_info.tm_sec  = dt->seconds;

    // Convert to Unix timestamp (UTC)
    return (uint32_t)mktime(&time_info); // Use `timegm()` for UTC-safe conversion
}

uint32_t get_timestamp()
{
	DateTime current;
	RTC_ReadDateTime(&current);
	return datetime_to_timestamp(&current);
}

static uint8_t bcdToDec(uint8_t bcd) {
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}


static uint8_t decToBcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}


