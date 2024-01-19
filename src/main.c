#define F_CPU 8000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include "../include/oled_driver.h"
//Time Functions and variables
void setTime(); //adjust_time
int timestamp_hr(int *h); //get hour
int timestamp_min(int *m); //get min
int timestamp_sec(int *s); // get sec
char tstr[10];
char tstr2[10];
char tstr3[10];
char str[10];
void addHour();
void addMin();
void time();
void timestamp();
int mode = 0;
int mode2 =0;
volatile int msec2;
int sec = 0;
int min = 0;
int hrs=0;
char str6[10];
char str5[10];
float bpm;
char str7[10];
int belowthres = 1;
int abreads = 0;
float avgbpm = 0.0;
int num = 0;
//EEPROM
char R_arrayr[10];
char W_arrayr[10];
char R_arrayhr[10];
char W_arrayhr[10];
char W_arrayavg[10];
char R_arrayavg[10];
char R_arraymin[10];
char W_arraymin[10];
char R_arraysec[10];
char W_arraysec[10];
char R_arrayecg[10];
char W_arrayecg[10];
char R_choice[10];
char W_choice[10];
int i,j,k;
//Variables and Functions for AD8232
unsigned long time1=0;
unsigned long time2 = 0;
volatile int msec = 0;
int sensorValue = 0;
float previousBpm=0;
float currentBpm =0;
float period=0;
float periodInSeconds=0;
char str2[10];
char str3[10];
char str4[10];
float volts=0;
char buffer[10];
//char arrecg[6][10];
//char * arr_ecg[10];
char time_1[10];
char time_2[10];
void calculateBPM();
void calculateBPM2();
int millis();
int bttn = 0;
//Menu Functions and variables
void menu();
void activc();
void menu2();
void checkMode();
void checkMode2();
int choice = 0;
int choice2 = 0;
int exit1 = 0;
int millis(){
msec++;
return (msec);
}
ISR(TIMER1_COMPA_vect)
{
checkMode();
//checkMode2();
}
ISR(TIMER0_COMPA_vect)
{
//checkMode();
msec2++;
if (mode2 == 2){
millis();
}
}
ISR(INT0_vect)
{
if(bttn == 0){
bttn = 1; //low power mode activated
OLED_Clear();
sleep_enable();
PRR = 0b01101111;
}
else{
PRR = 0x00;
bttn = 0;
sleep_disable();
OLED_Clear();
}
}
int main(void){
OLED_Init();
//_delay_ms(1000);
OLED_Clear();
DDRC = 0b00000111; //Attach buttons
//bttn set up
//INTO vect
DDRD &= ~(1 << DDD2); // Clear the PD2 pin
PORTD |= (1 << PORTD2);
MCUCR = 0x02; // falling edge triggered mode
EIMSK |= (1 << INT0);
set_sleep_mode(SLEEP_MODE_ADC);
sei();
init_adc1();
PORTC |= 1 << PINC0; //Mode
PORTC |= 1 << PINC1; //Increase
PORTC |= 1 << PINC2; //Decrease
//DDRC = 0x00;
//TIMER 1 set up
OCR1A = 7812;
TCCR1A = 0x00;
TCCR1B= 0x0D;
TIMSK0 |= (1<<OCIE1A); //enable Timer1 compare match int.
//TIMER 0 set up
TCCR0A = (1 << WGM01); //Set CTC bit
OCR0A = 250; //Number of ticks
TCCR0B = (1<<CS01)|(1<<CS00); //Set prescaler to 1024
TIMSK0 = (1 << OCIE0A);//Compare A match interrupt enable
//AD8232 LO+ and LODDRB |= (1 << PINB2);
DDRB |= (1 << PINB1);
//Buzzer
DDRB |= (1 << PINB0);
while (1)
{
exit1 = 1;
if(mode == 0)
{
menu();
OLED_Clear();
}
if(choice == 1)
{
//======= tester =======/
mode = 2;
OLED_SetCursor(0,0);
OLED_Printf("WELCOME!");
checkMode();
}
if(choice == 2)
{
//======= TIME =======/
setTime();
}
if(choice == 3)
{
mode = 2;
activc();
}
if(choice == 4)
{
mode = 2;
OLED_SetCursor(2,1);
OLED_Printf("Time");
if(hrs < 10)
{
//Single digits
OLED_SetCursor(3,1);
OLED_Printf("0");
OLED_Printf(R_arrayhr);
OLED_Printf(":");
}
else
{
//Double digits
OLED_SetCursor(3,1);
OLED_Printf(R_arrayhr);
OLED_Printf(":");
}
if(min < 10)
{
//Single digits
OLED_Printf("0");
OLED_Printf(R_arraymin);
OLED_Printf(":");
}
else
{
//Double digits
OLED_Printf(R_arraymin);
OLED_Printf(":");
}
if(sec < 10)
{
//Single digits
OLED_Printf("0");
OLED_Printf(R_arraysec);
}
else
{
//Double digits
OLED_Printf(R_arraysec);
}
OLED_SetCursor(0,0);
OLED_Printf(R_choice);
OLED_SetCursor(4,1);
itoa(abreads,str6,10);
OLED_Printf("Abnormal Readings");
OLED_Printf(R_arrayr);
OLED_SetCursor(5,1);
OLED_Printf("Avg BPM:");
OLED_Printf(R_arrayavg);
checkMode();
}
}
}
void menu()
{
int up_down = 1;
OLED_Clear();
while(mode == 0)
{
//increase bttn
if (bit_is_clear(PINC,1))
{
up_down++;
if(up_down > 4)
{
up_down = 1;
}
while(bit_is_clear(PINC,1))
{
_delay_ms(50);
}
}
//decrease bttn
if (bit_is_clear(PINC,2))
{
up_down --;
if(up_down < 0){
up_down = 4;
}
while(bit_is_clear(PINC,2))
{
_delay_ms(50);
}
}
switch (up_down)
{
case 1:
OLED_SetCursor(0,0);
OLED_Printf(" MENU");
OLED_SetCursor(1,1);
OLED_Printf(">HOME");
OLED_SetCursor(2,1);
OLED_Printf(" SET TIME");
OLED_SetCursor(3,1);
OLED_Printf(" ACTIVITY");
OLED_SetCursor(4,1);
OLED_Printf(" READINGS");
choice = 1;
break;
case 2:
OLED_SetCursor(0,0);
OLED_Printf(" MENU");
OLED_SetCursor(1,1);
OLED_Printf(" HOME");
OLED_SetCursor(2,1);
OLED_Printf(">SET TIME");
OLED_SetCursor(3,1);
OLED_Printf(" ACTIVITY");
OLED_SetCursor(4,1);
OLED_Printf(" READINGS");
choice = 2;
break;
case 3:
OLED_SetCursor(0,0);
OLED_Printf(" MENU");
OLED_SetCursor(1,1);
OLED_Printf(" HOME");
OLED_SetCursor(2,1);
OLED_Printf(" SET TIME");
OLED_SetCursor(3,1);
OLED_Printf(">ACTIVITY");
OLED_SetCursor(4,1);
OLED_Printf(" READINGS");
choice = 3;
break;
case 4:
OLED_SetCursor(0,0);
OLED_Printf(" MENU");
OLED_SetCursor(1,1);
OLED_Printf(" HOME");
OLED_SetCursor(2,1);
OLED_Printf(" SET TIME");
OLED_SetCursor(3,1);
OLED_Printf(" ACTIVITY");
OLED_SetCursor(4,1);
OLED_Printf(">READINGS");
choice = 4;
break;
}
checkMode();
}
}
//Checking if the mode bttn is pressed
void checkMode()
{
//======= Check if mode buttons is pressed =======/
if (bit_is_clear(PINC,0))
{
mode++;
if(mode > 2)
{
mode = 0;
}
while(bit_is_clear(PINC,0))
{
_delay_ms(1000);
}
}
}
void checkMode2()
{
//======= Check if mode buttons is pressed =======/
if (bit_is_clear(PINC,0))
{
mode2++;
if(mode2 > 2)
{
mode2 = 0;
}
while(bit_is_clear(PINC,0))
{
_delay_ms(1000);
}
}
}
int timestamp_hr(int *h){
return (*h);
}
int timestamp_min(int *m){
return (*m);
}
int timestamp_sec(int *s){
return (*s);
}
void setTime(){
hrs = 0;
min = 0;
sec = 0;
//PRINT TO OLED TIME STAMP
OLED_Clear();
OLED_SetCursor(1,1);
OLED_Printf ("SET");
OLED_SetCursor(10,1);
OLED_Printf("TIME");
_delay_ms(100);
OLED_Clear();
while(mode == 1)
{
//Adjust hours
OLED_SetCursor(1,1);
OLED_Printf("HOURS:");
itoa(hrs,str,10);
OLED_SetCursor(2,1);
OLED_Printf(str);
addHour();
checkMode();
}
_delay_ms(50);
OLED_Clear();
while(mode == 2)
{
//Adjust minutes
OLED_SetCursor(1,1);
OLED_Printf("MINUTES:");
itoa(min,str,10);
OLED_SetCursor(2,1);
OLED_Printf(str);
addMin();
checkMode();
}
}
void time(){
if(msec2 > 1000)
{
sec++;
msec2 = 0;
}
if (sec > 59){
min++;
sec = 0;
}
if (min > 59){
min = 0;
hrs ++;
}
if (min < 0){
hrs--;
min = 59;
}
if (hrs > 23){
hrs = 0;
}
if (hrs < 0)
{
hrs =23;
}
}
void addHour()
{
//======= Adding/Subtracting Hours =======/
if (bit_is_clear(PINC,1))
{//Increase hours
hrs++;
time();
while(bit_is_clear(PINC,1))
{
_delay_ms(100);
}
}
if (bit_is_clear(PINC,2))
{//Decrease hours
hrs--;
time();
while(bit_is_clear(PINC,2))
{
_delay_ms(100);
}
}
}
void addMin()
{
//======= Adding/Subtracting Minutes =======/
if (bit_is_clear(PINC,1))
{//Increase minutes
min++;
time();
while(bit_is_clear(PINC,1))
{
_delay_ms(100);
}
}
if (bit_is_clear(PINC,2))
{//Decrease minutes
min--;
time();
while(bit_is_clear(PINC,2))
{
_delay_ms(100);
}
}
}
void menu2()
{
int up_down = 1;
OLED_Clear();
while(mode2 == 0)
{
//increase bttn
if (bit_is_clear(PINC,1))
{
up_down++;
if(up_down > 4)
{
up_down = 1;
}
while(bit_is_clear(PINC,1))
{
_delay_ms(100);
}
}
//decrease bttn
if (bit_is_clear(PINC,2))
{
up_down --;
if(up_down < 0){
up_down = 4;
}
while(bit_is_clear(PINC,2))
{
_delay_ms(100);
}
}
switch (up_down)
{
case 1:
OLED_SetCursor(0,0);
OLED_Printf(" MENU");
OLED_SetCursor(1,1);
OLED_Printf(">NAPPING");
OLED_SetCursor(2,1);
OLED_Printf(" EXERCISE");
OLED_SetCursor(3,1);
OLED_Printf(" DAILY MOVES");
OLED_SetCursor(4,1);
OLED_Printf(" EXIT");
choice2 = 1;
break;
case 2:
OLED_SetCursor(0,0);
OLED_Printf(" MENU");
OLED_SetCursor(1,1);
OLED_Printf(" NAPPING");
OLED_SetCursor(2,1);
OLED_Printf(">EXERCISE");
OLED_SetCursor(3,1);
OLED_Printf(" DAILY MOVES");
OLED_SetCursor(4,1);
OLED_Printf(" EXIT");
choice2 = 2;
break;
case 3:
OLED_SetCursor(0,0);
OLED_Printf(" MENU");
OLED_SetCursor(1,1);
OLED_Printf(" NAPPING");
OLED_SetCursor(2,1);
OLED_Printf(" EXERCISE");
OLED_SetCursor(3,1);
OLED_Printf(">DAILY MOVES");
OLED_SetCursor(4,1);
OLED_Printf(" EXIT");
choice2 = 3;
break;
case 4:
OLED_SetCursor(0,0);
OLED_Printf(" MENU");
OLED_SetCursor(1,1);
OLED_Printf(" NAPPING");
OLED_SetCursor(2,1);
OLED_Printf(" EXERCISE");
OLED_SetCursor(3,1);
OLED_Printf(" DAILY MOVES");
OLED_SetCursor(4,1);
OLED_Printf(">EXIT");
choice2 = 4;
break;
}
checkMode2();
}
}
void activc(){
if (mode2 == 0){
PORTB &= ~(1<<0);
menu2();
OLED_Clear();
}
if(choice2 == 1)
{
mode2 = 2;
OLED_SetCursor(1,1);
timestamp();
OLED_Printf("CHOICE1");
char R_choice[10] = {"Choice1"};
memset(R_choice,0,10);
eeprom_busy_wait();
eeprom_write_block(W_choice,0,strlen(W_choice)); /* Write W_array from
EEPROM address 0 */
eeprom_read_block(R_choice,0,strlen(W_choice)); /* Read EEPROM from address
0 */
sensorValue = ReadADC(3);
dtostrf(sensorValue,4, 1, str2);
OLED_SetCursor (3,1);
OLED_Printf ("ADC:");
OLED_Printf (str2);
if(sensorValue>500 && belowthres == 1)
{
calculateBPM2();
dtostrf(currentBpm,4, 1, str5);
OLED_SetCursor (4,1);
OLED_Printf ("BPM:");
OLED_Printf (str5);
previousBpm = currentBpm;
if(currentBpm != previousBpm)
{
OLED_Clear();
}
if (currentBpm < 60 || (currentBpm > 120))
{
if (periodInSeconds > 0.6)
{
abreads+=1;
PORTB |= (1<<0);
}
}
else
{
PORTB &= ~(1<<0);
}
itoa (abreads, str6, 10);
for(k = 0; k < 10; k++){
W_arrayecg[k] = str5[k];
}
for(k = 0; k < 10; k++){
W_arrayr[k] = str6[k];
}
memset(R_arrayecg,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arrayecg,0,strlen(W_arrayecg));/* Write
W_array from EEPROM address 0 */
eeprom_read_block(R_arrayecg,0,strlen(W_arrayecg)); /* Read
EEPROM from address 0 */
memset(R_arrayr,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arrayr,0,strlen(W_arrayr)); /* Write
W_array from EEPROM address 0 */
eeprom_read_block(R_arrayr,0,strlen(W_arrayr)); /* Read
EEPROM from address 0 */
periodInSeconds =0;
belowthres = 0;
}
else
{
belowthres = 1;
}
checkMode2();
if (mode2 == 0){
for (i=0; i<=num; i++){
bpm = avgbpm/num;
}
dtostrf(bpm,4, 1, str7);
for(k = 0; k < 10; k++){
W_arrayavg[k] = str7[k];
}
memset(R_arrayavg,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arrayavg,0,strlen(W_arrayavg)); /* Write W_array from
EEPROM address 0 */
eeprom_read_block(R_arrayavg,0,strlen(W_arrayavg)); /* Read EEPROM from address
0 */
msec = 0;
abreads = 0;
currentBpm = 0;
}
}
if(choice2 == 2)
{
mode2 = 2;
OLED_SetCursor(1,1);
timestamp();
OLED_Printf("CHOICE2");
char R_choice[10] = {"Choice2"};
memset(R_choice,0,10);
eeprom_busy_wait();
eeprom_write_block(W_choice,0,strlen(W_choice)); /* Write W_array from
EEPROM address 0 */
eeprom_read_block(R_choice,0,strlen(W_choice)); /* Read EEPROM from address
0 */
sensorValue = ReadADC(3);
dtostrf(sensorValue,4, 1, str2);
OLED_SetCursor (3,1);
OLED_Printf ("ADC:");
OLED_Printf (str2);
if(sensorValue>500 && belowthres == 1)
{
calculateBPM();
dtostrf(currentBpm,4, 1, str5);
OLED_SetCursor (4,1);
OLED_Printf ("BPM:");
OLED_Printf (str5);
previousBpm = currentBpm;
if(currentBpm != previousBpm)
{
OLED_Clear();
}
if (currentBpm < 60 || (currentBpm > 200))
{
if (periodInSeconds > 0.6)
{
abreads+=1;
PORTB |= (1<<0);
}
}
else
{
PORTB &= ~(1<<0);
}
itoa (abreads, str6, 10);
for(k = 0; k < 10; k++){
W_arrayecg[k] = str5[k];
}
for(k = 0; k < 10; k++){
W_arrayr[k] = str6[k];
}
memset(R_arrayecg,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arrayecg,0,strlen(W_arrayecg));/* Write W_array from
EEPROM address 0 */
eeprom_read_block(R_arrayecg,0,strlen(W_arrayecg)); /* Read EEPROM from
address 0 */
memset(R_arrayr,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arrayr,0,strlen(W_arrayr)); /* Write W_array from
EEPROM address 0 */
eeprom_read_block(R_arrayr,0,strlen(W_arrayr)); /* Read EEPROM from
address 0 */
periodInSeconds =0;
belowthres = 0;
}
else
{
belowthres = 1;
}
checkMode2();
if (mode2 == 0){
for (i=0; i<=num; i++){
bpm = avgbpm/num;
}
dtostrf(bpm,4, 1, str7);
for(k = 0; k < 10; k++){
W_arrayavg[k] = str7[k];
}
memset(R_arrayavg,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arrayavg,0,strlen(W_arrayavg)); /* Write
W_array from EEPROM address 0 */
eeprom_read_block(R_arrayavg,0,strlen(W_arrayavg)); /* Read EEPROM from
address 0 */
msec = 0;
abreads = 0;
currentBpm = 0;
}
}
if(choice2 == 3)
{
mode2 = 2;
OLED_SetCursor(1,1);
timestamp();
OLED_Printf("CHOICE1");
char R_choice[10] = {"Choice1"};
memset(R_choice,0,10);
eeprom_busy_wait();
eeprom_write_block(W_choice,0,strlen(W_choice)); /* Write W_array from
EEPROM address 0 */
eeprom_read_block(R_choice,0,strlen(W_choice)); /* Read EEPROM from address
0 */
sensorValue = ReadADC(3);
dtostrf(sensorValue,4, 1, str2);
OLED_SetCursor (3,1);
OLED_Printf ("ADC:");
OLED_Printf (str2);
if(sensorValue>500 && belowthres == 1)
{
calculateBPM2();
dtostrf(currentBpm,4, 1, str5);
OLED_SetCursor (4,1);
OLED_Printf ("BPM:");
OLED_Printf (str5);
previousBpm = currentBpm;
if(currentBpm != previousBpm)
{
OLED_Clear();
}
if (currentBpm < 60 || (currentBpm > 120))
{
if (periodInSeconds > 0.5)
{
abreads+=1;
PORTB |= (1<<0);
}
}
else
{
PORTB &= ~(1<<0);
}
itoa (abreads, str6, 10);
for(k = 0; k < 10; k++){
W_arrayecg[k] = str5[k];
}
for(k = 0; k < 10; k++){
W_arrayr[k] = str6[k];
}
memset(R_arrayecg,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arrayecg,0,strlen(W_arrayecg));/* Write W_array from
EEPROM address 0 */
eeprom_read_block(R_arrayecg,0,strlen(W_arrayecg)); /* Read EEPROM from
address 0 */
memset(R_arrayr,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arrayr,0,strlen(W_arrayr)); /* Write W_array from
EEPROM address 0 */
eeprom_read_block(R_arrayr,0,strlen(W_arrayr)); /* Read EEPROM from
address 0 */
periodInSeconds =0;
belowthres = 0;
}
else
{
belowthres = 1;
}
checkMode2();
if (mode2 == 0){
for (i=0; i<=num; i++){
bpm = avgbpm/num;
}
dtostrf(bpm,4, 1, str7);
for(k = 0; k < 10; k++){
W_arrayavg[k] = str7[k];
}
memset(R_arrayavg,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arrayavg,0,strlen(W_arrayavg)); /* Write
W_array from EEPROM address 0 */
eeprom_read_block(R_arrayavg,0,strlen(W_arrayavg)); /* Read EEPROM from
address 0 */
msec = 0;
abreads = 0;
currentBpm = 0;
}
}
if (choice2 ==4){
mode = 0;
mode2 =0;
choice = 0;
choice2 = 0;
msec = 0;
exit1 = 1;
menu();
OLED_Clear();
}
}
void timestamp(){
timestamp_hr(&hrs);
timestamp_min(&min);
timestamp_sec(&sec);
itoa(hrs, tstr, 10);
itoa(min, tstr2, 10);
itoa(sec, tstr3, 10);
for(i = 0; i < 10; i++){
W_arrayhr[i] = tstr[i];
}
memset(R_arrayhr,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arrayhr,0,strlen(W_arrayhr)); /* Write W_array from EEPROM
address 0 */
eeprom_read_block(R_arrayhr,0,strlen(W_arrayhr));
// Store EEPROM MIN
for(i = 0; i < 10; i++){
W_arraymin[i] = tstr2[i];
}
memset(R_arraymin,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arraymin,0,strlen(W_arraymin));
eeprom_read_block(R_arraymin,0,strlen(W_arraymin)); //Read EEPROM from address 0
//STORE EEPROM SEC
for(j = 0; j < 10; j++){
W_arraysec[j] = tstr3[j];
}
memset(R_arraysec,0,10);
eeprom_busy_wait();
eeprom_write_block(W_arraysec,0,strlen(W_arraysec)); /* Write W_array from EEPROM
address 0 */
eeprom_read_block(R_arraysec,0,strlen(W_arraysec)); /* Read EEPROM from address 0 */
}
void calculateBPM2()
{
checkMode2();
//sensorValue = ReadADC(3);
if ((bit_is_clear(PINB,1)) || (bit_is_clear(PINB,2)))
{
if(time1 == 0)
{
time1 = millis(); // Ensures time1 is given a value when the
program begins.
while (sensorValue > 500) // Ensures only one time is recorded for
each pulse.
{
sensorValue = ReadADC(3);
}
}
else
{
time2 = millis();
/* if (time1 < 0){
PORTB &= ~(1<<0);
} */
period = (time2 - time1);
periodInSeconds = period/1000;
/* itoa(time1, time_1, 10);
OLED_SetCursor (5,1);
OLED_Printf ("TIME1:");
OLED_Printf (time_1);
itoa(time2, time_2, 10);
OLED_SetCursor (6,1);
OLED_Printf ("TIME2:");
OLED_Printf (time_2);
dtostrf(periodInSeconds,4, 1, str3);
OLED_SetCursor (2,1);
OLED_Printf ("Period:");
OLED_Printf (str3);
*/
if (periodInSeconds > 0.6 && periodInSeconds < 1){
currentBpm = (60.00/periodInSeconds);
}
num++;
avgbpm += currentBpm;
time1 = time2;
while (sensorValue > 500) // Ensures only one time is recorded
for each pulse
{
sensorValue = ReadADC(3);
}
_delay_ms(1000);
}
}
else{
OLED_SetCursor(4,1);
OLED_Printf("Check Connection");
}
}
void calculateBPM()
{
checkMode2();
//sensorValue = ReadADC(3);
if ((bit_is_clear(PINB,1)) || (bit_is_clear(PINB,2)))
{
if(time1 == 0)
{
time1 = millis(); // Ensures time1 is given a value when the
program begins.
while (sensorValue > 500) // Ensures only one time is recorded for
each pulse.
{
sensorValue = ReadADC(3);
}
}
else
{
time2 = millis();
/* if (time1 < 0){
PORTB &= ~(1<<0);
} */
period = (time2 - time1);
periodInSeconds = period/1000;
/* itoa(time1, time_1, 10);
OLED_SetCursor (5,1);
OLED_Printf ("TIME1:");
OLED_Printf (time_1);
itoa(time2, time_2, 10);
OLED_SetCursor (6,1);
OLED_Printf ("TIME2:");
OLED_Printf (time_2);
dtostrf(periodInSeconds,4, 1, str3);
OLED_SetCursor (2,1);
OLED_Printf ("Period:");
OLED_Printf (str3); */
if (periodInSeconds >= 0.5 && periodInSeconds < 1){
currentBpm = (60.00/periodInSeconds);
}
num++;
avgbpm += currentBpm;
time1 = time2;
while (sensorValue > 500) // Ensures only one time is recorded
for each pulse
{
sensorValue = ReadADC(3);
}
_delay_ms(1000);
}
}
else{
OLED_SetCursor(4,1);
OLED_Printf("Check Connection")