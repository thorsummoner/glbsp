//------------------------------------------------------------------------
//  GRID : Draws the map (lines, nodes, etc)
//------------------------------------------------------------------------
//
//  GL-Node Viewer (C) 2004-2005 Andrew Apted
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


#if 0
static void foo_win_close_CB(Fl_Widget *w, void *data)
{
}
#endif


//
// W_Grid Constructor
//
W_Grid::W_Grid(int X, int Y, int W, int H, const char *label) : 
        Fl_Widget(X, Y, W, H, label),
        zoom(DEF_GRID_ZOOM), zoom_mul(1.0),
        mid_x(0), mid_y(0),
		grid_MODE(1), partition_MODE(1), miniseg_MODE(2), shade_MODE(1),
		route_len(0)
{
	visit_route = new char[MAX_ROUTE];
}

//
// W_Grid Destructor
//
W_Grid::~W_Grid()
{
	delete[] visit_route;
}


void W_Grid::SetZoom(int new_zoom)
{
	if (new_zoom < MIN_GRID_ZOOM)
		new_zoom = MIN_GRID_ZOOM;

	if (new_zoom > MAX_GRID_ZOOM)
		new_zoom = MAX_GRID_ZOOM;

	if (zoom == new_zoom)
		return;

	zoom = new_zoom;

	zoom_mul = pow(2.0, (zoom / 2.0 - 9.0));

	//  fprintf(stderr, "Zoom %d  Mul %1.5f\n", zoom, zoom_mul);

	redraw();
}


void W_Grid::SetPos(double new_x, double new_y)
{
	mid_x = new_x;
	mid_y = new_y;

	redraw();
}

void W_Grid::FitBBox(double lx, double ly, double hx, double hy)
{
	double dx = hx - lx;
	double dy = hy - ly;

	zoom = MAX_GRID_ZOOM;

	for (; zoom > MIN_GRID_ZOOM; zoom--)
	{
		zoom_mul = pow(2.0, (zoom / 2.0 - 9.0));

		if (dx * zoom_mul < w() && dy * zoom_mul < h())
			break;
	}

	zoom_mul = pow(2.0, (zoom / 2.0 - 9.0));

	SetPos(lx + dx / 2.0, ly + dy / 2.0);
}

void W_Grid::MapToWin(double mx, double my, int *X, int *Y) const
{
	double hx = x() + w() / 2.0;
	double hy = y() + h() / 2.0;

	(*X) = I_ROUND(hx + (mx - mid_x) * zoom_mul);
	(*Y) = I_ROUND(hy - (my - mid_y) * zoom_mul);
}

void W_Grid::WinToMap(int X, int Y, double *mx, double *my) const
{
	double hx = x() + w() / 2.0;
	double hy = y() + h() / 2.0;

	(*mx) = mid_x + (X - hx) / zoom_mul;
	(*my) = mid_y - (Y - hy) / zoom_mul;
}

//------------------------------------------------------------------------

void W_Grid::resize(int X, int Y, int W, int H)
{
	Fl_Widget::resize(X, Y, W, H);
}

void W_Grid::draw()
{
	/// FIXME

	fl_push_clip(x(), y(), w(), h());

	fl_color(FL_BLACK);
	fl_rectf(x(), y(), w(), h());

	if (zoom >= 22)
	{
		fl_color(FL_GRAY_RAMP + 2);
		draw_grid(1);
	}

	if (zoom >= 16)
	{
		fl_color(fl_color_cube(0, 0, 1));
		draw_grid(8);
	}

	if (zoom >= 9)
	{
		fl_color(fl_color_cube(0, 0, 2));
		draw_grid(64);
	}

	if (true)
	{
		fl_color(fl_color_cube(0, 0, (zoom >= 6) ? 3 : 2));
		draw_grid(512);
	}

	node_c *root = lev_nodes.Get(lev_nodes.num - 1);

	if (partition_MODE == 1)
		draw_all_partitions();

	draw_node(root, 0, false);

	if (partition_MODE == 2)
		draw_all_partitions();

	fl_pop_clip();
}

void W_Grid::draw_grid(int spacing)
{
	if (grid_MODE == 0)
		return;

	double mlx = mid_x - w() * 0.5 / zoom_mul;
	double mly = mid_y - h() * 0.5 / zoom_mul;
	double mhx = mid_x + w() * 0.5 / zoom_mul;
	double mhy = mid_y + h() * 0.5 / zoom_mul;

	int gx = GRID_FIND(mid_x, spacing);
	int gy = GRID_FIND(mid_y, spacing);

	int x1 = x();
	int y1 = y();
	int x2 = x() + w();
	int y2 = y() + h();

	int dx, dy;
	int wx, wy;

	for (dx = 0; true; dx++)
	{
		if (gx + dx * spacing < mlx) continue;
		if (gx + dx * spacing > mhx) break;

		MapToWin(gx + dx * spacing, gy, &wx, &wy);
		fl_yxline(wx, y1, y2);
	}

	for (dx = -1; true; dx--)
	{
		if (gx + dx * spacing > mhx) continue;
		if (gx + dx * spacing < mlx) break;

		MapToWin(gx + dx * spacing, gy, &wx, &wy);
		fl_yxline(wx, y1, y2);
	}

	for (dy = 0; true; dy++)
	{
		if (gy + dy * spacing < mly) continue;
		if (gy + dy * spacing > mhy) break;

		MapToWin(gx, gy + dy * spacing, &wy, &wy);
		fl_xyline(x1, wy, x2);
	}

	for (dy = -1; true; dy--)
	{
		if (gy + dy * spacing > mhy) continue;
		if (gy + dy * spacing < mly) break;

		MapToWin(gx, gy + dy * spacing, &wy, &wy);
		fl_xyline(x1, wy, x2);
	}
}

void W_Grid::draw_partition(const node_c *nd, int ity)
{
	double mlx = mid_x - w() * 0.5 / zoom_mul;
	double mly = mid_y - h() * 0.5 / zoom_mul;
	double mhx = mid_x + w() * 0.5 / zoom_mul;
	double mhy = mid_y + h() * 0.5 / zoom_mul;

	double tlx, tly;
	double thx, thy;

	// intersect the partition line (which extends to infinity) with
	// the sides of the screen (in map coords).  Whether we use the
	// left/right sides to top/bottom depends on the angle of the
	// partition line.

	if (ABS(nd->dx) > ABS(nd->dy))
	{
		tlx = mlx;
		thx = mhx;
		tly = nd->y + nd->dy * (mlx - nd->x) / double(nd->dx);
		thy = nd->y + nd->dy * (mhx - nd->x) / double(nd->dx);

		if (MAX(tly, thy) < mly || MIN(tly, thy) > mhy)
			return;
	}
	else
	{
		tlx = nd->x + nd->dx * (mly - nd->y) / double(nd->dy);
		thx = nd->x + nd->dx * (mhy - nd->y) / double(nd->dy);
		tly = mly;
		thy = mhy;

		if (MAX(tlx, thx) < mlx || MIN(tlx, thx) > mhx)
			return;
	}

	int sx, sy;
	int ex, ey;

	MapToWin(tlx, tly, &sx, &sy);
	MapToWin(thx, thy, &ex, &ey);

	fl_color(fl_color_cube(ity, 0, ity));
	fl_line(sx, sy, ex, ey);
}

void W_Grid::draw_all_partitions()
{
	node_c * nodes[4];

	nodes[0] = nodes[1] = nodes[2] = NULL;
	nodes[3] = lev_nodes.Get(lev_nodes.num - 1);

	for (int rt_idx = 0; rt_idx < route_len; rt_idx++)
	{
		node_c *cur = nodes[3];

		nodes[0] = nodes[1];
		nodes[1] = nodes[2];
		nodes[2] = nodes[3];

		nodes[3] = (visit_route[rt_idx] == RT_LEFT) ? cur->l.node : cur->r.node;

		// should never reach a subsector
		SYS_NULL_CHECK(nodes[3]);
	}

	for (int n_idx = 1; n_idx < 4; n_idx++)
	{
		if (nodes[n_idx])
			draw_partition(nodes[n_idx], (n_idx == 3) ? 4 : n_idx);
	}
}

void W_Grid::draw_node(const node_c *nd, int pos, bool shade)
{
	if (pos < route_len)
	{
		// get drawing order correct, draw shaded side FIRST.

		if (visit_route[pos] == RT_RIGHT)
		{
			draw_child(&nd->l, pos, true);
			draw_child(&nd->r, pos, shade);
		}
		else
		{
			draw_child(&nd->r, pos, true);
			draw_child(&nd->l, pos, shade);
		}
	}
	else
	{
		draw_child(&nd->l, pos, shade);
		draw_child(&nd->r, pos, shade);
	}
}

void W_Grid::draw_child(const child_t *ch, int pos, bool shade)
{
	//!!! FIXME: bbox check

	if (ch->node)
	{
		draw_node(ch->node, pos + 1, shade);
	}
	else  /* Subsector */
	{
		subsec_c *sub = ch->subsec;

		for (seg_c *seg = sub->seg_list; seg; seg = seg->next)
		{
			if (! set_seg_color(seg, !shade))
				continue;

			draw_line(seg->start->x, seg->start->y, seg->end->x, seg->end->y);
		}
	}
}

bool W_Grid::set_seg_color(seg_c *seg, bool on)
{
	if (shade_MODE == 0 && !on)
		return false;
	
	if (shade_MODE == 2)
		on = true;

	if (! seg->linedef)  // miniseg
	{
		if (miniseg_MODE < 2)
			return false;

		 fl_color(on ? fl_color_cube(0,4,3) : fl_color_cube(0,2,2));
		 return true;
	}

	if (! seg->linedef->left || ! seg->linedef->right)  // 1-sided line
	{
		fl_color(on ? FL_WHITE : FL_GRAY_RAMP+12);  
		return true;
	}

	sector_c *front = seg->linedef->right->sector;
	sector_c *back  = seg->linedef->left->sector;

	int floor_min = MIN(front->floor_h, back->floor_h);
	int floor_max = MAX(front->floor_h, back->floor_h);

	int ceil_min = MIN(front->ceil_h, back->ceil_h);
//  int ceil_max = MAX(front->ceil_h, back->ceil_h);

	if (ceil_min <= floor_max)  // closed door ?
	{
		fl_color(on ? FL_RED : fl_color_cube(2,0,0));
		return true;
	}
	if (ceil_min - floor_max < 56)  // narrow vertical gap ?
	{
		fl_color(on ? fl_color_cube(4,4,0) : fl_color_cube(2,2,0));
		return true;
	}
	if (seg->linedef->flags & 1)  // marked impassable ?
	{
		fl_color(on ? FL_YELLOW : fl_color_cube(2,2,0));
		return true;
	}
	if (floor_max - floor_min > 24)  // unclimbable dropoff ?
	{
		fl_color(on ? FL_GREEN : fl_color_cube(0,3,0));
		return true;
	}

	if (miniseg_MODE < 1)
		return false;

	fl_color(FL_GRAY_RAMP + (on ? 14 : 6));  // everything else
	return true;
}

void W_Grid::draw_line(double x1, double y1, double x2, double y2)
{
	double mlx = mid_x - w() * 0.5 / zoom_mul;
	double mly = mid_y - h() * 0.5 / zoom_mul;
	double mhx = mid_x + w() * 0.5 / zoom_mul;
	double mhy = mid_y + h() * 0.5 / zoom_mul;

	// Based on Cohen-Sutherland clipping algorithm

	int out1 = MAP_OUTCODE(x1, y1, mlx, mly, mhx, mhy);
	int out2 = MAP_OUTCODE(x2, y2, mlx, mly, mhx, mhy);

/// PrintDebug("LINE (%1.3f,%1.3f) --> (%1.3f,%1.3f)\n", x1, y1, x2, y2);
/// PrintDebug("RECT (%1.3f,%1.3f) --> (%1.3f,%1.3f)\n", mlx, mly, mhx, mhy);
/// PrintDebug("  out1 = %d  out2 = %d\n", out1, out2);

	while ((out1 & out2) == 0 && (out1 | out2) != 0)
	{
/// PrintDebug("> LINE (%1.3f,%1.3f) --> (%1.3f,%1.3f)\n", x1, y1, x2, y2);
/// PrintDebug(">   out1 = %d  out2 = %d\n", out1, out2);

		// may be partially inside box, find an outside point
		int outside = (out1 ? out1 : out2);

		SYS_ZERO_CHECK(outside);

		double dx = x2 - x1;
		double dy = y2 - y1;

		if (fabs(dx) < 0.1 && fabs(dy) < 0.1)
			break;

		double tmp_x, tmp_y;

		// clip to each side
		if (outside & O_BOTTOM)
		{
			tmp_x = x1 + dx * (mly - y1) / dy;
			tmp_y = mly;
		}
		else if (outside & O_TOP)
		{
			tmp_x = x1 + dx * (mhy - y1) / dy;
			tmp_y = mhy;
		}
		else if (outside & O_LEFT)
		{
			tmp_y = y1 + dy * (mlx - x1) / dx;
			tmp_x = mlx;
		}
		else  /* outside & O_RIGHT */
		{
			SYS_ASSERT(outside & O_RIGHT);

			tmp_y = y1 + dy * (mhx - x1) / dx;
			tmp_x = mhx;
		}

/// PrintDebug(">   outside = %d  temp = (%1.3f, %1.3f)\n", tmp_x, tmp_y);
		SYS_ASSERT(out1 != out2);

		if (outside == out1)
		{
			x1 = tmp_x;
			y1 = tmp_y;

			out1 = MAP_OUTCODE(x1, y1, mlx, mly, mhx, mhy);
		}
		else
		{
			SYS_ASSERT(outside == out2);

			x2 = tmp_x;
			y2 = tmp_y;

			out2 = MAP_OUTCODE(x1, y1, mlx, mly, mhx, mhy);
		}
	}

	if (out1 & out2)
		return;

	int sx, sy;
	int ex, ey;

	MapToWin(x1, y1, &sx, &sy);
	MapToWin(x2, y2, &ex, &ey);

	fl_line(sx, sy, ex, ey);
}

void W_Grid::scroll(int dx, int dy)
{
	dx = dx * w() / 10;
	dy = dy * h() / 10;

	double mdx = dx / zoom_mul;
	double mdy = dy / zoom_mul;

	mid_x += mdx;
	mid_y += mdy;

// fprintf(stderr, "Scroll pix (%d,%d) map (%1.1f, %1.1f) mid (%1.1f, %1.1f)\n", dx, dy, mdx, mdy, mid_x, mid_y);

	redraw();
}

//------------------------------------------------------------------------

int W_Grid::handle(int event)
{
	switch (event)
	{
		case FL_FOCUS:
			return 1;

		case FL_KEYDOWN:
		case FL_SHORTCUT:
			return handle_key(Fl::event_key());

		case FL_ENTER:
		case FL_LEAVE:
			return 1;

		case FL_MOVE:
			//...
			return 1;

		case FL_PUSH:
			descend_by_mouse(Fl::event_x(), Fl::event_y());
			redraw();
			return 1;

		case FL_DRAG:
		case FL_RELEASE:
			// these are currently ignored.
			return 1;

		default:
			break;
	}

	return 0;  // unused
}

int W_Grid::handle_key(int key)
{
	if (key == 0)
		return 0;

	switch (key)
	{
		case '+': case '=':
			SetZoom(zoom + 1);
			return 1;

		case '-': case '_':
			SetZoom(zoom - 1);
			return 1;

		case FL_Left:
			scroll(-1, 0);
			return 1;

		case FL_Right:
			scroll(+1, 0);
			return 1;

		case FL_Up:
			scroll(0, +1);
			return 1;

		case FL_Down:
			scroll(0, -1);
			return 1;

		case 'g': case 'G':
			grid_MODE = (grid_MODE + 1) % 2;
			redraw();
			return 1;

		case 'p': case 'P':
			partition_MODE = (partition_MODE + 1) % 3;
			redraw();
			return 1;

		case 'm': case 'M':
			miniseg_MODE = (miniseg_MODE + 2) % 3;
			redraw();
			return 1;

		case 's': case 'S':
			shade_MODE = (shade_MODE + 1) % 3;
			redraw();
			return 1;

		case 'u': case 'U':
			if (route_len > 0)
				route_len--;
			redraw();
			return 1;
#if 0
		case '[':
			if (descend_tree(RT_LEFT))
				redraw();
			return 1;

		case ']':
			if (descend_tree(RT_RIGHT))
				redraw();
			return 1;
#endif
		case 'x':
			DialogShowAndGetChoice(ALERT_TXT, 0, "Please foo the joo.");
			return 1;

		default:
			break;
	}

	return 0;  // unused
}

bool W_Grid::descend_by_mouse(int wx, int wy)
{
	node_c *cur_nd = lowest_node();

	double mx, my;
	WinToMap(wx, wy, &mx, &my);

	// transpose coords to the origin, check side
	double ox = mx - cur_nd->x;
	double oy = my - cur_nd->y;

	if (oy * cur_nd->dx < ox * cur_nd->dy)
		return descend_tree(RT_RIGHT);
	else
		return descend_tree(RT_LEFT);
}

bool W_Grid::descend_tree(char side)
{
	if (route_len >= MAX_ROUTE)
		return false;

	node_c *cur_nd = lowest_node();

	// would it be a subsector ??
	if (((side == RT_LEFT) ? cur_nd->l.node : cur_nd->r.node) == NULL)
		return false;

	visit_route[route_len++] = side;
	return true;
}

node_c *W_Grid::lowest_node()
{
	node_c *cur = lev_nodes.Get(lev_nodes.num - 1);

	for (int rt_idx = 0; rt_idx < route_len; rt_idx++)
	{
		cur = (visit_route[rt_idx] == RT_LEFT) ? cur->l.node : cur->r.node;

		// should never reach a subsector
		SYS_NULL_CHECK(cur);
	}

	return cur;
}
