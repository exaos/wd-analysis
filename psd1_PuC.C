

//==================================================
void psd1_PuC()
{
  gROOT->LoadMacro("digipulse.c+g");
  // gROOT->LoadMacro("libdigipulse.so");
  // gROOT->LoadMacro("libdigipulse.so");
  gROOT->LoadMacro("wd_calc.cpp");

  ParaPulse_t p_wave;
  ParaPSD1_t  p_psd1;
  
  p_wave.bAutoBase = true;
  p_wave.polar = p_m;
  p_wave.fBase = 1024;
  p_wave.fBinResolution = 1;
  p_wave.fVResolution = 1;
  p_wave.fSwing = 3;
  p_wave.fThreshold = 10;
  p_wave.fTrigger = 0.4;
  
  // p_psd1.fT1 = -10;
  // p_psd1.fT2 =   0;
  // p_psd1.fT3 =  30;
  // p_psd1.fT4 =  50;
  // wd_calc( "../BC501-20150323/BC501_20150323_PuC_run6.root", p_wave, p_psd1,
  //          "BC501_PuC_par1.root" );

  // p_psd1.fT1 = -10;
  // p_psd1.fT2 = 2;
  // p_psd1.fT3 = 30;
  // p_psd1.fT4 = 50;
  // wd_calc( "../BC501-20150323/BC501_20150323_PuC_run6.root", p_wave, p_psd1,
  //          "BC501_PuC_par2.root" );
  
  p_psd1.fT1 = -10;
  p_psd1.fT2 = 8;
  p_psd1.fT3 = 60;
  p_psd1.fT4 = 60;
  wd_calc( "../BC501-20150323/BC501_20150323_PuC_run6.root", p_wave, p_psd1,
           "BC501_PuC_par3.root" );
  
  p_psd1.fT1 = -10;
  p_psd1.fT2 = 10;
  p_psd1.fT3 = 60;
  p_psd1.fT4 = 60;
  wd_calc( "../BC501-20150323/BC501_20150323_PuC_run6.root", p_wave, p_psd1,
           "BC501_PuC_par4.root" );

  p_psd1.fT1 = -10;
  p_psd1.fT2 = 8;
  p_psd1.fT3 = 90;
  p_psd1.fT4 = 90;
  wd_calc( "../BC501-20150323/BC501_20150323_PuC_run6.root", p_wave, p_psd1,
           "BC501_PuC_par5.root" );

  p_psd1.fT1 = -10;
  p_psd1.fT2 = 10;
  p_psd1.fT3 = 90;
  p_psd1.fT4 = 90;
  wd_calc( "../BC501-20150323/BC501_20150323_PuC_run6.root", p_wave, p_psd1,
           "BC501_PuC_par6.root" );

  p_psd1.fT1 = -10;
  p_psd1.fT2 = 10;
  p_psd1.fT3 = 100;
  p_psd1.fT4 = 100;
  wd_calc( "../BC501-20150323/BC501_20150323_PuC_run6.root", p_wave, p_psd1,
           "BC501_PuC_par7.root" );
}

//============================================================
