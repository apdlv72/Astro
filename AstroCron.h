#ifndef AstroCron_h
#define AstroCron_h

#include <Timezone.h>

class AstroCron {

  public:

    AstroCron(boolean D=false) {
        _D        =  D;
        _lastDow   = -1;
        _lastHHMM  = -1;
        _sunrise   = -1;
        _sundown   = -1;
        _timezone  = NULL;
    }

    void setLocation(float lo, float la) {
        _long = lo;
        _lat  = la;
    }

    void setTimezone(Timezone * tz) {
        _timezone  = tz;
    }

    String getSunriseHHMM() {
        return hhmmToStr(_sunrise);
    }

    String getSundownHHMM() {
        return hhmmToStr(_sundown);
    }

    const String getSunTime(char delim=';') {
        return getSunriseHHMM() + delim + getSundownHHMM();
    }

    int16_t getSunrise() {
        return _sunrise;
    }

    int16_t getSundown() {
        return _sundown;
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

        String fn = cl+"computeSunTime: ";

        //time_t now_utc = now();
        time_t now_loc = _timezone->toLocal(now_utc, &_tcr);
        //if (D) { _log(fn + "local time: " + printTime(now_loc, _tcr->abbrev)); }

        int d = day(now_utc);
        int m = month(now_utc);
        int y = year(now_utc);

        Astro astro;
        astro.setLocation(_long, _lat);
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
            _log(fn + "utc: _sunrise: " + floatToHHMM(sr));
            _log(fn + "utc: _sundown: " + floatToHHMM(sd));
        }

        time_t sunrise_utc = suntimeToUTC(now_utc, sr);
        time_t sundown_utc = suntimeToUTC(now_utc, sd);
        time_t sunrise_loc = _timezone->toLocal(sunrise_utc, &_tcr);
        time_t sundown_loc = _timezone->toLocal(sundown_utc, &_tcr);

        _sunrise = 100*hour(sunrise_loc)+minute(sunrise_loc);
        _sundown = 100*hour(sundown_loc)+minute(sundown_loc);

        if (D) {
            _log(fn + "loc: _sunrise: " + _sunrise);
            _log(fn + "loc: _sundown: " + _sundown);
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

    void setTimeKnown(boolean known)  {
        _timeKnown=known;
    }

    boolean isTimeKnown() {
        return _timeKnown;
    }

    int handle() {
        return handle(0 /* get current time yourself */, false /* no debug */);
    }

    void setConfigChanged(boolean changed=true) {
        _configChanged = changed;
    }

    virtual boolean hasConfigChanged() {
        boolean rtv = _configChanged;
        _configChanged = false;
        return rtv;
    }

    int handle(time_t now_utc, boolean D=false) {
        String fn = cl+"handle: ";
        int rtv = 0;
        if (D) { _log(fn + "entered, now_utc=" + now_utc); }

        if (0==now_utc) {
            if (!isTimeKnown()) {
                return -2; // String("time unknown");
            }
            now_utc = now();
        }

        if (NULL==_timezone) {
            return -3; // String("_timezone unknown");
        }

        time_t now_loc = _timezone->toLocal(now_utc, &_tcr);
        int mon  = month(now_loc);   // current local month
        int dom  = day(now_loc);     // current local day of month
        int dow  = weekday(now_loc); // current local day of week
        int hrs  = hour(now_loc);    // current local hour
        int min  = minute(now_loc);  // current local minute
        int hhmm = (100*hrs)+min;

        if (_lastHHMM==hhmm) {
            if (!hasConfigChanged()) {
                return -1; // String("no change");
            }
        }

        if (_lastHHMM<_sunrise && _sunrise<=hhmm) {
            _log(fn+"_lastHHMM=" + _lastHHMM+ ", _sunrise=" + _sunrise + ", hhmm=" + hhmm + " -> onSunrise()");
            onSunrise();
        }
        if (_lastHHMM<_sundown && _sundown<=hhmm) {
            _log(fn+"_lastHHMM=" + _lastHHMM+ ", _sundown=" + _sundown + ", hhmm=" + hhmm + " -> onSundown()");
            onSundown();
        }

        _lastHHMM = hhmm;

        if (_lastDow != dow) {
            if (D) { _log(fn + "dow: changed: change=>computeSunTime\n"); }
            int16_t sunriseOld = _sunrise;
            int16_t sundownOld = _sundown;
            AstroCron::computeSunTime(now_utc, D);
            if (sunriseOld!=_sunrise || sundownOld!=_sundown) {
                _log(fn + "onSuntimeChanged()");
                onSuntimeChanged();
            }
        }
        _lastDow = dow;
        rtv = handleSchedules(now_utc, mon, dom, dow, hrs, min, D);
        _log(fn + "handleSchedules returned " + rtv);

        return rtv;
    }

    virtual void onSuntimeChanged() {
    }

    virtual void onSunrise() {
    }

    virtual void onSundown() {
    }

    virtual int handleSchedules(time_t now_utc, int mon, int dom, int dow, int hrs, int min, boolean D) {
        // override this in subclass to do something useful
        return -4; // "nothing";
    }

  protected:

    virtual void _log(String msg) {
        if (_D) Serial.println(msg);
    }

    const static String cl; // = "AstroCron::";

    int16_t _sunrise;   //  = -1; // current day's _sunrise time (hh*100=mm)
    int16_t _sundown;   //  = -1; // current day's _sundown time (hh*100=mm)

  private:
    boolean          _D;
    boolean          _configChanged = true;
    boolean          _timeKnown     = false;
    float            _long = 0;
    float            _lat  = 0;
    Timezone       * _timezone;
    TimeChangeRule * _tcr; // pointer to the time change rule, use to get the TZ abbrev

    int8_t   _lastDow;  // = -1; // last day of week seen in handleSchedules()
    int16_t  _lastHHMM; // = -1; // last hour/minute seen in handleSchedules()
};

const String AstroCron::cl = "AstroCron::";

#endif
