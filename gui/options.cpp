//------------------------------------------------------------------------
// Options : Unix/FLTK Option boxes
//------------------------------------------------------------------------
//
//  GL-Friendly Node Builder (C) 2000-2001 Andrew Apted
//
//  Based on `BSP 2.3' by Colin Reed, Lee Killough and others.
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
#include "local.h"


#ifdef WIN32
#define BM_BUTTONTYPE  FL_ROUND_DOWN_BOX
#define BM_BUTTONSIZE  26
#else
#define BM_BUTTONTYPE  FL_DIAMOND_DOWN_BOX
#define BM_BUTTONSIZE  28
#endif


static void build_mode_radio_CB(Fl_Widget *w, void *data)
{
  boolean_g old_gwa = guix_info.gwa_mode;
 
  guix_win->build_mode->WriteInfo();

  // communicate with output file widget, for GWA mode
  if (old_gwa != guix_info.gwa_mode)
  {
    guix_win->files->GWA_Changed();
    guix_win->misc_opts->GWA_Changed();
  }
}


//
// BuildMode Constructor
//
Guix_BuildMode::Guix_BuildMode(int x, int y, int w, int h) :
    Fl_Group(x, y, w, h, "Build Mode")
{
  // cancel the automatic `begin' in Fl_Group constructor
  end();
  
  box(FL_THIN_UP_BOX);
  resizable(0);  // no resizing the kiddies, please

  labelfont(FL_HELVETICA | FL_BOLD);
  labeltype(FL_NORMAL_LABEL);
  align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP);

  // create the children

  gwa = new Fl_Check_Button(x+12, y+16+22*0, 
      BM_BUTTONSIZE, BM_BUTTONSIZE, "GWA Mode");
  gwa->down_box(BM_BUTTONTYPE);
  gwa->type(FL_RADIO_BUTTON);
  gwa->align(FL_ALIGN_RIGHT);
  gwa->callback((Fl_Callback *) build_mode_radio_CB);
  add(gwa);

  maybe_normal = new Fl_Check_Button(x+12, y+16+22*1,
      BM_BUTTONSIZE, BM_BUTTONSIZE, "GL, Normal if missing");
  maybe_normal->down_box(BM_BUTTONTYPE);
  maybe_normal->type(FL_RADIO_BUTTON);
  maybe_normal->align(FL_ALIGN_RIGHT);
  maybe_normal->callback((Fl_Callback *) build_mode_radio_CB);
  add(maybe_normal);

  both = new Fl_Check_Button(x+12, y+16+22*2,
      BM_BUTTONSIZE, BM_BUTTONSIZE, "GL and Normal nodes");
  both->down_box(BM_BUTTONTYPE);
  both->type(FL_RADIO_BUTTON);
  both->align(FL_ALIGN_RIGHT);
  both->callback((Fl_Callback *) build_mode_radio_CB);
  add(both);

  gl_only = new Fl_Check_Button(x+12, y+16+22*3,
      BM_BUTTONSIZE, BM_BUTTONSIZE, "GL nodes only");
  gl_only->down_box(BM_BUTTONTYPE);
  gl_only->type(FL_RADIO_BUTTON);
  gl_only->align(FL_ALIGN_RIGHT);
  gl_only->callback((Fl_Callback *) build_mode_radio_CB);
  add(gl_only);

  normal_only = new Fl_Check_Button(x+12, y+16+22*4,
      BM_BUTTONSIZE, BM_BUTTONSIZE, "Normal nodes only");
  normal_only->down_box(BM_BUTTONTYPE);
  normal_only->type(FL_RADIO_BUTTON);
  normal_only->align(FL_ALIGN_RIGHT);
  normal_only->callback((Fl_Callback *) build_mode_radio_CB);
  add(normal_only);

  ReadInfo();
}


//
// BuildMode Destructor
//
Guix_BuildMode::~Guix_BuildMode()
{
  WriteInfo();
}


void Guix_BuildMode::ReadInfo()
{
  if (guix_info.gwa_mode)
    gwa->setonly();
  
  else if (guix_info.no_gl)
    normal_only->setonly();
 
  else if (guix_info.no_normal)
    gl_only->setonly();
  
  else if (guix_info.force_normal)
    both->setonly();

  else
    maybe_normal->setonly();

  // redraw them all (just to be safe)
  gwa->redraw();
  normal_only->redraw();
  gl_only->redraw();
  both->redraw();
  maybe_normal->redraw();
}


void Guix_BuildMode::WriteInfo()
{
  // default: everything false
  guix_info.gwa_mode = FALSE;
  guix_info.no_gl = FALSE;
  guix_info.no_normal = FALSE;
  guix_info.force_normal = FALSE;

  if (gwa->value())
  {
    guix_info.gwa_mode = TRUE;
  }
  else if (normal_only->value())
  {
    guix_info.no_gl = TRUE;
    guix_info.force_normal = TRUE;
  }
  else if (gl_only->value())
  {
    guix_info.no_normal = TRUE;
  }
  else if (both->value())
  {
    guix_info.force_normal = TRUE;
  }
}


//------------------------------------------------------------------------


static void misc_opts_check_CB(Fl_Widget *w, void *data)
{
  guix_win->misc_opts->WriteInfo();
}


//
// MiscOptions Constructor
//
Guix_MiscOptions::Guix_MiscOptions(int x, int y, int w, int h) :
    Fl_Group(x, y, w, h, "Misc Options")
{
  // cancel the automatic `begin' in Fl_Group constructor
  end();
  
  box(FL_THIN_UP_BOX);
  resizable(0);  // no resizing the kiddies, please

  labelfont(FL_HELVETICA | FL_BOLD);
  labeltype(FL_NORMAL_LABEL);
  align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP);

  // create children

  warnings = new Fl_Check_Button(x+12, y+20+22*0, 22, 22,
      "Extra Warnings");
  warnings->down_box(FL_DOWN_BOX);
  warnings->align(FL_ALIGN_RIGHT);
  warnings->callback((Fl_Callback *) misc_opts_check_CB);
  add(warnings);

  v1_vert = new Fl_Check_Button(x+12, y+20+22*1, 22, 22,
      "V1 GL Nodes");
  v1_vert->down_box(FL_DOWN_BOX);
  v1_vert->align(FL_ALIGN_RIGHT);
  v1_vert->callback((Fl_Callback *) misc_opts_check_CB);
  add(v1_vert);

  no_reject = new Fl_Check_Button(x+12, y+20+22*2, 22, 22,
      "Don't clobber REJECT");
  no_reject->down_box(FL_DOWN_BOX);
  no_reject->align(FL_ALIGN_RIGHT);
  no_reject->callback((Fl_Callback *) misc_opts_check_CB);
  add(no_reject);

  pack_sides = new Fl_Check_Button(x+12, y+20+22*3, 22, 22,
      "Pack Sidedefs");
  pack_sides->down_box(FL_DOWN_BOX);
  pack_sides->align(FL_ALIGN_RIGHT);
  pack_sides->callback((Fl_Callback *) misc_opts_check_CB);
  add(pack_sides);

  ReadInfo();
}


//
// MiscOptions Destructor
//
Guix_MiscOptions::~Guix_MiscOptions()
{
  WriteInfo();
}


void Guix_MiscOptions::ReadInfo()
{
  v1_vert->value(guix_info.v1_vert ? 1 : 0);
  v1_vert->redraw();

  warnings->value(guix_info.mini_warnings ? 1 : 0);
  warnings->redraw();

  no_reject->value(guix_info.no_reject ? 1 : 0);
  no_reject->redraw();

  pack_sides->value(guix_info.pack_sides ? 1 : 0);
  pack_sides->redraw();

  GWA_Changed();
}


void Guix_MiscOptions::WriteInfo()
{
  guix_info.v1_vert = v1_vert->value() ? TRUE : FALSE;
  guix_info.no_reject = no_reject->value() ? TRUE : FALSE;
  guix_info.mini_warnings = warnings->value() ? TRUE : FALSE;
  guix_info.pack_sides = pack_sides->value() ? TRUE : FALSE;
}


void Guix_MiscOptions::GWA_Changed()
{
  if (guix_info.gwa_mode)
  {
    no_reject->hide();   /// deactivate
    pack_sides->hide();
  }
  else
  {
    no_reject->show();   /// activate
    pack_sides->show();
  }
}

