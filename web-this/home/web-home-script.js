//Home script
'use strict';

let rtc             = new Clock();

let diffMs          = 0;

let messagesOk      = false;
let fixMessagesOk   = false;
let timeMessagesOk  = false;
let ppsStable       = false;
let nmeaStable      = false;

let rtcIsSet        = false;
let clockIsSet      = false;
let sourceIsOk      = false;
let rateIsLocked    = false;
let timeIsLocked    = false;

const DISPLAY_LEAP_MS = 10000;

function parse()
{
    let lines = Ajax.response.split('\n');
    rtc.ms          = Ajax.date.getTime();
    rtc.ms         += parseInt(lines[0], 16);
    rtc.ms         -= Ajax.ms;
    diffMs          = rtc.ms + Ajax.ms - Date.now();
    messagesOk      = Ajax.hexToBit(lines[1], 11);
    fixMessagesOk   = Ajax.hexToBit(lines[1], 10);
    timeMessagesOk  = Ajax.hexToBit(lines[1],  9);
    ppsStable       = Ajax.hexToBit(lines[1],  8);
    nmeaStable      = Ajax.hexToBit(lines[1],  7);
    rtcIsSet        = Ajax.hexToBit(lines[1],  6);
    clockIsSet      = Ajax.hexToBit(lines[1],  5);
    sourceIsOk      = Ajax.hexToBit(lines[1],  4);
    rateIsLocked    = Ajax.hexToBit(lines[1],  3);
    timeIsLocked    = Ajax.hexToBit(lines[1],  2);
    rtc.months1970  = parseInt(lines[2], 16);
    rtc.leaps       = parseInt(lines[3], 16);
}
function display()
{
    let elem;
    elem = Ajax.getElementOrNull('ajax-msgs-ok'      ); if (elem) elem.setAttribute('dir', messagesOk      ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('ajax-fix-msgs-ok'  ); if (elem) elem.setAttribute('dir', fixMessagesOk   ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('ajax-time-msgs-ok' ); if (elem) elem.setAttribute('dir', timeMessagesOk  ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('ajax-pps-stable'   ); if (elem) elem.setAttribute('dir', ppsStable       ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('ajax-nmea-stable'  ); if (elem) elem.setAttribute('dir', nmeaStable      ? 'rtl' : 'ltr');
    
    elem = Ajax.getElementOrNull('ajax-rtc-set'      ); if (elem) elem.setAttribute('dir', rtcIsSet        ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('ajax-clock-set'    ); if (elem) elem.setAttribute('dir', clockIsSet      ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('ajax-source-ok'    ); if (elem) elem.setAttribute('dir', sourceIsOk      ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('ajax-rate-locked'  ); if (elem) elem.setAttribute('dir', rateIsLocked    ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('ajax-time-locked'  ); if (elem) elem.setAttribute('dir', timeIsLocked    ? 'rtl' : 'ltr');
    
    elem = Ajax.getElementOrNull('ajax-date-diff'    ); if (elem) elem.textContent = diffMs;
    
    elem = Ajax.getElementOrNull('ajax-leap-year'    ); if (elem) elem.value = rtc.months1970 ? rtc.leapYear  : '';
    elem = Ajax.getElementOrNull('ajax-leap-month'   ); if (elem) elem.value = rtc.months1970 ? rtc.leapMonth : '';
    elem = Ajax.getElementOrNull('ajax-leap-count'   ); if (elem) elem.value = rtc.leaps;
}

function handleTick() //This typically called every 100ms
{
    rtc.adjustLeap (Ajax.ms);
    rtc.displayTime(Ajax.ms);
}

Ajax.server     = '/home-ajax';
Ajax.onResponse = function() { parse(); display(); };
Ajax.onTick     = handleTick;
Ajax.init();

