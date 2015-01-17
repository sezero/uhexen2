/*
 * $Header: /HexenWorld/Siege/warrider.hc 3     5/25/98 1:39p Mgummelt $
 */

/*
==============================================================================

Q:\art\models\monsters\RdrWar\Rider\Final\warrider.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\RdrWar\Rider\Final
$origin 0 0 0
$base base skin
$skin skin
$flags 0

//
$frame RaxeA1       RaxeA2       RaxeA3       RaxeA4       RaxeA5       
$frame RaxeA6       RaxeA7       RaxeA8       RaxeA9       RaxeA10      
$frame RaxeA11      RaxeA12      

//
$frame RaxeD1       RaxeD2       RaxeD3       RaxeD4       RaxeD5       
$frame RaxeD6       RaxeD7       RaxeD8       RaxeD9       RaxeD10      
$frame RaxeD11      RaxeD12      RaxeD13      RaxeD14      RaxeD15      

//
$frame RgallA1      RgallA2      RgallA3      RgallA4      RgallA5      
$frame RgallA6      RgallA7      RgallA8      RgallA9      RgallA10     
$frame RgallA11     RgallA12     

//
$frame RgallB1      RgallB2      RgallB3      RgallB4      RgallB5      
$frame RgallB6      RgallB7      RgallB8      RgallB9      RgallB10     
$frame RgallB11     RgallB12     RgallB13     RgallB14     RgallB15     
$frame RgallB16     

//
$frame RgallC1      RgallC2      RgallC3      RgallC4      RgallC5      
$frame RgallC6      RgallC7      RgallC8      RgallC9      RgallC10     
$frame RgallC11     RgallC12     RgallC13     RgallC14     RgallC15     
$frame RgallC16     RgallC17     RgallC18     RgallC19     RgallC20     

//
$frame RrearA1      RrearA2      RrearA3      RrearA4      RrearA5      
$frame RrearA6      RrearA7      RrearA8      RrearA9      RrearA10     
$frame RrearA11     RrearA12     RrearA13     RrearA14     RrearA15     
$frame RrearA16     RrearA17     RrearA18     RrearA19     RrearA20     
$frame RrearA21     RrearA22     RrearA23     RrearA24     RrearA25     
$frame RrearA26     RrearA27     RrearA28     RrearA29     RrearA30     
$frame RrearA31     RrearA32     RrearA33     RrearA34     RrearA35     
$frame RrearA36     RrearA37     RrearA38     RrearA39     RrearA40     

//
$frame RtranA1      RtranA2      RtranA3      RtranA4      RtranA5      
$frame RtranA6      RtranA7      RtranA8      RtranA9      RtranA10     
$frame RtranA11     RtranA12     RtranA13     RtranA14     RtranA15     
$frame RtranA16     

//
$frame RtranB1      RtranB2      RtranB3      RtranB4      RtranB5      
$frame RtranB6      RtranB7      RtranB8      RtranB9      RtranB10     
$frame RtranB11     RtranB12     


// Array to align frames
float WarRiderFrames[6] =
{
	$RgallA1,
	$RgallB1,
	$RgallC1,

	$RtranA1,
	$RrearA1,
	$RtranB1
};


// Frame Code
void() frame_RaxeA1     = [ $RaxeA1    , frame_RaxeA2     ] {  };
void() frame_RaxeA2     = [ $RaxeA2    , frame_RaxeA3     ] {  };
void() frame_RaxeA3     = [ $RaxeA3    , frame_RaxeA4     ] {  };
void() frame_RaxeA4     = [ $RaxeA4    , frame_RaxeA5     ] {  };
void() frame_RaxeA5     = [ $RaxeA5    , frame_RaxeA6     ] {  };
void() frame_RaxeA6     = [ $RaxeA6    , frame_RaxeA7     ] {  };
void() frame_RaxeA7     = [ $RaxeA7    , frame_RaxeA8     ] {  };
void() frame_RaxeA8     = [ $RaxeA8    , frame_RaxeA9     ] {  };
void() frame_RaxeA9     = [ $RaxeA9    , frame_RaxeA10    ] {  };
void() frame_RaxeA10    = [ $RaxeA10   , frame_RaxeA11    ] {  };
void() frame_RaxeA11    = [ $RaxeA11   , frame_RaxeA12    ] {  };
void() frame_RaxeA12    = [ $RaxeA12   , frame_RaxeA1     ] {  };

void() frame_RaxeD1     = [ $RaxeD1    , frame_RaxeD2     ] {  };
void() frame_RaxeD2     = [ $RaxeD2    , frame_RaxeD3     ] {  };
void() frame_RaxeD3     = [ $RaxeD3    , frame_RaxeD4     ] {  };
void() frame_RaxeD4     = [ $RaxeD4    , frame_RaxeD5     ] {  };
void() frame_RaxeD5     = [ $RaxeD5    , frame_RaxeD6     ] {  };
void() frame_RaxeD6     = [ $RaxeD6    , frame_RaxeD7     ] {  };
void() frame_RaxeD7     = [ $RaxeD7    , frame_RaxeD8     ] {  };
void() frame_RaxeD8     = [ $RaxeD8    , frame_RaxeD9     ] {  };
void() frame_RaxeD9     = [ $RaxeD9    , frame_RaxeD10    ] {  };
void() frame_RaxeD10    = [ $RaxeD10   , frame_RaxeD11    ] {  };
void() frame_RaxeD11    = [ $RaxeD11   , frame_RaxeD12    ] {  };
void() frame_RaxeD12    = [ $RaxeD12   , frame_RaxeD13    ] {  };
void() frame_RaxeD13    = [ $RaxeD13   , frame_RaxeD14    ] {  };
void() frame_RaxeD14    = [ $RaxeD14   , frame_RaxeD15    ] {  };
void() frame_RaxeD15    = [ $RaxeD15   , frame_RaxeD1     ] {  };

void() frame_RgallA1    = [ $RgallA1   , frame_RgallA2    ] {  };
void() frame_RgallA2    = [ $RgallA2   , frame_RgallA3    ] {  };
void() frame_RgallA3    = [ $RgallA3   , frame_RgallA4    ] {  };
void() frame_RgallA4    = [ $RgallA4   , frame_RgallA5    ] {  };
void() frame_RgallA5    = [ $RgallA5   , frame_RgallA6    ] {  };
void() frame_RgallA6    = [ $RgallA6   , frame_RgallA7    ] {  };
void() frame_RgallA7    = [ $RgallA7   , frame_RgallA8    ] {  };
void() frame_RgallA8    = [ $RgallA8   , frame_RgallA9    ] {  };
void() frame_RgallA9    = [ $RgallA9   , frame_RgallA10   ] {  };
void() frame_RgallA10   = [ $RgallA10  , frame_RgallA11   ] {  };
void() frame_RgallA11   = [ $RgallA11  , frame_RgallA12   ] {  };
void() frame_RgallA12   = [ $RgallA12  , frame_RgallA1    ] {  };

void() frame_RgallB1    = [ $RgallB1   , frame_RgallB2    ] {  };
void() frame_RgallB2    = [ $RgallB2   , frame_RgallB3    ] {  };
void() frame_RgallB3    = [ $RgallB3   , frame_RgallB4    ] {  };
void() frame_RgallB4    = [ $RgallB4   , frame_RgallB5    ] {  };
void() frame_RgallB5    = [ $RgallB5   , frame_RgallB6    ] {  };
void() frame_RgallB6    = [ $RgallB6   , frame_RgallB7    ] {  };
void() frame_RgallB7    = [ $RgallB7   , frame_RgallB8    ] {  };
void() frame_RgallB8    = [ $RgallB8   , frame_RgallB9    ] {  };
void() frame_RgallB9    = [ $RgallB9   , frame_RgallB10   ] {  };
void() frame_RgallB10   = [ $RgallB10  , frame_RgallB11   ] {  };
void() frame_RgallB11   = [ $RgallB11  , frame_RgallB12   ] {  };
void() frame_RgallB12   = [ $RgallB12  , frame_RgallB13   ] {  };
void() frame_RgallB13   = [ $RgallB13  , frame_RgallB14   ] {  };
void() frame_RgallB14   = [ $RgallB14  , frame_RgallB15   ] {  };
void() frame_RgallB15   = [ $RgallB15  , frame_RgallB16   ] {  };
void() frame_RgallB16   = [ $RgallB16  , frame_RgallB1    ] {  };

void() frame_RgallC1    = [ $RgallC1   , frame_RgallC2    ] {  };
void() frame_RgallC2    = [ $RgallC2   , frame_RgallC3    ] {  };
void() frame_RgallC3    = [ $RgallC3   , frame_RgallC4    ] {  };
void() frame_RgallC4    = [ $RgallC4   , frame_RgallC5    ] {  };
void() frame_RgallC5    = [ $RgallC5   , frame_RgallC6    ] {  };
void() frame_RgallC6    = [ $RgallC6   , frame_RgallC7    ] {  };
void() frame_RgallC7    = [ $RgallC7   , frame_RgallC8    ] {  };
void() frame_RgallC8    = [ $RgallC8   , frame_RgallC9    ] {  };
void() frame_RgallC9    = [ $RgallC9   , frame_RgallC10   ] {  };
void() frame_RgallC10   = [ $RgallC10  , frame_RgallC11   ] {  };
void() frame_RgallC11   = [ $RgallC11  , frame_RgallC12   ] {  };
void() frame_RgallC12   = [ $RgallC12  , frame_RgallC13   ] {  };
void() frame_RgallC13   = [ $RgallC13  , frame_RgallC14   ] {  };
void() frame_RgallC14   = [ $RgallC14  , frame_RgallC15   ] {  };
void() frame_RgallC15   = [ $RgallC15  , frame_RgallC16   ] {  };
void() frame_RgallC16   = [ $RgallC16  , frame_RgallC17   ] {  };
void() frame_RgallC17   = [ $RgallC17  , frame_RgallC18   ] {  };
void() frame_RgallC18   = [ $RgallC18  , frame_RgallC19   ] {  };
void() frame_RgallC19   = [ $RgallC19  , frame_RgallC20   ] {  };
void() frame_RgallC20   = [ $RgallC20  , frame_RgallC1    ] {  };

void() frame_RrearA1    = [ $RrearA1   , frame_RrearA2    ] {  };
void() frame_RrearA2    = [ $RrearA2   , frame_RrearA3    ] {  };
void() frame_RrearA3    = [ $RrearA3   , frame_RrearA4    ] {  };
void() frame_RrearA4    = [ $RrearA4   , frame_RrearA5    ] {  };
void() frame_RrearA5    = [ $RrearA5   , frame_RrearA6    ] {  };
void() frame_RrearA6    = [ $RrearA6   , frame_RrearA7    ] {  };
void() frame_RrearA7    = [ $RrearA7   , frame_RrearA8    ] {  };
void() frame_RrearA8    = [ $RrearA8   , frame_RrearA9    ] {  };
void() frame_RrearA9    = [ $RrearA9   , frame_RrearA10   ] {  };
void() frame_RrearA10   = [ $RrearA10  , frame_RrearA11   ] {  };
void() frame_RrearA11   = [ $RrearA11  , frame_RrearA12   ] {  };
void() frame_RrearA12   = [ $RrearA12  , frame_RrearA13   ] {  };
void() frame_RrearA13   = [ $RrearA13  , frame_RrearA14   ] {  };
void() frame_RrearA14   = [ $RrearA14  , frame_RrearA15   ] {  };
void() frame_RrearA15   = [ $RrearA15  , frame_RrearA16   ] {  };
void() frame_RrearA16   = [ $RrearA16  , frame_RrearA17   ] {  };
void() frame_RrearA17   = [ $RrearA17  , frame_RrearA18   ] {  };
void() frame_RrearA18   = [ $RrearA18  , frame_RrearA19   ] {  };
void() frame_RrearA19   = [ $RrearA19  , frame_RrearA20   ] {  };
void() frame_RrearA20   = [ $RrearA20  , frame_RrearA21   ] {  };
void() frame_RrearA21   = [ $RrearA21  , frame_RrearA22   ] {  };
void() frame_RrearA22   = [ $RrearA22  , frame_RrearA23   ] {  };
void() frame_RrearA23   = [ $RrearA23  , frame_RrearA24   ] {  };
void() frame_RrearA24   = [ $RrearA24  , frame_RrearA25   ] {  };
void() frame_RrearA25   = [ $RrearA25  , frame_RrearA26   ] {  };
void() frame_RrearA26   = [ $RrearA26  , frame_RrearA27   ] {  };
void() frame_RrearA27   = [ $RrearA27  , frame_RrearA28   ] {  };
void() frame_RrearA28   = [ $RrearA28  , frame_RrearA29   ] {  };
void() frame_RrearA29   = [ $RrearA29  , frame_RrearA30   ] {  };
void() frame_RrearA30   = [ $RrearA30  , frame_RrearA31   ] {  };
void() frame_RrearA31   = [ $RrearA31  , frame_RrearA32   ] {  };
void() frame_RrearA32   = [ $RrearA32  , frame_RrearA33   ] {  };
void() frame_RrearA33   = [ $RrearA33  , frame_RrearA34   ] {  };
void() frame_RrearA34   = [ $RrearA34  , frame_RrearA35   ] {  };
void() frame_RrearA35   = [ $RrearA35  , frame_RrearA36   ] {  };
void() frame_RrearA36   = [ $RrearA36  , frame_RrearA37   ] {  };
void() frame_RrearA37   = [ $RrearA37  , frame_RrearA38   ] {  };
void() frame_RrearA38   = [ $RrearA38  , frame_RrearA39   ] {  };
void() frame_RrearA39   = [ $RrearA39  , frame_RrearA40   ] {  };
void() frame_RrearA40   = [ $RrearA40  , frame_RrearA1    ] {  };

void() frame_RtranA1    = [ $RtranA1   , frame_RtranA2    ] {  };
void() frame_RtranA2    = [ $RtranA2   , frame_RtranA3    ] {  };
void() frame_RtranA3    = [ $RtranA3   , frame_RtranA4    ] {  };
void() frame_RtranA4    = [ $RtranA4   , frame_RtranA5    ] {  };
void() frame_RtranA5    = [ $RtranA5   , frame_RtranA6    ] {  };
void() frame_RtranA6    = [ $RtranA6   , frame_RtranA7    ] {  };
void() frame_RtranA7    = [ $RtranA7   , frame_RtranA8    ] {  };
void() frame_RtranA8    = [ $RtranA8   , frame_RtranA9    ] {  };
void() frame_RtranA9    = [ $RtranA9   , frame_RtranA10   ] {  };
void() frame_RtranA10   = [ $RtranA10  , frame_RtranA11   ] {  };
void() frame_RtranA11   = [ $RtranA11  , frame_RtranA12   ] {  };
void() frame_RtranA12   = [ $RtranA12  , frame_RtranA13   ] {  };
void() frame_RtranA13   = [ $RtranA13  , frame_RtranA14   ] {  };
void() frame_RtranA14   = [ $RtranA14  , frame_RtranA15   ] {  };
void() frame_RtranA15   = [ $RtranA15  , frame_RtranA16   ] {  };
void() frame_RtranA16   = [ $RtranA16  , frame_RtranA1    ] {  };

void() frame_RtranB1    = [ $RtranB1   , frame_RtranB2    ] {  };
void() frame_RtranB2    = [ $RtranB2   , frame_RtranB3    ] {  };
void() frame_RtranB3    = [ $RtranB3   , frame_RtranB4    ] {  };
void() frame_RtranB4    = [ $RtranB4   , frame_RtranB5    ] {  };
void() frame_RtranB5    = [ $RtranB5   , frame_RtranB6    ] {  };
void() frame_RtranB6    = [ $RtranB6   , frame_RtranB7    ] {  };
void() frame_RtranB7    = [ $RtranB7   , frame_RtranB8    ] {  };
void() frame_RtranB8    = [ $RtranB8   , frame_RtranB9    ] {  };
void() frame_RtranB9    = [ $RtranB9   , frame_RtranB10   ] {  };
void() frame_RtranB10   = [ $RtranB10  , frame_RtranB11   ] {  };
void() frame_RtranB11   = [ $RtranB11  , frame_RtranB12   ] {  };
void() frame_RtranB12   = [ $RtranB12  , frame_RtranB1    ] {  };




void create_warrider(entity horse)
{
	entity rider;

	rider = spawn();

	rider.solid = SOLID_NOT;
	rider.movetype = MOVETYPE_NONE;
	rider.origin = horse.origin;
	rider.angles = self.angles;

	setmodel (rider, "models/warrider.mdl");
	rider.skin = 0;

	horse.movechain = rider;
	rider.flags = rider.flags | FL_MOVECHAIN_ANGLE;
}

