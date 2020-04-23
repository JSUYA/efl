#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_mi_state_private.h"
//#include "efl_ui_mi_controller_part.eo.h"
//#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_MI_STATE_CLASS

#define MY_CLASS_NAME "Efl_Ui_Mi_State"


EOLIAN static void
_efl_ui_mi_state_sector_set(Eo *eo_obj EINA_UNUSED, Efl_Ui_Mi_State_Data *pd,
                                const char *start, const char *end)
{
   if (!start) return ;

   if (pd->start) free(pd->start);
   pd->start = strdup(start);

   if (!end) return ;

   if (pd->end) free(pd->end);
   pd->end = strdup(end);

}

EOLIAN void
_efl_ui_mi_state_sector_get(const Eo *obj EINA_UNUSED, Efl_Ui_Mi_State_Data *pd, const char **start, const char **end)
{
   if (start)
     *start = pd->start;
   if (end)
     *end = pd->end;
}

Efl_Ui_Mi_Rule *
_efl_ui_mi_state_rule_get(const Eo *obj, Efl_Ui_Mi_State_Data *pd,
                               const char *rule_name)
{

   Eo *controller = efl_key_data_get(obj, "controller");
   if (!controller) {
       ERR("Controller is NULL\n");
   }
   Eo *rule = efl_ui_mi_controller_rule_get(controller, rule_name);
   if (rule)
     pd->rule_list = eina_list_append(pd->rule_list, rule);
   return rule;
}

static void
_activate_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_State_Data *pd = (Efl_Ui_Mi_State_Data*)data;
#if DEBUG
   printf("CALLED ACTIVATE CALLBACK (%s)\n", pd->start);
#endif

   Eina_List *l;
   Eo *rule;
   EINA_LIST_FOREACH(pd->rule_list, l, rule)
     efl_event_callback_call(rule, EFL_UI_MI_RULE_EVENT_ACTIVATE, event->object);
}

static void
_trigger_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_State_Data *pd = (Efl_Ui_Mi_State_Data*)data;
#if DEBUG
   printf("CALLED TRIGGER CALLBACK (%s)\n", pd->start);
#endif

}

static void
_feedback_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_State_Data *pd = (Efl_Ui_Mi_State_Data*)data;
#if DEBUG
   printf("CALLED FEEDBACK CALLBACK (%s)(%f) rule : %d\n", pd->start, event->info? *(double*)event->info:-1, eina_list_count(pd->rule_list));
#endif
   Eina_List *l;
   Eo *rule;
   EINA_LIST_FOREACH(pd->rule_list, l, rule)
     efl_event_callback_call(rule, EFL_UI_MI_RULE_EVENT_UPDATE, event->object);
}

static void
_deactivate_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_State_Data *pd = (Efl_Ui_Mi_State_Data*)data;
#if DEBUG
   printf("CALLED DEACTIVATE CALLBACK (%s)\n", pd->start);
#endif
   Eina_List *l;
   Eo *rule;
   EINA_LIST_FOREACH(pd->rule_list, l, rule)
     efl_event_callback_call(rule, EFL_UI_MI_RULE_EVENT_DEACTIVATE, event->object);
}

static void
_feedback_done_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Mi_State_Data *pd = (Efl_Ui_Mi_State_Data*)data;
#if DEBUG
   printf("CALLED FEEDBACK_DONE CALLBACK (%s)\n", pd->start);
#endif
}

EOLIAN static void
_efl_ui_mi_state_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Mi_State_Data *pd)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   efl_event_callback_priority_add(obj, EFL_UI_MI_STATE_EVENT_ACTIVATE, EFL_CALLBACK_PRIORITY_BEFORE, _activate_cb, pd);
   efl_event_callback_priority_add(obj, EFL_UI_MI_STATE_EVENT_DEACTIVATE, EFL_CALLBACK_PRIORITY_BEFORE, _deactivate_cb, pd);

   efl_event_callback_priority_add(obj, EFL_UI_MI_STATE_EVENT_TRIGGER, EFL_CALLBACK_PRIORITY_BEFORE, _trigger_cb, pd);
   efl_event_callback_priority_add(obj, EFL_UI_MI_STATE_EVENT_FEEDBACK, EFL_CALLBACK_PRIORITY_BEFORE, _feedback_cb, pd);
   efl_event_callback_priority_add(obj, EFL_UI_MI_STATE_EVENT_FEEDBACK_DONE, EFL_CALLBACK_PRIORITY_BEFORE, _feedback_done_cb, pd);

   pd->start = NULL;
   pd->end = NULL;
}

EOLIAN static void
_efl_ui_mi_state_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Mi_State_Data *pd EINA_UNUSED)
{
   efl_canvas_group_del(efl_super(obj, MY_CLASS));

   if (pd->start) free(pd->start);
   if (pd->end) free(pd->end);
}

EOLIAN static void
_efl_ui_mi_state_efl_object_destructor(Eo *obj,
                                          Efl_Ui_Mi_State_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_mi_state_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Mi_State_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   //evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   return obj;
}

/* Efl.Part begin */
/*ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_State_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_State_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_mi_controller, EFL_UI_MI_CONTROLLER, Efl_Ui_Mi_State_Data)
#include "efl_ui_mi_controller_part.eo.c"*/
/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_MI_STATE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_mi_state)

#include "efl_ui_mi_state.eo.c"
