#include "DetectorConst.hpp"
#include <cmath>

// ===================================================================================
// ===================================================================================
// ============           Scales of detectors                             ============
// ============            Correction Factors                             ============
// ============      Length below are basically expressed in "mm"         ============
// ============            Excemptions must be expressed.                 ============
// ============              N.Chikuma 2016/06/26                         ============
// ===================================================================================
// ===================================================================================

//run mode (option -m)
int PROTON            = 2;
int HORIZONTAL        = 3;
int VERTICAL          = 4;
int INGWaterModule    = 5;
int INGWaterModuleBG  = 6;
int B2Water           = 7;
int B2CH              = 8;
int B2BG              = 9;
int B2BGd1ING         = 10;

//module ID
int MOD_INGRID_H      = 6;  //0-6
int MOD_INGRID_V      = 13; //7-13
int MOD_PM            = 16;
int MOD_ONAXIS_WM     = 20;
int MOD_B2_WM         = 21;
int MOD_B2_CH         = 22;
int MOD_B2_INGRID     = 23;

int SideView = 1;
int TopView  = 0;
int SideView_i = 0;
int TopView_i  = 1;

int NUMMAX_MODULE = 30;
int NUMINGMOD     = 14;
int NUMB2MOD      = 3;

//channel ID normalization
int NORMMOD = 2000;
int NORMPLN = 100;

double PE_THRESHOLD = 0.;
double VETO_PE_THRESHOLD = 2.5;

const double PI = M_PI;

// set mod# put in B2MotherLV
// 0-6:ING_h, 7-13:ING_v, 16:PM, 20:OnAxisWM,
// 21:B2WM, 22:B2CH, 23:B2d1ING, 24:B2d2ING, 25:B2s1ING, 26:B2s2ING
//const int NotPlaceMod[3] = {24,26,30};
const int NotPlaceMod[4] = {24,25,26,30};

// Detector Overlap checking

const bool check_MotherLV = true;
const bool check_moduleLV = true;
const bool check_ING      = false;
const bool check_PM       = false;
const bool check_WM       = false;
const bool check_CHM      = false;

const double C_WorldSizeX = 40000./2.; //mm //half of each side
const double C_WorldSizeY = 40000./2.; //mm
const double C_WorldSizeZ = 40000./2.; //mm

// =============================================
// =========      ND Hall           ============
// =============================================
// -------- Positions in World -----------------
const double C_HallDirtPosX      = -2167.;  //mm
const double C_HallDirtPosY      =     0.;  //mm
const double C_HallDirtPosZ      =  1700.;  //mm
const double C_PillarPosX        = -8235.;  //mm
const double C_PillarPosY        = -4443.;  //mm
const double C_PillarPosZ        =  2631.;  //mm
const double C_B2FloorPosY       = -5943.;  //mm
// --------    Sizes     -----------------
// The dimension of Hall dirt volume
const double C_HallDirtRadiusMin =  8500.;  //mm 
const double C_HallDirtRadiusMax = 13200.;  //mm
const double C_HallDirtHeight    = 10000.;  //mm
const double C_HallDirtSPhi      = 0.;
const double C_HallDirtDPhi      = 2.*PI;
// Pillar
const double C_PillarX           = 1000./2.;  //mm //half of each side
const double C_PillarY           = 3000./2.;  //mm
const double C_PillarZ           = 4000./2.;  //mm


// =============================================
// =========    Mother Volumes      ============
// =============================================
// -------- Positions in World -----------------
// INGRID 7 horizontal modules
const double C_INGHMotherPosX =     0.;  //mm
const double C_INGHMotherPosY =     0.;  //mm
const double C_INGHMotherPosZ =     0.;  //mm
// INGRID 7 vertical modules
const double C_INGVMotherPosX =     0.;  //mm
const double C_INGVMotherPosY =     0.;  //mm
const double C_INGVMotherPosZ = -4000.;  //mm
// Proton Module Position & On-axis WaterModule
const double C_PMMotherPosX   =     0.;  //mm
const double C_PMMotherPosY   =     0.;  //mm
const double C_PMMotherPosZ   = -1200.;  //mm
// B2 modules 
const double C_B2MotherPosX   = -5735.;  //mm
const double C_B2MotherPosY   = 1000.+C_B2FloorPosY;  //mm
const double C_B2MotherPosZ   =  2633.;  //mm

// --------    Sizes of box    -----------------
// INGRID 7 horizontal modules 
const double C_INGHMotherSizeX = 10500./2.;  //mm //half of each side
const double C_INGHMotherSizeY =  2000./2.;  //mm
const double C_INGHMotherSizeZ =  1360./2.;  //mm
// INGRID 7 vertical modules
const double C_INGVMotherSizeX =  2000./2.;  //mm //half of each side
const double C_INGVMotherSizeY = 10500./2.;  //mm
const double C_INGVMotherSizeZ =  1360./2.;  //mm
// Proton Module Position & On-axis WaterModule
const double C_PMMotherSizeX   =  2000./2.;  //mm //half of each side
const double C_PMMotherSizeY   =  2000./2.;  //mm
const double C_PMMotherSizeZ   =   960./2.;  //mm
// B2 modules 
const double C_B2MotherSizeX   =  1500./2.;  //mm //half of each side
const double C_B2MotherSizeY   =  1500./2.;  //mm
const double C_B2MotherSizeZ   =  3000./2.;  //mm


// ==========================================================================
// ================== Modules Volume  =======================================
// ==========================================================================
// ------------- Positions ---------------------------
// Positions between each INGRID modules in "INGH(V)Mother"
const double C_INGSpace    =  1500.;  //mm // b/w centers of two adjacent modules
const double C_INGStart    = -4500.;  //mm // center of the first module
// Each sub-module in "B2Mother" 
const double C_B2WMPosX    =     0. ;  //mm
const double C_B2WMPosY    =     0. ;  //mm
const double C_B2WMPosZ    =  -480. ;  //mm
const double C_B2CHPosX    =     0. ;  //mm
const double C_B2CHPosY    =     0. ;  //mm
const double C_B2CHPosZ    = -1250.1;  //mm
const double C_B2d1INGPosX =     0. ;  //mm
const double C_B2d1INGPosY =  -100. ;  //mm
const double C_B2d1INGPosZ =   500.1;  //mm
const double C_B2d2INGPosX =   750.1;  //mm
const double C_B2d2INGPosY =     0. ;  //mm
const double C_B2d2INGPosZ =   650.1;  //mm
const double C_B2s1INGPosX = -1500.1;  //mm //right
const double C_B2s1INGPosY =     0. ;  //mm 
const double C_B2s1INGPosZ =  -480. ;  //mm
const double C_B2s2INGPosX =  1625.1;  //mm //left
const double C_B2s2INGPosY =     0. ;  //mm
const double C_B2s2INGPosZ =  -750. ;  //mm
// ------------ Sizes of box --------------------------
// INGRID Module volume (for each)
const double C_INGSizeX    = 1500./2.;  //mm //half of each side
const double C_INGSizeY    = 1500./2.;  //mm
const double C_INGSizeZ    = 1300./2.;  //mm
// Proton Module volume 
const double C_PMSizeX     = 1500./2.;  //mm //half of each side
const double C_PMSizeY     = 1500./2.;  //mm
const double C_PMSizeZ     =  940./2.;  //mm
// Water/CH Module volume (both OnAxis/B2)
const double C_WMSizeX     = 1500./2.;  //mm //half of each side
const double C_WMSizeY     = 1500./2.;  //mm
const double C_WMSizeZ     =  600./2.;  //mm


// =============================================
// =========          INGRID        ============
// =============================================
const int    C_INGNumPln          =   11    ;
const int    C_INGNumCh           =   24    ;
const int    C_INGNumVetoCh       =   22    ;
const double C_INGPlnDist         =  107.   ;  //mm // b/w two adjacent planes
const double C_INGPlnStart        = -540.   ;  //mm // center of 1st vertical scinti plane(pln#0)
const double C_INGChStart         = -575.   ;  //mm // center of ch#0
const double C_INGIronStart       = -481.5  ;  //mm // center of 1st iron
// -------- Detector dimenstion ----------------
// Iron
const double C_INGIronThick       =    65.  ;  //mm
const double C_INGIronXY          =  1240.  ;  //mm
const double C_INGTotMassIron     =    99.54;  //ton
// Scintillators
const double C_INGScintiLength    =  1200.  ;  //mm
const double C_INGScintiWidth     =    50.  ;  //mm
const double C_INGScintiThick     =    10.  ;  //mm
const double C_INGScintiHoleDia_a =     1.3 ;  //mm
const double C_INGScintiHoleDia_b =     1.95;  //mm
const double C_INGTotMassScinti   =     3.74;  //ton
// 8 Vertices of Scintillator Cross Section for Extruded Solid
const double C_INGScintiVertexX[8] = { -23.616 ,-24.389,-24.389,-23.616, 
                                             23.616, 24.389, 24.389, 23.616}; //mm
const double C_INGScintiVertexY[8] = {  -4.71  , -0.5  ,  0.5  ,  4.71, 
                                             4.71  ,  0.5  , -0.5  , -4.71 }; //mm
// Long veto planes
const double C_INGLVetoLength     =  1300.  ;  //mm
const double C_INGLVetoWidth      =    50.  ;  //mm
const double C_INGLVetoThick      =    10.  ;  //mm
const double C_INGVetoStartZ      =  -525.  ;  //mm center of 1st veto pln
// Short veto planes
const double C_INGSVetoLength     =  1120.  ;  //mm 
const double C_INGSVetoWidth      =    50.  ;  //mm 
const double C_INGSVetoThick      =    10.  ;  //mm 
// Veto Positions for INGRID
const double C_INGVetoPos1X       =    59.  ;  //mm //top
const double C_INGVetoPos1Y       =   684.  ;  //mm
const double C_INGVetoPos2X       =    -9.  ;  //mm //bottom
const double C_INGVetoPos2Y       =  -659.  ;  //mm
const double C_INGVetoPos3X       =   709.  ;  //mm //left
const double C_INGVetoPos3Y       =     3.  ;  //mm
const double C_INGVetoPos4X       =  -705.75;  //mm //right
const double C_INGVetoPos4Y       =   -37.  ;  //mm


// =============================================
// =========      Proton Module     ============
// =============================================
const int    C_PMNumPln            =   18    ;
const int    C_PMNumVetoPln        =    4    ;
const int    C_PMNumVetoCh         =   17    ;
const int    C_PMNumCh             =   32    ;
const int    C_PMNumCh_mid         =   16    ;  //Num of scibar type scinti
const int    C_PMNumCh_side        =    8    ;  //Num of INGRID type scinti for each side
const int    C_PMNumCh1            =   24    ; //Num of ch for 1st pln
const double C_PMPlnDist           =   23.   ;   //mm // b/w two adjacent H pln & V pln
const double C_PMPlnDist_First     =   27.   ;   //mm // b/w 1st V pln & 2nd H pln
const double C_PMPlnStart          = -404.5  ;   //mm // center of 1st horizontal plane(pln#0)
const double C_PMChStart           = -575.   ;   //mm // center of ch#0
// -------- Detector dimenstion ----------------
// Scintillators
const double C_PMScintiLength      =  1200.  ;   //mm
const double C_PMScintiWidth       =    25.  ;   //mm
const double C_PMScintiThick       =    13.  ;   //mm
const double C_PMScintiHoleDia     =     0.9 ;   //mm;
const double C_PMTotMassScinti     = 0.56904 ;   //ton
const double C_PMTotMassVetoSci    = 0.028848;   //ton
// 8 Vertices of Scintillator Cross section for Extruded Solid
const double C_PMScintiVertexX[8]  = {-11.672,-12.21 ,-12.21 ,-11.672, 
                                              11.672, 12.21 , 12.21 , 11.672}; //mm
const double C_PMScintiVertexY[8]  = { - 6.17, -3.5  ,  3.5  ,  6.17 , 
                                                6.17,  3.5  , -3.5  , -6.17}; //mm
// Long veto planes
const double C_PMLVetoLength       =  1250.   ; //mm 
const double C_PMLVetoWidth        =    50.   ; //mm 
const double C_PMLVetoThick        =    10.   ; //mm 
const double C_PMVetoStartZ        =  -400.   ; //mm // center of 1st veto pln
// Short veto planes
const double C_PMSVetoLength       =  1200.   ; //mm 
const double C_PMSVetoWidth        =    50.   ; //mm 
const double C_PMSVetoThick        =    10.   ; //mm 
// Veto Positions for ProtomModule
const double C_PMVetoPos1X         =    -5.   ;  //mm //top
const double C_PMVetoPos1Y         =   655.   ;  //mm
const double C_PMVetoPos2X         =    15.   ;  //mm //bottom
const double C_PMVetoPos2Y         =  -655.   ;  //mm
const double C_PMVetoPos3X         =   655.   ;  //mm //left
const double C_PMVetoPos3Y         =   -25.   ;  //mm
const double C_PMVetoPos4X         =  -655.   ;  //mm //right
const double C_PMVetoPos4Y         =     5.   ;  //mm



// =============================================
// =========      WAGASCI           ============
// =============================================
const int    C_WMNumView           =     2    ;
const int    C_WMNumPln            =     8    ;
const int    C_WMNumCh             =    80    ;
const int    C_WMNumLayer          =    16    ;
const int    C_WMNumXYLayerCh      =    40    ;
const int    C_WMNumGridCh         =    20    ;  // # of ch of grid for each X or Y layer
const double C_WMPlnDist           =    57.   ;  //mm // b/w two adjacent X-layers (planes)
const double C_WMLayerDist         =    28.5  ;  //mm // b/w two adjacent X and Y-layers (layers)
const double C_WMPlnStart          =  -226.5  ;  //mm // center of 1st X-layer(pln#0 side view)
const double C_WMChStart           =  -487.5  ;  //mm // center of ch#0
const double C_WMGridChStart       =  -474.9  ;  //mm // center of ch#40 & ch#60
// -------- Detector dimenstion ----------------
// Water target
const double C_WMWaterTargetSizeX  =  1256.   ;  //mm
const double C_WMWaterTargetSizeY  =  1256.   ;  //mm
const double C_WMWaterTargetSizeZ  =   466.   ;  //mm
// CH target
const double C_WMCHCellTargetSizeX =  4.6  ;  //mm
const double C_WMCHCellTargetSizeY =  4.6  ;  //mm
const double C_WMCHCellTargetSizeZ =  2.5  ;  //mm

// Scintillators
const double C_WMScintiLength      =  1000.   ;  //mm
const double C_WMScintiWidth       =    25.   ;  //mm
const double C_WMScintiThick       =     3.   ;  //mm
const double C_WMTrueScintiWidth   =    22.5   ;  //mm
const double C_WMTrueScintiThick   =     2.8   ;  //mm
// groove for fiber
const double C_WMScintiHoleLength  =  1000.1  ;  //mm 
const double C_WMScintiHoleWidth   =    1.21  ;  //mm
const double C_WMScintiHoleThick   =    1.21  ;  //mm
const double C_WMScintiHoleShift1  =    3.9   ;  //mm
const double C_WMScintiHoleShitt2  =    0.9   ;  //mm
// slit for 3D grid strudture
const double C_WMScintiSlitLength  =     3.5  ;  //mm 
const double C_WMScintiSlitWidth   =    13.1  ;  //mm
const double C_WMScintiSlitThick   =     3.1  ;  //mm
const double C_WMScintiSlitStep    =     50.  ;  //mm
const double C_WMScintiSlitShift   =     5.5  ;  //mm
// Margin from scintillator edge to bundle(MPPC)
const double C_WMFiberMargin       =   200.   ;  //mm
// 8 Vertices of Scintillator Cross section for Extruded Solid
const double C_WMScintiVertexX[8] = { -10.8, -12.0, -12.0, -10.8, 
                                              10.8,  12.0,  12.0,  10.8}; //mm
const double C_WMScintiVertexY[8] = { - 1.5, - 1.0,   1.0,   1.5, 
                                               1.5,   1.0,  -1.0,  -1.5};  //mm
// Other components
// Aluminum tank dimension (water tank)
const double C_WMAlTankSizeX       =  1456.   ;  //mm
const double C_WMAlTankSizeY       =  1456.   ;  //mm
const double C_WMAlTankSizeZ       =   606.   ;  //mm



// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------


// =======================================================
// =========      Detecor Response           =============
// =======================================================

// Energy deposit correction factors
const double C_Corr_Birks       =   0.0208; // used in SciBooNE MC
// Scinti & Fiber
const double C_ScintiAttLeng    =  10.46  ; //cm
const double C_WMScintiAttLeng  =   4.0   ; //cm added
const double C_FiberAttLeng     = 241.7   ; //cm
const double C_TransTimeInFiber =   1./28.; //  1cm/2.8e10[cm/s] * 1e9 [ns]
const double C_INGRIDFactor     =   1.08  ; //P.E. factor for INGRID scintillator
const double C_ScibarFactor     =   1.77  ; //P.E. factor for SciBar scintillator
const double C_WMFactor         =   3.    ; //P.E. factor for WAGASCI scintillator
// MPPC
const double C_MeV2PE           =  45.9   ; // v3r4
const double C_MPPCPixel        = 667.    ; //v3
const double C_Eff_PDE          =  -0.275 ; //*1.134;  // @deltaV = 1.1 V
const double C_ETC_CONST        = 5.0     ;  // v3
const double C_rPDE             = 1.7     ;  // @deltaV = 1.1 V
const double C_CrossAfterRate   = 0.09    ;//*1.051;  // @deltaV = 1.1 V
const double C_PixelGainVari    = 0.031   ;  // gain variation among pixels
// ADC
const double C_Pedestal         = 0.      ;//145;  // pedeltal of ADC counts
const double C_Gain             = 10.     ;  // Gain ADC counts of high gain channel
const double C_LowGain          = 1.      ;  // Gain ADC counts of low gain channel
const double C_ADCtoCharge      = 135.5   ;  // ADC to Charge
const double C_LowADCtoCharge   = 14.29   ;  // ADC to Charge
const double C_LowGain_corr     = 14.29/13.55;
const double C_NonLinADCTh[4]   = {0.65 ,  3.2,   4.2,   14.};
const double C_NonLinADC1[4]    = {135.5, 217., 158.6,  5.1 };
const double C_NonLinADC2[4]    = {   0., -53., 133.9, 778.6};
const double C_ADCSaturation    = 850.    ;
const double C_LowNonLinADCTh[4]= {7.   ,  27.,  35.5, 178.4};
const double C_LowNonLinADC1[4] = {14.29,  26., 21.12,   0.7};
const double C_LowNonLinADC2[4] = {   0., -82., 48.24, 775.1};
const double C_LowADCSaturation = 900.    ;
const double C_ElecNoise        = 1.7     ;  // sigma of high gain electronics noise
const double C_LowElecNoise     = 1.2     ;  // sigma of low gain electronics noise

