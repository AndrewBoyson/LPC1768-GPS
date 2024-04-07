//Nmea script
'use strict';

const NMEA_DEG_UNIT = 10000000;
const NMEA_HGT_UNIT =  1000000;

let gpsTrace     = false;
let gpsVerbose   = false;
let msgTrace     = false;
let cmdTrace     = false;
let lat          = 0;
let lng          = 0;
let hgt          = 0;
let hgtAboveMsl  = 0;
let mslAboveWgs  = 0;
let dop          = 0;
let satCount     = 0;
let fixQuality   = 0;
let sensorHeight = 0;
let bucketCount  = 0;
let buckets      = '';

function parseVariables(text)
{
    let lines = text.split('\n');
    gpsTrace     = Ajax.hexToBit        (lines[0], 0);
    gpsVerbose   = Ajax.hexToBit        (lines[0], 1);
    msgTrace     = Ajax.hexToBit        (lines[0], 2);
    cmdTrace     = Ajax.hexToBit        (lines[0], 3);
    lat          = Ajax.hexToSignedInt32(lines[1]);
    lng          = Ajax.hexToSignedInt32(lines[2]);
    hgt          = Ajax.hexToSignedInt32(lines[3]);
    hgtAboveMsl  = Ajax.hexToSignedInt32(lines[4]);
    mslAboveWgs  = Ajax.hexToSignedInt32(lines[5]);
    dop          = Ajax.hexToSignedInt32(lines[6]);
    satCount     = Ajax.hexToSignedInt32(lines[7]);
    fixQuality   = Ajax.hexToSignedInt32(lines[8]);
    sensorHeight = Ajax.hexToSignedInt32(lines[9]);
}
function parseBuckets(text)
{
    let lines = text.split('\n');
    bucketCount  = Ajax.hexToSignedInt32(lines[0]);
    buckets      = '';
    for (let i = 0; i < bucketCount; i++)
    {
        buckets += (i * 50).toString().padStart(3, '0') + ' ' + Ajax.hexToSignedInt32(lines[i + 1]) + '\r\n';
    }
}
function parse()
{
    let topics = Ajax.response.split('\f');
    parseVariables(topics[0]);
    parseBuckets  (topics[1]);
}
function degToString(plus, minus, deg)
{
    deg /= NMEA_DEG_UNIT;
    if (deg >= 0) return plus  +    deg.toFixed(5);
    else          return minus + (-deg).toFixed(5);
}
function hgtToString(plus, minus, hgt)
{
    hgt /= NMEA_HGT_UNIT;
    if (hgt >= 0) return plus  +    hgt.toFixed(1);
    else          return minus + (-hgt).toFixed(1);
}
function display()
{
   let elem;
   elem = Ajax.getElementOrNull('ajax-trace-gps'    ); if (elem) elem.setAttribute('dir', gpsTrace   ? 'rtl' : 'ltr');
   elem = Ajax.getElementOrNull('ajax-trace-verbose'); if (elem) elem.setAttribute('dir', gpsVerbose ? 'rtl' : 'ltr');
   elem = Ajax.getElementOrNull('ajax-trace-msg'    ); if (elem) elem.setAttribute('dir', msgTrace   ? 'rtl' : 'ltr');
   elem = Ajax.getElementOrNull('ajax-trace-cmd'    ); if (elem) elem.setAttribute('dir', cmdTrace   ? 'rtl' : 'ltr');

   elem = Ajax.getElementOrNull('ajax-position'     ); if (elem) elem.textContent = degToString('N', 'S', lat) + ' ' +
                                                                                    degToString('E', 'W', lng) + ' ' +
                                                                                    hgtToString('H', 'D', hgt);
   elem = Ajax.getElementOrNull('ajax-msl-above-wgs'); if (elem) elem.textContent = hgtToString('H', 'D', mslAboveWgs);
   elem = Ajax.getElementOrNull('ajax-hgt-above-msl'); if (elem) elem.textContent = hgtToString('H', 'D', hgtAboveMsl);
   elem = Ajax.getElementOrNull('ajax-gnd-above-msl'); if (elem) elem.textContent = hgtToString('H', 'D', hgtAboveMsl - sensorHeight * NMEA_HGT_UNIT);

   elem = Ajax.getElementOrNull('ajax-dop'          ); if (elem) elem.textContent = dop / 100;
   elem = Ajax.getElementOrNull('ajax-sat-count'    ); if (elem) elem.textContent = satCount;
   elem = Ajax.getElementOrNull('ajax-fix-quality'  ); if (elem) elem.textContent = fixQuality;
   
   elem = Ajax.getElementOrNull('ajax-sensor-hgt'   ); if (elem) elem.value       = sensorHeight;
   
   elem = Ajax.getElementOrNull('ajax-bucket-count' ); if (elem) elem.textContent = bucketCount;
   elem = Ajax.getElementOrNull('ajax-buckets' );      if (elem) elem.textContent = buckets;
}

Ajax.server     = '/nmea-ajax';
Ajax.onResponse = function() { parse(); display(); };
Ajax.init();