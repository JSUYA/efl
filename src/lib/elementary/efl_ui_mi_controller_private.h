#ifndef EFL_UI_MI_CONTROLLER_PRIVATE_H
#define EFL_UI_MI_CONTROLLER_PRIVATE_H

#include "Elementary.h"
#include "efl_ui_mi_controller.eo.h"

typedef struct _Efl_Ui_Mi_Controller_Data Efl_Ui_Mi_Controller_Data;

struct _Efl_Ui_Mi_Controller_Data
{
   Efl_Ui_Vg_Animation* anim;
   Efl_Ui_Mi_State *current_state;
   Eina_Bool first_play;
   Eina_Array* states;
   Eina_Array* rules;
   Efl_Ui_Mi_Controller_Repeat_Mode repeat_mode;
   Eina_Bool is_feedback_play;
   //Eina_Inarray *states;             //array of Efl_Ui_Mi_State*;

};

#define EFL_UI_MI_CONTROLLER_DATA_GET(o, sd) \
  Efl_Ui_Mi_Controller_Data * sd = efl_data_scope_safe_get(o, EFL_UI_MI_CONTROLLER_CLASS)

#define EFL_UI_MI_CONTROLLER_DATA_GET_OR_RETURN(o, ptr)   \
  EFL_UI_MI_CONTROLLER_DATA_GET(o, ptr);                  \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       ERR("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return;                                          \
    }

#endif
