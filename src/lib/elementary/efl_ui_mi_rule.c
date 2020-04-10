#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_mi_rule_private.h"
#include "efl_ui_mi_rule_part_text.eo.h"
//#include "efl_ui_mi_controller_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_MI_RULE_CLASS

#define MY_CLASS_NAME "Efl_Ui_Mi_Rule"

static Efl_VG* _find_keypath_node(Efl_Ui_Mi_Rule_Data *pd);
static void _calculate_event_rect(Efl_Ui_Mi_Rule_Data *pd, Eina_Rect r);
static void _calculate_text_part(Efl_Ui_Mi_Rule_Data *pd, Eina_Rect r);

static void _trigger_cb(void *data, const Efl_Event *event);
static void _feedback_cb(void *data, const Efl_Event *event);
static void _activate_cb(void *data, const Efl_Event *event);
static void _deactivate_cb(void *data, const Efl_Event *event);

void
_parent_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Efl_Ui_Mi_Rule_Data *pd = data;

   Efl_VG *key_node = _find_keypath_node(pd);
   Eina_Rect r;
   efl_gfx_path_bounds_get(key_node, &r);

   _calculate_event_rect(pd, r);
   _calculate_text_part(pd, r);
   //FIXME: update the event rect when the parent is resized.
}

static void
tap_gesture_cb(void *data , const Efl_Event *ev)
{
   Eo* obj = (Eo*)data;
   Efl_Canvas_Gesture *g = ev->info;
   EFL_UI_MI_RULE_DATA_GET_OR_RETURN(obj, pd);

   switch(efl_gesture_state_get(g))
   {
/*      case EFL_GESTURE_STATE_STARTED:
         break;
      case EFL_GESTURE_STATE_CANCELED:
         break;*/
      case EFL_GESTURE_STATE_FINISHED:
         efl_event_callback_call(obj, EFL_EVENT_GESTURE_TAP, g);
         break;
      default:
         break;
   }
}

static void
flick_gesture_cb(void *data , const Efl_Event *ev)
{
   Eo* obj = (Eo*)data;
   EFL_UI_MI_RULE_DATA_GET_OR_RETURN(obj, pd);
   Efl_Canvas_Gesture *g = ev->info;
   switch(efl_gesture_state_get(g))
   {
      case EFL_GESTURE_STATE_STARTED:
         break;
      case EFL_GESTURE_STATE_CANCELED:
         break;
      case EFL_GESTURE_STATE_FINISHED:
         efl_event_callback_call(obj, EFL_EVENT_GESTURE_FLICK, g);
         break;
      default:
         break;
   }
}

static void
momentum_gesture_cb(void *data , const Efl_Event *ev)
{
   Eo* obj = (Eo*)data;
   EFL_UI_MI_RULE_DATA_GET_OR_RETURN(obj, pd);
   Efl_Canvas_Gesture *g = ev->info;
   Eina_Position2D pos = efl_gesture_hotspot_get(g);
   Eina_Vector2 m = efl_gesture_momentum_get(g);
   unsigned int t = efl_gesture_timestamp_get(g);

#if DEBUG
   printf("KTH Momentum Gesture updated x,y=<%d,%d> momentum=<%f %f> time=<%d>\n",
          pos.x, pos.y, m.x, m.y, t);
#endif

   switch(efl_gesture_state_get(g))
   {
      case EFL_GESTURE_STATE_STARTED:
         break;
      case EFL_GESTURE_STATE_UPDATED:
         break;
      case EFL_GESTURE_STATE_CANCELED:
         break;
      case EFL_GESTURE_STATE_FINISHED:
         efl_event_callback_call(obj, EFL_EVENT_GESTURE_MOMENTUM, g);
         break;
      default:
         break;
   }
}

//FIXME: tempororay function for matching vg node keypath
//       use begin and end words
static void
_convert_vg_node_keypath(char *keypath, char *vg_node_keypath)
{
   int i = 0;
   char *p = strchr(keypath, '.');
   if (p == NULL)
     return;

   char *cur = keypath;
   while(1)
   {
      if (cur == p)
        break;

      vg_node_keypath[i] = *cur;
      i++;
      cur++;
   }

   vg_node_keypath[i] = ' ';
   i++;

   p = strrchr(keypath, '.');
   p++;
   while(1)
   {
      vg_node_keypath[i] = *p;
      i++;
      if (*p == '\0')
        break;
      p++;
   }
}


EOLIAN static void
_efl_ui_mi_rule_keypath_set(Eo *obj EINA_UNUSED, Efl_Ui_Mi_Rule_Data *pd, Eina_Stringshare *keypath)
{
   if (!keypath) return;

   Evas *e = evas_object_evas_get(pd->controller);
   if (pd->event_rect == NULL)
     pd->event_rect = evas_object_rectangle_add(e);

   evas_object_color_set(pd->event_rect, 0, 0, 0, 0);
//#if DEBUG
   evas_object_color_set(pd->event_rect, 128, 0, 0, 128);
//#endif

   efl_event_callback_add(pd->event_rect, EFL_EVENT_GESTURE_TAP, tap_gesture_cb, obj);
   efl_event_callback_add(pd->event_rect, EFL_EVENT_GESTURE_FLICK, flick_gesture_cb, obj);
   efl_event_callback_add(pd->event_rect, EFL_EVENT_GESTURE_MOMENTUM, momentum_gesture_cb, obj);

#if DEBUG
   printf("%s (%p)\n", efl_class_name_get(efl_class_get(parent)), parent);
#endif

   eina_stringshare_replace(&pd->keypath, keypath);

   evas_object_event_callback_add(pd->controller, EVAS_CALLBACK_RESIZE, _parent_resize_cb, pd);
}

EOLIAN const Eina_Stringshare*
_efl_ui_mi_rule_keypath_get(const Eo *obj EINA_UNUSED, Efl_Ui_Mi_Rule_Data *pd)
{
   return pd->keypath;
}

EOLIAN static void
_efl_ui_mi_rule_value_provider_override(Eo *obj EINA_UNUSED, Efl_Ui_Mi_Rule_Data *pd, Efl_Gfx_Vg_Value_Provider *value_provider)
{
   if (!value_provider) return;
   Eo *parent;
   Eo *object = obj;
   Evas *e = NULL;
   do
     {
        parent = efl_parent_get(object);
        if (!parent) continue;

        if (efl_class_get(parent) == EFL_UI_MI_CONTROLLER_CLASS) break;
        object = parent;
     } while(!e);

   if (!efl_gfx_vg_value_provider_keypath_get(value_provider))
     {
        const buf[255];
        sprintf(buf, "%s.*.*", pd->keypath);
        Eina_Stringshare* new_keypath = eina_stringshare_add(buf);
        efl_gfx_vg_value_provider_keypath_set(value_provider, new_keypath);
        eina_stringshare_del(new_keypath);
     }
   efl_ui_mi_controller_value_provider_override(parent, value_provider);
}

void _efl_ui_mi_rule_current_state_set(Eo *obj, Efl_Ui_Mi_Rule_Data *pd, Efl_Ui_Mi_State *current_state)
{
   if (!current_state || pd->current_state == current_state) return;

   if (pd->current_state)
     {
        efl_event_callback_del(pd->current_state, EFL_UI_MI_STATE_EVENT_TRIGGER, _trigger_cb, NULL);
        efl_event_callback_del(pd->current_state, EFL_UI_MI_STATE_EVENT_FEEDBACK, _feedback_cb, NULL);
        efl_event_callback_del(pd->current_state, EFL_UI_MI_STATE_EVENT_ACTIVATE, _activate_cb, NULL);
        efl_event_callback_del(pd->current_state, EFL_UI_MI_STATE_EVENT_DEACTIVATE, _deactivate_cb, NULL);
     }

   pd->current_state = current_state;
   efl_event_callback_add(current_state, EFL_UI_MI_STATE_EVENT_TRIGGER, _trigger_cb, pd);
   efl_event_callback_add(current_state, EFL_UI_MI_STATE_EVENT_FEEDBACK, _feedback_cb, pd);
   efl_event_callback_add(current_state, EFL_UI_MI_STATE_EVENT_ACTIVATE, _activate_cb, pd);
   efl_event_callback_add(current_state, EFL_UI_MI_STATE_EVENT_DEACTIVATE, _deactivate_cb, pd);
}


Efl_Ui_Mi_State *_efl_ui_mi_rule_current_state_get(const Eo *obj, Efl_Ui_Mi_Rule_Data *pd)
{
   return pd->current_state;
}

EOLIAN static void
_efl_ui_mi_rule_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Mi_Rule_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->current_state = NULL;
}

EOLIAN static void
_efl_ui_mi_rule_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Mi_Rule_Data *pd EINA_UNUSED)
{
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_mi_rule_efl_object_destructor(Eo *obj,
                                          Efl_Ui_Mi_Rule_Data *pd EINA_UNUSED)
{
   if (pd->keypath) eina_stringshare_del(pd->keypath);
   efl_destructor(efl_super(obj, MY_CLASS));
}

Efl_VG*
_find_keypath_node(Efl_Ui_Mi_Rule_Data *pd)
{
   Efl_VG *key_node = NULL;

   Evas_Object *anim = efl_key_data_get(pd->controller, "anim");
   Evas_Object *vg = efl_key_data_get(anim, "vg_obj");

#if DEBUG
   prinitf("keypath finding, vg:%p, root node:%p, keypath:%s\n",
           vg,
           evas_object_vg_root_node_get(vg)
           pd->keypath);
#endif

   Efl_VG *root = evas_object_vg_root_node_get(vg);

   Eina_List *list = efl_canvas_vg_container_children_direct_get(root);
   Eina_List *l, *llist;
   Efl_VG *layer, *node;
   EINA_LIST_FOREACH(list, l, layer)
   {
     char *layer_keypath = efl_key_data_get(layer, "_lot_node_name");
     if (!strcmp(layer_keypath, pd->keypath))
       {
#if DEBUG
          printf("matched keypath is %s\n", buf);
#endif
          key_node = layer;
          break;
       }
   }
   return key_node;
}

void
_calculate_event_rect(Efl_Ui_Mi_Rule_Data *pd, Eina_Rect r)
{
   int x, y;
   evas_object_geometry_get(pd->controller, &x, &y, NULL, NULL);
   r.pos.x += x;
   r.pos.y += y;
   evas_object_move(pd->event_rect, r.pos.x, r.pos.y);
   evas_object_resize(pd->event_rect, r.size.w, r.size.h);
}

void
_calculate_text_part(Efl_Ui_Mi_Rule_Data *pd, Eina_Rect r)
{
   int x, y, w, h;
   int px, py;
   evas_object_geometry_get(pd->controller, &px, &py, NULL, NULL);
   r.pos.x += px;
   r.pos.y += py;
   Evas *e = evas_object_evas_get(pd->controller);
   Evas_Object *top = evas_object_top_get(e);
   evas_object_stack_below(pd->text_part, top);
   evas_object_geometry_get(pd->text_part, &x, &y, &w, &h);
   evas_object_move(pd->text_part, (r.pos.x + r.size.w/2) -(w/2) , (r.pos.y + r.size.h/2) - (h/2));
}

static void
_trigger_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_Rule_Data *pd = (Efl_Ui_Mi_Rule_Data *)data;
   Efl_VG *key_node = _find_keypath_node(pd);
   Eina_Rect r;
   efl_gfx_path_bounds_get(key_node, &r);

#if DEBUG
   printf("keypath node bounds get: %d %d %d %d\n", r.pos.x, r.pos.y, r.size.w, r.size.h);
#endif

   _calculate_event_rect(pd, r);
   _calculate_text_part(pd, r);
}


static void
_feedback_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_Rule_Data *pd = (Efl_Ui_Mi_Rule_Data *)data;
   Efl_VG *key_node = _find_keypath_node(pd);
   Eina_Rect r;
   efl_gfx_path_bounds_get(key_node, &r);

#if DEBUG
   printf("keypath :%s node bounds get: %d %d %d %d\n",pd->keypath, r.pos.x, r.pos.y, r.size.w, r.size.h);
#endif

   _calculate_event_rect(pd, r);
   _calculate_text_part(pd, r);
}

static void
_activate_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_Rule_Data *pd = (Efl_Ui_Mi_Rule_Data *)data;
   Efl_Ui_Mi_State *current_state = efl_ui_mi_controller_current_state_get(pd->controller);
#if DEBUG
   const char *name1;
   const char *name2;
   efl_ui_mi_state_sector_get(event->object, &name1, NULL);
   efl_ui_mi_state_sector_get(current_state, &name2, NULL);
   printf("Activate callback %s current : %s rect : %p\n",name1, name2 ? name2:"NULL", pd->event_rect);
#endif
   const char *registed_state;
   efl_ui_mi_state_sector_get(event->object, &registed_state, NULL);

   if (event->object == current_state || !strcmp(registed_state, "*"))
     {
        evas_object_show(pd->event_rect);
        evas_object_show(pd->text_part);
     }
}

static void
_deactivate_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_Rule_Data *pd = (Efl_Ui_Mi_Rule_Data *)data;
   Efl_Ui_Mi_State *current_state = efl_ui_mi_controller_current_state_get(pd->controller);
#if DEBUG
   const char *name1;
   const char *name2;
   efl_ui_mi_state_sector_get(event->object, &name1, NULL);
   efl_ui_mi_state_sector_get(current_state, &name2, NULL);
   printf("Deactivate callback %s current : %s rect : %p\n",name1, name2 ? name2:"NULL", pd->event_rect);
#endif
   const char *registed_state;
   efl_ui_mi_state_sector_get(event->object, &registed_state, NULL);

   if (event->object == current_state || !strcmp(registed_state, "*"))
     {
        evas_object_hide(pd->event_rect);
        efl_text_set(pd->text_part, "");
        evas_object_hide(pd->text_part);

     }
}

EOLIAN static Eo *
_efl_ui_mi_rule_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Mi_Rule_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   //evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   Eo *parent = NULL;
   Eo *object = obj;
   do {
        parent = efl_parent_get(object);
        if (!parent)
          break;

        if (efl_class_get(parent) == EFL_UI_MI_CONTROLLER_CLASS)
          {
             pd->controller = parent;
             break;
          }
        object = parent;
   } while(1);

   //FIXME: Make text style and size set optionable
   Evas *e = evas_object_evas_get(pd->controller);
   pd->text_part = evas_object_text_add(e);
   evas_object_text_style_set(pd->text_part, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_font_set(pd->text_part, "DejaVu", 30);
   evas_object_color_set(pd->text_part, 0, 0, 0, 255);
   evas_object_pass_events_set(pd->text_part, EINA_TRUE);

   return obj;
}

EOLIAN Efl_Object *
_efl_ui_mi_rule_efl_part_part_get(const Eo *obj, Efl_Ui_Mi_Rule_Data *pd, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   if (eina_streq(part, "efl.text"))
     return ELM_PART_IMPLEMENT(EFL_UI_MI_RULE_PART_TEXT_CLASS, obj, part);

  return NULL;
}

EOLIAN void
_efl_ui_mi_rule_part_text_efl_text_text_set(Eo *obj, void *pd, const char *text)
{
   Elm_Part_Data *data = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   EFL_UI_MI_RULE_DATA_GET_OR_RETURN(data->obj, rpd);

   evas_object_text_text_set(rpd->text_part, text);

   Efl_VG *key_node = _find_keypath_node(rpd);
   Eina_Rect r;
   efl_gfx_path_bounds_get(key_node, &r);

   _calculate_text_part(rpd, r);
}

EOLIAN const char *
_efl_ui_mi_rule_part_text_efl_text_text_get(const Eo *obj, void *pd)
{
  return NULL;
}

#include "efl_ui_mi_rule_part_text.eo.c"

/* Efl.Part begin */
/*ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_Rule_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_Rule_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_Rule_Data)
#include "efl_ui_mi_controller_part.eo.c"*/
/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_MI_RULE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_mi_rule)

#include "efl_ui_mi_rule.eo.c"
