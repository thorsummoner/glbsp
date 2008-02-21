//------------------------------------------------------------------------
//  BRUSH Creation
//------------------------------------------------------------------------
//
//  Doom-2-Brush (C) 2008 Andrew Apted
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//------------------------------------------------------------------------

// this includes everything we need
#include "defs.h"


#define DUMMY_TEX  "e7/e7panelwood2"


class brush_side_c
{
public:
  double x1, y1;
  double x2, y2;

public:
   brush_side_c() { }
  ~brush_side_c() { }
};

static void GetBrushSides(subsec_c *sub, std::vector<brush_side_c>& sides)
{
  sides.clear();

  for (unsigned int k = 0; k < sub->seg_list.size(); k++)
  {
    seg_c * seg = sub->seg_list[k];

    // check if already present (co-linear segs)
    bool already = false;
    for (unsigned int n = 0; n < sides.size(); n++)
    {
      brush_side_c& b = sides[n];

      double ps = PerpDist(seg->start->x, seg->start->y,
                           b.x1, b.y1, b.x2, b.y2);

      double pe = PerpDist(seg->end->x, seg->end->y,
                           b.x1, b.y1, b.x2, b.y2);

      if (fabs(ps) < 0.02 && fabs(pe) < 0.02)
      {
        already = true; break;
      }
    }

    if (already)
      continue;

    brush_side_c new_b;

    // make the brush_side face OUTWARDS (segs face inwards)
    new_b.x1 = seg->end->x;
    new_b.y1 = seg->end->y;
    new_b.x2 = seg->start->x;
    new_b.y2 = seg->start->y;

    sides.push_back(new_b);
  }
}


void Brush_ConvertSectors(void)
{
  std::vector<brush_side_c> sides;

  for (int i = 0; i < lev_subsecs.num; i++)
  {
    subsec_c *sub = lev_subsecs.Get(i);

    sector_c *S = sub->sector;

    if (! sub->sector)
    {
      fprintf(stderr, "Warning: subsec:%d has no sector\n", i);
      continue;
    }

    GetBrushSides(sub, sides);

    if (sides.size() < 3)
    {
      fprintf(stderr, "Warning: subsec:%d in sector:%d invalid (only %d sides)\n",
              i, S->index, (int)sides.size());
      continue;
    }

    for (int is_ceil = 0; is_ceil < 2; is_ceil++)
    {
      double z1 = is_ceil ? S->ceil_h        : S->floor_h - 64.0;
      double z2 = is_ceil ? S->ceil_h + 64.0 : S->floor_h;

      fprintf(map_fp, "// %s sector:%d subsec:%d\n",
              is_ceil ? "Ceiling" : "Floor", S->index, i);

      fprintf(map_fp, "{\n");

      // Top
      fprintf(map_fp, "  ( %1.1f %1.1f %1.1f ) ( %1.1f %1.1f %1.1f ) ( %1.1f %1.1f %1.1f ) %s 0 0 0 1 1\n",
          0.0, 0.0, z2,  0.0, 1.0, z2,  1.0, 0.0, z2,
          DUMMY_TEX);

      // Bottom
      fprintf(map_fp, "  ( %1.1f %1.1f %1.1f ) ( %1.1f %1.1f %1.1f ) ( %1.1f %1.1f %1.1f ) %s 0 0 0 1 1\n",
          0.0, 0.0, z1,  1.0, 0.0, z1,  0.0, 1.0, z1,
          DUMMY_TEX);

      // Sides
      for (unsigned int k = 0; k < sides.size(); k++)
      {
        brush_side_c& b = sides[k];

        fprintf(map_fp, "  ( %1.1f %1.1f %1.1f ) ( %1.1f %1.1f %1.1f ) ( %1.1f %1.1f %1.1f ) %s 0 0 0 1 1\n",
            b.x1, b.y1, z1,  b.x1, b.y1, z2,  b.x2, b.y2, z2,
            DUMMY_TEX);
      }

      fprintf(map_fp, "}\n");
    }
  }
}


//------------------------------------------------------------------------

void Brush_ConvertWalls(void)
{
  for (int i = 0; i < lev_linedefs.num; i++)
  {
    linedef_c *L = lev_linedefs[i];

    @@@
  }
}


//------------------------------------------------------------------------

void Brush_WriteField(const char *field, const char *val_str, ...)
{
  fprintf(map_fp, "  \"%s\"  \"", field);

  va_list arg_ptr;

  va_start(arg_ptr, val_str);
  vfprintf(map_fp, val_str, arg_ptr);
  va_end(arg_ptr);

  fprintf(map_fp, "\"\n");
}

void Brush_ConvertThings(void)
{
  for (int i = 0; i < lev_things.num; i++)
  {
    thing_c *T = lev_things.Get(i);

    // FIXME: thing --> entity conversion
    if (T->type != 1)
      continue;

    int z = 128;  // FIXME !!!!

    fprintf(map_fp, "// thing #%d type:%d\n", i, T->type);
    fprintf(map_fp, "{\n");

    Brush_WriteField("classname", "info_player_start");
    Brush_WriteField("origin", "%d %d %d", T->x, T->y, z + 25);

    // FIXME: Brush_WriteField("angle", "%d", T->angle);

    fprintf(map_fp, "}\n");
  }
}

