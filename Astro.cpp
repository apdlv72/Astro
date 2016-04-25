

#include <Astro.h>


Astro::Astro()
{
	setLocation(LOCATION_BERLIN);
	setTimezone(TZ_CET);
	setDaylightSaving(0);
}

Astro::	Astro(float latitude, float longitude)
{
	setLocation(latitude, longitude);
	setTimezone(TZ_UTC);
	setDaylightSaving(0);
}

Astro::	Astro(float latitude, float longitude, float timezone=TZ_UTC)
{
	setLocation(latitude, longitude);
	setTimezone(timezone);
	setDaylightSaving(0);
}

Astro::	Astro(float latitude, float longitude, float timezone, int16_t year)
{
	setLocation(latitude, longitude);
	setTimezone(timezone);
	setDaylightSaving(0);
	setYear(year);
}

void Astro::setTimezone(float timezone)
{
	// range of timezones: 0,..,14]
	// 14 * 4 = 56 < 127
	mTimezone4 = timezone*4;
}

void Astro::setLocation(float latitude, float longitude)
{
	//Serial.print("setLocation: latitude="); Serial.print(latitude,5); Serial.print(", longitude="); Serial.println(longitude,5);
	mLatitudeRad = PI*latitude/180;
	mLongitude   = longitude;
	//Serial.print("setLocation: mLatitudeRad="); Serial.println(mLatitudeRad, 5);
	//Serial.print("setLocation: mLongitude="); Serial.println(mLongitude, 5);
}

void Astro::setYear(int16_t year)
{
	mYear.year    = year;
	mYear.leapDay = isLeapYear(year);
}

void Astro::setDaylightSaving(int8_t dls)
{
	mDaylight = dls;
}

uint8_t Astro::isLeapYear(int year)
{
	if (year<0) return 0;
	if (0==year%400) return 1;
	if ((0==year%4) && (0!=year%100)) return 1;
	return 0;
}

uint8_t Astro::getDaysInMonth(int month)
{
	switch (month)
	{
	case 2: return 28+mYear.leapDay;
	case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
	}
	return 0;
}

uint16_t Astro::getDayOfYear(uint8_t dom, uint8_t month)
{
	return mYear.year<0 ? -1 : getDayOfYear(dom, month, mYear.leapDay);
}

uint16_t Astro::getDayOfYear(uint8_t dom, uint8_t month, uint16_t year)
{
	return _getDayOfYear(dom, month, isLeapYear(year));
}

void Astro::dump() 
{
	Serial.print("dump: mLatitudeRad="); Serial.println(mLatitudeRad, 5);
	Serial.print("dump: mLongitude="); Serial.println(mLongitude, 5);
}

void Astro::getTimes(uint16_t dayOfYear, float & sunrise, float & sundown)
{
	float doy = dayOfYear;

	// declination of the sun = 0.4095*sin(0.016906*(30-80.086)) = -0.30677 rad = -17.58
	float declination = 0.4095f*sin(0.016906f*(doy-80.086f));
	
	float _d = 180.0*declination/PI;
	//Serial.print("declination: "); Serial.print(declination, 10); Serial.print(" / "); Serial.println(_d, 10);
	
	//Serial.print("mLatitudeRad: "); Serial.println(mLatitudeRad, 10);
	float _l = 180.0*mLatitudeRad/PI;
	//Serial.print("latitude: "); Serial.println(_l, 10);
	//Serial.print("SUN_HEIGHT: "); Serial.println(SUN_HEIGHT, 10);
	
	float tDiff = 12*acos( (sin(SUN_HEIGHT) - sin(mLatitudeRad)*sin(declination) ) / (cos(mLatitudeRad)*cos(declination))) / PI;
	//Serial.print("tDiff: "); Serial.println(tDiff, 10);

	// time equation = -0.171*sin(0.0337*30 + 0.465) - 0.1299*sin(0.01787*30 - 0.168) = -0.217 Stunden = WOZ - MOZ
	float tEquation = -0.171f * sin(0.0337f * doy + 0.465f) - 0.1299f*sin(0.01787f * doy - 0.168f);
	//Serial.print("tEquation: "); Serial.println(tEquation, 10);

	float noon = 12-tEquation;
	//Serial.print("noon: "); Serial.println(noon, 10);	

	// sunrise at 12 - 4.479 = 7.521 Uhr "real" local timet.
	sunrise = noon - tDiff;
	sundown = noon + tDiff;
	//Serial.print("sunrise: "); Serial.println(sunrise, 10);
	//Serial.print("sundown: "); Serial.println(sundown, 10);

	//Serial.print("mLongitude: "); Serial.println(mLongitude);
	//Serial.print("mTimezone4: "); Serial.println(mTimezone4);
	//Serial.print("mDaylight:  "); Serial.println(mDaylight);
	
	// take into consideration the time zone:
	float deltaTZ = (-mLongitude/15.0f) + (0.25f*mTimezone4) + mDaylight;
	//Serial.print("deltaTZ: "); Serial.println(deltaTZ, 10);
	
	sunrise += deltaTZ;
	sundown += deltaTZ;
}

int Astro::toHHMM(float hour, char buff[6])
{
	int full = floor(hour);
	int mins = round((hour-full)*60);

	buff[0] = '0'+(full/10);
	buff[1] = '0'+(full%10);
	buff[2] = ':';
	buff[3] = '0'+(mins/10);
	buff[4] = '0'+(mins%10);
	buff[5] = 0;

	// return 1 if argument was valid
	return 0<=hour && hour<=24;
}


	// this function avoids re-computation of the leap day when called frequently
uint16_t Astro::_getDayOfYear(uint8_t dom, uint8_t month, uint8_t leapDay)
{
	uint16_t daysBefore = leapDay;
	switch (month)
	{
		case  1: daysBefore  = 0; break;
		case  2: daysBefore  = 31; break;
		case  3: daysBefore += 31+28; break;
		case  4: daysBefore += 31+28+31; break;
		case  5: daysBefore += 31+28+31+30; break;
		case  6: daysBefore += 31+28+31+30+31; break;
		case  7: daysBefore += 31+28+31+30+31+30; break;
		case  8: daysBefore += 31+28+31+30+31+30+31; break;
		case  9: daysBefore += 31+28+31+30+31+30+31+31; break;
		case 10: daysBefore += 31+28+31+30+31+30+31+31+30; break;
		case 11: daysBefore += 31+28+31+30+31+30+31+31+30+31; break;
		case 12: daysBefore += 31+28+31+30+31+30+31+31+30+31+30; break;
	}
	return daysBefore + dom;
}


