#include <Arduino.h>
#include <stdint.h>
#include <cli/cli.hpp>

#include "insight/insight.hpp"
#include "version/version.h"

Cli cli;
Insight insight;
uint32_t ramp=0;
uint32_t rect=0;
uint32_t tmp = 0;
uint32_t loopCnt = 0;
float sinus=0;

const uint32_t period_ms = 100;

int8_t cmd_ver(char *argv[], uint8_t argc)
{
    Serial.printf("%s %s, Copyright (C) 2021 Julian Friedrich\n", 
         VERSION_PROJECT, VERSION_GIT_SHORT);
    Serial.printf("Build:    %s, %s\n", VERSION_DATE, VERSION_TIME);
    Serial.printf("Git Repo: %s\n", VERSION_GIT_REMOTE_ORIGIN);
    Serial.printf("Revision: %s\n", VERSION_GIT_LONG);
    Serial.printf("\n");
    Serial.printf("This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you\n");
    Serial.printf("are welcome to redistribute it under certain conditions.\n");
    Serial.printf("See GPL v3 licence at https://www.gnu.org/licenses/ for details.\n\n");

    return 0;
}

int8_t cmd_reset(char *argv[], uint8_t argc)
{
   insight.reset();
   Serial.printf("Insight reset, you have to add data to the stream now.\n");

   return 0;
}

int8_t cmd_add(char *argv[], uint8_t argc)
{
   uint32_t millis = 0;
   
   if (argc != 1)
   {
      return -1;
   }

   if (strcmp(argv[0], "ramp") == 0)
   {
      insight.add(&ramp, "ramp");
      Serial.printf("Added ramp to insight\n");
   }
   else if (strcmp(argv[0], "rect") == 0)
   {
      insight.add(&rect, "rect");
      Serial.printf("Added ramp to insight\n");
   }
   else if (strcmp(argv[0], "sinus") == 0)
   {
      insight.add(&sinus, "sinus");
      Serial.printf("Added ramp to insight\n");
   }
   else if (strcmp(argv[0], "loopCnt") == 0)
   {
      insight.add(&loopCnt, "loopCnt");
      Serial.printf("Added ramp to insight\n");
   }
   else
   {
      Serial.printf("Unknown global variable\n");
   }

   return 0;
}

int8_t cmd_addptr(char *argv[], uint8_t argc)
{
   void *ptr=0;
   dataTypes_t type;
   char *succ;

   if (argc != 3)
   {
      return -1;
   }

   ptr = (void*) strtoul(argv[0], &succ, 0);
   if (ptr == 0)
   {
      Serial.printf("ERROR: Invalid pointer!\n");
      return 0;
   }

   if (strcmp(argv[1], "u32") == 0)
   {
      type = dataType_uint_32;
   }
   else if (strcmp(argv[1], "f") == 0)
   {
      type = dataType_float;
   }
   else
   {
      Serial.printf("ERROR: Type not supported, see info on globals after reset.\n");
      return 0;
   }

   Serial.printf("Adding 0x%0x, as tpye %d with name %s\n", ptr, type, argv[2]);
   insight.add(ptr, type, argv[2]);

   return 0;
}


int8_t cmd_start(char *argv[], uint8_t argc)
{
   if (!insight.isEnabled())
   {
      insight.enable(true);
      Serial.printf("Insight enabled\n");
   }

   return 0;
}

int8_t cmd_stop(char *argv[], uint8_t argc)
{
   if (insight.isEnabled())
   {
      insight.enable(false);
      Serial.printf("Insight disabled\n");
   }

   return 0;
}

int8_t cmd_toggle(char *argv[], uint8_t argc)
{
   if (!insight.isEnabled())
   {
      insight.enable(true);
      Serial.printf("Insight enabled\n");
   }
   else
   {
      insight.enable(false);
      Serial.printf("Insight disabled\n");
   }

   return 0;
}

int8_t cmd_pause(char *argv[], uint8_t argc)
{

   if (!insight.isPaused())
   {
      insight.pause(true);
      Serial.printf("Insight pasue on\n");
   }
   else
   {
      insight.pause(false);
      Serial.printf("Insight pause off\n");
   }
   return 0;
}

int8_t cmd_setPeriod(char *argv[], uint8_t argc)
{
   uint32_t millis = 0;
   
   if (argc != 1)
   {
      return -1;
   }

   millis = strtoul(argv[0],0 ,10);

   insight.setPeriod(millis);
   Serial.printf("Insight task period set to %ums\n", insight.getPeriod());
   
   return 0;
}

int8_t cmd_status(char *argv[], uint8_t argc)
{
   Serial.printf("Insight status:\n");
   Serial.printf("Task period %ums\n", insight.getPeriod());
   Serial.printf("Stream %s\n", insight.isEnabled() ? "Enabled" : "Disabled");
   Serial.printf("Pause %s\n", insight.isPaused() ? "on" : "off");
   return 0;
}

cliCmd_t cmdTable[] =
{
   CLI_CMD_DEF(ver),
   CLI_CMD_DEF(reset),
   CLI_CMD_DEF(add),
   CLI_CMD_DEF(addptr),
   CLI_CMD_DEF(start),
   CLI_CMD_DEF(stop),
   CLI_CMD_DEF(toggle),
   CLI_CMD_DEF(pause),
   CLI_CMD_DEF(setPeriod),
   CLI_CMD_DEF(status)
};

void setup() 
{
   Serial.begin(115200);
   while (!Serial);   
   Serial.println();

   cmd_ver(0, 0);
   Serial.printf("\n");

   Serial.printf("Address and Type of global variables:\n");
   Serial.printf("ramp:    (uint32_t) 0x%0x\n", &ramp);
   Serial.printf("rect:    (uint32_t) 0x%0x\n", &rect);
   Serial.printf("sinus:   (float)    0x%0x\n", &sinus);
   Serial.printf("loopCnt: (uint32_t) 0x%0x\n", &loopCnt);
   Serial.printf("\n");
   
   insight.add(&ramp, "ramp");
   insight.add(&rect, "rect");
   Serial.printf("Added ramp and and rect to insight as default for tests.\n");
   
   insight.setPeriod(period_ms);
   Serial.printf("Insight task period set to: %ldms\n", insight.getPeriod());
   Serial.printf("\n");

   cli.begin(cmdTable);
}

void loop() 
{
   static uint32_t rampTick = 0;
   static uint32_t rectTick = 0;
   static uint32_t sinusTick = 0;
   static uint32_t loopTick = 0;
   uint32_t tick = millis();

   if (tick - rampTick > period_ms/2)
   {
      ramp++;
      ramp = ramp > 200 ? 0 : ramp;
      rampTick = tick;
   }

   if (tick - rectTick > (period_ms*5))
   {
      rect = rect == 0 ? 200 : 0; 
      rectTick = tick;
   }

   if (tick - sinusTick > period_ms)
   {
      sinus = 100 + 50 * sin(((float)tick)/2);
      sinusTick = tick;
   }

   if (tick - loopTick > period_ms)
   {
      loopCnt = tmp;
      tmp = 0;
      loopTick = tick;
   }

   insight.task(tick);
   cli.read();

   tmp++;
}