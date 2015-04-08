#ifndef DIGIPULSE_H
#define DIGIPULSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
  
//====================
// A typical pulseform header
typedef struct PulseHeader {
  uint32_t   nLen;
  uint32_t   nBoardID;
  uint32_t   nChannel;
  uint32_t   nEventID;
  uint32_t   nPattern;
  uint32_t   nTimeStamp;
  uint32_t   nDCoffset;
} PulseHeader_t;

// A typical pulseform structure
// The resolution often used for digitizer: 8/10/12/13/14-bit
typedef struct PulseForm {
  int   ndigi;  // choice: 1,2,4 -- 8,16,32-bit; typical: 2
  int   nlen;   // data length
  void *data;   // the pointer to data array
} PulseForm_t;

// -- for 8-bit digitizer
typedef struct PulseForm8 {
  int       nlen;
  uint8_t  *data;
} PulseForm8_t;

// -- for 10~16-bit digitizer
typedef struct PulseForm16 {
  int       nlen;
  uint16_t *data;
} PulseForm16_t;

//====================
// Units mostly used
// - time   -- ns
// - heght  -- mV
// - charge -- C, or Coulomb
typedef struct PulseQuantity { // Physical quantities from Pulseform
  double  fBase;    // baseline (mV)
  double  fSwing;   // fluctuation of the baseline (mV)
  double  fPeak;    // peak position (ns)
  double  fPeakH;   // peak height (mV)
  double  fPpre;    // rising/falling time previous to the peak (ns)
  double  fPpost;   // falling/rising time after the peak (ns)
  double  fFWHM;    // FWHM -- full width at half the maximal (ns)
  double  fTrigger; // trigger position: (ns)
  double  fTpre;    // duration before the trigger (ns)
  double  fTpost;   // duration after the trigger (ns)
  double  fQ;       // integral of the pulseform, corresponding to total charge
  double  fQtot;    // sum up all points
  double  fQpre;    // charge before the peak (above threshold)
  double  fQpost;   // charge after the peak (above threshold)
} PulseQuantity_t;

//====================
// Parameters for analysis
typedef enum pulsepolarity { p_m=1, p_p=2, p_bi=3, p_auto=4 } PulsePolar_t;

typedef bool (*ptrCutFuncO)(int, uint32_t *);
typedef bool (*ptrCutFuncU)(int, int *);

typedef struct ParameterPulse {
  double  fDCoffset;      // DC offset (unit: mV)
  double  fTrigger;       // trigger: N%
  double  fThreshold;     // threshold (unit: mV)
  double  fBinResolution; // the bin resolution (unit: ns; default: 1)
  double  fBase;          // baseline (unit: mV)
  double  fVResolution;   // the voltage resolution (uint: mV; default: 1)
  double  fSwing;         // the fluctuation of baseline (unit: mV)
  
  // software gate: (start, end); (unit: ns)
  struct Gate {
    double start;
    double end;
  } fGate;
  bool    bAutoBase;      // automatically search the baseline or not
  PulsePolar_t polar;     // pulse polarity: 1=-, 2=+, 3=bipolar
  
  // the cut function: bool p_cut(int, int*)
  ptrCutFuncO  fp_ocut;   // cut for original data
  ptrCutFuncU  fp_ucut;   // Cut for unified data
} ParaPulse_t;

//====================
// PSD1
typedef struct PulsePSD1 {
  double  fPSD1;    // pulse-shape discrimination for particles, usually n/gamma
  double  fQ1;
  double  fQ2;
} PulsePSD1_t;

typedef struct ParaPSD1 {
  double  fTrigger;
  double  fT1;
  double  fT2;
  double  fT3;
  double  fT4;
} ParaPSD1_t;

//====================

typedef struct ExtremeValue {
  int  idx_min;
  int  i_min;
  int  idx_max;
  int  i_max;
} ExtremeVal_t;

typedef struct ExtremeValues {
  int  nlen;
  int *idx;
  int *val;
} ExtremeValues_t;

//============================================================
// cast data array and prepare for analysis
bool cast_data(uint32_t **d_orig, int **d_usig, double **d_smth, ExtremeVal_t *ev,
               const PulseForm_t *pulse, ParaPulse_t *parap);

// get pulse quantity
bool get_quantity(PulseQuantity_t **pq,
                  const PulseForm_t *pulse, ParaPulse_t *parap);

// get pulse psd1
bool get_psd1(PulsePSD1_t **psd1,
              const PulseForm_t *pulse, ParaPulse_t *parap, ParaPSD1_t *ppsd1);


#ifdef __cplusplus
};
#endif

#endif

