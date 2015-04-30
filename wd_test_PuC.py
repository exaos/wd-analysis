#!/usr/bin/env python
# -*- coding: utf-8 -*- 
'''
@Exaos
'''

import ROOT

ROOT.gROOT.LoadMacro("digipulse.c++g")
ROOT.gROOT.LoadMacro("wd_calc.cpp")

para_p=ROOT.ParaPulse_t()

para_p.ePolar = ROOT.ep_m
para_p.bAutoBase = True
para_p.fBase     = 1024
para_p.fBinResolution = 1
para_p.fVResolution   = 1
para_p.fSwing   = 3
para_p.fTrigger = 0.4
para_p.fThreshold = 10
para_p.fDCoffset  = 0.
para_p.bGate  = True
para_p.fsGate.start = 50
para_p.fsGate.end   = 350
para_p.pf_ocut  = ROOT.NULL
para_p.pf_ucut  = ROOT.NULL

bExtP = True

bPSD  = True
para_psd = ROOT.ParaPSD_QR_t()

para_psd.fT1 = -10
para_psd.fT2 = -10
para_psd.fT3 =  10
para_psd.fT4 =  100

T = ROOT.TChain("wdPulse")
T.Add("test/wd-BC501-PuC.root")

ROOT.wd_calc( T, para_p, "tmp.root", bExtP, bPSD, para_psd,
              "BC501", "pq_BC501", "Pulse of BC501", "RECREATE" )



