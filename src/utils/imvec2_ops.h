#ifndef IMVEC2OPS_H
#define IMVEC2OPS_H

#include "imgui.h"

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }
inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }
inline ImVec2 operator*(const ImVec2& a, float s)         { return ImVec2(a.x * s, a.y * s); }
inline ImVec2 operator*(float s, const ImVec2& a)         { return ImVec2(a.x * s, a.y * s); }
inline ImVec2 operator/(const ImVec2& a, float s)         { return ImVec2(a.x / s, a.y / s); }


#endif // NUMBERSFORMAT_H