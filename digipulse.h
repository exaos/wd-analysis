#ifndef __DIGIPULSE_H__
#define __DIGIPULSE_H__

#ifdef __cplusplus
extern "C" {
#endif
 
#ifdef __linux__
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#endif // __linux__
  
#ifdef __WIN32__
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8  uint8_t;
#endif // __WIN32__
  
  // Units mostly used
  // - time   -- ns
  // - heght  -- mV
  // - charge -- C, or Coulomb
  
  //============================================================
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
    int    ndigi;  // choice: 1,2,4 -- 8,16,32-bit; typical: 2
    int    nlen;   // data length
    void  *data;   // the pointer to data array
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

  // data point in pulse
  typedef struct PulsePoint {
    int  i; // index
    int  v; // value
  } PulsePoint_t;

  //============================================================

  // Parameters for analysis
  typedef enum PulsePolarity { p_m=1, p_p=2, p_bi=3, p_auto=4 } PulsePolar_t;

  // function pointer type to cut the original pulse, possible:
  //   bool cut_orig_data( int *nlen, const uint32_t d_orig );
  typedef bool (*ptrCutFuncO)(int *, const uint32_t *);
  // function pointer type to cut the unified pulse, possible:
  //   bool cut_usig_data( int *nlen, const int d_usig );
  typedef bool (*ptrCutFuncU)(int *, const int *);
  // function pointer to smooth data, possible:
  //   bool smooth_data( double **data, const int *nlen, const int *orig_data );
  // typedef bool (*ptrFuncSMTH)(double **, const int *, const int *);

  typedef struct ParameterPulse {    
    // pulse polarity: 1=-, 2=+, 3=bipolar
    PulsePolar_t polar;
      
    // automatically search the baseline or not
    bool    bAutoBase;
    double  fBase;          // baseline (unit: mV)

    double  fBinResolution; // the bin resolution (unit: ns; default: 1)
    double  fVResolution;   // the voltage resolution (uint: mV; default: 1)
    double  fSwing;         // the fluctuation of baseline (unit: mV)
    double  fTrigger;       // trigger: N%
    double  fThreshold;     // threshold (unit: mV)
    double  fDCoffset;      // DC offset (unit: mV)
    
    // software gate: (start, end); (unit: ns)
    bool   bGate; // switcher to use software gate
    struct Gate {
      double start;
      double end;
    } fGate;

    // parameters for smoothing data
    // bool         bSmooth; // switcher to smooth data
    // ptrFuncSMTH  fp_smth; // function to smooth data
    
    // the cut function: bool p_cut(int, int*)
    ptrCutFuncO  fp_ocut; // cut for original data
    ptrCutFuncU  fp_ucut; // Cut for unified data
  } ParaPulse_t;
  
  // Pulse portrait: most common quantities
  typedef struct PulsePortrait {
    double  fBase;    // baseline (mV)
    double  fSwing;   // fluctuation of the baseline (mV)
    double  fPeak;    // peak position (ns)    
    double  fPeakH;   // peak height (mV)
    double  fTrigger; // trigger position: (ns)
  } PulsePortrait_t;

  // cast data array and prepare for analysis
  bool dp_cast_data(   uint32_t **d_orig, int **d_usig,
                       int *nlen, PulsePortrait_t *ppt,
                       const PulseForm_t *pulse, ParaPulse_t *para_p );
  bool dp_cast_data_8( uint32_t **d_orig, int **d_usig, 
                       int *nlen, PulsePortrait_t *ppt,
                       const PulseForm8_t *pulse, ParaPulse_t *para_p );
  bool dp_cast_data_16( uint32_t **d_orig, int **d_usig,
                        int *nlen,  PulsePortrait_t *ppt,
                        const PulseForm16_t *pulse, ParaPulse_t *para_p );

  //============================================================

  // extented portrait: ...
  typedef struct PulsePortraitExt {
    double  fPpre;    // rising/falling time previous to the peak (ns)
    double  fPpost;   // falling/rising time after the peak (ns)
    double  fFWHM;    // FWHM -- full width at half the maximal (ns)
    double  fTpre;    // above threshold: duration before the trigger (ns)
    double  fTpost;   // under threshold: duration after the trigger (ns)
  } PulsePortraitExt_t;

  // get extended portrait of the pulse
  bool dp_get_portrait_ext( PulsePortraitExt_t *ppet, int *nlen, const int *d_sig,
                            PulsePortrait_t *ppt, ParaPulse_t *para_p );

  //============================================================

  // some integrals
  typedef struct PulseQ {
    double  fQ;       // integral above the threshold
    double  fQtot;    // sum up all points
    double  fQpre;    // charge before the peak (above threshold)
    double  fQpost;   // charge after the peak (above threshold)
  } PulseQ_t;

  // get charge
  bool dp_get_q( PulseQ_t *pq, int *nlen, const int *d_sig,
                 PulsePortrait_t *ppt, ParaPulse_t *para_p );

  //============================================================

  // Pulse-shape discrimination: ratio of components
  typedef struct PulsePSD_QR {
    double  fPSD;    // ratio of components: fQtail / fQlong
    double  fQlong;  // component: long gate
    double  fQshort; // component: short gate
    double  fQtail;  // component: long - short
  } PulsePSD_QR_t;

  // parameters for calculate PSD_QR
  //   - long gate:  T1~T4
  //   - short gate: T2-T3
  // Usually, T1==T2.
  typedef struct ParaPSD_QR {
    double  fT1;
    double  fT2;
    double  fT3;
    double  fT4;
  } ParaPSD_QR_t;

  // get PSD using ratio of components
  bool dp_get_psd_qr( PulsePSD_QR_t *psd, int *nlen, const int *d_sig,
                      PulsePortrait_t *ppt, ParaPulse_t *para_p,
                      ParaPSD_QR_t *para_psd );

  // get PSD using ratio of components: fix range
  bool dp_get_psd_qr_fib( PulsePSD_QR_t *psd, int *nlen, const int *d_sig,
                          PulsePortrait_t *ppt, ParaPulse_t *para_p );


#ifdef __cplusplus
};
#endif

#endif // __DIGIPULSE_H__

