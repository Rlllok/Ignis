float IF_Lerp(float a, float b, float t)
{
  return a * (1.0f - t) + b * t;
}

float IG_InverseLerp(float a, float b, float v)
{
  return (v - a) / (b - a);
}

float IG_Remap(float value, float in_min, float in_max, float out_min, float out_max)
{
  return value / (in_max - in_min) * (out_max - out_min);
}
