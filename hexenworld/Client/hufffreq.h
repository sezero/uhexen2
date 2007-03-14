/*
	hufffreq.h
	huffman freq table for use in hexenworld networking

	$Id: hufffreq.h,v 1.2 2007-03-14 21:03:33 sezero Exp $
*/

/*
	0.27588720,
	0.04243389,
	0.01598893,
	0.00737722,
	0.00557754,
	0.00547342,
	0.00823988,
	0.00449177,
	0.00986108,
	0.00560728,
	0.00654431,
	0.00376298,
	0.00498260,
	0.00400095,
	0.00655918,
	0.00232025,
	0.00504209,
	0.00285570,
	0.00124937,
	0.00147247,
	0.00226076,
	0.00141298,
	0.00467026,
	0.00336139,
	0.00123449,
	0.00126424,
	0.00166582,
	0.00129399,
	0.00114525,
	0.00116013,
	0.00078829,
	0.00080317,
	0.00639557,
	0.00123449,
	0.00127911,
	0.00102627,
	0.00182943,
	0.00141298,
	0.00269209,
	0.00127911,
	0.00224589,
	0.00093703,
	0.01972216,
	0.00135348,
	0.00477437,
	0.00337627,
	0.00743671,
	0.00765981,
	0.01951394,
	0.00319779,
	0.00330190,
	0.00267722,
	0.00235000,
	0.00159146,
	0.00285570,
	0.00141298,
	0.00151709,
	0.00139810,
	0.00468513,
	0.00117500,
	0.00202279,
	0.00544367,
	0.00096677,
	0.00136836,
	0.00913228,
	0.00316804,
	0.00138323,
	0.00078829,
	0.00942975,
	0.00590475,
	0.00168070,
	0.00089241,
	0.00095190,
	0.00166582,
	0.00077342,
	0.00071392,
	0.00086266,
	0.00060981,
	0.00075854,
	0.00065443,
	0.00126424,
	0.00047595,
	0.00068418,
	0.00099652,
	0.00065443,
	0.00096677,
	0.00072880,
	0.00050570,
	0.00071392,
	0.00102627,
	0.00120475,
	0.00056519,
	0.00281108,
	0.00175506,
	0.00047595,
	0.00154684,
	0.00160633,
	0.01091710,
	0.00365886,
	0.00355475,
	0.00937026,
	0.01105096,
	0.00404557,
	0.00206741,
	0.00389684,
	0.00456614,
	0.00215665,
	0.00191867,
	0.01072374,
	0.01051551,
	0.00467026,
	0.00867121,
	0.00542880,
	0.00105601,
	0.00649969,
	0.01280602,
	0.00510159,
	0.00316804,
	0.00456614,
	0.00523545,
	0.00157658,
	0.00191867,
	0.00121962,
	0.00065443,
	0.00080317,
	0.00052057,
	0.00080317,
	0.00147247,
	0.02236963,
	0.00258798,
	0.00059494,
	0.00060981,
	0.00038671,
	0.00044620,
	0.00055032,
	0.00093703,
	0.00072880,
	0.00062468,
	0.00059494,
	0.00047595,
	0.00044620,
	0.00182943,
	0.00066930,
	0.00077342,
	0.00114525,
	0.00062468,
	0.00059494,
	0.00049082,
	0.00059494,
	0.00074367,
	0.00090728,
	0.00069905,
	0.00075854,
	0.00077342,
	0.00069905,
	0.00055032,
	0.00075854,
	0.00049082,
	0.00104114,
	0.00062468,
	0.00397121,
	0.00080317,
	0.00072880,
	0.00062468,
	0.00163608,
	0.00046108,
	0.00055032,
	0.00056519,
	0.00102627,
	0.00071392,
	0.00092215,
	0.00078829,
	0.00660380,
	0.00058006,
	0.00065443,
	0.00049082,
	0.00118987,
	0.00077342,
	0.00065443,
	0.00077342,
	0.00151709,
	0.00083291,
	0.00074367,
	0.00098165,
	0.00108576,
	0.00081804,
	0.00145760,
	0.00144272,
	0.00394146,
	0.00104114,
	0.00099652,
	0.00209715,
	0.00502722,
	0.00309367,
	0.00123449,
	0.00096677,
	0.00090728,
	0.00206741,
	0.00147247,
	0.00147247,
	0.00129399,
	0.00087753,
	0.00120475,
	0.00116013,
	0.00145760,
	0.00120475,
	0.00138323,
	0.00123449,
	0.00166582,
	0.00087753,
	0.00171044,
	0.00239462,
	0.00156171,
	0.00154684,
	0.00217152,
	0.00226076,
	0.00211203,
	0.00153196,
	0.00174019,
	0.00145760,
	0.00168070,
	0.00145760,
	0.00123449,
	0.00165095,
	0.00188893,
	0.00138323,
	0.00120475,
	0.00154684,
	0.00138323,
	0.00191867,
	0.01866615,
	0.00139810,
	0.00153196,
	0.00093703,
	0.00121962,
	0.00116013,
	0.00075854,
	0.00105601,
	0.00432817,
	0.00315317,
	0.00407532,
	0.00227563,
	0.00081804,
	0.00121962,
	0.00110063,
	0.00090728,
	0.00108576,
	0.00065443,
	0.00096677,
	0.00655918,
	0.00153196,
	0.00251361,
	0.00312342,
	0.00243924,
	0.00660380,
	0.01700033
*/
	0.14473691,
	0.01147017,
	0.00167522,
	0.03831121,
	0.00356579,
	0.03811315,
	0.00178254,
	0.00199644,
	0.00183511,
	0.00225716,
	0.00211240,
	0.00308829,
	0.00172852,
	0.00186608,
	0.00215921,
	0.00168891,
	0.00168603,
	0.00218586,
	0.00284414,
	0.00161833,
	0.00196043,
	0.00151029,
	0.00173932,
	0.00218370,
	0.00934121,
	0.00220530,
	0.00381211,
	0.00185456,
	0.00194675,
	0.00161977,
	0.00186680,
	0.00182071,
	0.06421956,
	0.00537786,
	0.00514019,
	0.00487155,
	0.00493925,
	0.00503143,
	0.00514019,
	0.00453520,
	0.00454241,
	0.00485642,
	0.00422407,
	0.00593387,
	0.00458130,
	0.00343687,
	0.00342823,
	0.00531592,
	0.00324890,
	0.00333388,
	0.00308613,
	0.00293776,
	0.00258918,
	0.00259278,
	0.00377105,
	0.00267488,
	0.00227516,
	0.00415997,
	0.00248763,
	0.00301555,
	0.00220962,
	0.00206990,
	0.00270369,
	0.00231694,
	0.00273826,
	0.00450928,
	0.00384380,
	0.00504728,
	0.00221251,
	0.00376961,
	0.00232990,
	0.00312574,
	0.00291688,
	0.00280236,
	0.00252436,
	0.00229461,
	0.00294353,
	0.00241201,
	0.00366590,
	0.00199860,
	0.00257838,
	0.00225860,
	0.00260646,
	0.00187256,
	0.00266552,
	0.00242641,
	0.00219450,
	0.00192082,
	0.00182071,
	0.02185930,
	0.00157439,
	0.00164353,
	0.00161401,
	0.00187544,
	0.00186248,
	0.03338637,
	0.00186968,
	0.00172132,
	0.00148509,
	0.00177749,
	0.00144620,
	0.00192442,
	0.00169683,
	0.00209439,
	0.00209439,
	0.00259062,
	0.00194531,
	0.00182359,
	0.00159096,
	0.00145196,
	0.00128199,
	0.00158376,
	0.00171412,
	0.00243433,
	0.00345704,
	0.00156359,
	0.00145700,
	0.00157007,
	0.00232342,
	0.00154198,
	0.00140730,
	0.00288807,
	0.00152830,
	0.00151246,
	0.00250203,
	0.00224420,
	0.00161761,
	0.00714383,
	0.08188576,
	0.00802537,
	0.00119484,
	0.00123805,
	0.05632671,
	0.00305156,
	0.00105584,
	0.00105368,
	0.00099246,
	0.00090459,
	0.00109473,
	0.00115379,
	0.00261223,
	0.00105656,
	0.00124381,
	0.00100326,
	0.00127550,
	0.00089739,
	0.00162481,
	0.00100830,
	0.00097229,
	0.00078864,
	0.00107240,
	0.00084409,
	0.00265760,
	0.00116891,
	0.00073102,
	0.00075695,
	0.00093916,
	0.00106880,
	0.00086786,
	0.00185600,
	0.00608367,
	0.00133600,
	0.00075695,
	0.00122077,
	0.00566955,
	0.00108249,
	0.00259638,
	0.00077063,
	0.00166586,
	0.00090387,
	0.00087074,
	0.00084914,
	0.00130935,
	0.00162409,
	0.00085922,
	0.00093340,
	0.00093844,
	0.00087722,
	0.00108249,
	0.00098598,
	0.00095933,
	0.00427593,
	0.00496661,
	0.00102775,
	0.00159312,
	0.00118404,
	0.00114947,
	0.00104936,
	0.00154342,
	0.00140082,
	0.00115883,
	0.00110769,
	0.00161112,
	0.00169107,
	0.00107816,
	0.00142747,
	0.00279804,
	0.00085922,
	0.00116315,
	0.00119484,
	0.00128559,
	0.00146204,
	0.00130215,
	0.00101551,
	0.00091756,
	0.00161184,
	0.00236375,
	0.00131872,
	0.00214120,
	0.00088875,
	0.00138570,
	0.00211960,
	0.00094060,
	0.00088083,
	0.00094564,
	0.00090243,
	0.00106160,
	0.00088659,
	0.00114514,
	0.00095861,
	0.00108753,
	0.00124165,
	0.00427016,
	0.00159384,
	0.00170547,
	0.00104431,
	0.00091395,
	0.00095789,
	0.00134681,
	0.00095213,
	0.00105944,
	0.00094132,
	0.00141883,
	0.00102127,
	0.00101911,
	0.00082105,
	0.00158448,
	0.00102631,
	0.00087938,
	0.00139290,
	0.00114658,
	0.00095501,
	0.00161329,
	0.00126542,
	0.00113218,
	0.00123661,
	0.00101695,
	0.00112930,
	0.00317976,
	0.00085346,
	0.00101190,
	0.00189849,
	0.00105728,
	0.00186824,
	0.00092908,
	0.00160896
