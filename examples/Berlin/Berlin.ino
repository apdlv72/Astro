
#include <Astro.h>

const static int YEAR = 2013;

Astro astro(LOCATION_BERLIN, TZ_CET, YEAR);

void setup()
{
	Serial.begin(9600);
}


void loop()
{
	float sr, sd;
	char sunrise[6];
	char sundown[6];

	for (int month=1; month<=12; month++)
	{
		int days = astro.getDaysInMonth(month);

		for (int dom=1; dom<=days; dom++)
		{		
			int doy = astro.getDayOfYear(dom, month);

			astro.getTimes(doy, sr, sd);
			astro.toHHMM(sr, sunrise);
			astro.toHHMM(sd, sundown);

			Serial.print(doy);   Serial.print(". day: "); 
			Serial.print(month); Serial.print("/"); Serial.print(dom); 
			Serial.print(": sunrise at "); Serial.print(sunrise);  
			Serial.print(", sundown at "); Serial.print(sundown);
			Serial.println();
		}
	}

	delay(10*1000);
}

