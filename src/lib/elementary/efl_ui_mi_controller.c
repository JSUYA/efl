#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_mi_state.eo.h"
#include "efl_ui_mi_controller_private.h"
//#include "efl_ui_mi_controller_part.eo.h"
//#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_MI_CONTROLLER_CLASS

#define MY_CLASS_NAME "Efl_Ui_Mi_Controller"

static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";

/* smart callbacks coming from efl_ui_mi_controller objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_FOCUSED, ""},
   {SIG_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

EOLIAN static void
_efl_ui_mi_controller_repeat_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Mi_Controller_Data *pd, Efl_Ui_Mi_Controller_Repeat_Mode mode)
{
   pd->repeat_mode = mode;
   switch (mode)
     {
      case EFL_UI_MI_CONTROLLER_REPEAT_MODE_NONE:
      case EFL_UI_MI_CONTROLLER_REPEAT_MODE_ONCE:
         efl_player_playback_loop_set(pd->anim, EINA_FALSE);
         break;
      case EFL_UI_MI_CONTROLLER_REPEAT_MODE_LOOP:
         efl_player_playback_loop_set(pd->anim, EINA_TRUE);
         break;
      case EFL_UI_MI_CONTROLLER_REPEAT_MODE_PLAY_REWIND:
         /* Implement not yet */
         break;
     }
}

EOLIAN static Efl_Ui_Mi_Controller_Repeat_Mode
_efl_ui_mi_controller_repeat_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Mi_Controller_Data *pd)
{
   return pd->repeat_mode;
}

Eina_Bool
_efl_ui_mi_controller_trigger(Eo *obj, Efl_Ui_Mi_Controller_Data *pd, Efl_Ui_Mi_State *state, Eina_Bool animation)
{

   Efl_Ui_Mi_State *itr_state;
   Eina_Array_Iterator iter;
   unsigned int i;
   const char *_start = NULL;
   const char *_end = NULL;
   Eina_Bool find = EINA_FALSE;

   // Check valid state.
   EINA_ARRAY_ITER_NEXT(pd->states, i, itr_state, iter)
     {
        if (itr_state == state)
          {
             find = EINA_TRUE;
             break;
          }
     }
   if (!find)  // Invalid state, play all frame
     efl_player_playing_set(pd->anim, EINA_TRUE);

   // Find Start, End frame
   Efl_Ui_Mi_State *current_state = pd->current_state;
   if (current_state)
     {
        efl_ui_mi_state_sector_get(current_state, &_start, NULL); // start state or 0 frame
     }
   efl_ui_mi_state_sector_get(state, &_end, NULL); // Goal state

   if (state != pd->current_state)
     efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_DEACTIVATE, NULL);

   if (_end)
     {
        if (!animation)
          {
             Eo *vg = efl_key_data_get(pd->anim, "vg_obj");
             int end_frame;
             efl_gfx_frame_controller_sector_get(vg, _end, &end_frame, NULL);
             efl_ui_vg_animation_frame_set(pd->anim, end_frame);
             efl_player_paused_set(pd->anim, EINA_TRUE);
             pd->current_state = state;
             efl_event_callback_call(state, EFL_UI_MI_STATE_EVENT_TRIGGER, NULL);
             pd->is_feedback_play = EINA_FALSE;
             efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_ACTIVATE, NULL);
             //efl_event_callback_call(state, EFL_UI_MI_STATE_EVENT_FEEDBACK_DONE, NULL);
          }
        else
          {
             if (!_start)
               {
                  ERR("First State didn't support animated trigger\n");
                  return EINA_FALSE;
               }
             efl_ui_vg_animation_playing_sector(pd->anim, _start, _end);
             if (efl_player_paused_get(pd->anim))
               efl_player_paused_set(pd->anim, EINA_FALSE);
             pd->current_state = current_state;
             efl_event_callback_call(current_state, EFL_UI_MI_STATE_EVENT_TRIGGER, NULL);
             pd->is_feedback_play = EINA_FALSE;
             efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_ACTIVATE, NULL);
          }
     }
   else
     ERR("No start point");

   return EINA_TRUE;
}

Eina_Bool
_efl_ui_mi_controller_feedback(Eo *obj, Efl_Ui_Mi_Controller_Data *pd, Efl_Ui_Mi_State *state)
{
   Efl_Ui_Mi_State *itr_state;
   Eina_Array_Iterator iter;
   unsigned int i = 0;
   unsigned int c = 0;
   const char *_start = NULL;
   const char *_end = NULL;
   Eina_Bool find = EINA_FALSE;
   int states_num = eina_array_count_get(pd->states);

   if (state != pd->current_state)
     efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_DEACTIVATE, NULL);

   // Check valid state.
   EINA_ARRAY_ITER_NEXT(pd->states, c, itr_state, iter)
     {
        i++;
        if (itr_state == state)
          {
             find = EINA_TRUE;
             break;
          }
     }
   if (!find || i == states_num)
     {
        ERR("find : %d, i : %d == states_num %d", find, i, states_num);
        return EINA_FALSE;
     }

   // Find Start, End frame
   efl_ui_mi_state_sector_get(state, &_start, NULL);

   if (i < states_num)
     {
        efl_ui_mi_state_sector_get(eina_array_data_get(pd->states, i), &_end, NULL);
        efl_ui_vg_animation_playing_sector(pd->anim, _start, _end);
        pd->current_state = state;
        pd->is_feedback_play = EINA_TRUE;

        efl_event_callback_call(state, EFL_UI_MI_STATE_EVENT_ACTIVATE, NULL);
     }
   return EINA_FALSE;

}

Eina_Bool
_efl_ui_mi_controller_trigger_next(Eo *obj, Efl_Ui_Mi_Controller_Data *pd, Eina_Bool animation)
{
   Efl_Ui_Mi_State *itr_state;
   Eina_Array_Iterator iter;
   unsigned int i = 0;
   unsigned int c = 0;
   Eina_Bool find = EINA_FALSE;
   int states_num = eina_array_count_get(pd->states);
   // Check valid state.
   EINA_ARRAY_ITER_NEXT(pd->states, c, itr_state, iter)
     {
        i++;
        if (itr_state == pd->current_state)
          {
             find = EINA_TRUE;
             break;
          }
     }
   if (!find || i >= states_num)
     {
        ERR("find : %d, i : %d == states_num %d", find, i, states_num);
        return EINA_FALSE;
     }
   efl_ui_mi_controller_trigger(obj, eina_array_data_get(pd->states, i), animation);
   return EINA_TRUE;
}

Eina_Bool
_efl_ui_mi_controller_trigger_prev(Eo *obj, Efl_Ui_Mi_Controller_Data *pd, Eina_Bool animation)
{
   //change state and play
   return EINA_FALSE;
}

EOLIAN static Eo*
_efl_ui_mi_controller_state_get(const Eo *eo_obj, Efl_Ui_Mi_Controller_Data *pd,
                                const char *state_name)
{
   Efl_Ui_Mi_State *state;
   Eina_Array_Iterator iter;
   unsigned int i;
   const char *_name;

   EINA_ARRAY_ITER_NEXT(pd->states, i, state, iter)
    {
        efl_ui_mi_state_sector_get(state, &_name, NULL);
        if (_name && !strcmp(state_name, _name))
          return state;
    }

   return NULL;
}

EOLIAN static Eo*
_efl_ui_mi_controller_current_state_get(const Eo *eo_obj, Efl_Ui_Mi_Controller_Data *pd)
{
   return pd->current_state;
}

Eina_Bool
_efl_ui_mi_controller_state_add(Eo *obj, Efl_Ui_Mi_Controller_Data *pd, Efl_Ui_Mi_State *state)
{

   if (!state) return EINA_FALSE;
   if (!pd->states)
     {
         pd->states = eina_array_new(sizeof(Efl_Ui_Mi_State*));
     }

   eina_array_push(pd->states, state);
   return EINA_FALSE;
}

EOLIAN static Eo*
_efl_ui_mi_controller_rule_get(const Eo *eo_obj, Efl_Ui_Mi_Controller_Data *pd,
                               const char *rule_name)
{
   Efl_Ui_Mi_Rule *rule;
   Eina_Array_Iterator iter;
   unsigned int i;
   const char *_name;

   EINA_ARRAY_ITER_NEXT(pd->rules, i, rule, iter)
     {
        if (!strcmp(rule_name , efl_ui_mi_rule_keypath_get(rule)))
          return rule;
     }

   return NULL;
}

Eina_Bool
_efl_ui_mi_controller_rule_add(Eo *obj, Efl_Ui_Mi_Controller_Data *pd, Efl_Ui_Mi_Rule *rule)
{

   if (!rule) return EINA_FALSE;
   if (!pd->rules)
     {
        pd->rules = eina_array_new(sizeof(Efl_Ui_Mi_Rule*));
     }

   eina_array_push(pd->rules, rule);
   return EINA_FALSE;
}

static void
_sizing_eval(Eo *obj, void *data)
{
   Efl_Ui_Mi_Controller_Data *pd = data;
   if (!efl_file_loaded_get(pd->anim)) return;

   Eo *vg = efl_key_data_get(pd->anim, "vg_obj");
   Eina_Size2D size = efl_canvas_vg_object_default_min_get(vg);

   double hw,hh;
   efl_gfx_hint_weight_get(obj, &hw, &hh);

   if (hw == 0.0 && hh == 0.0)
   {
     //FIXME: fit_mode value should be changed to enum
     efl_canvas_vg_object_fit_mode_set(vg, 2);
     efl_gfx_hint_size_min_set(obj, size);
     efl_gfx_hint_size_max_set(obj, size);
   }

}

void
_create_state(Eo *parent, Efl_Ui_Mi_Controller_Data *pd)
{
   //temporary
   Eo *vg = efl_key_data_get(pd->anim, "vg_obj");
   Eina_List *sector_list = efl_gfx_frame_controller_sector_list_get(vg);
   Eo *state = NULL;

   if (sector_list)
     {
        Eina_List *l;
        Efl_Gfx_Frame_Sector_Data *sector;
        const char *start_sector = NULL, *end_sector = NULL;

        EINA_LIST_FOREACH(sector_list, l, sector)
          {
             start_sector = end_sector;
             end_sector = sector->name;
             if (start_sector && end_sector)
               {
                  state = efl_add(EFL_UI_MI_STATE_CLASS, parent);
                  efl_key_data_set(state, "controller", parent);
                  efl_ui_mi_state_sector_set(state, start_sector, end_sector);
                  efl_ui_mi_controller_state_add(parent, state);
               }
          }

        //Last state is end contents. this is not playable.
        start_sector = end_sector;
        state = efl_add(EFL_UI_MI_STATE_CLASS, parent);
        efl_key_data_set(state, "controller", parent);
        efl_ui_mi_state_sector_set(state, start_sector, NULL);
        efl_ui_mi_controller_state_add(parent, state);

        EINA_LIST_FOREACH(sector_list, l, sector)
          {
             free((char*)sector->name);
             free(sector);
          }
        eina_list_free(sector_list);
     }
}

void
_create_rules(Eo *parent, Efl_Ui_Mi_Controller_Data *pd, Efl_VG* root)
{
   Eina_List *list = efl_canvas_vg_container_children_direct_get(root);
   Eina_List *l, *ll, *llist;
   Efl_VG *layer, *node;
   Eo* rule = NULL;
   EINA_LIST_FOREACH(list, l, layer)
     {
        char *layer_keypath = efl_key_data_get(layer, "_lot_node_name");
        if (layer_keypath)
          {
             rule = efl_add(EFL_UI_MI_RULE_CLASS, parent);
             efl_ui_mi_rule_keypath_set(rule, layer_keypath);
             efl_ui_mi_controller_rule_add(parent, rule);
          }
     }
}

EOLIAN static Eina_Error
_efl_ui_mi_controller_efl_file_load(Eo *obj, Efl_Ui_Mi_Controller_Data *pd)
{
   Eina_Error err;
   Eina_Bool ret;
   const char *file;
   const char *key;

   if (efl_file_loaded_get(obj)) return 0;

   err = efl_file_load(efl_super(obj, MY_CLASS));
   if (err) return err;

   file = efl_file_get(obj);
   key = efl_file_key_get(obj);
   ret = efl_file_simple_load(pd->anim, file, key);
   if (!ret)
     {
        ERR("Efl.Ui.Mi_Controller File load fail : %s\n", file);
        efl_file_unload(obj);
        return eina_error_get();
     }
   _sizing_eval(obj, pd);

   _create_state(obj, pd);
   Eo *vg = efl_key_data_get(pd->anim, "vg_obj");
   Eo* root = efl_canvas_vg_object_root_node_get(vg);
   if (root)
     {
        _create_rules(obj, pd, root);
     }
   return 0;
}

EOLIAN static void
_efl_ui_mi_controller_efl_gfx_view_view_size_set(Eo *obj EINA_UNUSED,
                                                  Efl_Ui_Mi_Controller_Data *pd,
                                                  Eina_Size2D size)
{
   efl_gfx_view_size_set(pd->anim, size);
}

EOLIAN Eina_Size2D
_efl_ui_mi_controller_efl_gfx_view_view_size_get(const Eo *obj EINA_UNUSED,
                                                  Efl_Ui_Mi_Controller_Data *pd)
{
   Eina_Size2D view_size = efl_gfx_view_size_get(pd->anim);

   return view_size;
}
static void
_size_hint_event_cb(void *data, const Efl_Event *event)
{
   _sizing_eval(event->object, data);
}


Eo*
_find_current_state(Eo *obj, int frame)
{
   Efl_Ui_Mi_Controller_Data *pd = efl_data_scope_safe_get(obj, MY_CLASS);

   Efl_Ui_Mi_State *state;
   Eina_Array_Iterator iter;
   int i;
   Eo *vg = efl_key_data_get(pd->anim, "vg_obj");

   EINA_ARRAY_ITER_NEXT(pd->states, i, state, iter)
     {
        const char *_s, *_e;
        int s, e = -999;
        efl_ui_mi_state_sector_get(state, &_s, NULL);
        efl_gfx_frame_controller_sector_get(vg, _s, &s, &s);
        if (i + 1 < eina_array_count_get(pd->states)){
             efl_ui_mi_state_sector_get(eina_array_data_get(pd->states,i + 1), &_e, NULL);
             if (_e) efl_gfx_frame_controller_sector_get(vg, _e, &e, &e);
        }
        if (s <= frame && frame < e){
             return state;
        }
     }
   return NULL;
}

double
_get_current_progress_on_state(Eo* obj, int frame, Eo* state)
{
   Efl_Ui_Mi_Controller_Data *pd = efl_data_scope_safe_get(obj, MY_CLASS);
   Efl_Ui_Mi_State *item_state;
   Eina_Array_Iterator iter;
   int i;
   Eo *vg = efl_key_data_get(pd->anim, "vg_obj");

   EINA_ARRAY_ITER_NEXT(pd->states, i, item_state, iter)
     {
        const char *_s, *_e;
        int s, e = -999;
        if (item_state == state)
          {
             efl_ui_mi_state_sector_get(state, &_s, NULL);
             efl_gfx_frame_controller_sector_get(vg, _s, &s, &s);
             if (i + 1 < eina_array_count_get(pd->states))
               {
                  efl_ui_mi_state_sector_get(eina_array_data_get(pd->states,i + 1), &_e, NULL);
                  if (_e) efl_gfx_frame_controller_sector_get(vg, _e, &e, &e);
                  else return 1.0;
               }
             else
               return 1.0;

             return ((double)frame - (double)s) / ((double)e - (double)s);
          }
     }
   return 0.0;
}

static void
_animation_playback_finished_cb(void *data, const Efl_Event *event)
{
   Eo* obj = data;
   Efl_Ui_Mi_Controller_Data *pd = efl_data_scope_safe_get(obj, MY_CLASS);
   if (!pd) return ;

   Efl_Ui_Mi_State *cur_state = pd->current_state;
   efl_event_callback_call(cur_state, EFL_UI_MI_STATE_EVENT_FEEDBACK_DONE, NULL);
   if (pd->is_feedback_play && pd->repeat_mode != EFL_UI_MI_CONTROLLER_REPEAT_MODE_LOOP)
     {
        // Need to check direction
        int min_frame = efl_ui_vg_animation_min_frame_get(event->object);
        efl_ui_vg_animation_frame_set(event->object, min_frame);
        pd->current_state =  _find_current_state(obj, min_frame);
        efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_ACTIVATE, NULL);
     }
   else
     {
        efl_event_callback_call(cur_state, EFL_UI_MI_STATE_EVENT_DEACTIVATE, NULL);

        int cur_frame = efl_ui_vg_animation_frame_get(event->object);
        efl_ui_vg_animation_frame_set(event->object, cur_frame + 1);
        cur_state = _find_current_state(obj, cur_frame + 1);
        if (cur_state)
           pd->current_state = cur_state;
        if (!pd->is_feedback_play)
          efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_ACTIVATE, NULL);
        efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_TRIGGER, NULL);

     }

}

static void
_animation_playback_progress_changed_cb(void *data, const Efl_Event *event)
{
   Eo* obj = data;
   Efl_Ui_Mi_Controller_Data *pd = efl_data_scope_safe_get(obj, MY_CLASS);
   if (!pd || !event->info) return;

   Efl_Ui_Mi_State *cur_state = _find_current_state(obj, efl_ui_vg_animation_frame_get(event->object));
   double progress = *(double*)event->info;

   if (progress != 0.0 && progress != 1.0)
     {
        if (pd->current_state != cur_state)
          {
             efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_FEEDBACK_DONE, NULL);
             if (!pd->is_feedback_play)
               efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_DEACTIVATE, NULL);
             if (cur_state)
               pd->current_state = cur_state;
             if (!pd->is_feedback_play) {
                  efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_ACTIVATE, NULL);
                  efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_TRIGGER, NULL);
             }
          }
        else
          {
             double progress = _get_current_progress_on_state(obj, efl_ui_vg_animation_frame_get(event->object), pd->current_state);
             efl_event_callback_call(pd->current_state, EFL_UI_MI_STATE_EVENT_FEEDBACK, (double*)&progress);
          }
     }

   Eo *vg = efl_key_data_get(pd->anim, "vg_obj");
   Eina_Size2D size = efl_canvas_vg_object_default_min_get(vg);

   double hw,hh;
   efl_gfx_hint_weight_get(obj, &hw, &hh);

   if (hw == 0.0 && hh == 0.0)
   {
     //FIXME: fit_mode value should be changed to enum
     efl_canvas_vg_object_fit_mode_set(vg, 2);
     efl_gfx_hint_size_min_set(obj, size);
     efl_gfx_hint_size_max_set(obj, size);
   }
}

EOLIAN static void
_efl_ui_mi_controller_value_provider_override(Eo *obj EINA_UNUSED, Efl_Ui_Mi_Controller_Data *pd, Efl_Gfx_Vg_Value_Provider *value_provider)
{
   if (!value_provider) return;
   efl_ui_vg_animation_value_provider_override(pd->anim, value_provider);
}

EOLIAN static void
_efl_ui_mi_controller_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Mi_Controller_Data *pd)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   pd->anim = efl_add(EFL_UI_VG_ANIMATION_CLASS, obj);
   elm_widget_resize_object_set(obj, pd->anim);

   efl_event_callback_add(pd->anim, EFL_PLAYER_EVENT_PLAYBACK_FINISHED, _animation_playback_finished_cb, obj);
   efl_event_callback_add(pd->anim, EFL_PLAYER_EVENT_PLAYBACK_PROGRESS_CHANGED, _animation_playback_progress_changed_cb, obj);
   efl_key_data_set(obj, "anim", pd->anim);

   //pd->cur_state_idx = 0;
   pd->first_play = EINA_TRUE;
   pd->is_feedback_play = EINA_FALSE;

   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _size_hint_event_cb, pd);
}

EOLIAN static void
_efl_ui_mi_controller_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Mi_Controller_Data *pd EINA_UNUSED)
{
   efl_canvas_group_del(efl_super(obj, MY_CLASS));

   if (pd->states) eina_array_free(pd->states);
}

EOLIAN static void
_efl_ui_mi_controller_efl_object_destructor(Eo *obj,
                                          Efl_Ui_Mi_Controller_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_mi_controller_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Mi_Controller_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   return obj;
}

/* Efl.Part begin */
/*ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_Controller_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_Controller_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_Controller_Data)
#include "efl_ui_mi_controller_part.eo.c"*/
/* Efl.Part end */

/* Internal EO APIs and hidden overrides */


#define EFL_UI_MI_CONTROLLER_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_mi_controller)

#include "efl_ui_mi_controller.eo.c"
