/* 
 * File:   eeprom_workaround.h
 * Author: Gokul
 *
 * Created on February 7, 2017, 2:33 PM
 */

#ifndef EEPROM_WORKAROUND_H
#define	EEPROM_WORKAROUND_H

unsigned char Eeprom_ReadByte(unsigned int address);
void Eeprom_WriteByte(unsigned int address, unsigned char data);

