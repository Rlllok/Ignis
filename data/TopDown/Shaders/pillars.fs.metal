using namespace metal;

struct PillarsData {
  float3   player_position;
  float3   camera_position;
  float4x4 camera_inverse;
  float4   color;
  float2   resolution;
};

struct VertexOutput {
  float4 position [[position]];
};

float SDF_Box(float3 position, float3 dimension) {
  float3 t = abs(position) - dimension;
  return length(max(t, 0.0f)) + min(max(t.x, max(t.y, t.z)), 0.0f);
}

float SDF_Scene(float3 position, float3 player_position) {
  float result = 500.0f;

  float cell_size = 3.5f;
  float2 cell_center = round(position.xz/cell_size);
  float3 q = position - float3(0.0f, -20.0f, 0.0f);
  float t = 1.0f - smoothstep(0.0f, 15.0f, length(cell_size*cell_center - player_position.xz));
  float pillar_height = 20.0f + 8.0f*t;
  q.xz = position.xz - cell_size*cell_center;
  q.y -= -20.0f;
  q.y -= 0.5f*pillar_height;

  result = SDF_Box(q, float3(0.5f, 0.5f*pillar_height, 0.5f));

  return result;
}

float3 CalculateNormals(float3 position, float3 player_position) {
  float2 e = float2(0.01f, 0.0f);
  float3 result = SDF_Scene(position, player_position) - float3(
    SDF_Scene(position - e.xyy, player_position),
    SDF_Scene(position - e.yxy, player_position),
    SDF_Scene(position - e.yyx, player_position)
  );
  return normalize(result);
}

fragment float4 FragmentMain(
  VertexOutput input [[stage_in]],
  constant PillarsData* pillars_data [[buffer(1)]]
) {
  float2 ndc = (input.position.xy/pillars_data->resolution)*2.0f - 1.0f;
  ndc.y *= -1.0f;
  float ray_length = 0.0f;
  float3 ray_origin = pillars_data->camera_position;
  float4 target = pillars_data->camera_inverse*float4(ndc, 1.0f, 1.0f);
  target /= target.w;
  float3 ray_direction = normalize(target.xyz - ray_origin);
  for (int i = 0; i < 50; i += 1) {
    float3 position = ray_origin + ray_direction*ray_length;
    float ray_step = SDF_Scene(position, pillars_data->player_position);
    if (ray_length > 100.0f || ray_step < 0.001f) {
      break;
    }
    ray_length += ray_step;
  }

  float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
  if (ray_length < 100.0f) {
    float3 position = ray_origin + ray_direction*ray_length;
    float3 normal = CalculateNormals(position, pillars_data->player_position);
    color = pillars_data->color;
    float3 light_direction = normalize(float3(1.0f, 1.0f, 1.0f));
    float diffuse_factor = max(dot(normal, light_direction), 0.0f);
    color.rgb *= diffuse_factor;
  }
  return color;
}

