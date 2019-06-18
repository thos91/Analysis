#ifndef DETECTORCONST_H_INCLUDE
#define DETECTORCONST_H_INCLUDE

// ===================================================================================
// ===================================================================================
// ============           Scales of detectors                             ============
// ============            Correction Factors                             ============
// ============      Length below are basically expressed in "mm"         ============
// ============            Excemptions must be expressed.                 ============
// ============              N.Chikuma 2016/06/26                         ============
// ===================================================================================
// ===================================================================================

#define WATERTANK_X 466  //mm
#define WATERTANK_Y 1256 //mm

//run mode (option -m)
extern int PROTON           ;
extern int HORIZONTAL       ;
extern int VERTICAL         ;
extern int INGWaterModule   ;
extern int INGWaterModuleBG ;
extern int B2Water          ;
extern int B2CH             ;
extern int B2BG             ;
extern int B2BGd1ING        ;

//module ID
extern int MOD_INGRID_H     ; //0-6
extern int MOD_INGRID_V     ; //7-13
extern int MOD_PM           ;
extern int MOD_ONAXIS_WM    ;
extern int MOD_B2_WM        ;
extern int MOD_B2_CH        ;
extern int MOD_B2_INGRID    ;

extern int SideView ;
extern int TopView  ;
extern int SideView_i ; //for INGRID, PM
extern int TopView_i ;

extern int NUMMAX_MODULE ;
extern int NUMINGMOD     ;
extern int NUMB2MOD      ;

//channel ID normalization
extern int NORMMOD ;
extern int NORMPLN ;

extern double PE_THRESHOLD ;
extern double VETO_PE_THRESHOLD ;

extern const double PI;

// set mod# put in B2MotherLV
// 0-6:ING_h, 7-13:ING_v, 16:PM, 20:OnAxisWM,
// 21:B2WM, 22:B2CH, 23:B2d1ING, 24:B2d2ING, 25:B2s1ING, 26:B2s2ING
//extern const int NotPlaceMod[3] = {24,26,30};
//
extern const int NotPlaceMod[4] ;

// Detector Overlap checking

extern const bool check_MotherLV;
extern const bool check_moduleLV;
extern const bool check_ING     ;
extern const bool check_PM      ;
extern const bool check_WM      ;
extern const bool check_CHM     ;

extern const double C_WorldSizeX ; //mm //half of each side
extern const double C_WorldSizeY ; //mm
extern const double C_WorldSizeZ ; //mm

// =============================================
// =========      ND Hall           ============
// =============================================

// -------- Positions in World -----------------
extern const double C_HallDirtPosX   ;  //mm
extern const double C_HallDirtPosY   ;  //mm
extern const double C_HallDirtPosZ   ;  //mm
extern const double C_PillarPosX     ;  //mm
extern const double C_PillarPosY     ;  //mm
extern const double C_PillarPosZ     ;  //mm
extern const double C_B2FloorPosY    ;  //mm
// --------    Sizes     -----------------
// The dimension of Hall dirt volume
extern const double C_HallDirtRadiusMin ;  //mm 
extern const double C_HallDirtRadiusMax ;  //mm
extern const double C_HallDirtHeight    ;  //mm
extern const double C_HallDirtSPhi      ;
extern const double C_HallDirtDPhi      ;
// Pillar
extern const double C_PillarX           ;  //mm //half of each side
extern const double C_PillarY           ;  //mm
extern const double C_PillarZ           ;  //mm


// =============================================
// =========    Mother Volumes      ============
// =============================================

// -------- Positions in World -----------------
// INGRID 7 horizontal modules
extern const double C_INGHMotherPosX ;  //mm
extern const double C_INGHMotherPosY ;  //mm
extern const double C_INGHMotherPosZ ;  //mm
// INGRID 7 vertical modules
extern const double C_INGVMotherPosX ;  //mm
extern const double C_INGVMotherPosY ;  //mm
extern const double C_INGVMotherPosZ ;  //mm
// Proton Module Position & On-axis WaterModule
extern const double C_PMMotherPosX   ;  //mm
extern const double C_PMMotherPosY   ;  //mm
extern const double C_PMMotherPosZ   ;  //mm
// B2 modules 
extern const double C_B2MotherPosX   ;  //mm
extern const double C_B2MotherPosY   ;  //mm
extern const double C_B2MotherPosZ   ;  //mm

// --------    Sizes of box    -----------------
// INGRID 7 horizontal modules 
extern const double C_INGHMotherSizeX ;  //mm //half of each side
extern const double C_INGHMotherSizeY ;  //mm
extern const double C_INGHMotherSizeZ ;  //mm
// INGRID 7 vertical modules
extern const double C_INGVMotherSizeX ;  //mm //half of each side
extern const double C_INGVMotherSizeY ;  //mm
extern const double C_INGVMotherSizeZ ;  //mm
// Proton Module Position & On-axis WaterModule
extern const double C_PMMotherSizeX   ;  //mm //half of each side
extern const double C_PMMotherSizeY   ;  //mm
extern const double C_PMMotherSizeZ   ;  //mm
// B2 modules 
extern const double C_B2MotherSizeX   ;  //mm //half of each side
extern const double C_B2MotherSizeY   ;  //mm
extern const double C_B2MotherSizeZ   ;  //mm


// ==========================================================
// ================== Modules Volume  =======================
// ==========================================================

// ------------- Positions ---------------------------
// Positions between each INGRID modules in "INGH(V)Mother"
extern const double C_INGSpace    ; 
extern const double C_INGStart    ;  //mm // center of the first module
// Each sub-module in "B2Mother" 
extern const double C_B2WMPosX    ;  //mm
extern const double C_B2WMPosY    ;  //mm
extern const double C_B2WMPosZ    ;  //mm
extern const double C_B2CHPosX    ;  //mm
extern const double C_B2CHPosY    ;  //mm
extern const double C_B2CHPosZ    ;  //mm
extern const double C_B2d1INGPosX ;  //mm
extern const double C_B2d1INGPosY ;  //mm
extern const double C_B2d1INGPosZ ;  //mm
extern const double C_B2d2INGPosX ;  //mm
extern const double C_B2d2INGPosY ;  //mm
extern const double C_B2d2INGPosZ ;  //mm
extern const double C_B2s1INGPosX ;  //mm //right
extern const double C_B2s1INGPosY ;  //mm 
extern const double C_B2s1INGPosZ ;  //mm
extern const double C_B2s2INGPosX ;  //mm //left
extern const double C_B2s2INGPosY ;  //mm
extern const double C_B2s2INGPosZ ;  //mm
// ------------ Sizes of box --------------------------
// INGRID Module volume (for each)
extern const double C_INGSizeX    ;  //mm //half of each side
extern const double C_INGSizeY    ;  //mm
extern const double C_INGSizeZ    ;  //mm
// Proton Module volume 
extern const double C_PMSizeX     ;  //mm //half of each side
extern const double C_PMSizeY     ;  //mm
extern const double C_PMSizeZ     ;  //mm
// Water/CH Module volume (both OnAxis/B2)
extern const double C_WMSizeX     ;  //mm //half of each side
extern const double C_WMSizeY     ;  //mm
extern const double C_WMSizeZ     ;  //mm


// =============================================
// =========          INGRID        ============
// =============================================

extern const int    C_INGNumPln          ;
extern const int    C_INGNumCh           ;
extern const int    C_INGNumVetoCh       ;
extern const double C_INGPlnDist         ;  //mm // b/w two adjacent planes
extern const double C_INGPlnStart        ;  //mm // center of 1st vertical scinti plane(pln#0)
extern const double C_INGChStart         ;  //mm // center of ch#0
extern const double C_INGIronStart       ;  //mm // center of 1st iron
// -------- Detector dimenstion ----------------
// Iron
extern const double C_INGIronThick       ;  //mm
extern const double C_INGIronXY          ;  //mm
extern const double C_INGTotMassIron     ;  //ton
// Scintillators
extern const double C_INGScintiLength    ;  //mm
extern const double C_INGScintiWidth     ;  //mm
extern const double C_INGScintiThick     ;  //mm
extern const double C_INGScintiHoleDia_a ;  //mm
extern const double C_INGScintiHoleDia_b ;  //mm
extern const double C_INGTotMassScinti   ;  //ton
// 8 Vertices of Scintillator Cross Section for Extruded Solid
extern const double C_INGScintiVertexX[8]; //mm
extern const double C_INGScintiVertexY[8]; //mm
// Long veto planes
extern const double C_INGLVetoLength     ;  //mm
extern const double C_INGLVetoWidth      ;  //mm
extern const double C_INGLVetoThick      ;  //mm
extern const double C_INGVetoStartZ      ;  //mm center of 1st veto pln
// Short veto planes
extern const double C_INGSVetoLength     ;  //mm 
extern const double C_INGSVetoWidth      ;  //mm 
extern const double C_INGSVetoThick      ;  //mm 
// Veto Positions for INGRID
extern const double C_INGVetoPos1X       ;  //mm //top
extern const double C_INGVetoPos1Y       ;  //mm
extern const double C_INGVetoPos2X       ;  //mm //bottom
extern const double C_INGVetoPos2Y       ;  //mm
extern const double C_INGVetoPos3X       ;  //mm //left
extern const double C_INGVetoPos3Y       ;  //mm
extern const double C_INGVetoPos4X       ;  //mm //right
extern const double C_INGVetoPos4Y       ;  //mm


// =============================================
// =========      Proton Module     ============
// =============================================

extern const int    C_PMNumPln            ;
extern const int    C_PMNumVetoPln        ;
extern const int    C_PMNumVetoCh         ;
extern const int    C_PMNumCh             ;
extern const int    C_PMNumCh_mid         ;  //Num of scibar type scinti
extern const int    C_PMNumCh_side        ;  //Num of INGRID type scinti for each side
extern const int    C_PMNumCh1            ; //Num of ch for 1st pln
extern const double C_PMPlnDist           ;   //mm // b/w two adjacent H pln & V pln
extern const double C_PMPlnDist_First     ;   //mm // b/w 1st V pln & 2nd H pln
extern const double C_PMPlnStart          ;   //mm // center of 1st horizontal plane(pln#0)
extern const double C_PMChStart           ;   //mm // center of ch#0
// -------- Detector dimenstion ----------------
// Scintillators
extern const double C_PMScintiLength      ;   //mm
extern const double C_PMScintiWidth       ;   //mm
extern const double C_PMScintiThick       ;   //mm
extern const double C_PMScintiHoleDia     ;   //mm;
extern const double C_PMTotMassScinti     ;   //ton
extern const double C_PMTotMassVetoSci    ;   //ton
// 8 Vertices of Scintillator Cross section for Extruded Solid
extern const double C_PMScintiVertexX[8]  ; //mm
extern const double C_PMScintiVertexY[8]  ; //mm
// Long veto planes
extern const double C_PMLVetoLength       ; //mm 
extern const double C_PMLVetoWidth        ; //mm 
extern const double C_PMLVetoThick        ; //mm 
extern const double C_PMVetoStartZ        ; //mm // center of 1st veto pln
// Short veto planes
extern const double C_PMSVetoLength       ; //mm 
extern const double C_PMSVetoWidth        ; //mm 
extern const double C_PMSVetoThick        ; //mm 
// Veto Positions for ProtomModule
extern const double C_PMVetoPos1X         ;  //mm //top
extern const double C_PMVetoPos1Y         ;  //mm
extern const double C_PMVetoPos2X         ;  //mm //bottom
extern const double C_PMVetoPos2Y         ;  //mm
extern const double C_PMVetoPos3X         ;  //mm //left
extern const double C_PMVetoPos3Y         ;  //mm
extern const double C_PMVetoPos4X         ;  //mm //right
extern const double C_PMVetoPos4Y         ;  //mm



// =============================================
// =========      WAGASCI           ============
// =============================================

extern const int    C_WMNumView           ;
extern const int    C_WMNumPln            ;
extern const int    C_WMNumCh             ;
extern const int    C_WMNumLayer          ;
extern const int    C_WMNumXYLayerCh      ;
extern const int    C_WMNumGridCh         ;  // # of ch of grid for each X or Y layer
extern const double C_WMPlnDist           ;  //mm // b/w two adjacent X-layers (planes)
extern const double C_WMLayerDist         ;  //mm // b/w two adjacent X and Y-layers (layers)
extern const double C_WMPlnStart          ;  //mm // center of 1st X-layer(pln#0 side view)
extern const double C_WMChStart           ;  //mm // center of ch#0
extern const double C_WMGridChStart       ;  //mm // center of ch#40 & ch#60
// -------- Detector dimenstion ----------------
// Water target
extern const double C_WMWaterTargetSizeX  ;  //mm
extern const double C_WMWaterTargetSizeY  ;  //mm
extern const double C_WMWaterTargetSizeZ  ;  //mm
// CH target
extern const double C_WMCHCellTargetSizeX ;  //mm
extern const double C_WMCHCellTargetSizeY ;  //mm
extern const double C_WMCHCellTargetSizeZ ;  //mm

// Scintillators
extern const double C_WMScintiLength      ;  //mm
extern const double C_WMScintiWidth       ;  //mm
extern const double C_WMScintiThick       ;  //mm
extern const double C_WMTrueScintiWidth       ;  //mm
extern const double C_WMTrueScintiThick       ;  //mm
// groove for fiber
extern const double C_WMScintiHoleLength  ;  //mm 
extern const double C_WMScintiHoleWidth   ;  //mm
extern const double C_WMScintiHoleThick   ;  //mm
extern const double C_WMScintiHoleShift1  ;  //mm
extern const double C_WMScintiHoleShitt2  ;  //mm
// slit for 3D grid strudture
extern const double C_WMScintiSlitLength  ;  //mm 
extern const double C_WMScintiSlitWidth   ;  //mm
extern const double C_WMScintiSlitThick   ;  //mm
extern const double C_WMScintiSlitStep    ;  //mm
extern const double C_WMScintiSlitShift   ;  //mm
// Margin from scintillator edge to bundle(MPPC)
extern const double C_WMFiberMargin       ;  //mm
// 8 Vertices of Scintillator Cross section for Extruded Solid
extern const double C_WMScintiVertexX[8]  ; //mm
extern const double C_WMScintiVertexY[8]  ;  //mm
// Other components
// Aluminum tank dimension (water tank)
extern const double C_WMAlTankSizeX       ;  //mm
extern const double C_WMAlTankSizeY       ;  //mm
extern const double C_WMAlTankSizeZ       ;  //mm


// =======================================================
// =========      Detecor Response           =============
// =======================================================

// Energy deposit correction factors
extern const double C_Corr_Birks       ; // used in SciBooNE MC
// Scinti & Fiber
extern const double C_ScintiAttLeng    ; //cm
extern const double C_WMScintiAttLeng  ; //cm added
extern const double C_FiberAttLeng     ; //cm
extern const double C_TransTimeInFiber ; //  1cm/2.8e10[cm/s] * 1e9 [ns]
extern const double C_INGRIDFactor     ; //P.E. factor for INGRID scintillator
extern const double C_ScibarFactor     ; //P.E. factor for SciBar scintillator
extern const double C_WMFactor         ; //P.E. factor for WAGASCI scintillator
// MPPC
extern const double C_MeV2PE           ; // v3r4
extern const double C_MPPCPixel        ; //v3
extern const double C_Eff_PDE          ; //*1.134;  // @deltaV = 1.1 V
extern const double C_ETC_CONST        ;  // v3
extern const double C_rPDE             ;  // @deltaV = 1.1 V
extern const double C_CrossAfterRate   ;//*1.051;  // @deltaV = 1.1 V
extern const double C_PixelGainVari    ;  // gain variation among pixels
// ADC
extern const double C_Pedestal         ;//145;  // pedeltal of ADC counts
extern const double C_Gain             ;  // Gain ADC counts of high gain channel
extern const double C_LowGain          ;  // Gain ADC counts of low gain channel
extern const double C_ADCtoCharge      ;  // ADC to Charge
extern const double C_LowADCtoCharge   ;  // ADC to Charge
extern const double C_LowGain_corr     ;
extern const double C_NonLinADCTh[4]   ;
extern const double C_NonLinADC1[4]    ;
extern const double C_NonLinADC2[4]    ;
extern const double C_ADCSaturation    ;
extern const double C_LowNonLinADCTh[4];
extern const double C_LowNonLinADC1[4] ;
extern const double C_LowNonLinADC2[4] ;
extern const double C_LowADCSaturation ;
extern const double C_ElecNoise        ;  // sigma of high gain electronics noise
extern const double C_LowElecNoise     ;  // sigma of low gain electronics noise

#endif
