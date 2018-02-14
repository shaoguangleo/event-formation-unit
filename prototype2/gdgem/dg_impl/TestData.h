#pragma once
#include <vector>

struct Hit
{
	unsigned int fec;
	unsigned int chip_id;
	unsigned int framecounter;
	unsigned int srs_timestamp;
	unsigned int channel;
	unsigned int bcid;
	unsigned int tdc;
	unsigned int adc;
	unsigned int overthreshold;
};


// Scrambled FrameCounter
std::vector<Hit> err_fc_error
{//              *
	{ 1,  7, 255783212, 236388352,  7, 4094, 202, 109, 1},
	{ 1, 10, 255783211, 236388352, 34, 839, 96, 93, 1},
};

// 'scrambled' (0) values occasionally seen
std::vector<Hit> err_bcid_tdc_error
{ //                                 *  * X
	{ 1,  0, 255783208, 236388352, 18, 0, 0, 574, 1},
	{ 1,  0, 255783208, 236388352, 19, 0, 0, 400, 1},
	//                                 *  * Y
	{ 1, 10, 255783212, 236388352, 36, 0, 0, 750, 1},
	{ 1, 10, 255783212, 236388352, 37, 0, 0, 410, 1},
};

// Scrambled trigger time
std::vector<Hit> err_triggertime_error
{
	{ 1, 14, 255783214, 236453888, 41, 4094, 161, 191, 1},
	{ 1, 14, 255783214, 236453888, 42, 4094, 193, 106, 1},
	{ 1,  1, 255783217, 236388352, 62, 2736, 209, 240, 1},
	{ 1,  1, 255783217, 236388352, 63, 2736, 210, 0, 1},
};


std::vector<Hit> Run16_line_110168_110323
{
{ 1,  0, 255783208, 236388352,  7, 839, 178, 176, 1},
{ 1,  0, 255783208, 236388352,  8, 839, 224, 352, 1},
{ 1,  0, 255783208, 236388352,  9, 839, 201, 464, 1},
{ 1,  0, 255783208, 236388352, 10, 838, 136, 270, 1},
{ 1,  0, 255783208, 236388352, 11, 841, 214, 447, 1},
{ 1,  0, 255783208, 236388352, 12, 840, 136, 288, 1},
{ 1,  0, 255783208, 236388352, 13, 840, 142, 160, 1},
{ 1,  0, 255783208, 236388352, 14, 840, 170, 120, 1},
{ 1,  0, 255783208, 236388352, 15, 840, 171, 143, 1},
{ 1,  0, 255783208, 236388352, 16, 840, 161, 224, 1},
{ 1,  0, 255783208, 236388352, 17, 840, 155, 528, 1},
{ 1,  0, 255783208, 236388352, 18, 840, 192, 574, 1},
{ 1,  0, 255783208, 236388352, 19, 840, 184, 400, 1},
{ 1,  0, 255783208, 236388352, 20, 840, 218, 183, 1},
{ 1,  1, 255783209, 236388352, 51, 0, 168, 144, 1},
{ 1,  1, 255783209, 236388352, 43, 2, 192, 216, 1},
{ 1,  1, 255783209, 236388352, 44, 2, 218, 304, 1},
{ 1,  1, 255783209, 236388352, 45, 1, 120, 108, 1},
{ 1,  6, 255783210, 236388352, 29, 2052, 88, 174, 1},
{ 1,  6, 255783210, 236388352, 30, 2056, 197, 121, 1},
{ 1,  6, 255783210, 236388352, 31, 2056, 176, 109, 1},
{ 1,  6, 255783210, 236388352, 32, 2056, 152, 145, 1},
{ 1,  6, 255783210, 236388352, 33, 2056, 161, 122, 1},
{ 1,  6, 255783210, 236388352, 34, 2056, 139, 185, 1},
{ 1,  6, 255783210, 236388352, 35, 2056, 122, 323, 1},
{ 1,  6, 255783210, 236388352, 36, 2056, 154, 184, 1},
{ 1,  6, 255783210, 236388352, 11, 2056, 136, 104, 1},
{ 1,  6, 255783210, 236388352, 12, 2056, 192, 193, 1},
{ 1,  6, 255783210, 236388352, 13, 2055, 96, 288, 1},
{ 1,  6, 255783210, 236388352, 14, 2055, 97, 288, 1},
{ 1,  6, 255783210, 236388352, 15, 2055, 131, 218, 1},
{ 1,  6, 255783210, 236388352, 23, 2054, 153, 96, 1},
{ 1,  6, 255783210, 236388352, 24, 2054, 160, 128, 1},
{ 1,  6, 255783210, 236388352, 26, 2054, 128, 237, 1},
{ 1,  6, 255783210, 236388352, 27, 2053, 160, 215, 1},
{ 1,  6, 255783210, 236388352, 28, 2053, 192, 336, 1},
{ 1,  6, 255783210, 236388352, 23, 2691, 172, 101, 1},
{ 1,  6, 255783210, 236388352, 56, 3805, 178, 170, 1},
{ 1,  6, 255783210, 236388352, 57, 3805, 164, 484, 1},
{ 1,  6, 255783210, 236388352, 58, 3805, 137, 400, 1},
{ 1,  6, 255783210, 236388352, 59, 3804, 64, 100, 1},
{ 1,  6, 255783210, 236388352, 53, 1, 128, 108, 1},
{ 1,  6, 255783210, 236388352, 54, 1, 144, 90, 1},
{ 1,  7, 255783211, 236388352,  2, 4095, 224, 187, 1},
{ 1,  7, 255783211, 236388352,  3, 4095, 208, 113, 1},
{ 1,  7, 255783211, 236388352,  7, 4094, 202, 109, 1},
{ 1, 10, 255783212, 236388352, 34, 839, 96, 93, 1},
{ 1, 10, 255783212, 236388352, 35, 840, 152, 383, 1},
{ 1, 10, 255783212, 236388352, 36, 840, 148, 750, 1},
{ 1, 10, 255783212, 236388352, 37, 840, 152, 410, 1},
{ 1, 10, 255783212, 236388352, 38, 839, 120, 114, 1},
{ 1, 10, 255783212, 236388352, 23, 840, 104, 103, 1},
{ 1, 10, 255783212, 236388352, 24, 841, 136, 138, 1},
{ 1, 10, 255783212, 236388352, 25, 841, 136, 433, 1},
{ 1, 10, 255783212, 236388352, 26, 841, 171, 497, 1},
{ 1, 10, 255783212, 236388352, 27, 840, 104, 345, 1},
{ 1, 10, 255783212, 236388352, 28, 840, 104, 463, 1},
{ 1, 10, 255783212, 236388352, 29, 840, 128, 288, 1},
{ 1, 10, 255783212, 236388352, 30, 840, 185, 112, 1},
{ 1, 11, 255783213, 236388352, 14, 838, 32, 224, 1},
{ 1, 11, 255783213, 236388352, 15, 838, 31, 625, 1},
{ 1, 11, 255783213, 236388352, 16, 838, 29, 545, 1},
{ 1, 11, 255783213, 236388352, 17, 838, 41, 132, 1},
{ 1, 11, 255783213, 236388352,  8, 2007, 39, 167, 1},
{ 1, 11, 255783213, 236388352,  9, 2007, 55, 113, 1},
{ 1, 14, 255783214, 236388352, 32, 2052, 80, 96, 1},
{ 1, 14, 255783214, 236388352, 33, 2052, 101, 134, 1},
{ 1, 14, 255783214, 236388352, 34, 2052, 123, 109, 1},
{ 1, 14, 255783214, 236388352, 35, 2052, 104, 96, 1},
{ 1, 14, 255783214, 236388352, 36, 2052, 120, 92, 1},
{ 1, 14, 255783214, 236388352, 37, 2052, 127, 104, 1},
{ 1, 14, 255783214, 236388352, 38, 2052, 101, 129, 1},
{ 1, 14, 255783214, 236388352, 39, 2053, 208, 106, 1},
{ 1, 14, 255783214, 236388352, 40, 2053, 177, 119, 1},
{ 1, 14, 255783214, 236388352, 41, 2054, 178, 80, 1},
{ 1, 14, 255783214, 236388352, 42, 2054, 169, 98, 1},
{ 1, 14, 255783214, 236388352, 43, 2054, 105, 137, 1},
{ 1, 14, 255783214, 236388352, 44, 2054, 72, 152, 1},
{ 1, 14, 255783214, 236388352, 45, 2055, 168, 145, 1},
{ 1, 14, 255783214, 236388352, 46, 2055, 136, 160, 1},
{ 1, 14, 255783214, 236388352, 47, 2055, 136, 149, 1},
{ 1, 14, 255783214, 236388352, 48, 2056, 208, 159, 1},
{ 1, 14, 255783214, 236388352, 49, 2056, 160, 216, 1},
{ 1, 14, 255783214, 236388352, 50, 2056, 147, 346, 1},
{ 1, 14, 255783214, 236388352, 51, 2056, 120, 263, 1},
{ 1, 14, 255783214, 236388352, 52, 2056, 145, 96, 1},
{ 1, 14, 255783214, 236388352, 13, 2056, 65, 158, 1},
{ 1, 14, 255783214, 236388352, 15, 2055, 88, 124, 1},
{ 1, 14, 255783214, 236388352, 16, 2055, 177, 128, 1},
{ 1, 14, 255783214, 236388352, 17, 2055, 176, 157, 1},
{ 1, 14, 255783214, 236388352, 18, 2055, 160, 96, 1},
{ 1, 14, 255783214, 236388352, 21, 2055, 153, 111, 1},
{ 1, 14, 255783214, 236388352, 22, 2055, 136, 139, 1},
{ 1, 14, 255783214, 236388352, 23, 2055, 96, 192, 1},
{ 1, 14, 255783214, 236388352, 24, 2055, 87, 169, 1},
{ 1, 14, 255783214, 236388352, 25, 2055, 131, 112, 1},
{ 1, 14, 255783214, 236388352, 26, 2055, 176, 120, 1},
{ 1, 14, 255783214, 236388352, 27, 2055, 179, 128, 1},
{ 1, 14, 255783214, 236388352, 28, 2054, 144, 211, 1},
{ 1, 14, 255783214, 236388352, 29, 2054, 172, 251, 1},
{ 1, 14, 255783214, 236388352, 30, 2053, 80, 134, 1},
{ 1, 14, 255783214, 236388352, 31, 2053, 168, 96, 1},
{ 1, 14, 255783214, 236388352, 13, 2691, 74, 132, 1},
{ 1, 14, 255783214, 236388352, 12, 2691, 80, 89, 1},
{ 1, 14, 255783214, 236388352,  0, 3804, 104, 92, 1},
{ 1, 14, 255783214, 236388352,  1, 3804, 72, 220, 1},
{ 1, 14, 255783214, 236388352,  2, 3805, 129, 695, 1},
{ 1, 14, 255783214, 236388352,  3, 3805, 108, 466, 1},
{ 1, 14, 255783214, 236388352,  4, 3804, 105, 112, 1},
{ 1, 14, 255783214, 236388352,  7, 4093, 58, 94, 1},
{ 1, 14, 255783214, 236388352,  8, 4094, 92, 96, 1},
{ 1, 14, 255783214, 236388352,  9, 4095, 63, 161, 1},
{ 1, 14, 255783214, 236388352, 10, 4095, 53, 87, 1},
{ 1, 14, 255783214, 236388352, 11, 2, 89, 96, 1},
{ 1, 14, 255783214, 236388352, 12, 0, 90, 81, 1},
{ 1, 14, 255783214, 236388352, 13, 0, 76, 205, 1},
{ 1, 14, 255783214, 236388352, 15, 1, 160, 92, 1},
{ 1, 14, 255783214, 236388352, 17, 2, 189, 265, 1},
{ 1, 14, 255783214, 236388352, 18, 2, 161, 378, 1},
{ 1, 14, 255783214, 236388352, 19, 1, 104, 108, 1},
{ 1, 14, 255783214, 236388352, 26, 1, 168, 87, 1},
{ 1, 14, 255783214, 236388352, 27, 1, 160, 145, 1},
{ 1, 14, 255783214, 236388352, 28, 1, 168, 96, 1},
{ 1, 14, 255783214, 236388352, 34, 0, 168, 88, 1},
{ 1, 14, 255783214, 236388352, 35, 4095, 88, 95, 1},
{ 1, 14, 255783214, 236388352, 39, 4095, 208, 192, 1},
{ 1, 14, 255783214, 236388352, 40, 4094, 121, 238, 1},
{ 1, 14, 255783214, 236388352, 41, 4094, 161, 191, 1},
{ 1, 14, 255783214, 236388352, 42, 4094, 193, 106, 1},
{ 1,  1, 255783217, 236453888, 62, 2736, 209, 240, 1},
{ 1,  1, 255783217, 236453888, 63, 2736, 210, 0, 1},
{ 1,  6, 255783218, 236453888,  2, 2736, 180, 214, 1},
{ 1,  6, 255783218, 236453888,  3, 2736, 145, 229, 1},
{ 1,  6, 255783218, 236453888,  4, 2737, 201, 167, 1},
{ 1,  6, 255783218, 236453888,  9, 2738, 178, 92, 1},
{ 1,  6, 255783218, 236453888, 22, 4045, 96, 106, 1},
{ 1,  6, 255783218, 236453888, 23, 4045, 144, 144, 1},
{ 1,  6, 255783218, 236453888, 24, 4045, 153, 120, 1},
{ 1, 11, 255783221, 236453888,  7, 2008, 32, 139, 1},
{ 1, 11, 255783221, 236453888, 59, 4044, 77, 144, 1},
{ 1, 11, 255783221, 236453888, 60, 4045, 172, 128, 1},
{ 1, 11, 255783221, 236453888, 61, 4045, 168, 135, 1},
{ 1, 14, 255783222, 236453888, 23, 2737, 168, 125, 1},
{ 1, 14, 255783222, 236453888, 24, 2737, 155, 333, 1},
{ 1, 14, 255783222, 236453888, 25, 2737, 168, 396, 1},
{ 1, 14, 255783222, 236453888, 26, 2737, 206, 290, 1},
{ 1, 14, 255783222, 236453888, 27, 2736, 144, 336, 1},
{ 1, 14, 255783222, 236453888, 28, 2736, 184, 368, 1},
{ 1, 14, 255783222, 236453888, 29, 2736, 168, 656, 1},
{ 1, 14, 255783222, 236453888, 30, 2736, 142, 663, 1},
{ 1, 14, 255783222, 236453888, 31, 2736, 216, 208, 1},
{ 1, 14, 255783222, 236453888, 30, 3611, 153, 71, 1},
{ 1, 14, 255783222, 236453888,  0, 4046, 104, 96, 1},
{ 1, 14, 255783222, 236453888,  6, 4048, 91, 86, 1},
{ 1, 14, 255783222, 236453888, 16, 1732, 109, 112, 1},
{ 1, 15, 255783223, 236453888, 22, 1492, 152, 53, 1}};
