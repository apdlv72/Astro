Astro
=====

Arduino library to compute sunrise / sundown times based on geographic location and current time
with a reasonable compromise between accuracy and computational effort. In moste cases the computes 
times will differ by a few minutes only shich should be sufficient for most applications.

The formula used here is absed on what I learnde from the following link (in German only, 
but google translate is your friend):

http://lexikon.astronomie.info/zeitgleichung/ 

Theres rudimentary support for some date calculation that will copute the day of year for a given 
day/month/year as well as a list of coordinates for some major cites in the world.

