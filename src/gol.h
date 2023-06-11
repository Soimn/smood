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
  return (Float_Bits){ .bits = _mm_extract_ps(_mm_exp_ps(_mm_set1_ps(n)), 0) }.f;
}

f32
Sigma1(f32 x, f32 a, f32 alpha)
{
  return 1 / (1 + Exp(-(x-a)*(4/alpha)));
}

f32
Sigma2(f32 x, f32 a, f32 b, f32 alpha)
{
  return Sigma1(x, a, alpha)*(1 - Sigma1(x, b, alpha));
}

f32
SigmaM(f32 x, f32 y, f32 m, f32 alpha)
{
  return x*(1 - Sigma1(m, 0.5f, alpha)) + y*Sigma1(m, 0.5f, alpha);
}

f32
S(f32 n, f32 m, f32 alpha, f32 b1, f32 b2, f32 d1, f32 d2)
{
  return Sigma2(n, SigmaM(b1, d1, m, alpha), SigmaM(b2, d2, m, alpha), alpha);
}

void
Init(f32* grid, u32 grid_width, u32 grid_height)
{
  for (u32 y = 0; y <= grid_height; ++y)
  {
    for (u32 x = 0; x <= grid_width; ++x)
    {
      u32 random_u32;
      while (!_rdrand32_step(&random_u32));

      f32 rand_unit_f32 = (Float_Bits){ .bits = (127 << 23) | (random_u32 >> 9) }.f - 1;

      grid[y*grid_width + x] = rand_unit_f32;
    }
  }
}

void
Tick(u32* screen, u32 screen_width, u32 screen_height, f32* old_grid, f32* new_grid, u32 grid_width, u32 grid_height)
{
  imm r_i       = 7;
  imm r_a       = 21;
  float b1      = 0.278f;
  float b2      = 0.365f;
  float d1      = 0.267f;
  float d2      = 0.445f;
  float alpha_n = 0.028f;
  float alpha_m = 0.0147f;
  float dt      = 0.1667f;

  for (imm y = 0; y < grid_height; ++y)
  {
    for (imm x = 0; x < grid_width; ++x)
    {
      f32 m = 0;
      u32 M = 0;
      f32 n = 0;
      u32 N = 0;

      imm padded_radius = (imm)(r_a + 1);
      for (imm dy = -padded_radius; dy <= padded_radius; ++dy)
      {
        for (imm dx = -padded_radius; dx <= padded_radius; ++dx)
        {
          u32 wrapped_y  = (u32)(y+dy+grid_height)%grid_height;
          u32 wrapped_x  = (u32)(x+dx+grid_width) %grid_width;
          f32 f          = old_grid[wrapped_y*grid_width + wrapped_x];

          imm l_sq = dx*dx + dy*dy;

          if (l_sq < r_i)
          {
            m += f;
            M += 1;
          }
          else if (l_sq < r_a)
          {
            n += f;
            N += 1;
          }
        }
      }

      m /= M;
      n /= N;

      // TODO: alpha
      f32 s = 2*S(n, m, alpha_n, b1, b2, d1, d2) - 1;

      f32 f     = old_grid[y*grid_width + x];
      f32 df_dt = s*f;

      new_grid[y*grid_width + x] = f + df_dt*dt;
    }
  }

  // TODO: up or down scale, maybe?
  u32 grid_cell_pixel_size = 8;
  for (u32 y = 0; y < screen_height; ++y)
  {
    for (u32 x = 0; x < screen_width; ++x)
    {
      f32 value = new_grid[(y/grid_cell_pixel_size)*grid_width + (x/grid_cell_pixel_size)];

      u32 r = (u32)CLAMP(value*255, 0, 255);
      u32 color = 0xFF000000 | (r << 16) | (r << 8) | (r);
      for (u32 uy = y; uy < MIN(screen_height, y + grid_cell_pixel_size); ++uy)
      {
        for (u32 ux = x; ux < MIN(screen_width, x + grid_cell_pixel_size); ++ux)
        {
          screen[uy*screen_width + ux] = color;
        }
      }
    }
  }
}
