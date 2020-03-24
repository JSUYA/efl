#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_mi_rule_private.h"
//#include "efl_ui_mi_controller_part.eo.h"
//#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_MI_RULE_CLASS

#define MY_CLASS_NAME "Efl_Ui_Mi_Rule"

void
_tb_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
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

EOLIAN static void
_efl_ui_mi_rule_keypath_set(Eo *obj EINA_UNUSED, Efl_Ui_Mi_Rule_Data *pd, Eina_Stringshare *keypath)
{
   if (!keypath) return;
   Eo *parent;
   Eo *object = obj;
   Evas *e = NULL;
   do
     {
        parent = efl_parent_get(object);
        if (!parent) continue;

        if (efl_class_get(parent) == EFL_UI_MI_CONTROLLER_CLASS) 
          {
             e = evas_object_evas_get(parent);
             pd->controller = parent;
          }
        object = parent;
     } while(!e);

   if (pd->event_rect == NULL)
     pd->event_rect = evas_object_rectangle_add(e);

   evas_object_color_set(pd->event_rect, 0, 0, 0, 0);
#if DEBUG
   evas_object_color_set(pd->event_rect, 128, 0, 0, 128);
#endif

   efl_event_callback_add(pd->event_rect, EFL_EVENT_GESTURE_TAP, tap_gesture_cb, obj);

#if DEBUG
   printf("%s (%p)\n", efl_class_name_get(efl_class_get(parent)), parent);
#endif

   eina_stringshare_replace(&pd->keypath, keypath);

   evas_object_event_callback_add(parent, EVAS_CALLBACK_RESIZE, _tb_resize, pd->event_rect);
   if (!strcmp(keypath, "*"))
     {
        return ;
     }

   /* tracking keypath object */

   /* Set rect geometry */


}

EOLIAN const Eina_Stringshare*
_efl_ui_mi_rule_keypath_get(const Eo *obj EINA_UNUSED, Efl_Ui_Mi_Rule_Data *pd)
{
   return NULL;
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
      efl_gfx_vg_value_provider_keypath_set(value_provider, pd->keypath);
   efl_ui_mi_controller_value_provider_override(parent, value_provider);
}

EOLIAN static void
_efl_ui_mi_rule_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Mi_Rule_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);


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

static Efl_VG*
_find_keypath_node(Efl_Ui_Mi_Rule_Data *pd)
{
   char buf[256];
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
   Eina_List *l, *ll, *llist;
   Efl_VG *layer, *node;
   EINA_LIST_FOREACH(list, l, layer)
   {
     char *layer_keypath = efl_key_data_get(layer, "_lot_node_name");
     Eina_List *llist = efl_canvas_vg_container_children_direct_get(layer);
     EINA_LIST_FOREACH(llist, ll, node)
     {
        char *shape_keypath = efl_key_data_get(node, "_lot_node_name");
        snprintf(buf, sizeof(buf), "%s %s", layer_keypath, shape_keypath);
        if (!strcmp(buf, pd->keypath))
          {
#if DEBUG
             printf("matched keypath is %s\n", buf);
#endif
             key_node = node;
             break;
          }
        if (key_node != NULL)
          break;
     }
   }
   return key_node;
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

   evas_object_move(pd->event_rect, r.pos.x, r.pos.y);
   evas_object_resize(pd->event_rect, r.size.w, r.size.h);
}


static void
_feedback_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_Rule_Data *pd = (Efl_Ui_Mi_Rule_Data *)data;
   Efl_VG *key_node = _find_keypath_node(pd);
   Eina_Rect r;
   efl_gfx_path_bounds_get(key_node, &r);

#if DEBUG
   printf("keypath node bounds get: %d %d %d %d\n", r.pos.x, r.pos.y, r.size.w, r.size.h);
#endif

   evas_object_move(pd->event_rect, r.pos.x, r.pos.y);
   evas_object_resize(pd->event_rect, r.size.w, r.size.h);
}

static void
_activate_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_Rule_Data *pd = (Efl_Ui_Mi_Rule_Data *)data;
   evas_object_show(pd->event_rect);
}

static void
_deactivate_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_Rule_Data *pd = (Efl_Ui_Mi_Rule_Data *)data;
   evas_object_hide(pd->event_rect);
}

EOLIAN static Eo *
_efl_ui_mi_rule_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Mi_Rule_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   //evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   efl_event_callback_add(efl_parent_get(obj), EFL_UI_MI_STATE_EVENT_TRIGGER, _trigger_cb, pd);
   efl_event_callback_add(efl_parent_get(obj), EFL_UI_MI_STATE_EVENT_FEEDBACK, _feedback_cb, pd);
   efl_event_callback_add(efl_parent_get(obj), EFL_UI_MI_STATE_EVENT_ACTIVATE, _activate_cb, pd);
   efl_event_callback_add(efl_parent_get(obj), EFL_UI_MI_STATE_EVENT_DEACTIVATE, _deactivate_cb, pd);

   return obj;
}

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
