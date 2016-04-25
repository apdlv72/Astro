Astro
=====

Arduino library to compute sunrise / sundown times based on geographic location and current time
with a reasonable compromise between accuracy and computational effort. In most cases the computed
times will differ by a few minutes only which should be sufficient for most applications.
Use of this library is targetet at e.g. outside illumination timers.

The formula used here is based on what I learnde from the following link (in German only, 
but google translate is your friend):

http://lexikon.astronomie.info/zeitgleichung/ 

There is basic support for some date calculation that will compute the day of year for a given 
day/month/year as well as a list of coordinates for some major cites in the world.

Included is also a cron like scheduler that can "run in the background" (with regular calls to
a handle() method) and will call a virtual method every minute and update sunrise/sundown times
whenever a new day starts.



