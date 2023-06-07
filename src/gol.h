#define MIN(A, B) ((A) > (B) ? (B) : (A))
#define MAX(A, B) ((A) < (B) ? (B) : (A))
#define CLAMP(N, L, U) ((N) < (L) ? (L) : ((N) > (U) ? (U) : (N)))

#define PI32 3.1415926535f

f32
Sqrt(f32 n)
{
  return (Float_Bits){ .bits = _mm_extract_ps(_mm_sqrt_ps(_mm_set1_ps(n)), 0)}.f;
}

f32
Exp(f32 n)
{
  return (Float_Bits){ .bits = _mm_extract_ps(_mm_exp_ps(_mm_set1_ps(n)), 0) }.f
}

// NOTE: modified to read float cell values
void
ConwayStep(f32* old_grid, f32* new_grid, u32 grid_width, u32 grid_height)
{
  for (umm y = 0; y < grid_height; ++y)
  {
    for (umm x = 0; x < grid_width; ++x)
    {
      u32 alive_neighbours = 0;

      for (umm ny = (umm)MAX(0, (imm)y - 1); ny < MIN(grid_height, y + 2); ++ny)
      {
        for (umm nx = (umm)MAX(0, (imm)x - 1); nx < MIN(grid_width, x + 2); ++nx)
        {
          alive_neighbours += (old_grid[ny*grid_width + nx] > 0.5f);
        }
      }

      bool cell_is_alive = (old_grid[y*grid_width + x] > 0.5f);
      alive_neighbours -= cell_is_alive;

      if (cell_is_alive && alive_neighbours == 2 || alive_neighbours == 3) new_grid[y*grid_width + x] = 1;
      else                                                                 new_grid[y*grid_width + x] = 0;
    }
  }
}

u32
CellValueToColor(f32 value)
{
  u32 r = (u32)CLAMP(value*255, 0, 255);
  return 0xFF000000 | (r << 16) | (r << 8) | r;
}

void
BlockCopy(f32* grid, u32 grid_width, u32 grid_height, f32* block, u32 x, u32 y, u32 width, u32 height)
{
  for (u32 dy = 0; dy < height; ++dy)
  {
    for (u32 dx = 0; dx < width; ++dx)
    {
      grid[(y+dy)*grid_width + (x+dx)] = block[dy*width + dx];
    }
  }
}

void
ConwayPulsar(f32* grid, u32 grid_width, u32 grid_height, u32 x, u32 y)
{
  u32 block_size = 15;
  f32 block[15*15] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0,
    0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0,
    0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0,
    0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0,
    0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };

  BlockCopy(grid, grid_width, grid_height, block, x - block_size, y - block_size, block_size, block_size);
}

typedef struct Smooth_Params
{
  float alpha;
  float ri, ra;
  float b;
  float b1, b2;
  float d1, d2;
} Smooth_Params;

static f32
Sigma1(Smooth_Params params, f32 x, f32 a)
{
  return 1 / (1 + Exp(-(x - a)*(4/params.alpha));
}

static f32
Sigma2(Smooth_Params params, f32 x, f32 a, f32 b)
{
  return Sigma1(params, x, a) * (1 - Sigma1(params, x, b));
}

static f32
SigmaM(Smooth_Params params, f32 x, f32 y, f32 m)
{
  return x*(1 - Sigma1(params, m, 0.5f)) + y*Sigma1(params, m, 0.5f);
}

static f32
S(Smooth_Params params, f32 n, f32 m)
{
  return Sigma2(params, n, SigmaM(params, params.b1, params.d1, m), SigmaM(params, params.b2, params.d2, m));
}

static f32
M(Smooth_Params params, f32* grid, u32 grid_width, u32 grid_height, u32 x, u32 y)
{
  f32 acc = 0;
  imm r = (imm)(params.ri + params.b + 0.5f);
  for (imm dy = -r; dy <= r; ++dy)
  {
    for (imm dx = -r; dx <= r; ++dx)
    {
      umm gx = (x + dx + grid_width)  % grid_width;
      umm gy = (y + dy + grid_height) % grid_height;

      f32 cell_value = grid[gy*grid_width + gx];

      float l = Sqrt((f32)(dx*dx + dy*dy));
      if      (l <  params.ri - (f32)params.b/2) acc += cell_value;
      else if (l <= params.ri + (f32)params.b/2) acc += cell_value * (f32)(params.ri + (f32)params.b/2 - l)/params.b;
    }
  }

  return acc / (PI32 * params.ri * params.ri);
}

static f32
N(Smooth_Params params, f32* grid, u32 grid_width, u32 grid_height, u32 x, u32 y)
{
  f32 acc = 0;
  imm r = (imm)(params.ri + params.b + 0.5f);
  for (imm dy = -r; dy <= r; ++dy)
  {
    for (imm dx = -r; dx <= r; ++dx)
    {
      umm gx = (x + dx + grid_width)  % grid_width;
      umm gy = (y + dy + grid_height) % grid_height;

      f32 cell_value = grid[gy*grid_width + gx];

      float l = Sqrt((f32)(dx*dx + dy*dy));
      if      (l <  params.ri - (f32)params.b/2) acc += cell_value;
      else if (l <= params.ri + (f32)params.b/2) acc += cell_value * (f32)(params.ri + (f32)params.b/2 - l)/params.b;
    }
  }

  return acc / (PI32 * params.ri * params.ri);
}

void
Init(f32* grid, u32 grid_width, u32 grid_height)
{
  ConwayPulsar(grid, grid_width, grid_height, grid_width/2, grid_height/2);
}

void
Tick(u32* screen, u32 screen_width, u32 screen_height, f32* old_grid, f32* new_grid, u32 grid_width, u32 grid_height)
{
  ConwayStep(old_grid, new_grid, grid_width, grid_height);

  // TODO: up or down scale, maybe?
  for (u32 y = 0; y < screen_height; ++y)
  {
    for (u32 x = 0; x < screen_width; ++x)
    {
      u32 color = CellValueToColor(new_grid[(y/4)*grid_width + (x/4)]);
      for (u32 uy = y; uy < MIN(screen_height, y + 4); ++uy)
      {
        for (u32 ux = x; ux < MIN(screen_width, x + 4); ++ux)
        {
          screen[uy*screen_width + ux] = color;
        }
      }
    }
  }
}
