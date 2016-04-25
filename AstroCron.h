#ifndef AstroCron_h
#define AstroCron_h

#include <Timezone.h>

class AstroCron {

  public:

    AstroCron(boolean D=false) {
        _D        =  D;
        lastDow  = -1;
        last_hhmm = -1;
        sunrise   = -1;
        sundown   = -1;
        timezone  = NULL;
    }

    void setTimezone(Timezone * tz) {
        timezone  = tz;
    }

    String getSunriseHHMM() {
        return hhmmToStr(sunrise);
    }

    String getSundownHHMM() {
        return hhmmToStr(sundown);
    }

    int16_t getSunrise() {
        return sunrise;
    }

    int16_t getSundown() {
        return sundown;
    }

    static int floatToHHMM(float f) {
        int minutes = f*60;
        return 100*(minutes/60)+(minutes%60);
    }

    static String hhmmToStr(int hhmm) {
        int hh = hhmm/100;
        int mm = hhmm%100;
        String rtv = "";
        if (hh<10) rtv+='0';
        rtv+=hh;
        if (mm<10) rtv+='0';
        rtv+=mm;
        return rtv;
    }

    time_t suntimeToUTC(time_t utc_now, float hhmm) {
        tmElements_t tm;
        tm.Second    = 0;
        tm.Hour    = floor(hhmm);
        tm.Minute  = floor((hhmm-tm.Hour)*60);
        tm.Day     = day(utc_now);
        tm.Month   = month(utc_now);
        tm.Year    = year(utc_now)-1970;
        time_t utc_rtv = makeTime(tm);
        return utc_rtv;
    }

    void computeSunTime(time_t now_utc, boolean D) {

        String fn = "computeSunTime: ";

        //time_t now_utc = now();
        time_t now_loc = timezone->toLocal(now_utc, &tcr);
        //if (D) { _log(fn + "local time: " + printTime(now_loc, tcr->abbrev)); }

        int d = day(now_utc);
        int m = month(now_utc);
        int y = year(now_utc);

        Astro astro;
        astro.setLocation(LOCATION_HOME);
        astro.setTimezone(TZ_UTC); // compute all times in utc
        astro.setDaylightSaving(0);
        astro.setYear(y); // needed to check if year has leap day
        uint16_t doy = astro.getDayOfYear(d, m, y);

        if (D) {
            _log(fn + "" + y + "-" + m + "-" + d + ", doy: " + doy);
        }

        float sr, sd; // sun rise, sun down
        astro.getTimes(doy, sr, sd);

        if (D) {
            _log(fn + "utc: sunrise: " + floatToHHMM(sr));
            _log(fn + "utc: sundown: " + floatToHHMM(sd));
        }

        time_t sunrise_utc = suntimeToUTC(now_utc, sr);
        time_t sundown_utc = suntimeToUTC(now_utc, sd);
        time_t sunrise_loc = timezone->toLocal(sunrise_utc, &tcr);
        time_t sundown_loc = timezone->toLocal(sundown_utc, &tcr);

        sunrise = 100*hour(sunrise_loc)+minute(sunrise_loc);
        sundown = 100*hour(sundown_loc)+minute(sundown_loc);

        if (D) {
            _log(fn + "loc: sunrise: " + sunrise);
            _log(fn + "loc: sundown: " + sundown);
        }
    }

    int add(int hhmm, int hhmm_add) {
        int hh = (hhmm/100) + (hhmm_add/100);
        int mm = (hhmm%100) + (hhmm_add%100);
        hh += mm/60;
        mm %= 60;
        hh %= 24;
        return (100*hh)+mm;
    }

    int sub(int hhmm, int hhmm_sub) {
        int hh = (hhmm/100) - (hhmm_sub/100);
        int mm = (hhmm%100) - (hhmm_sub%100);

        while (mm<0) { mm += 60; hh--; }
        while (hh<0) { hh += 24; }

        hh += mm/60;
        mm %= 60;
        hh %= 24;
        return (100*hh)+mm;
    }

    boolean time_known;

    void setTimeKnown(boolean known)  {
        time_known=known;
    }

    boolean isTimeKnown() {
        return time_known;
    }

    String handle() {
        return handle(0, false);
    }

    String handle(time_t now_utc, boolean D=false) {
        String fn = cl+"handle: ";
        String rtv = "";
        if (D) { _log(fn + "entered, now_utc=" + now_utc); }

        if (0==now_utc) {
            if (!isTimeKnown()) {
                return String("time unknown");
            }
            now_utc = now();
        }

        if (NULL==timezone) {
            return String("timezone unknown");
        }

        time_t now_loc = timezone->toLocal(now_utc, &tcr);
        int mon  = month(now_loc);   // current local month
        int dom  = day(now_loc);     // current local day of month
        int dow  = weekday(now_loc); // current local day of week
        int hrs  = hour(now_loc);    // current local hour
        int min  = minute(now_loc);  // current local minute
        int hhmm = 100*hrs+min;

        if (last_hhmm == hhmm) {
            return String("no change");;
        }
        last_hhmm = hhmm;

        if (lastDow != dow) {
            if (D) { _log(fn + "dow: changed: change => computeSunTime\n"); }
            AstroCron::computeSunTime(now_utc, D);
        }
        lastDow = dow;
        rtv = handleSchedules(now_utc, mon, dom, dow, hrs, min, D);
        _log(fn + "handleSchedules returned " + rtv);

        return rtv;
    }


    virtual String handleSchedules(time_t now_utc, int mon, int dom, int dow, int hrs, int min, boolean D) {
        // override this in subclass to do something useful
        return "nothing";
    }

  protected:

    virtual void _log(String msg) {
        if (_D) Serial.println(msg);
    }

    String cl = "AstroCron::";

    int16_t sunrise;   //  = -1; // current day's sunrise time (hh*100=mm)
    int16_t sundown;   //  = -1; // current day's sundown time (hh*100=mm)

  private:
    boolean          _D;
    Timezone       * timezone;
    TimeChangeRule * tcr; // pointer to the time change rule, use to get the TZ abbrev
    time_t           utc;

    int8_t  lastDow;  //  = -1; // last day of week seen in handleSchedules()
    int8_t  last_hhmm; // = -1; // last hour/minute seen in handleSchedules()
};

#endif
