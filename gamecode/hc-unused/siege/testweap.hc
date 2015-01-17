/*
 * $Header: /HexenWorld/Siege/testweap.hc 3     5/25/98 1:39p Mgummelt $
 */

/*
==============================================================================

q:\art\models\monsters\hydra\final\test.hc

==============================================================================
*/

// For building the model
$cd q:\art\models\monsters\hydra\final
$origin 0 0 0
$base base skin1
$skin skin1
$flags 0

//
$frame hadie1       hadie2       hadie3       hadie4       hadie5       
$frame hadie6       hadie7       hadie8       hadie9       hadie10      
$frame hadie11      hadie12      hadie13      hadie14      hadie15      
$frame hadie16      hadie17      hadie18      hadie19      hadie20      
$frame hadie21      hadie22      hadie23      hadie24      hadie25      
$frame hadie26      hadie27      hadie28      hadie29      hadie30      
$frame hadie31      hadie32      hadie33      hadie34      hadie35      
$frame hadie36      

//
$frame hapan1       hapan2       hapan3       hapan4       hapan5       
$frame hapan6       hapan7       hapan8       hapan9       hapan10      

//
$frame hlft1        hlft2        hlft3        hlft4        hlft5        
$frame hlft6        hlft7        hlft8        hlft9        hlft10       
$frame hlft11       hlft12       hlft13       hlft14       hlft15       
$frame hlft16       hlft17       hlft18       hlft19       hlft20       
$frame hlft21       hlft22       hlft23       hlft24       hlft25       
$frame hlft26       hlft27       hlft28       hlft29       hlft30       

//
$frame hopen1       hopen2       hopen3       hopen4       hopen5       
$frame hopen6       hopen7       hopen8       

//
$frame hrit1        hrit2        hrit3        hrit4        hrit5        
$frame hrit6        hrit7        hrit8        hrit9        hrit10       
$frame hrit11       hrit12       hrit13       hrit14       hrit15       
$frame hrit16       hrit17       hrit18       hrit19       hrit20       
$frame hrit21       hrit22       hrit23       hrit24       hrit25       
$frame hrit26       hrit27       hrit28       hrit29       hrit30       

//
$frame hsdie1       hsdie2       hsdie3       hsdie4       hsdie5       
$frame hsdie6       hsdie7       hsdie8       hsdie9       hsdie10      
$frame hsdie11      hsdie12      hsdie13      hsdie14      hsdie15      
$frame hsdie16      hsdie17      hsdie18      hsdie19      hsdie20      
$frame hsdie21      hsdie22      hsdie23      hsdie24      hsdie25      
$frame hsdie26      hsdie27      hsdie28      hsdie29      hsdie30      
$frame hsdie31      hsdie32      hsdie33      hsdie34      hsdie35      
$frame hsdie36      

//
$frame hspan1       hspan2       hspan3       hspan4       hspan5       
$frame hspan6       hspan7       hspan8       hspan9       hspan10      

//
$frame hspit1       hspit2       hspit3       hspit4       hspit5       
$frame hspit6       hspit7       hspit8       hspit9       hspit10      
$frame hspit11      hspit12      

//
$frame hswim1       hswim2       hswim3       hswim4       hswim5       
$frame hswim6       hswim7       hswim8       hswim9       hswim10      
$frame hswim11      hswim12      hswim13      hswim14      hswim15      
$frame hswim16      hswim17      hswim18      hswim19      hswim20      

//
$frame htent1       htent2       htent3       htent4       htent5       
$frame htent6       htent7       htent8       htent9       htent10      
$frame htent11      htent12      htent13      htent14      htent15      
$frame htent16      htent17      htent18      htent19      htent20      
$frame htent21      htent22      htent23      htent24      


/*

// Frame Code
void() hydra_big_hadie1     = [ $hadie1    , hydra_big_hadie2     ] { self.weaponframe = 0; };
void() hydra_big_hadie2     = [ $hadie2    , hydra_big_hadie3     ] { self.weaponframe = 1; };
void() hydra_big_hadie3     = [ $hadie3    , hydra_big_hadie4     ] { self.weaponframe = 2; };
void() hydra_big_hadie4     = [ $hadie4    , hydra_big_hadie5     ] { self.weaponframe = 3; };
void() hydra_big_hadie5     = [ $hadie5    , hydra_big_hadie6     ] { self.weaponframe = 4; };
void() hydra_big_hadie6     = [ $hadie6    , hydra_big_hadie7     ] { self.weaponframe = 5; };
void() hydra_big_hadie7     = [ $hadie7    , hydra_big_hadie8     ] { self.weaponframe = 6; };
void() hydra_big_hadie8     = [ $hadie8    , hydra_big_hadie9     ] { self.weaponframe = 7; };
void() hydra_big_hadie9     = [ $hadie9    , hydra_big_hadie10    ] { self.weaponframe = 8; };
void() hydra_big_hadie10    = [ $hadie10   , hydra_big_hadie11    ] { self.weaponframe = 9; };
void() hydra_big_hadie11    = [ $hadie11   , hydra_big_hadie12    ] { self.weaponframe = 10; };
void() hydra_big_hadie12    = [ $hadie12   , hydra_big_hadie13    ] { self.weaponframe = 11; };
void() hydra_big_hadie13    = [ $hadie13   , hydra_big_hadie14    ] { self.weaponframe = 12; };
void() hydra_big_hadie14    = [ $hadie14   , hydra_big_hadie15    ] { self.weaponframe = 13; };
void() hydra_big_hadie15    = [ $hadie15   , hydra_big_hadie16    ] { self.weaponframe = 14; };
void() hydra_big_hadie16    = [ $hadie16   , hydra_big_hadie17    ] { self.weaponframe = 15; };
void() hydra_big_hadie17    = [ $hadie17   , hydra_big_hadie18    ] { self.weaponframe = 16; };
void() hydra_big_hadie18    = [ $hadie18   , hydra_big_hadie19    ] { self.weaponframe = 17; };
void() hydra_big_hadie19    = [ $hadie19   , hydra_big_hadie20    ] { self.weaponframe = 18; };
void() hydra_big_hadie20    = [ $hadie20   , hydra_big_hadie21    ] { self.weaponframe = 19; };
void() hydra_big_hadie21    = [ $hadie21   , hydra_big_hadie22    ] { self.weaponframe = 20; };
void() hydra_big_hadie22    = [ $hadie22   , hydra_big_hadie23    ] { self.weaponframe = 21; };
void() hydra_big_hadie23    = [ $hadie23   , hydra_big_hadie24    ] { self.weaponframe = 22; };
void() hydra_big_hadie24    = [ $hadie24   , hydra_big_hadie25    ] { self.weaponframe = 23; };
void() hydra_big_hadie25    = [ $hadie25   , hydra_big_hadie26    ] { self.weaponframe = 24; };
void() hydra_big_hadie26    = [ $hadie26   , hydra_big_hadie27    ] { self.weaponframe = 25; };
void() hydra_big_hadie27    = [ $hadie27   , hydra_big_hadie28    ] { self.weaponframe = 26; };
void() hydra_big_hadie28    = [ $hadie28   , hydra_big_hadie29    ] { self.weaponframe = 27; };
void() hydra_big_hadie29    = [ $hadie29   , hydra_big_hadie30    ] { self.weaponframe = 28; };
void() hydra_big_hadie30    = [ $hadie30   , hydra_big_hadie31    ] { self.weaponframe = 29; };
void() hydra_big_hadie31    = [ $hadie31   , hydra_big_hadie32    ] { self.weaponframe = 30; };
void() hydra_big_hadie32    = [ $hadie32   , hydra_big_hadie33    ] { self.weaponframe = 31; };
void() hydra_big_hadie33    = [ $hadie33   , hydra_big_hadie34    ] { self.weaponframe = 32; };
void() hydra_big_hadie34    = [ $hadie34   , hydra_big_hadie35    ] { self.weaponframe = 33; };
void() hydra_big_hadie35    = [ $hadie35   , hydra_big_hadie36    ] { self.weaponframe = 34; };
void() hydra_big_hadie36    = [ $hadie36   , self.th_run          ] { self.weaponframe = 35; };

void() hydra_big_hapan1     = [ $hapan1    , hydra_big_hapan2     ] { self.weaponframe = 36; };
void() hydra_big_hapan2     = [ $hapan2    , hydra_big_hapan3     ] { self.weaponframe = 37; };
void() hydra_big_hapan3     = [ $hapan3    , hydra_big_hapan4     ] { self.weaponframe = 38; };
void() hydra_big_hapan4     = [ $hapan4    , hydra_big_hapan5     ] { self.weaponframe = 39; };
void() hydra_big_hapan5     = [ $hapan5    , hydra_big_hapan6     ] { self.weaponframe = 40; };
void() hydra_big_hapan6     = [ $hapan6    , hydra_big_hapan7     ] { self.weaponframe = 41; };
void() hydra_big_hapan7     = [ $hapan7    , hydra_big_hapan8     ] { self.weaponframe = 42; };
void() hydra_big_hapan8     = [ $hapan8    , hydra_big_hapan9     ] { self.weaponframe = 43; };
void() hydra_big_hapan9     = [ $hapan9    , hydra_big_hapan10    ] { self.weaponframe = 44; };
void() hydra_big_hapan10    = [ $hapan10   , self.th_run          ] { self.weaponframe = 45; };

void() hydra_big_hlft1      = [ $hlft1     , hydra_big_hlft2      ] { self.weaponframe = 46; };
void() hydra_big_hlft2      = [ $hlft2     , hydra_big_hlft3      ] { self.weaponframe = 47; };
void() hydra_big_hlft3      = [ $hlft3     , hydra_big_hlft4      ] { self.weaponframe = 48; };
void() hydra_big_hlft4      = [ $hlft4     , hydra_big_hlft5      ] { self.weaponframe = 49; };
void() hydra_big_hlft5      = [ $hlft5     , hydra_big_hlft6      ] { self.weaponframe = 50; };
void() hydra_big_hlft6      = [ $hlft6     , hydra_big_hlft7      ] { self.weaponframe = 51; };
void() hydra_big_hlft7      = [ $hlft7     , hydra_big_hlft8      ] { self.weaponframe = 52; };
void() hydra_big_hlft8      = [ $hlft8     , hydra_big_hlft9      ] { self.weaponframe = 53; };
void() hydra_big_hlft9      = [ $hlft9     , hydra_big_hlft10     ] { self.weaponframe = 54; };
void() hydra_big_hlft10     = [ $hlft10    , hydra_big_hlft11     ] { self.weaponframe = 55; };
void() hydra_big_hlft11     = [ $hlft11    , hydra_big_hlft12     ] { self.weaponframe = 56; };
void() hydra_big_hlft12     = [ $hlft12    , hydra_big_hlft13     ] { self.weaponframe = 57; };
void() hydra_big_hlft13     = [ $hlft13    , hydra_big_hlft14     ] { self.weaponframe = 58; };
void() hydra_big_hlft14     = [ $hlft14    , hydra_big_hlft15     ] { self.weaponframe = 59; };
void() hydra_big_hlft15     = [ $hlft15    , hydra_big_hlft16     ] { self.weaponframe = 60; };
void() hydra_big_hlft16     = [ $hlft16    , hydra_big_hlft17     ] { self.weaponframe = 61; };
void() hydra_big_hlft17     = [ $hlft17    , hydra_big_hlft18     ] { self.weaponframe = 62; };
void() hydra_big_hlft18     = [ $hlft18    , hydra_big_hlft19     ] { self.weaponframe = 63; };
void() hydra_big_hlft19     = [ $hlft19    , hydra_big_hlft20     ] { self.weaponframe = 64; };
void() hydra_big_hlft20     = [ $hlft20    , hydra_big_hlft21     ] { self.weaponframe = 65; };
void() hydra_big_hlft21     = [ $hlft21    , hydra_big_hlft22     ] { self.weaponframe = 66; };
void() hydra_big_hlft22     = [ $hlft22    , hydra_big_hlft23     ] { self.weaponframe = 67; };
void() hydra_big_hlft23     = [ $hlft23    , hydra_big_hlft24     ] { self.weaponframe = 68; };
void() hydra_big_hlft24     = [ $hlft24    , hydra_big_hlft25     ] { self.weaponframe = 69; };
void() hydra_big_hlft25     = [ $hlft25    , hydra_big_hlft26     ] { self.weaponframe = 70; };
void() hydra_big_hlft26     = [ $hlft26    , hydra_big_hlft27     ] { self.weaponframe = 71; };
void() hydra_big_hlft27     = [ $hlft27    , hydra_big_hlft28     ] { self.weaponframe = 72; };
void() hydra_big_hlft28     = [ $hlft28    , hydra_big_hlft29     ] { self.weaponframe = 73; };
void() hydra_big_hlft29     = [ $hlft29    , hydra_big_hlft30     ] { self.weaponframe = 74; };
void() hydra_big_hlft30     = [ $hlft30    , self.th_run();          ] { self.weaponframe = 75; };

void() hydra_big_hopen1     = [ $hopen1    , hydra_big_hopen2     ] { self.weaponframe = 76; };
void() hydra_big_hopen2     = [ $hopen2    , hydra_big_hopen3     ] { self.weaponframe = 77; };
void() hydra_big_hopen3     = [ $hopen3    , hydra_big_hopen4     ] { self.weaponframe = 78; };
void() hydra_big_hopen4     = [ $hopen4    , hydra_big_hopen5     ] { self.weaponframe = 79; };
void() hydra_big_hopen5     = [ $hopen5    , hydra_big_hopen6     ] { self.weaponframe = 80; };
void() hydra_big_hopen6     = [ $hopen6    , hydra_big_hopen7     ] { self.weaponframe = 81; };
void() hydra_big_hopen7     = [ $hopen7    , hydra_big_hopen8     ] { self.weaponframe = 82; };
void() hydra_big_hopen8     = [ $hopen8    , self.th_run();          ] { self.weaponframe = 83; };

void() hydra_big_hrit1      = [ $hrit1     , hydra_big_hrit2      ] { self.weaponframe = 84; };
void() hydra_big_hrit2      = [ $hrit2     , hydra_big_hrit3      ] { self.weaponframe = 85; };
void() hydra_big_hrit3      = [ $hrit3     , hydra_big_hrit4      ] { self.weaponframe = 86; };
void() hydra_big_hrit4      = [ $hrit4     , hydra_big_hrit5      ] { self.weaponframe = 87; };
void() hydra_big_hrit5      = [ $hrit5     , hydra_big_hrit6      ] { self.weaponframe = 88; };
void() hydra_big_hrit6      = [ $hrit6     , hydra_big_hrit7      ] { self.weaponframe = 89; };
void() hydra_big_hrit7      = [ $hrit7     , hydra_big_hrit8      ] { self.weaponframe = 90; };
void() hydra_big_hrit8      = [ $hrit8     , hydra_big_hrit9      ] { self.weaponframe = 91; };
void() hydra_big_hrit9      = [ $hrit9     , hydra_big_hrit10     ] { self.weaponframe = 92; };
void() hydra_big_hrit10     = [ $hrit10    , hydra_big_hrit11     ] { self.weaponframe = 93; };
void() hydra_big_hrit11     = [ $hrit11    , hydra_big_hrit12     ] { self.weaponframe = 94; };
void() hydra_big_hrit12     = [ $hrit12    , hydra_big_hrit13     ] { self.weaponframe = 95; };
void() hydra_big_hrit13     = [ $hrit13    , hydra_big_hrit14     ] { self.weaponframe = 96; };
void() hydra_big_hrit14     = [ $hrit14    , hydra_big_hrit15     ] { self.weaponframe = 97; };
void() hydra_big_hrit15     = [ $hrit15    , hydra_big_hrit16     ] { self.weaponframe = 98; };
void() hydra_big_hrit16     = [ $hrit16    , hydra_big_hrit17     ] { self.weaponframe = 99; };
void() hydra_big_hrit17     = [ $hrit17    , hydra_big_hrit18     ] { self.weaponframe = 100; };
void() hydra_big_hrit18     = [ $hrit18    , hydra_big_hrit19     ] { self.weaponframe = 101; };
void() hydra_big_hrit19     = [ $hrit19    , hydra_big_hrit20     ] { self.weaponframe = 102; };
void() hydra_big_hrit20     = [ $hrit20    , hydra_big_hrit21     ] { self.weaponframe = 103; };
void() hydra_big_hrit21     = [ $hrit21    , hydra_big_hrit22     ] { self.weaponframe = 104; };
void() hydra_big_hrit22     = [ $hrit22    , hydra_big_hrit23     ] { self.weaponframe = 105; };
void() hydra_big_hrit23     = [ $hrit23    , hydra_big_hrit24     ] { self.weaponframe = 106; };
void() hydra_big_hrit24     = [ $hrit24    , hydra_big_hrit25     ] { self.weaponframe = 107; };
void() hydra_big_hrit25     = [ $hrit25    , hydra_big_hrit26     ] { self.weaponframe = 108; };
void() hydra_big_hrit26     = [ $hrit26    , hydra_big_hrit27     ] { self.weaponframe = 109; };
void() hydra_big_hrit27     = [ $hrit27    , hydra_big_hrit28     ] { self.weaponframe = 110; };
void() hydra_big_hrit28     = [ $hrit28    , hydra_big_hrit29     ] { self.weaponframe = 111; };
void() hydra_big_hrit29     = [ $hrit29    , hydra_big_hrit30     ] { self.weaponframe = 112; };
void() hydra_big_hrit30     = [ $hrit30    , self.th_run();          ] { self.weaponframe = 113; };

void() hydra_big_hsdie1     = [ $hsdie1    , hydra_big_hsdie2     ] { self.weaponframe = 114; };
void() hydra_big_hsdie2     = [ $hsdie2    , hydra_big_hsdie3     ] { self.weaponframe = 115; };
void() hydra_big_hsdie3     = [ $hsdie3    , hydra_big_hsdie4     ] { self.weaponframe = 116; };
void() hydra_big_hsdie4     = [ $hsdie4    , hydra_big_hsdie5     ] { self.weaponframe = 117; };
void() hydra_big_hsdie5     = [ $hsdie5    , hydra_big_hsdie6     ] { self.weaponframe = 118; };
void() hydra_big_hsdie6     = [ $hsdie6    , hydra_big_hsdie7     ] { self.weaponframe = 119; };
void() hydra_big_hsdie7     = [ $hsdie7    , hydra_big_hsdie8     ] { self.weaponframe = 120; };
void() hydra_big_hsdie8     = [ $hsdie8    , hydra_big_hsdie9     ] { self.weaponframe = 121; };
void() hydra_big_hsdie9     = [ $hsdie9    , hydra_big_hsdie10    ] { self.weaponframe = 122; };
void() hydra_big_hsdie10    = [ $hsdie10   , hydra_big_hsdie11    ] { self.weaponframe = 123; };
void() hydra_big_hsdie11    = [ $hsdie11   , hydra_big_hsdie12    ] { self.weaponframe = 124; };
void() hydra_big_hsdie12    = [ $hsdie12   , hydra_big_hsdie13    ] { self.weaponframe = 125; };
void() hydra_big_hsdie13    = [ $hsdie13   , hydra_big_hsdie14    ] { self.weaponframe = 126; };
void() hydra_big_hsdie14    = [ $hsdie14   , hydra_big_hsdie15    ] { self.weaponframe = 127; };
void() hydra_big_hsdie15    = [ $hsdie15   , hydra_big_hsdie16    ] { self.weaponframe = 128; };
void() hydra_big_hsdie16    = [ $hsdie16   , hydra_big_hsdie17    ] { self.weaponframe = 129; };
void() hydra_big_hsdie17    = [ $hsdie17   , hydra_big_hsdie18    ] { self.weaponframe = 130; };
void() hydra_big_hsdie18    = [ $hsdie18   , hydra_big_hsdie19    ] { self.weaponframe = 131; };
void() hydra_big_hsdie19    = [ $hsdie19   , hydra_big_hsdie20    ] { self.weaponframe = 132; };
void() hydra_big_hsdie20    = [ $hsdie20   , hydra_big_hsdie21    ] { self.weaponframe = 133; };
void() hydra_big_hsdie21    = [ $hsdie21   , hydra_big_hsdie22    ] { self.weaponframe = 134; };
void() hydra_big_hsdie22    = [ $hsdie22   , hydra_big_hsdie23    ] { self.weaponframe = 135; };
void() hydra_big_hsdie23    = [ $hsdie23   , hydra_big_hsdie24    ] { self.weaponframe = 136; };
void() hydra_big_hsdie24    = [ $hsdie24   , hydra_big_hsdie25    ] { self.weaponframe = 137; };
void() hydra_big_hsdie25    = [ $hsdie25   , hydra_big_hsdie26    ] { self.weaponframe = 138; };
void() hydra_big_hsdie26    = [ $hsdie26   , hydra_big_hsdie27    ] { self.weaponframe = 139; };
void() hydra_big_hsdie27    = [ $hsdie27   , hydra_big_hsdie28    ] { self.weaponframe = 140; };
void() hydra_big_hsdie28    = [ $hsdie28   , hydra_big_hsdie29    ] { self.weaponframe = 141; };
void() hydra_big_hsdie29    = [ $hsdie29   , hydra_big_hsdie30    ] { self.weaponframe = 142; };
void() hydra_big_hsdie30    = [ $hsdie30   , hydra_big_hsdie31    ] { self.weaponframe = 143; };
void() hydra_big_hsdie31    = [ $hsdie31   , hydra_big_hsdie32    ] { self.weaponframe = 144; };
void() hydra_big_hsdie32    = [ $hsdie32   , hydra_big_hsdie33    ] { self.weaponframe = 145; };
void() hydra_big_hsdie33    = [ $hsdie33   , hydra_big_hsdie34    ] { self.weaponframe = 146; };
void() hydra_big_hsdie34    = [ $hsdie34   , hydra_big_hsdie35    ] { self.weaponframe = 147; };
void() hydra_big_hsdie35    = [ $hsdie35   , hydra_big_hsdie36    ] { self.weaponframe = 148; };
void() hydra_big_hsdie36    = [ $hsdie36   , self.th_run();          ] { self.weaponframe = 149; };

void() hydra_big_hspan1     = [ $hspan1    , hydra_big_hspan2     ] { self.weaponframe = 150; };
void() hydra_big_hspan2     = [ $hspan2    , hydra_big_hspan3     ] { self.weaponframe = 151; };
void() hydra_big_hspan3     = [ $hspan3    , hydra_big_hspan4     ] { self.weaponframe = 152; };
void() hydra_big_hspan4     = [ $hspan4    , hydra_big_hspan5     ] { self.weaponframe = 153; };
void() hydra_big_hspan5     = [ $hspan5    , hydra_big_hspan6     ] { self.weaponframe = 154; };
void() hydra_big_hspan6     = [ $hspan6    , hydra_big_hspan7     ] { self.weaponframe = 155; };
void() hydra_big_hspan7     = [ $hspan7    , hydra_big_hspan8     ] { self.weaponframe = 156; };
void() hydra_big_hspan8     = [ $hspan8    , hydra_big_hspan9     ] { self.weaponframe = 157; };
void() hydra_big_hspan9     = [ $hspan9    , hydra_big_hspan10    ] { self.weaponframe = 158; };
void() hydra_big_hspan10    = [ $hspan10   , self.th_run();          ] { self.weaponframe = 159; };

void() hydra_big_hspit1     = [ $hspit1    , hydra_big_hspit2     ] { self.weaponframe = 160; };
void() hydra_big_hspit2     = [ $hspit2    , hydra_big_hspit3     ] { self.weaponframe = 161; };
void() hydra_big_hspit3     = [ $hspit3    , hydra_big_hspit4     ] { self.weaponframe = 162; };
void() hydra_big_hspit4     = [ $hspit4    , hydra_big_hspit5     ] { self.weaponframe = 163; };
void() hydra_big_hspit5     = [ $hspit5    , hydra_big_hspit6     ] { self.weaponframe = 164; };
void() hydra_big_hspit6     = [ $hspit6    , hydra_big_hspit7     ] { self.weaponframe = 165; };
void() hydra_big_hspit7     = [ $hspit7    , hydra_big_hspit8     ] { self.weaponframe = 166; };
void() hydra_big_hspit8     = [ $hspit8    , hydra_big_hspit9     ] { self.weaponframe = 167; };
void() hydra_big_hspit9     = [ $hspit9    , hydra_big_hspit10    ] { self.weaponframe = 168; };
void() hydra_big_hspit10    = [ $hspit10   , hydra_big_hspit11    ] { self.weaponframe = 169; };
void() hydra_big_hspit11    = [ $hspit11   , hydra_big_hspit12    ] { self.weaponframe = 170; };
void() hydra_big_hspit12    = [ $hspit12   , self.th_run();          ] { self.weaponframe = 171; };

void() hydra_big_hswim1     = [ $hswim1    , hydra_big_hswim2     ] { self.weaponframe = 172; };
void() hydra_big_hswim2     = [ $hswim2    , hydra_big_hswim3     ] { self.weaponframe = 173; };
void() hydra_big_hswim3     = [ $hswim3    , hydra_big_hswim4     ] { self.weaponframe = 174; };
void() hydra_big_hswim4     = [ $hswim4    , hydra_big_hswim5     ] { self.weaponframe = 175; };
void() hydra_big_hswim5     = [ $hswim5    , hydra_big_hswim6     ] { self.weaponframe = 176; };
void() hydra_big_hswim6     = [ $hswim6    , hydra_big_hswim7     ] { self.weaponframe = 177; };
void() hydra_big_hswim7     = [ $hswim7    , hydra_big_hswim8     ] { self.weaponframe = 178; };
void() hydra_big_hswim8     = [ $hswim8    , hydra_big_hswim9     ] { self.weaponframe = 179; };
void() hydra_big_hswim9     = [ $hswim9    , hydra_big_hswim10    ] { self.weaponframe = 180; };
void() hydra_big_hswim10    = [ $hswim10   , hydra_big_hswim11    ] { self.weaponframe = 181; };
void() hydra_big_hswim11    = [ $hswim11   , hydra_big_hswim12    ] { self.weaponframe = 182; };
void() hydra_big_hswim12    = [ $hswim12   , hydra_big_hswim13    ] { self.weaponframe = 183; };
void() hydra_big_hswim13    = [ $hswim13   , hydra_big_hswim14    ] { self.weaponframe = 184; };
void() hydra_big_hswim14    = [ $hswim14   , hydra_big_hswim15    ] { self.weaponframe = 185; };
void() hydra_big_hswim15    = [ $hswim15   , hydra_big_hswim16    ] { self.weaponframe = 186; };
void() hydra_big_hswim16    = [ $hswim16   , hydra_big_hswim17    ] { self.weaponframe = 187; };
void() hydra_big_hswim17    = [ $hswim17   , hydra_big_hswim18    ] { self.weaponframe = 188; };
void() hydra_big_hswim18    = [ $hswim18   , hydra_big_hswim19    ] { self.weaponframe = 189; };
void() hydra_big_hswim19    = [ $hswim19   , hydra_big_hswim20    ] { self.weaponframe = 190; };
void() hydra_big_hswim20    = [ $hswim20   , self.th_run();          ] { self.weaponframe = 191; };

void() hydra_big_htent1     = [ $htent1    , hydra_big_htent2     ] { self.weaponframe = 192; };
void() hydra_big_htent2     = [ $htent2    , hydra_big_htent3     ] { self.weaponframe = 193; };
void() hydra_big_htent3     = [ $htent3    , hydra_big_htent4     ] { self.weaponframe = 194; };
void() hydra_big_htent4     = [ $htent4    , hydra_big_htent5     ] { self.weaponframe = 195; };
void() hydra_big_htent5     = [ $htent5    , hydra_big_htent6     ] { self.weaponframe = 196; };
void() hydra_big_htent6     = [ $htent6    , hydra_big_htent7     ] { self.weaponframe = 197; };
void() hydra_big_htent7     = [ $htent7    , hydra_big_htent8     ] { self.weaponframe = 198; };
void() hydra_big_htent8     = [ $htent8    , hydra_big_htent9     ] { self.weaponframe = 199; };
void() hydra_big_htent9     = [ $htent9    , hydra_big_htent10    ] { self.weaponframe = 200; };
void() hydra_big_htent10    = [ $htent10   , hydra_big_htent11    ] { self.weaponframe = 201; };
void() hydra_big_htent11    = [ $htent11   , hydra_big_htent12    ] { self.weaponframe = 202; };
void() hydra_big_htent12    = [ $htent12   , hydra_big_htent13    ] { self.weaponframe = 203; };
void() hydra_big_htent13    = [ $htent13   , hydra_big_htent14    ] { self.weaponframe = 204; };
void() hydra_big_htent14    = [ $htent14   , hydra_big_htent15    ] { self.weaponframe = 205; };
void() hydra_big_htent15    = [ $htent15   , hydra_big_htent16    ] { self.weaponframe = 206; };
void() hydra_big_htent16    = [ $htent16   , hydra_big_htent17    ] { self.weaponframe = 207; };
void() hydra_big_htent17    = [ $htent17   , hydra_big_htent18    ] { self.weaponframe = 208; };
void() hydra_big_htent18    = [ $htent18   , hydra_big_htent19    ] { self.weaponframe = 209; };
void() hydra_big_htent19    = [ $htent19   , hydra_big_htent20    ] { self.weaponframe = 210; };
void() hydra_big_htent20    = [ $htent20   , hydra_big_htent21    ] { self.weaponframe = 211; };
void() hydra_big_htent21    = [ $htent21   , hydra_big_htent22    ] { self.weaponframe = 212; };
void() hydra_big_htent22    = [ $htent22   , hydra_big_htent23    ] { self.weaponframe = 213; };
void() hydra_big_htent23    = [ $htent23   , hydra_big_htent24    ] { self.weaponframe = 214; };
void() hydra_big_htent24    = [ $htent24   , self.th_run();          ] { self.weaponframe = 215; };

*/




void () FireTestWeapon =
{
   dprint("Testing Sequence ");
   dprint(ftos(self.cnt));
   dprint("\n");

   if (self.cnt == 0)
      hydra_big_hadie1();
   else if (self.cnt == 1)
      hydra_big_hapan1();
   else if (self.cnt == 2)
      hydra_big_hlft1();
   else if (self.cnt == 3)
      hydra_big_hopen1();
   else if (self.cnt == 4)
      hydra_big_hrit1();
   else if (self.cnt == 5)
      hydra_big_hsdie1();
   else if (self.cnt == 6)
      hydra_big_hspan1();
   else if (self.cnt == 7)
      hydra_big_hspit1();
   else if (self.cnt == 8)
      hydra_big_hswim1();
   else if (self.cnt == 9)
      hydra_big_htent1();

   self.cnt = self.cnt + 1;
   if (self.cnt >= 10)
      self.cnt = 0;
};

