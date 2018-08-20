#pragma once
#include "../stdafx.h"

array inRange(array image, int r_low, int g_low, int b_low, int r_high, int g_high, int b_high);
array inRange(array image, int low, int height);

array inRange(array image, int r_low, int g_low, int b_low, int r_high, int g_high, int b_high) {
	seq row(image.dims(0)), col(image.dims(1));
	return inRange(image(row, col, 0), r_low, r_high) && inRange(image(row, col, 1), g_low, g_high) && inRange(image(row, col, 2), b_low, b_high);
}

array inRange(array image, int low, int high) {
	return image >= low && image <= high;
}