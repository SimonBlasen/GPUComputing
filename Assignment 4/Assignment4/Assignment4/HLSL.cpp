#include "HLSL.h"

#ifdef _MSC_VER

#include <math.h>

float hlsl::acos(float x) {
	return ::acosf(x);
}

float hlsl::asin(float x) {
	return ::asinf(x);
}

float hlsl::atan2(float y, float x) {
	return ::atan2f(y, x);
}

float hlsl::cos(float x) {
	return ::cos(x);
}

float hlsl::sin(float x) {
	return ::sin(x);
}

float hlsl::sqrt(float x) {
	return ::sqrt(x);
}

float hlsl::tan(float x) {
	return ::tan(x);
}

#endif
