#!/bin/sh
#
#    File:    geo-convert-test.sh
#    Author:  Marvin Smith
#    Date:    5/6/2014
#
#    Purpose: Unit tests for geo-convert
#



#  Run application converting UTM to Geodetic DD
#echo 'Testing UTM to Geodetic-DD'
LATITUDE=`./release/bin/geo-convert -c -i -utm:18:323394:4307396 -o -geod-dd | sed 's/,/ /g' | cut -d ' ' -f 1`
LONGITUDE=`./release/bin/geo-convert -c -i -utm:18:323394:4307396 -o -geod-dd | sed 's/,/ /g' | cut -d ' ' -f 1`
ALTITUDE=`./release/bin/geo-convert -c -i -utm:18:323394:4307396 -o -geod-dd | sed 's/,/ /g' | cut -d ' ' -f 1`



#  Convert Geodetic DD to UTM
#echo 'Testing Geodetic-DD to UTM'
#./release/bin/geo-convert -c -i -geod-dd:38.8977:-77.0365 -o -utm


