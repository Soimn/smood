typedef signed __int8  i8;
typedef signed __int16 i16;
typedef signed __int32 i32;
typedef signed __int64 i64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef u8 bool;
#define true ((bool)1)
#define false ((bool)0)

typedef float f32;

typedef i64 imm;
typedef u64 umm;

#define ASSERT(EX) ((EX) ? 1 : (*(volatile int*)0 = 0, 0))

#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

typedef struct Game_State
{
	u32 screen_width;
	u32 screen_height;
	u32* screen_backbuffer;
} Game_State;

typedef void (*Game_Tick_Func)(Game_State* state);
