#include "tuner.h"

#include <avr/eeprom.h>
#include "../eeprom.h"

uint8_t *bufFM;
static tunerIC _tuner;

static uint16_t _freq, _fMin, _fMax;
static uint8_t _mono;
static uint8_t _step1, _step2;

void tunerInit(void)
{
	uint8_t ctrl;

	ctrl = eeprom_read_byte(eepromFMCtrl);
	_tuner = eeprom_read_byte(eepromFMTuner);
	_freq = eeprom_read_word(eepromFMFreq);
	_fMin = eeprom_read_word(eepromFMFreqMin);
	_fMax = eeprom_read_word(eepromFMFreqMax);
	_mono = eeprom_read_byte(eepromFMMono);
	_step1 = eeprom_read_byte(eepromFMStep1);
	_step2 = eeprom_read_byte(eepromFMStep2);

	if (_tuner >= TUNER_END)
		_tuner = TUNER_TEA5767;

	switch (_tuner) {
	case TUNER_TEA5767:
		tea5767Init(ctrl);
		break;
	case TUNER_RDA5807:
		rda580xInit(RDA5807_DIRECT_FREQ);
		break;
	case TUNER_TUX032:
		tux032Init();
		break;
	case TUNER_RDA5802:
		rda580xInit(RDA5807_GRID_FREQ);
		break;
	default:
		break;
	}

	tunerSetFreq(_freq);

	return;
}

tunerIC tunerGetType(void)
{
	return _tuner;
}

void tunerSetFreq(uint16_t freq)
{
	if (freq > _fMax)
		freq = _fMax;
	if (freq < _fMin)
		freq = _fMin;

	_freq = freq;

	switch (_tuner) {
	case TUNER_TEA5767:
		tea5767SetFreq(_freq, _mono);
		break;
	case TUNER_RDA5807:
		rda580xSetFreq(_freq, _mono, RDA5807_DIRECT_FREQ);
		break;
	case TUNER_TUX032:
		tux032SetFreq(_freq);
		break;
	case TUNER_RDA5802:
		rda580xSetFreq(_freq, _mono, RDA5807_GRID_FREQ);
		break;
	default:
		break;
	}

	return;
}

uint16_t tunerGetFreq(void)
{
	return _freq;
}

uint8_t tunerGetMono(void)
{
	return _mono;
}

void tunerChangeFreq(int8_t mult)
{
	uint16_t freq;

	if (mult > 0) {
		if (_freq >= 7600)
			freq = _freq + _step2 * mult;
		else
			freq = _freq + _step1 * mult;
	} else {
		if (_freq <= 7600)
			freq = _freq + _step1 * mult;
		else
			freq = _freq + _step2 * mult;
	}

	tunerSetFreq(freq);

	return;
}

void tunerReadStatus(void)
{
	switch (_tuner) {
	case TUNER_TEA5767:
		bufFM = tea5767ReadStatus();
		break;
	case TUNER_RDA5807:
	case TUNER_RDA5802:
		bufFM = rda580xReadStatus();
		break;
	case TUNER_TUX032:
		bufFM = tux032ReadStatus();
		break;
	default:
		break;
	}

	return;
}

void tunerSwitchMono(void)
{
	_mono = !_mono;

	if (_tuner == TUNER_TEA5767 ||
	    _tuner == TUNER_RDA5807 ||
	    _tuner == TUNER_RDA5807)
		tunerSetFreq(tunerGetFreq());

	return;
}

uint8_t tunerStereo(void)
{
	uint8_t ret = 1;

	switch (_tuner) {
	case TUNER_TEA5767:
		ret = TEA5767_BUF_STEREO(bufFM) && !_mono;
		break;
	case TUNER_RDA5807:
	case TUNER_RDA5802:
		ret = RDA5807_BUF_STEREO(bufFM) && !_mono;
		break;
	case TUNER_TUX032:
		ret = !TUX032_BUF_STEREO(bufFM);
		break;
	default:
		break;
	}

	return ret;
}

uint8_t tunerLevel(void)
{
	uint8_t ret = 0;
	uint8_t rawLevel;

	switch (_tuner) {
	case TUNER_TEA5767:
		ret = (bufFM[3] & TEA5767_LEV_MASK) >> 4;
		break;
	case TUNER_RDA5807:
	case TUNER_RDA5802:
		rawLevel = (bufFM[2] & RDA5807_RSSI) >> 1;
		if (rawLevel < 24)
			ret = 0;
		else
			ret = (rawLevel - 24) >> 1;
		break;
	case TUNER_TUX032:
		if (tunerStereo())
			ret = 13;
		else
			ret = 3;
		break;
	default:
		break;
	}

	return ret;
}

/* Find station number (1..50) in EEPROM */
uint8_t tunerStationNum(void)
{
	uint8_t i;

	for (i = 0; i < FM_COUNT; i++)
		if (eeprom_read_word(eepromStations + i) == _freq)
			return i + 1;

	return 0;
}

/* Find favourite station number (1..10) in EEPROM */
uint8_t tunerFavStationNum(void)
{
	uint8_t i;

	for (i = 0; i < FM_COUNT; i++)
		if (eeprom_read_word(eepromFavStations + i) == _freq)
			return i + 1;

	return 0;
}

/* Find nearest next/prev stored station */
void tunerNextStation(int8_t direction)
{
	uint8_t i;
	uint16_t freqCell;
	uint16_t freqFound = _freq;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word(eepromStations + i);
		if (freqCell != 0xFFFF) {
			if (direction == SEARCH_UP) {
				if (freqCell > _freq) {
					freqFound = freqCell;
					break;
				}
			} else {
				if (freqCell < _freq) {
					freqFound = freqCell;
				} else {
					break;
				}
			}
		}
	}

	tunerSetFreq(freqFound);

	return;
}

/* Load station by number */
void tunerLoadStation(uint8_t num)
{
	uint16_t freqCell = eeprom_read_word(eepromStations + num);

	if (freqCell != 0xFFFF)
		tunerSetFreq(freqCell);

	return;
}

/* Load favourite station by number */
void tunerLoadFavStation(uint8_t num)
{
	if (eeprom_read_word(eepromFavStations + num) != 0)
		tunerSetFreq(eeprom_read_word(eepromFavStations + num));

	return;
}

/* Load favourite station by number */
void tunerStoreFavStation(uint8_t num)
{
	if (eeprom_read_word(eepromFavStations + num) == _freq)
		eeprom_update_word(eepromFavStations + num, 0);
	else
		eeprom_update_word(eepromFavStations + num, _freq);

	return;
}

/* Save/delete station from eeprom */
void tunerStoreStation(void)
{
	uint8_t i, j;
	uint16_t freqCell;
	uint16_t freq;

	freq = _freq;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word(eepromStations + i);
		if (freqCell < freq)
			continue;
		if (freqCell == freq) {
			for (j = i; j < FM_COUNT; j++) {
				if (j == FM_COUNT - 1)
					freqCell = 0xFFFF;
				else
					freqCell = eeprom_read_word(eepromStations + j + 1);
				eeprom_update_word(eepromStations + j, freqCell);
			}
			break;
		} else {
			for (j = i; j < FM_COUNT; j++) {
				freqCell = eeprom_read_word(eepromStations + j);
				eeprom_update_word(eepromStations + j, freq);
				freq = freqCell;
			}
			break;
		}
	}

	return;
}

void tunerSetMute(uint8_t mute)
{
	switch (_tuner) {
	case TUNER_TEA5767:
		tea5767SetMute(mute);
		break;
	case TUNER_RDA5807:
	case TUNER_RDA5802:
		rda580xSetMute(mute);
		break;
	case TUNER_TUX032:
		tux032SetMute(mute);
		break;
	default:
		break;
	}

	return;
}

void tunerSetVolume(int8_t value)
{
	switch (_tuner) {
	case TUNER_RDA5807:
	case TUNER_RDA5802:
		rda580xSetVolume(value);
		break;
	default:
		break;
	}

	return;
}

void tunerPowerOn(void)
{
	switch (_tuner) {
	case TUNER_TEA5767:
		tea5767PowerOn();
		break;
	case TUNER_RDA5807:
	case TUNER_RDA5802:
		rda580xPowerOn();
		break;
	case TUNER_TUX032:
		tux032PowerOn();
		break;
	default:
		break;
	}

	tunerSetFreq(_freq);

	return;
}

void tunerPowerOff(void)
{
	eeprom_update_word(eepromFMFreq, _freq);
	eeprom_update_byte(eepromFMMono, _mono);
	eeprom_update_byte(eepromFMTuner, _tuner);

	switch (_tuner) {
	case TUNER_TEA5767:
		tea5767PowerOff();
		break;
	case TUNER_RDA5807:
	case TUNER_RDA5802:
		rda580xPowerOff();
		break;
	case TUNER_TUX032:
		tux032PowerOff();
		break;
	default:
		break;
	}

	return;
}
