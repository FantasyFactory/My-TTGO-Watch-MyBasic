/*This file represents the hardware bindings that allow BASIC code to access arduino functions

** Copyright (C) 2018 Daniel Dunn
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of
** this software and associated documentation files (the "Software"), to deal in
** the Software without restriction, including without limitation the rights to
** use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
** the Software, and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
** FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
** IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

extern "C"
{
#include "my_basic.h"
}

#include <TTGO.h>
#include "hardware/pmu.h"
#include "hardware/motor.h"


static void _unref(struct mb_interpreter_t* s, void* d) {
  free(d);
}

static int bas_bytearray(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t len;
  mb_assert(s && l);

  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_pop_int(s, l, &len));
  mb_check(mb_attempt_close_bracket(s, l));

  mb_value_t ret;
  //A little safety margin against off by one user error.
  void * p = malloc(len + 2);
  mb_make_ref_value(s, p, &ret, _unref, 0, 0, 0, 0);
  mb_check(mb_push_value(s, l, ret));

  return result;
}

int xprintf(const char *format, ...)
{
  int result = MB_FUNC_OK;
  char *buf = (char *)malloc(128); // Massimo 128 bytes per linea ? Nessun controllo qui !

  va_list ap;
  va_start(ap, format);
  vsnprintf(buf, 128, format, ap);
  for (char *p = &buf[0]; *p; p++) // emulate cooked mode for newlines
  {
    if (*p == '\n')
      Serial.write('\r');
    Serial.write(*p);
  }
  va_end(ap);
  free(buf);
  Serial.println("\n");

  return result;
}


//HardwareSerial Serial1(1);
//HardwareSerial Serial2(2);
HardwareSerial Serial3(3);
HardwareSerial * serports[3] = {&Serial, &Serial1, &Serial2};

//serbegin(portnumber, baudrate, [txpin, rxpin])
int bas_serbegin(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t port = 0;
  int64_t baud = 0;

  int64_t rxpin = 0;
  int64_t txpin = 0;

  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_pop_int(s, l, &port));
  mb_check(mb_pop_int(s, l, &baud));

  if (mb_has_arg(s, l)) {
    mb_check(mb_pop_int(s, l, &rxpin));
    mb_check(mb_pop_int(s, l, &txpin));
    serports[port]->begin(baud, SERIAL_8N1, rxpin, txpin);
  }
  else
  {
    mb_check(mb_attempt_close_bracket(s, l));
    serports[port]->begin(baud);
    return result;
  }
  mb_check(mb_attempt_close_bracket(s, l));
  return result;
}

//sersend(port, data, len)
int bas_sersend(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t port = 0;
  void * data = 0;
  int64_t len = 0;

  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_pop_int(s, l, &port));

  mb_value_t arg;
  mb_make_nil(arg);
  mb_check(mb_pop_value(s, l, &arg));
  mb_check(mb_get_ref_value(s, l, arg, &data));
 
  mb_check(mb_pop_int(s, l, &len));
  mb_check(mb_attempt_close_bracket(s, l));

  serports[port]->write((uint8_t *)data, len);

  return result;
}

int bas_pinMode(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t m = 0;
  int64_t n = 0;

  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_pop_int(s, l, &m));
  mb_check(mb_pop_int(s, l, &n));
  mb_check(mb_attempt_close_bracket(s, l));

  //Dunno,maybe there's some pins or something that should not be set up that way.
  pinMatrixInDetach(m, false, false);
  pinMatrixOutDetach(m, false, false);
  pinMode(m, n);
  return result;
}

int bas_digitalWrite(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t m = 0;
  int64_t n = 0;

  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_pop_int(s, l, &m));
  mb_check(mb_pop_int(s, l, &n));
  mb_check(mb_attempt_close_bracket(s, l));
  digitalWrite(m, n);
  return result;
}

int bas_analogWrite(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t m = 0;
  int64_t n = 0;

  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_pop_int(s, l, &m));
  mb_check(mb_pop_int(s, l, &n));
  mb_check(mb_attempt_close_bracket(s, l));
  //analogWrite(m, n);
  return result;
}

int bas_analogRead(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t n = 0;
  int r = 0;
  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_pop_int(s, l, &n));
  mb_check(mb_attempt_close_bracket(s, l));
  r = analogRead(n);
  mb_check(mb_push_int(s, l, r));

  return result;
}

int bas_delay(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t n = 0;

  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_pop_int(s, l, &n));
  mb_check(mb_attempt_close_bracket(s, l));

  vTaskDelay(n/portTICK_PERIOD_MS);
  //delay(n);

  return result;
}

int bas_millis(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t r = 0;
  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_attempt_close_bracket(s, l));
  r = millis();
  mb_check(mb_push_int(s, l, r));

  return result;
}

int bas_gettime(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;

  struct tm  info;
  char time_str[6]={0,0,0,0,0,0};
  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_attempt_close_bracket(s, l));

  time_t now;
  time( &now );
  localtime_r( &now, &info );
  int h = info.tm_hour;
  int m = info.tm_min;
  snprintf( time_str, sizeof(time_str), "%02d:%02d", h, m );
  //Serial.printf("GetTime=\"%s\"", time_str);
  mb_check(mb_push_string(s, l, mb_memdup(time_str, strlen(time_str)+1)));

  return result;
}

//PEEK(bytearray, index)
int bas_peek(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  void * n = 0;
  int64_t r = 0;
  int64_t idx =0;
  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));

  mb_value_t arg;
  mb_make_nil(arg);
  mb_check(mb_pop_value(s, l, &arg));
  mb_check(mb_get_ref_value(s, l, arg, &n));
  mb_check(mb_pop_int(s, l, &idx));

  mb_check(mb_attempt_close_bracket(s, l));
  uint8_t * p = (uint8_t *)n+idx;
  r = *p;
  mb_check(mb_push_int(s, l, r));

  return result;
}

///Peekas(x,l,n), peek at position N in bytearray x interpreting it as an l-byte integer.
int bas_peekas(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t vlen = 0;
  void * n = 0;
  int64_t idx;

  int64_t r = 0;
  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));

  mb_value_t arg;
  mb_make_nil(arg);
  mb_check(mb_pop_value(s, l, &arg));
  mb_check(mb_get_ref_value(s, l, arg, &n));

  mb_check(mb_pop_int(s, l, &idx));
  mb_check(mb_pop_int(s, l, &vlen));

  mb_check(mb_attempt_close_bracket(s, l));

  uint8_t * p = (uint8_t *)(n + idx);

  while (vlen)
  {
    r *= 256;
    r = *p;
    vlen -= 1;
    p += 1;
  }
  mb_check(mb_push_int(s, l, r));

  return result;
}

//poke(bytearray, index, value)
int bas_poke(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  void * n = 0;
  int64_t idx;

  int64_t val;

  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));

  mb_value_t arg;
  mb_make_nil(arg);
  mb_check(mb_pop_value(s, l, &arg));
  mb_check(mb_get_ref_value(s, l, arg, &n));

  mb_check(mb_pop_int(s, l, &idx));
  mb_check(mb_pop_int(s, l, &val));

  mb_check(mb_attempt_close_bracket(s, l));

  uint8_t * p = ((uint8_t *)n);
  *p = (uint8_t)val;
  return result;
}

///pokeas(x,l,n, v), peek at position N interpreting it as an l-byte integer.
int bas_pokeas(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t vlen = 0;
  void * n = 0;
  int64_t idx;

  int64_t val;
  int64_t r = 0;
  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));

  mb_value_t arg;
  mb_make_nil(arg);
  mb_check(mb_pop_value(s, l, &arg));
  mb_check(mb_get_ref_value(s, l, arg, &n));

  mb_check(mb_pop_int(s, l, &idx));
  mb_check(mb_pop_int(s, l, &vlen));
  mb_check(mb_pop_int(s, l, &val));

  mb_check(mb_attempt_close_bracket(s, l));

  uint8_t * p = (uint8_t *)(n + idx);

  int i = 0;

  while (vlen)
  {

    p[i] = ((uint8_t*)&val)[i];
    i++;
    vlen -= 1;
  }
  mb_check(mb_push_int(s, l, r));

  return result;
}


/************************ TTGO-Watch specific **************************/

int bas_getbattpct(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;

  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_attempt_close_bracket(s, l));

  int r;
  r=pmu_get_battery_percent();
  Serial.printf("GetBatteryPct=%d%%", r);
  mb_check(mb_push_int(s, l, r));

  return result;
}

int bas_motor_vibe(struct mb_interpreter_t* s, void** l) {
  int result = MB_FUNC_OK;
  int64_t n = 0;

  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  mb_check(mb_pop_int(s, l, &n));
  mb_check(mb_attempt_close_bracket(s, l));

  motor_vibe( 1 / 10 );

  return result;
}

/************************ Enabler **************************/

void enableArduinoBindings(struct mb_interpreter_t* bas)
{
  mb_register_func(bas, "analogRead", bas_analogRead);
  mb_register_func(bas, "pinMode", bas_pinMode);
  mb_register_func(bas, "digitalWrite", bas_digitalWrite);
  mb_register_func(bas, "analogWrite", bas_analogWrite);

  mb_register_func(bas, "millis", bas_millis);
  mb_register_func(bas, "DELAY", bas_delay);
  mb_register_func(bas, "GetTime", bas_gettime);

  mb_register_func(bas, "peek", bas_peek);
  mb_register_func(bas, "poke", bas_poke);
  mb_register_func(bas, "peekas", bas_peekas);
  mb_register_func(bas, "pokeas", bas_pokeas);
  mb_register_func(bas, "malloc", bas_bytearray);
  mb_register_func(bas, "sersend", bas_sersend);

  /**** TTGO Watch specific ***/
  mb_register_func(bas, "GetBatteryPct", bas_getbattpct);
  mb_register_func(bas, "Motor", bas_motor_vibe);

}

void enableSerialPrint(struct mb_interpreter_t* bas) {
  mb_set_printer(bas, xprintf);
}


