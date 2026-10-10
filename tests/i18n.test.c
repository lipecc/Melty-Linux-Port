/* SPDX-License-Identifier: MIT — lightweight locale-table regression test. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/i18n.h"
int main(void){
  for(int lang=0;lang<ML_LANG_COUNT;lang++){
    if(ml_from_code(ml_codes[lang])!=lang)return 10;
    for(int key=0;key<ML_MESSAGE_COUNT;key++){
      if(!ml_get(lang,key)[0])return 11;
    }
  }
  setenv("MELTY_LANG","pt-BR",1);
  if(ml_detect()!=ML_PT)return 12;
  setenv("MELTY_LANG","ja_JP.UTF-8",1);
  if(ml_detect()!=ML_JA)return 13;
  setenv("MELTY_LANG","zz",1);
  setenv("LC_ALL","C.UTF-8",1);
  setenv("LC_MESSAGES","es_ES.UTF-8",1);
  if(ml_detect()!=ML_ES)return 14;
  printf("PASS: %d UI languages, %d translated messages each\n",ML_LANG_COUNT,ML_MESSAGE_COUNT);
  return 0;
}
