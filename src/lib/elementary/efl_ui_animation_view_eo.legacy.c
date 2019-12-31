
EAPI void
elm_animation_view_auto_play_set(Efl_Ui_Animation_View *obj, Eina_Bool auto_play)
{
   efl_ui_animation_view_autoplay_set(obj, auto_play);
}

EAPI Eina_Bool
elm_animation_view_auto_play_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_autoplay_get(obj);
}

EAPI void
elm_animation_view_auto_repeat_set(Efl_Ui_Animation_View *obj, Eina_Bool autorepeat)
{
   efl_ui_animation_view_autorepeat_set(obj, autorepeat);
}

EAPI Eina_Bool
elm_animation_view_auto_repeat_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_autorepeat_get(obj);
}

EAPI Eina_Bool
elm_animation_view_speed_set(Efl_Ui_Animation_View *obj, double speed)
{
   if (!obj) return EINA_FALSE;
   efl_player_playback_speed_set(obj, speed);
   return EINA_TRUE;
}

EAPI double
elm_animation_view_speed_get(const Efl_Ui_Animation_View *obj)
{
   return efl_player_playback_speed_get(obj);
}

EAPI double
elm_animation_view_duration_time_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_duration_time_get(obj);
}

EAPI void
elm_animation_view_progress_set(Efl_Ui_Animation_View *obj, double progress)
{
   efl_player_playback_progress_set(obj, progress);
}

EAPI double
elm_animation_view_progress_get(const Efl_Ui_Animation_View *obj)
{
   return efl_player_playback_progress_get(obj);
}

EAPI void
elm_animation_view_frame_set(Efl_Ui_Animation_View *obj, int frame_num)
{
   efl_ui_animation_view_frame_set(obj, frame_num);
}

EAPI int
elm_animation_view_frame_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_frame_get(obj);
}

EAPI Eina_Bool
elm_animation_view_play(Efl_Ui_Animation_View *obj)
{
   return efl_player_playing_set(obj, EINA_TRUE);
}

EAPI Eina_Bool
elm_animation_view_play_back(Efl_Ui_Animation_View *obj)
{
   double speed = efl_player_playback_speed_get(obj);
   efl_player_playback_speed_set(obj, speed < 0 ? speed * -1 : speed);
   return efl_player_playing_set(obj, EINA_TRUE);
}

EAPI Eina_Bool
elm_animation_view_pause(Efl_Ui_Animation_View *obj)
{
   return efl_player_paused_set(obj, EINA_TRUE);
}

EAPI Eina_Bool
elm_animation_view_resume(Efl_Ui_Animation_View *obj)
{
   return efl_player_paused_set(obj, EINA_FALSE);
}

EAPI Eina_Bool
elm_animation_view_stop(Efl_Ui_Animation_View *obj)
{
   return efl_player_playing_set(obj, EINA_FALSE);
}

EAPI Eina_Size2D
elm_animation_view_default_size_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_default_view_size_get(obj);
}

EAPI Eina_Bool
elm_animation_view_is_playing_back(Efl_Ui_Animation_View *obj)
{
   return (efl_ui_animation_view_state_get(obj) == EFL_UI_ANIMATION_VIEW_STATE_PLAYING_BACKWARDS);
}

EAPI int
elm_animation_view_frame_count_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_frame_count_get(obj);
}

EAPI void
elm_animation_view_min_progress_set(Efl_Ui_Animation_View *obj, double min_progress)
{
   efl_ui_animation_view_min_progress_set(obj, min_progress);
}

EAPI double
elm_animation_view_min_progress_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_min_progress_get(obj);
}

EAPI void
elm_animation_view_max_progress_set(Efl_Ui_Animation_View *obj, double max_progress)
{
   efl_ui_animation_view_max_progress_set(obj, max_progress);
}

EAPI double
elm_animation_view_max_progress_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_max_progress_get(obj);
}

EAPI void
elm_animation_view_min_frame_set(Efl_Ui_Animation_View *obj, int min_frame)
{
   efl_ui_animation_view_min_frame_set(obj, min_frame);
}

EAPI int
elm_animation_view_min_frame_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_min_frame_get(obj);
}

EAPI void
elm_animation_view_max_frame_set(Efl_Ui_Animation_View *obj, int max_frame)
{
   efl_ui_animation_view_max_frame_set(obj, max_frame);
}

EAPI int
elm_animation_view_max_frame_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_max_frame_get(obj);
}
