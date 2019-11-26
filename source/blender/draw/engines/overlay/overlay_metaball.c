/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2019, Blender Foundation.
 */

/** \file
 * \ingroup draw_engine
 */

#include "DRW_render.h"

#include "DNA_meta_types.h"

#include "BKE_object.h"

#include "DEG_depsgraph_query.h"

#include "ED_mball.h"

#include "overlay_private.h"

void OVERLAY_metaball_cache_init(OVERLAY_Data *vedata)
{
  OVERLAY_PassList *psl = vedata->psl;
  OVERLAY_PrivateData *pd = vedata->stl->pd;

  OVERLAY_InstanceFormats *formats = OVERLAY_shader_instance_formats_get();

#define BUF_INSTANCE DRW_shgroup_call_buffer_instance

  for (int i = 0; i < 2; i++) {
    DRWState state = DRW_STATE_WRITE_COLOR | DRW_STATE_WRITE_DEPTH | DRW_STATE_DEPTH_LESS_EQUAL;
    DRW_PASS_CREATE(psl->metaball_ps[i], state | pd->clipping_state);

    /* Reuse armature shader as it's perfect to outline ellipsoids. */
    struct GPUVertFormat *format = formats->instance_bone;
    struct GPUShader *sh = OVERLAY_shader_armature_sphere(true);
    DRWShadingGroup *grp = DRW_shgroup_create(sh, psl->metaball_ps[i]);
    DRW_shgroup_uniform_block(grp, "globalsBlock", G_draw.block_ubo);
    pd->mball.handle[i] = BUF_INSTANCE(grp, format, DRW_cache_bone_point_wire_outline_get());
  }
}

static void metaball_instance_data_set(
    BoneInstanceData *data, Object *ob, const float *pos, const float radius, const float color[4])
{
  /* Bone point radius is 0.05. Compensate for that. */
  mul_v3_v3fl(data->mat[0], ob->obmat[0], radius / 0.05f);
  mul_v3_v3fl(data->mat[1], ob->obmat[1], radius / 0.05f);
  mul_v3_v3fl(data->mat[2], ob->obmat[2], radius / 0.05f);
  mul_v3_m4v3(data->mat[3], ob->obmat, pos);
  /* WATCH: Reminder, alpha is wiresize. */
  OVERLAY_bone_instance_data_set_color(data, color);
}

void OVERLAY_edit_metaball_cache_populate(OVERLAY_Data *vedata, Object *ob)
{
  const bool do_in_front = (ob->dtx & OB_DRAWXRAY) != 0;
  const bool is_select = DRW_state_is_select();
  OVERLAY_PrivateData *pd = vedata->stl->pd;
  MetaBall *mb = ob->data;

  const float *color;
  const float col_radius[4] = {0.63f, 0.19f, 0.19f, 1.0f};           /* 0x3030A0 */
  const float col_radius_select[4] = {0.94f, 0.63f, 0.63f, 1.0f};    /* 0xA0A0F0 */
  const float col_stiffness[4] = {0.19f, 0.63f, 0.19f, 1.0f};        /* 0x30A030 */
  const float col_stiffness_select[4] = {0.63f, 0.94f, 0.63f, 1.0f}; /* 0xA0F0A0 */

  int select_id = 0;
  if (is_select) {
    const Object *orig_object = DEG_get_original_object(ob);
    select_id = orig_object->runtime.select_id;
  }

  LISTBASE_FOREACH (MetaElem *, ml, mb->editelems) {
    const bool is_selected = (ml->flag & SELECT) != 0;
    const bool is_scale_radius = (ml->flag & MB_SCALE_RAD) != 0;
    float stiffness_radius = ml->rad * atanf(ml->s) / (float)M_PI_2;
    BoneInstanceData instdata;

    if (is_select) {
      DRW_select_load_id(select_id | MBALLSEL_RADIUS);
    }
    color = (is_selected && is_scale_radius) ? col_radius_select : col_radius;
    metaball_instance_data_set(&instdata, ob, &ml->x, ml->rad, color);
    DRW_buffer_add_entry_struct(pd->mball.handle[do_in_front], &instdata);

    if (is_select) {
      DRW_select_load_id(select_id | MBALLSEL_STIFF);
    }
    color = (is_selected && !is_scale_radius) ? col_stiffness_select : col_stiffness;
    metaball_instance_data_set(&instdata, ob, &ml->x, stiffness_radius, color);
    DRW_buffer_add_entry_struct(pd->mball.handle[do_in_front], &instdata);

    select_id += 0x10000;
  }
}

void OVERLAY_metaball_cache_populate(OVERLAY_Data *vedata, Object *ob)
{
  const bool do_in_front = (ob->dtx & OB_DRAWXRAY) != 0;
  OVERLAY_PrivateData *pd = vedata->stl->pd;
  MetaBall *mb = ob->data;
  const DRWContextState *draw_ctx = DRW_context_state_get();

  float *color;
  DRW_object_wire_theme_get(ob, draw_ctx->view_layer, &color);

  LISTBASE_FOREACH (MetaElem *, ml, &mb->elems) {
    /* Draw radius only. */
    BoneInstanceData instdata;
    metaball_instance_data_set(&instdata, ob, &ml->x, ml->rad, color);
    DRW_buffer_add_entry_struct(pd->mball.handle[do_in_front], &instdata);
  }
}

void OVERLAY_metaball_draw(OVERLAY_Data *vedata)
{
  OVERLAY_PassList *psl = vedata->psl;

  DRW_draw_pass(psl->metaball_ps[0]);
}

void OVERLAY_metaball_in_front_draw(OVERLAY_Data *vedata)
{
  OVERLAY_PassList *psl = vedata->psl;

  DRW_draw_pass(psl->metaball_ps[1]);
}