#include <stdlib.h>
#include <string.h>

#define TOTALSKIMS 43
#define MAXZONES 3500
#define MAXSTRING 10000
#define MAXMATRICES 50
#ifndef SAMPLEZONES
#define SAMPLEZONES 1
#endif
#define TIMEPERIODS 5
#define PURPOSES 6
#define OPCOST 12.0
#define COUNTIES 9

// RELATIONSHIP
#define GQINST 22
#define GQNONINST 23

// DISTRICT USED FOR CBD FLAG
#define CBD 1
#define CBD_NORTH 2

// PURPOSES
#define WORK 1
#define GRADESCHOOL 2
#define HIGHSCHOOL 3
#define COLLEGE 4
#define OTHER 5
#define WORKBASED 6

//Following defines map to table locations in transit matrix files
#define INNMOTMATRICES 1
#define NMOTDIST 0

#define INTERMMATRICES 1
#define TERMTIME 0

#define INTRMATRICES 13

#define ABLVT  0
#define ABMVT  1
#define ABPVT  2
#define ABBVT  3
#define ABACC  4
#define ABEGR  5
#define ABFWT  6
#define ABXWT  7
#define ABDST  8
#define ABFAR  9
#define ABBRD  10
#define ABDRVDIST 11
#define ABAUX  12

#define ABMAT 13

#define APLVT  0
#define APMVT  1
#define APPVT  2
#define APACC  3
#define APEGR  4
#define APFWT  5
#define APXWT  6
#define APDST  7
#define APFAR  8
#define APBRD  9
#define APDRVDIST 10
#define APAUX  11

#define APMAT 12

#define WBLVT  0
#define WBMVT  1
#define WBPVT  2
#define WBBVT  3
#define WBACC  4
#define WBEGR  5
#define WBFWT  6
#define WBXWT  7
#define WBDST  8
#define WBFAR  9
#define WBBRD  10
#define WBAUX  11

#define WBMAT 12

#define WLLVT  0
#define WLACC  1
#define WLEGR  2
#define WLFWT  3
#define WLXWT  4
#define WLDST  5
#define WLFAR  6
#define WLBRD  7
#define WLAUX  8

#define WLMAT 9

#define WMLVT  0
#define WMMVT  1
#define WMACC  2
#define WMEGR  3
#define WMFWT  4
#define WMXWT  5
#define WMDST  6
#define WMFAR  7
#define WMBRD  8
#define WMAUX  9

#define WMMAT 10

#define WPLVT  0
#define WPMVT  1
#define WPPVT  2
#define WPACC  3
#define WPEGR  4
#define WPFWT  5
#define WPXWT  6
#define WPDST  7
#define WPFAR  8
#define WPBRD  9
#define WPAUX 10

#define WPMAT 11

#define INHWMATRICES 22
#define INHWMATRICESTOD 21
#define DAT      0
#define DAD      1
#define DABT     2  // cost of tolls (bridge)
#define S2T      3
#define S2D      4
#define S2BT     5
#define S3T      6
#define S3D      7
#define S3BT     8
#define TOLLDAT  9
#define TOLLDAD  10
#define TOLLDABT 11  // cost of tolls (bridge)
#define TOLLDAVT 12  // cost of tolls (value)
#define TOLLS2T  13
#define TOLLS2D  14
#define TOLLS2BT 15
#define TOLLS2VT 16
#define TOLLS3T  17
#define TOLLS3D  18
#define TOLLS3BT 19
#define TOLLS3VT 20
#define PASSTHRU 21

// used for reading alternate format time-of-day skims
#define INHWYMATRICESTOD_ALT 21
#define TOLLDAT_ALT  1
#define TOLLDAD_ALT  2
#define TOLLDABT_ALT 3   // cost of tolls (bridge)
#define TOLLDAVT_ALT 4   // cost of tolls (value)
#define TOLLS2T_ALT  8
#define TOLLS2D_ALT  9
#define TOLLS2BT_ALT 10
#define TOLLS2VT_ALT 11
#define TOLLS3T_ALT  15
#define TOLLS3D_ALT  16
#define TOLLS3BT_ALT 17
#define TOLLS3VT_ALT 18


//Following defines map to the tables in RAM
#define WLWMATRICES 2    //walk-local-walk
#define WMWMATRICES 2    //walk-muni-walk
#define WPWMATRICES 2    //walk-premium-walk
#define WBWMATRICES 2    //walk-bart-walk
#define DPWMATRICES 3    //drive-premium-walk
#define WPDMATRICES 3    //walk-premium-drive
#define DBWMATRICES 3    //drive-bart-walk
#define WBDMATRICES 3    //walk-bart-drive
#define HWYMATRICES 6
#define TOLLMATRICES 9

//Walk-Local-Walk
#define WLWTIM 0
#define WLWFAR 1

//Walk-Muni-Walk
#define WMWTIM 0
#define WMWFAR 1

//Walk-Prem-Walk
#define WPWTIM 0
#define WPWFAR 1

//Walk-BART-Walk
#define WBWTIM 0
#define WBWFAR 1

//Drive-Prem-Walk
#define DPWTIM 0
#define DPWFAR 1
#define DPWDRVDIST 2

//Walk-Prem-Drive
#define WPDTIM 0
#define WPDFAR 1
#define WPDDRVDIST 2

//Drive-BART-Walk
#define DBWTIM 0
#define DBWFAR 1
#define DBWDRVDIST 2

//Walk-Bart-Drive
#define WBDTIM 0
#define WBDFAR 1
#define WBDDRVDIST 2

//Highway
#define HWYDAT  0
#define HWYDAC  1  // includes operating cost + bridge toll
#define HWYS2T  2
#define HWYS2C  3
#define HWYS3T  4
#define HWYS3C  5
#define HWYDAVT 6  // value toll
#define HWYS2VT 7
#define HWYS3VT 8

//Tour Modes
#define TOURMODES 10

#define TOURHWYDA   1
#define TOURHWYSR2  2
#define TOURHWYSR3  3
#define TOURTOLLDA  4
#define TOURTOLLSR2 5
#define TOURTOLLSR3 6
#define TOURWALK    7
#define TOURBIKE    8
#define TOURWTRN    9
#define TOURDTRN    10

//Base modes chosen
#define BASEMODES 17

#define AUTOBASEMODES 6
#define AUTOPAIDMODES 3

#define HWYDA   1
#define HWYSR2  2
#define HWYSR3  3
#define TOLLDA  4
#define TOLLSR2 5
#define TOLLSR3 6
#define PAIDDA  7
#define PAIDSR2 8
#define PAIDSR3 9
#define WALK    10
#define BIKE    11
#define WLOCL   12
#define WMUNI   13
#define WPREM   14
#define WBART   15
#define DPREM   16
#define DBART   17

//Output modes--written to TP+ tables, 1-15 are same
#define OUTTABLES 19

#define DPREMW 16
#define WPREMD 17
#define DBARTW 18
#define WBARTD 19

//MAIN PERIODS
#define EA 1
#define AM 2
#define MD 3
#define PM 4
#define EV 5

// periods for trip time-of-day
#define SUBPERIODS 21

#define EA300  1
#define EA500  2
#define AM600  3
#define AM630  4
#define AM700  5
#define AM730  6
#define AM800  7
#define AM830  8
#define MD900  9
#define MD1000 10
#define MD1100 11
#define MD130  12
#define MD230  13
#define PM330  14
#define PM400  15
#define PM430  16
#define PM500  17
#define PM530  18
#define PM600  19
#define EV630  20
#define EV730  21

