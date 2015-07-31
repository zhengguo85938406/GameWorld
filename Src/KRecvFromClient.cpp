#include "stdafx.h"
#include "KPlayerServer.h"
#include "Engine/KG_CreateGUID.h"
#include "Common/CRC32.h"
#include "KRelayClient.h"
#include "KSO3World.h"
#include "KHero.h"
#include "KScene.h"
#include "Engine/KG_Time.h"
#include "Engine/KTextFilter.h"
#include "KBall.h"
#include "KPlayer.h"
#include "KLSClient.h"
#include "KFellowship.h"
#include "KLogClient.h"

void KPlayerServer::InitProcotolProcess()
{
    memset(m_ProcessProtocolFuns, 0, sizeof(m_ProcessProtocolFuns));
    memset(m_nProtocolSize, 0, sizeof(m_nProtocolSize));

    REGISTER_EXTERNAL_FUNC(c2s_handshake_request, &KPlayerServer::OnHandshakeRequest, sizeof(KC2S_HandShakeRequest));
    REGISTER_EXTERNAL_FUNC(c2s_loading_complete, &KPlayerServer::OnLoadingComplete, sizeof(KC2S_Loading_Complete));
    REGISTER_EXTERNAL_FUNC(c2s_player_logout, &KPlayerServer::OnPlayerLogout, sizeof(KC2S_PlayerLogout));
    REGISTER_EXTERNAL_FUNC(c2s_ping_signal, &KPlayerServer::OnPingSignal, sizeof(KC2S_Ping_Signal));
	REGISTER_EXTERNAL_FUNC(c2s_submit_limitplay_info, &KPlayerServer::OnSubmitLimitPlayInfo, sizeof(KC2S_Submit_Limitplay_Info));
    
    REGISTER_EXTERNAL_FUNC(c2s_move_action, &KPlayerServer::OnMoveAction, sizeof(KC2S_MoveAction));
	REGISTER_EXTERNAL_FUNC(c2s_change_face_direction, &KPlayerServer::OnChangeFaceDirection, sizeof(KC2S_ChangeFaceDirection));
    REGISTER_EXTERNAL_FUNC(c2s_hero_jump, &KPlayerServer::OnHeroJump, sizeof(KC2S_HeroJump));
    REGISTER_EXTERNAL_FUNC(c2s_take_ball, &KPlayerServer::OnPlayerTakeBall, sizeof(KC2S_TakeBall));
    REGISTER_EXTERNAL_FUNC(c2s_shoot_basket, &KPlayerServer::OnPlayerShootBasket, sizeof(KC2S_ShootBasket));
    REGISTER_EXTERNAL_FUNC(c2s_aimat, &KPlayerServer::OnPlayerAimAt, sizeof(KC2S_Aimat));
    REGISTER_EXTERNAL_FUNC(c2s_pickup_doodad, &KPlayerServer::OnPlayerPickupObject, sizeof(KC2S_PickupDoodad));
    REGISTER_EXTERNAL_FUNC(c2s_use_doodad, &KPlayerServer::OnPlayerUseDoodad, sizeof(KC2S_UseDoodad));
    REGISTER_EXTERNAL_FUNC(c2s_drop_doodad, &KPlayerServer::OnPlayerDropObject, sizeof(KC2S_DropDoodad));
	REGISTER_EXTERNAL_FUNC(c2s_run_mode, &KPlayerServer::OnPlayerChangeRunMode, sizeof(KC2S_RunMode));
    REGISTER_EXTERNAL_FUNC(c2s_shoot_doodad, &KPlayerServer::OnPlayerShootObject, sizeof(KC2S_ShootDoodad));
    REGISTER_EXTERNAL_FUNC(c2s_operate_ai_teammate, &KPlayerServer::OnOperateAITeammate, sizeof(KC2S_Operate_AI_Teammate));
    
    REGISTER_EXTERNAL_FUNC(c2s_hero_skill, &KPlayerServer::OnHeroSkill, sizeof(KC2S_HeroSkill));
	REGISTER_EXTERNAL_FUNC(c2s_pass_ball, &KPlayerServer::OnPlayerPassBall, sizeof(KC2S_PassBall));

    REGISTER_EXTERNAL_FUNC(c2s_apply_create_room, &KPlayerServer::OnApplyCreateRoom, sizeof(KApply_Create_Room));
    REGISTER_EXTERNAL_FUNC(c2s_apply_join_room, &KPlayerServer::OnApplyJoinRoom, sizeof(KApply_Join_Room));
    REGISTER_EXTERNAL_FUNC(c2s_invite_player_join_room, &KPlayerServer::OnInvitePlayerJoinRoom, sizeof(KInvite_Player_Join_Room));
    REGISTER_EXTERNAL_FUNC(c2s_switch_room_host, &KPlayerServer::OnSwitchRoomHost, sizeof(KApply_Switch_Room_Host));
    REGISTER_EXTERNAL_FUNC(c2s_switch_battle_map, &KPlayerServer::OnSwtichBattleMap, sizeof(KApply_Switch_Battle_Map));
    REGISTER_EXTERNAL_FUNC(c2s_set_room_password, &KPlayerServer::OnSetRoomPassword, sizeof(KApply_Set_Room_Password));
    REGISTER_EXTERNAL_FUNC(c2s_apply_leave_room, &KPlayerServer::OnApplyLeaveRoom, sizeof(KApply_Leave_Room));
    REGISTER_EXTERNAL_FUNC(c2s_apply_kickout_other, &KPlayerServer::OnApplyKickoutOther, sizeof(KApply_Kickout_Other));    
    REGISTER_EXTERNAL_FUNC(c2s_apply_room_info, &KPlayerServer::OnApplyRoomInfo, sizeof(KApply_Room_Info));
    REGISTER_EXTERNAL_FUNC(c2s_apply_set_ready, &KPlayerServer::OnApplySetReady, sizeof(KApply_Set_Ready));
    REGISTER_EXTERNAL_FUNC(c2s_apply_start_game, &KPlayerServer::OnApplyStartGame, sizeof(KApply_Start_Game));
    REGISTER_EXTERNAL_FUNC(c2s_roommember_change_pos_request, &KPlayerServer::OnRoomMemberChangePosRequest, sizeof(KRoomMember_Change_Pos_Request));
    REGISTER_EXTERNAL_FUNC(c2s_change_roomname, &KPlayerServer::OnChangeRoomName, sizeof(KChange_RoomName));
    REGISTER_EXTERNAL_FUNC(c2s_try_joinroom, &KPlayerServer::OnTryJoinRoom, sizeof(KC2S_TryJoinRoom));
    REGISTER_EXTERNAL_FUNC(c2s_accept_or_refuse_join_room, &KPlayerServer::OnAcceptOrRefuseJoinRoom, sizeof(KC2S_Accept_Or_Refuse_Join_Room));

    REGISTER_EXTERNAL_FUNC(c2s_playerlist_request, &KPlayerServer::OnPlayerListRequest, sizeof(KPlayerListRequest));

    REGISTER_EXTERNAL_FUNC(c2s_create_team_request, &KPlayerServer::OnCreateTeamRequest, sizeof(KC2S_Create_Team_Request));
    REGISTER_EXTERNAL_FUNC(c2s_team_invite_player_request, &KPlayerServer::OnTeamInvitePlayerRequest, sizeof(KC2S_Team_Invite_Player_Request));
    REGISTER_EXTERNAL_FUNC(c2s_team_kickout_player_request, &KPlayerServer::OnTeamKickoutPlayerRequest, sizeof(KC2S_Team_Kickout_Player_Request));
    REGISTER_EXTERNAL_FUNC(c2s_team_leave_request, &KPlayerServer::OnTeamLeaveRequest, sizeof(KC2S_Team_Leave_Request));
    REGISTER_EXTERNAL_FUNC(c2s_team_accept_or_refuse_invite, &KPlayerServer::OnTeamAcceptOrRefuseInvite, sizeof(KC2S_Team_Accept_Or_Refuse_Invite));
    REGISTER_EXTERNAL_FUNC(c2s_team_ready_request, &KPlayerServer::OnTeamReadyRequest, sizeof(KC2S_Team_Ready_Request));

    REGISTER_EXTERNAL_FUNC(c2s_automatch_request, &KPlayerServer::OnAutoMatchRequest, sizeof(KC2S_AutoMatch_Request));
    REGISTER_EXTERNAL_FUNC(c2s_cancel_automatch_request, &KPlayerServer::OnCancelAutoMatchRequest, sizeof(KC2S_Cancel_AutoMatch_Request));
    REGISTER_EXTERNAL_FUNC(c2s_auto_joinroom, &KPlayerServer::OnAutoJoinRoom, sizeof(KC2S_AutoJoinRoom));
    REGISTER_EXTERNAL_FUNC(c2s_apply_set_room_aimode, &KPlayerServer::OnApplySetRoomAIMode, sizeof(KC2S_Apply_Set_Room_Aimode));

    REGISTER_EXTERNAL_FUNC(c2s_talk_message, &KPlayerServer::OnTalkMessage, UNDEFINED_PROTOCOL_SIZE);

    REGISTER_EXTERNAL_FUNC(c2s_client_log, &KPlayerServer::OnClientLog, UNDEFINED_PROTOCOL_SIZE);
    REGISTER_EXTERNAL_FUNC(c2s_gmcommand, &KPlayerServer::OnGMCommand, UNDEFINED_PROTOCOL_SIZE);

    REGISTER_EXTERNAL_FUNC(c2s_get_fellowship_list_request, &KPlayerServer::OnGetFellowshipListRequest, sizeof(KGet_Fellowship_List_Request));
    REGISTER_EXTERNAL_FUNC(c2s_add_fellowship_request, &KPlayerServer::OnAddFellowshipRequest, sizeof(KAdd_Fellowship_Request));
    REGISTER_EXTERNAL_FUNC(c2s_del_fellowship_request, &KPlayerServer::OnDelFellowshipRequest, sizeof(KDel_Fellowship_Request));
    REGISTER_EXTERNAL_FUNC(c2s_add_fellowship_group_request, &KPlayerServer::OnAddFellowshipGroupRequest, sizeof(KAdd_Fellowship_Group_Request));
    REGISTER_EXTERNAL_FUNC(c2s_del_fellowship_group_request, &KPlayerServer::OnDelFellowshipGroupRequest, sizeof(KDel_Fellowship_Group_Request));
    REGISTER_EXTERNAL_FUNC(c2s_rename_fellowship_group_request, &KPlayerServer::OnRenameFellowshipGroupRequest, sizeof(KRename_Fellowship_Group_Request));
    REGISTER_EXTERNAL_FUNC(c2s_set_fellowship_remark_request, &KPlayerServer::OnSetFellowshipRemarkRequest, sizeof(KSet_Fellowship_Remark));
    REGISTER_EXTERNAL_FUNC(c2s_set_fellowship_group_request, &KPlayerServer::OnSetFellowshipGroupRequest, sizeof(KSet_Fellowship_Group));

    REGISTER_EXTERNAL_FUNC(c2s_sync_user_preferences, &KPlayerServer::OnSyncUserPreferences, UNDEFINED_PROTOCOL_SIZE);

    REGISTER_EXTERNAL_FUNC(c2s_get_award, &KPlayerServer::OnPlayerGetAwardRequest, sizeof(KC2S_Award));
    REGISTER_EXTERNAL_FUNC(c2s_buy_goods_request, &KPlayerServer::OnPlayerBuyGoodsRequest, sizeof(KC2S_Buy_Goods));
    REGISTER_EXTERNAL_FUNC(c2s_sell_goods_request, &KPlayerServer::OnPlayerSellGoodsRequest, sizeof(KC2S_Sell_Goods));
    REGISTER_EXTERNAL_FUNC(c2s_repair_single_equip_request, &KPlayerServer::OnPlayerRepairSingleEquipRequest, sizeof(KC2S_Repair_Single_Equip));
    REGISTER_EXTERNAL_FUNC(c2s_repair_all_equip_request, &KPlayerServer::OnPlayerRepairAllEquipRequest, sizeof(KC2S_Repair_All_Equip));
    REGISTER_EXTERNAL_FUNC(c2s_present_goods_request, &KPlayerServer::OnPlayerPresentGoodsRequeset, UNDEFINED_PROTOCOL_SIZE);
    REGISTER_EXTERNAL_FUNC(c2s_change_teamlogo_request, &KPlayerServer::OnPlayerChangeTeamLogoRequest, sizeof(KC2S_Change_TeamLogo_Request));
	REGISTER_EXTERNAL_FUNC(c2s_clear_teamlogo_new_flag, &KPlayerServer::OnPlayerClearTeamLogoNewFlag, sizeof(KC2S_Clear_TeamLogo_New_Flag));

    REGISTER_EXTERNAL_FUNC(c2s_hero_sub_skill, &KPlayerServer::OnHeroSubSkill, sizeof(KC2S_HeroSubSkill));
    REGISTER_EXTERNAL_FUNC(c2s_hero_grab_skill, &KPlayerServer::OnHeroGrabSkill, sizeof(KC2S_HeroGrabSkill));

    REGISTER_EXTERNAL_FUNC(c2s_get_training_hero_list, &KPlayerServer::OnGetTrainingHeroListRequest, sizeof(KC2S_Get_Training_Hero_List));
    REGISTER_EXTERNAL_FUNC(c2s_begin_training_hero_request, &KPlayerServer::OnBeginTrainingHeroRequest, sizeof(KC2S_Begin_Training_Hero_Request));
    REGISTER_EXTERNAL_FUNC(c2s_end_training_hero_request, &KPlayerServer::OnEndTrainingHeroRequest, sizeof(KC2S_End_Training_Hero_Request));
    REGISTER_EXTERNAL_FUNC(c2s_buy_training_seat_request, &KPlayerServer::OnBuyTrainingSeatRequest, sizeof(KC2S_Buy_Training_Seat_Request));
    REGISTER_EXTERNAL_FUNC(c2s_upgrade_teacher_level, &KPlayerServer::OnUpgradeTeacherLevel, sizeof(KC2S_Upgrade_Teacher_Level));
    

    REGISTER_EXTERNAL_FUNC(c2s_send_mail_request, &KPlayerServer::OnSendMailRequest, UNDEFINED_PROTOCOL_SIZE);
    REGISTER_EXTERNAL_FUNC(c2s_get_maillist_request, &KPlayerServer::OnGetMaillistRequest, sizeof(KC2S_Get_Maillist_Request));
    REGISTER_EXTERNAL_FUNC(c2s_query_mail_content, &KPlayerServer::OnQueryMailContent, sizeof(KC2S_Query_Mail_Content));
    REGISTER_EXTERNAL_FUNC(c2s_acquire_mail_money_request, &KPlayerServer::OnAcquireMailMoneyRequest, sizeof(KC2S_Acquire_Mail_Money_Request));
    REGISTER_EXTERNAL_FUNC(c2s_acquire_mail_item_request, &KPlayerServer::OnAcquireMailItemRequest, sizeof(KC2S_Acquire_Mail_Item_Request));
    REGISTER_EXTERNAL_FUNC(c2s_acquire_mail_all_request, &KPlayerServer::OnAcquireMailAllRequest, sizeof(KC2S_Acquire_Mail_All_Request));
    REGISTER_EXTERNAL_FUNC(c2s_set_mail_read, &KPlayerServer::OnSetMailRead, sizeof(KC2S_Set_Mail_Read));
    REGISTER_EXTERNAL_FUNC(c2s_delete_mail, &KPlayerServer::OnDeleteMail, sizeof(KC2S_Delete_Mail));

    REGISTER_EXTERNAL_FUNC(c2s_select_main_hero_request, &KPlayerServer::OnSelectMainHeroRequest, sizeof(KC2S_Select_Main_Hero_Request));
    REGISTER_EXTERNAL_FUNC(c2s_select_assist_hero_request, &KPlayerServer::OnSelectAssistHeroRequest, sizeof(KC2S_Select_Assist_Hero_Request));
    REGISTER_EXTERNAL_FUNC(c2s_set_vip_autorepair_flag, &KPlayerServer::OnSetVIPAutoRepairFlag, sizeof(KC2S_Set_VIP_AutoRepair_Flag));
    
    REGISTER_EXTERNAL_FUNC(c2s_get_quest_list_request, &KPlayerServer::OnGetQuestListRequest, sizeof(KC2S_Get_Quest_List_Request));
    REGISTER_EXTERNAL_FUNC(c2s_set_quest_value_request, &KPlayerServer::OnSetQuestValueRequest, sizeof(KC2S_Set_Quest_Value_Request));
    REGISTER_EXTERNAL_FUNC(c2s_accept_quest_request, &KPlayerServer::OnAcceptQuestRequest, sizeof(KC2S_Accept_Quest_Request));
    REGISTER_EXTERNAL_FUNC(c2s_finish_quest_request, &KPlayerServer::OnFinishQuestRequest, sizeof(KC2S_Finish_Quest_Request));
    REGISTER_EXTERNAL_FUNC(c2s_cancel_quest_request, &KPlayerServer::OnCancelQuestRequest, sizeof(KC2S_Cancel_Quest_Request));
    REGISTER_EXTERNAL_FUNC(c2s_done_quest_directly_request, &KPlayerServer::OnDoneQuestDirectlyRequest, sizeof(KC2S_Done_Quest_Directly_Request));

    REGISTER_EXTERNAL_FUNC(c2s_exchange_item_request, &KPlayerServer::OnExchangeItemRequest, sizeof(KC2S_Exchange_Item_Request));
    REGISTER_EXTERNAL_FUNC(c2s_destroy_item_request, &KPlayerServer::OnDestroyItemRequest, sizeof(KC2S_Destroy_Item_Request));
    REGISTER_EXTERNAL_FUNC(c2s_use_item_request, &KPlayerServer::OnUseItemRequest, sizeof(KC2S_Use_Item_Request));

    REGISTER_EXTERNAL_FUNC(c2s_apply_enter_mission, &KPlayerServer::OnApplyEnterMissionRequest, sizeof(KC2S_Apply_Enter_Mission));
    REGISTER_EXTERNAL_FUNC(c2s_apply_leave_mission, &KPlayerServer::OnApplyLeaveMissionRequest, sizeof(KC2S_Apply_Leave_Mission));
    REGISTER_EXTERNAL_FUNC(c2s_rename_request, &KPlayerServer::OnRenameRequest, sizeof(KC2S_Rename_Request));
    REGISTER_EXTERNAL_FUNC(c2s_get_daily_quest_list_request, &KPlayerServer::OnGetDailyQuestRequest, sizeof(KC2S_Get_Daily_Quest_List_Request));
    
    REGISTER_EXTERNAL_FUNC(c2s_create_pve_team_request, &KPlayerServer::OnCreatePveTeamRequest, sizeof(KC2S_Create_Pve_Team_Request));
    REGISTER_EXTERNAL_FUNC(c2s_quick_start_pve_request, &KPlayerServer::OnQuickStartPveRequest, sizeof(KC2S_Quick_Start_Pve_Request));
	REGISTER_EXTERNAL_FUNC(c2s_apply_join_pve_team_request, &KPlayerServer::OnApplyJoinPveTeamRequest, sizeof(KC2S_Apply_Join_Pve_Team_Request));
    REGISTER_EXTERNAL_FUNC(c2s_apply_pve_team_info_request, &KPlayerServer::OnApplyPveTeamInfoRequest, sizeof(KC2S_Apply_Pve_Team_Info_Request));
    REGISTER_EXTERNAL_FUNC(c2s_pve_team_invite_player_request, &KPlayerServer::OnPveTeamInvitePlayerRequest, sizeof(KC2S_Pve_Team_Invite_Player_Request));
    REGISTER_EXTERNAL_FUNC(c2s_pve_team_kickout_player_request, &KPlayerServer::OnPveTeamKickoutPlayerRequest, sizeof(KC2S_Pve_Team_Kickout_Player_Request));
    REGISTER_EXTERNAL_FUNC(c2s_pve_team_leave_request, &KPlayerServer::OnPveTeamLeaveRequest, sizeof(KC2S_Pve_Team_Leave_Request));
    REGISTER_EXTERNAL_FUNC(c2s_pve_team_accept_or_refuse_invite, &KPlayerServer::OnPveTeamAcceptOrRefuseInvite, sizeof(KC2S_Pve_Team_Accept_Or_Refuse_Invite));
    REGISTER_EXTERNAL_FUNC(c2s_pve_team_ready_request, &KPlayerServer::OnPveTeamReadyRequest, sizeof(KC2S_Pve_Team_Ready_Request));
    REGISTER_EXTERNAL_FUNC(c2s_apply_set_pve_room_aimode, &KPlayerServer::OnApplySetPveTeamAiMode, sizeof(KC2S_Apply_Set_PVE_Room_Aimode));
    REGISTER_EXTERNAL_FUNC(c2s_pve_team_start_game_request, &KPlayerServer::OnPveTeamStartGameRequest, sizeof(KC2S_Pve_Team_Start_Game_Request));
    REGISTER_EXTERNAL_FUNC(c2s_apply_extend_player_package, &KPlayerServer::OnApplyExtendPlayerPackage, sizeof(KC2S_Apply_Extend_Player_Package));
    REGISTER_EXTERNAL_FUNC(c2s_cut_stack_item_request, &KPlayerServer::OnCutStackItemRequest, sizeof(KC2S_Cut_Stack_Item_Request));
    REGISTER_EXTERNAL_FUNC(c2s_pve_team_change_mission, &KPlayerServer::OnPveTeamChangeMissionRequest, sizeof(KC2S_Pve_Team_Change_Mission));
    REGISTER_EXTERNAL_FUNC(c2s_try_join_pve_team, &KPlayerServer::OnTryJoinPveTeam, sizeof(KC2S_Try_Join_Pve_Team));
    REGISTER_EXTERNAL_FUNC(c2s_auto_join_pve_team, &KPlayerServer::OnAutoJoinPveTeam, sizeof(KC2S_Auto_Join_Pve_Team));
    REGISTER_EXTERNAL_FUNC(c2s_send_network_delay, &KPlayerServer::OnSendNetworkDelay, sizeof(KC2S_Send_Network_Delay));
    REGISTER_EXTERNAL_FUNC(c2s_activity_award_request, &KPlayerServer::OnActivityAwardRequest, sizeof(KC2S_Activity_Award_Request));
    REGISTER_EXTERNAL_FUNC(c2s_stat_client_info, &KPlayerServer::OnStatClientInfo, sizeof(KC2S_Stat_Client_Info));
    REGISTER_EXTERNAL_FUNC(c2s_stat_client_loading_info, &KPlayerServer::OnStatClientLoadingInfo, sizeof(KC2S_Stat_Client_Loading_Info));
    REGISTER_EXTERNAL_FUNC(c2s_apply_scene_obj, &KPlayerServer::OnApplySceneObj, sizeof(KC2S_Apply_Scene_Obj));
    REGISTER_EXTERNAL_FUNC(c2s_apply_scene_hero_data, &KPlayerServer::OnApplySceneHeroData, sizeof(KC2S_Apply_Scene_Hero_Data));
    REGISTER_EXTERNAL_FUNC(c2s_normal_shootball, &KPlayerServer::OnNormalShootBall, sizeof(KC2S_Normal_ShootBall));
    REGISTER_EXTERNAL_FUNC(c2s_skill_shootball, &KPlayerServer::OnSkillShootBall, sizeof(KC2S_Skill_ShootBall));
    REGISTER_EXTERNAL_FUNC(c2s_normal_slamball, &KPlayerServer::OnNormalSlamBall, sizeof(KC2S_Normal_SlamBall));
    REGISTER_EXTERNAL_FUNC(c2s_skill_slamball, &KPlayerServer::OnSkillSlamBall, sizeof(KC2S_Skill_SlamBall));

    REGISTER_EXTERNAL_FUNC(c2s_makingmachineinfo_request, &KPlayerServer::OnMakingMachineInfoRequest, sizeof(KC2S_MakingMachineInfo_Request));
    REGISTER_EXTERNAL_FUNC(c2s_produceitem_request, &KPlayerServer::OnProduceItemRequest, sizeof(KC2S_ProduceItem_Request));

    REGISTER_EXTERNAL_FUNC(c2s_apply_upgrade_gym_equip, &KPlayerServer::OnApplyUpgradeGymEquip, sizeof(KC2S_Apply_Upgrade_Gym_Equip));
    REGISTER_EXTERNAL_FUNC(c2s_apply_use_gym_equip, &KPlayerServer::OnApplyUseGymEquip, sizeof(KC2S_Apply_Use_Gym_Equip));
    REGISTER_EXTERNAL_FUNC(c2s_direct_end_use_gym_equip, &KPlayerServer::OnApplyDirectEndUseGymEquip, sizeof(KC2S_Direct_End_Use_Gym_Equip));
    REGISTER_EXTERNAL_FUNC(c2s_buy_gym_slot, &KPlayerServer::OnApplyBuyGymSlot, sizeof(KC2S_Buy_Gym_Slot));
    REGISTER_EXTERNAL_FUNC(c2s_strengthen_equip, &KPlayerServer::OnStrengthenEquipRequest, sizeof(KC2S_Strengthen_Equip));
    REGISTER_EXTERNAL_FUNC(c2s_buy_fatigue_point, &KPlayerServer::OnBuyFatiguePointRequest, sizeof(KC2S_Buy_Fatigue_Point));

    REGISTER_EXTERNAL_FUNC(c2s_report_item_read, &KPlayerServer::OnReportItemRead, sizeof(KC2S_Report_Item_Read));

    REGISTER_EXTERNAL_FUNC(c2s_buy_fashion_request, &KPlayerServer::OnBuyFashionRequest, sizeof(KC2S_Buy_Fashion_Request));
    REGISTER_EXTERNAL_FUNC(c2s_change_hero_fashion_request, &KPlayerServer::OnChangeHeroFashionRequest, sizeof(KC2S_Change_Hero_Fashion_Request));
    REGISTER_EXTERNAL_FUNC(c2s_charge_fashion_request, &KPlayerServer::OnChargeFashionRequest, sizeof(KC2S_Charge_Fashion_Request));

    REGISTER_EXTERNAL_FUNC(c2s_buy_cheerleading_slot,           &KPlayerServer::OnBuyCheerleaingSlot,       sizeof(KC2S_Buy_Cheerleading_Slot));
    REGISTER_EXTERNAL_FUNC(c2s_update_cheerleading_slotinfo,    &KPlayerServer::OnUpdateCheerleadingSlotInfo,    sizeof(KC2S_Update_Cheerleading_Slotinfo));
    REGISTER_EXTERNAL_FUNC(c2s_recharge_cheerleading_item,      &KPlayerServer::OnRechareCheerleadingItem,  sizeof(KC2S_Recharge_Cheerleading_Item));

    REGISTER_EXTERNAL_FUNC(c2s_clear_upgradecd_request,  &KPlayerServer::OnClearUpgradeCDRequest,  sizeof(KC2S_Clear_UpgradeCD_Request));
    REGISTER_EXTERNAL_FUNC(c2s_upgrade_gym_equip_request,  &KPlayerServer::OnUpgradeGymEquipRequest,  sizeof(KC2S_Upgrade_Gym_Equip_Request));    
    REGISTER_EXTERNAL_FUNC(c2s_hero_use_equip_request,  &KPlayerServer::OnHeroUseEquipRequest,  sizeof(KC2S_Hero_Use_Equip_Request));
    REGISTER_EXTERNAL_FUNC(c2s_direct_finish_use_equip_request,  &KPlayerServer::OnDirectFinishUseEquipRequest,  sizeof(KC2S_Direct_Finish_Use_Equip_Request));
    
    REGISTER_EXTERNAL_FUNC(c2s_buy_hero_request,  &KPlayerServer::OnBuyHeroRequest,  sizeof(KC2S_Buy_Hero_Request));
    REGISTER_EXTERNAL_FUNC(c2s_fire_hero_request,  &KPlayerServer::OnFireHeroRequest,  sizeof(KC2S_Fire_Hero_Request));
    REGISTER_EXTERNAL_FUNC(c2s_buy_hero_solt_request,  &KPlayerServer::OnBuyHeroSoltRequest,  sizeof(KC2S_Buy_Hero_Solt_Request));

    REGISTER_EXTERNAL_FUNC(c2s_bs_buy_land,  &KPlayerServer::OnBsBuyLandRequest,  sizeof(KC2S_Bs_Buy_Land));
    REGISTER_EXTERNAL_FUNC(c2s_bs_build_store,  &KPlayerServer::OnBsBuildStoreRequest,  sizeof(KC2S_Bs_Build_Store));
    REGISTER_EXTERNAL_FUNC(c2s_bs_upgrade_store,  &KPlayerServer::OnBsUpgradeStoreRequest,  sizeof(KC2S_Bs_Upgrade_Store));
    REGISTER_EXTERNAL_FUNC(c2s_bs_get_store_profit,  &KPlayerServer::OnBsGetStoreProfit,  sizeof(KC2S_Bs_Get_Store_Profit));
    REGISTER_EXTERNAL_FUNC(c2s_bs_clear_get_store_profit_cd,  &KPlayerServer::OnBsClearGetStoreProfitCD,  sizeof(KC2S_Bs_Clear_Get_Store_Profit_CD));
    REGISTER_EXTERNAL_FUNC(c2s_bs_get_all_store_profit,  &KPlayerServer::OnBsGetAllStoreProfit,  sizeof(KC2S_Bs_Get_All_Store_Profit));
    REGISTER_EXTERNAL_FUNC(c2s_bs_clear_all_get_store_profit_cd,  &KPlayerServer::OnBsClearAllGetStoreProfitCD,  sizeof(KC2S_Bs_Clear_All_Get_Store_Profit_CD));
    REGISTER_EXTERNAL_FUNC(c2s_bs_change_store_type,  &KPlayerServer::OnBsChangeStoreType,  sizeof(KC2S_Bs_Change_Store_Type));


    REGISTER_EXTERNAL_FUNC(c2s_apply_upgrade_safe_box,  &KPlayerServer::OnApplyUpgradeSafeBox,  sizeof(KC2S_Apply_Upgrade_Safe_Box));
	REGISTER_EXTERNAL_FUNC(c2s_dialog_completed, &KPlayerServer::OnDialogCompleted, sizeof(KC2S_Dialog_Completed));
    REGISTER_EXTERNAL_FUNC(c2s_scene_pause, &KPlayerServer::OnScenePause, sizeof(KC2S_Scene_Pause));
    REGISTER_EXTERNAL_FUNC(c2s_start_drama_request, &KPlayerServer::OnStartDramaRequest, sizeof(KC2S_Start_Drama_Request)); 
    
    REGISTER_EXTERNAL_FUNC(c2s_create_club, &KPlayerServer::OnCreateClubRequest, sizeof(KC2S_Create_Club)); 
    REGISTER_EXTERNAL_FUNC(c2s_get_member_list, &KPlayerServer::OnGetMemberListRequest, sizeof(KC2S_Get_Member_List)); 
    REGISTER_EXTERNAL_FUNC(c2s_apply_club, &KPlayerServer::OnApplyClubRequest, sizeof(KC2S_Apply_Club)); 
    REGISTER_EXTERNAL_FUNC(c2s_accept_app, &KPlayerServer::OnAcceptAppRequest, sizeof(KC2S_Accept_App)); 
    REGISTER_EXTERNAL_FUNC(c2s_invite_player, &KPlayerServer::OnInviteJoinClub, sizeof(KC2S_Invite_Player)); 
    REGISTER_EXTERNAL_FUNC(c2s_accept_invite, &KPlayerServer::OnAccetpInviteRequest, sizeof(KC2S_Accept_Invite)); 
    REGISTER_EXTERNAL_FUNC(c2s_set_club_post, &KPlayerServer::OnSetClubPostRequest, sizeof(KC2S_Set_Club_Post)); 
    REGISTER_EXTERNAL_FUNC(c2s_del_club_member, &KPlayerServer::OnDelMemberRequest, sizeof(KC2S_Del_Club_Member)); 
    REGISTER_EXTERNAL_FUNC(c2s_set_club_txt, &KPlayerServer::OnSetClubTxtRequest, UNDEFINED_PROTOCOL_SIZE); 
    REGISTER_EXTERNAL_FUNC(c2s_get_random_club, &KPlayerServer::OnGetRandClubRequest, sizeof(KC2S_Get_Random_Club)); 
    REGISTER_EXTERNAL_FUNC(c2s_apply_list, &KPlayerServer::OnGetApplyListRequest, sizeof(KC2S_Apply_List)); 
    REGISTER_EXTERNAL_FUNC(c2s_get_club_apply_num, &KPlayerServer::OnGetClubApplyNumRequest, sizeof(KC2S_Get_Club_Apply_Num)); 
    REGISTER_EXTERNAL_FUNC(c2s_switch_apply, &KPlayerServer::OnSwitchApplyRequest, sizeof(KC2S_Switch_Apply)); 
	REGISTER_EXTERNAL_FUNC(c2s_find_club, &KPlayerServer::OnFindClubRequest, sizeof(KC2S_Find_Club)); 
	REGISTER_EXTERNAL_FUNC(c2s_get_club_list, &KPlayerServer::OnGetClubListRequest, sizeof(KC2S_Get_Club_List)); 

    REGISTER_EXTERNAL_FUNC(c2s_finish_achievement, &KPlayerServer::OnFinishAchievementRequest, sizeof(KC2S_Finish_Achievement)); 
    REGISTER_EXTERNAL_FUNC(c2s_increase_achievement_value, &KPlayerServer::OnIncreaseAchievementValue, sizeof(KC2S_Increase_Achievement_Value)); 

    REGISTER_EXTERNAL_FUNC(c2s_get_tester_award, &KPlayerServer::OnGetTesterAward, sizeof(KC2S_Get_Tester_Award)); 
    REGISTER_EXTERNAL_FUNC(c2s_view_online_award, &KPlayerServer::OnViewOnlineAward, sizeof(KC2S_View_Online_Award)); 
    REGISTER_EXTERNAL_FUNC(c2s_get_online_award, &KPlayerServer::OnGetOnlineAward, sizeof(KC2S_Get_Online_Award)); 
    REGISTER_EXTERNAL_FUNC(c2s_get_award_table_info, &KPlayerServer::OnGetAwardTableInfo, sizeof(KC2S_Get_Award_Table_Info)); 
    REGISTER_EXTERNAL_FUNC(c2s_get_levelup_award, &KPlayerServer::OnGetLevelupAward, sizeof(KC2S_Get_Levelup_Award));

    REGISTER_EXTERNAL_FUNC(c2s_request_playervalue_group, &KPlayerServer::OnRequestPlayerValueGroup, sizeof(KC2S_Request_PlayerValue_Group));

    REGISTER_EXTERNAL_FUNC(c2s_client_call_request, &KPlayerServer::OnClientCallRequest, sizeof(KC2S_Client_Call_Request));
	REGISTER_EXTERNAL_FUNC(c2s_vip_charge_self, &KPlayerServer::OnVIPChargeSelf, sizeof(KC2S_Vip_Charge_Self));
    REGISTER_EXTERNAL_FUNC(c2s_vip_charge_freind, &KPlayerServer::OnVIPChargeFriend, sizeof(KC2S_Vip_Charge_Friend));
    REGISTER_EXTERNAL_FUNC(c2s_get_random_quest, &KPlayerServer::OnGetRandomQuest, sizeof(KC2S_Get_Random_Quest));
    REGISTER_EXTERNAL_FUNC(c2s_skip_random_quest, &KPlayerServer::OnSkipRandomQuest, sizeof(KC2S_Skip_Random_Quest));
    REGISTER_EXTERNAL_FUNC(c2s_select_hero_skill, &KPlayerServer::OnSelectHeroSkill, sizeof(KC2S_Select_Hero_Skill));
    REGISTER_EXTERNAL_FUNC(c2s_cast_slamball_skill, &KPlayerServer::OnCastSlamBallSkill, sizeof(KC2S_Cast_SlamBall_Skill));
    REGISTER_EXTERNAL_FUNC(c2s_normal_slamballex, &KPlayerServer::OnNormalSlamBallEx, sizeof(KC2S_Normal_SlamBallEx));
    REGISTER_EXTERNAL_FUNC(c2s_apply_active_award, &KPlayerServer::OnApplyActiveAward, sizeof(KC2S_Apply_Active_Award));

    REGISTER_EXTERNAL_FUNC(c2s_findrole_in_toplist, &KPlayerServer::OnFindRoleInTopListRequest, sizeof(KC2S_FindRole_In_TopList));
    REGISTER_EXTERNAL_FUNC(c2s_request_toplist_page, &KPlayerServer::OnRequestTopListPage, sizeof(KC2S_Request_TopList_Page));
    REGISTER_EXTERNAL_FUNC(c2s_request_friend_hero_list, &KPlayerServer::OnRequestFriendHeroList, sizeof(KC2S_Request_Friend_Hero_List));
    REGISTER_EXTERNAL_FUNC(c2s_request_friend_hero_info, &KPlayerServer::OnRequestFriendHeroInfo, sizeof(KC2S_Request_Friend_Hero_Info));
    REGISTER_EXTERNAL_FUNC(c2s_set_refuse_stranger, &KPlayerServer::OnSetRefuseStranger, sizeof(KC2S_Set_Refuse_Stranger));

    REGISTER_EXTERNAL_FUNC(c2s_request_random_hall_player_list, &KPlayerServer::OnRequestRandomHallPlayerList, sizeof(KC2S_Request_Random_Hall_Player_List));
    REGISTER_EXTERNAL_FUNC(c2s_get_enter_scene_herolist_request, &KPlayerServer::OnGetEnterSceneHeroListRequest, sizeof(KC2S_Header));
    REGISTER_EXTERNAL_FUNC(c2s_sync_enter_scene_progress, &KPlayerServer::OnSyncEnterSceneProgress, sizeof(KC2S_Sync_Enter_Scene_Progress));
    REGISTER_EXTERNAL_FUNC(c2s_request_recommend_friend, &KPlayerServer::OnReqeustRecommendFriend, sizeof(KC2S_Request_Recommend_Friend));
    REGISTER_EXTERNAL_FUNC(c2s_decompose_equip_request, &KPlayerServer::OnDecomposeEquipRequest, UNDEFINED_PROTOCOL_SIZE);
    REGISTER_EXTERNAL_FUNC(c2s_request_club_name, &KPlayerServer::OnRequestClubName, sizeof(KC2S_Request_Club_Name));
    REGISTER_EXTERNAL_FUNC(c2s_close_battle_report, &KPlayerServer::OnCloseBattleReport, sizeof(KC2S_Header));
    REGISTER_EXTERNAL_FUNC(c2s_rank_quick_start_request, &KPlayerServer::OnRankQuickStartRequest, sizeof(KC2S_Rank_Quick_Start_Request));

    REGISTER_EXTERNAL_FUNC(c2s_change_room_host, &KPlayerServer::OnChangeRoomHost, sizeof(KC2S_Change_Room_Host));
    REGISTER_EXTERNAL_FUNC(c2s_freepvp_quick_start, &KPlayerServer::OnFreePVPQucikStart, sizeof(KC2S_FreePVP_Quick_Start));
    REGISTER_EXTERNAL_FUNC(c2s_change_current_city, &KPlayerServer::OnChangeCurrentCity, sizeof(KC2S_Change_Current_City));

	//AutoCode:注册协议
}

KHero* KPlayerServer::GetFightingHeroByConnection(int nConnIndex)
{
    KHero* pResult = NULL;
    KPlayer* pPlayer = GetPlayerByConnection(nConnIndex);
    if (pPlayer)
    {
        pResult = pPlayer->GetFightingHero();
    }
    
    return pResult;
}

void KPlayerServer::OnHandshakeRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    KC2S_HandShakeRequest*  pRequest        = (KC2S_HandShakeRequest*)pData;
    KPlayer*                pPlayer         = NULL;
	KScene*                 pScene          = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRequest->roleID);
    KGLOG_PROCESS_ERROR(pPlayer);

    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsWaitForConnect);

    bRetCode = memcmp(&pPlayer->m_Guid, &pRequest->guid, sizeof(pPlayer->m_Guid));
    KGLOG_PROCESS_ERROR(bRetCode == 0);

    pRequest->navigatorInfo[sizeof(pRequest->navigatorInfo) - 1] = '\0';
    PLAYER_LOG(pPlayer, "client,navigatorInfo,%s", pRequest->navigatorInfo);
    PLAYER_LOG(pPlayer, "client,clientStageSize,%d,%d", pRequest->stageWidth, pRequest->stageHeight);

    bRetCode = Attach(pPlayer, nConnIndex);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPlayer->m_eGameStatus  = gsWaitForPermit;
    pPlayer->m_nTimer       = 0;

    g_RelayClient.DoConfirmPlayerLoginRequest(pPlayer->m_dwID, pPlayer->m_dwClientIP);

    KGLogPrintf(KGLOG_INFO, "player (%s:%u) handshake success!", pPlayer->m_szAccount, pPlayer->m_dwID);

    bResult = true;
Exit0:
    if (!bResult)
    {
        KGLogPrintf(KGLOG_INFO, "Handshake error, connection: %d\n", nConnIndex);
    }
    return;
}

void KPlayerServer::OnApplySceneObj(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    KScene*     pScene      = NULL;	
    KC2S_Apply_Scene_Obj* pNoitfy = (KC2S_Apply_Scene_Obj*)pData;
    KPlayer* pPlayer = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsWaitForNewMapLoading);

    pScene = g_pSO3World->GetScene(pPlayer->m_dwMapID, pPlayer->m_nCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);

    // 同步场景信息给玩家自己
    pScene->SyncAllObjToPlayer(pPlayer);

    DoSyncObjEnd(nConnIndex);

    LogInfo("account %s apply scene object", pPlayer->m_szAccount);

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (pPlayer)
        {
            pPlayer->ReturnToHall();
        }
    }
    return;
}

void KPlayerServer::OnApplySceneHeroData(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    KScene*                     pScene          = NULL;
    KPlayer*                    pPlayer         = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsWaitForNewMapLoading);

    pScene = g_pSO3World->GetScene(pPlayer->m_dwMapID, pPlayer->m_nCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);

    bRetCode = pScene->AddHeroForPlayer(pPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);
    ++pScene->m_nEnterPlayerCount;

    // 检查所有人加入
    if (pScene->m_Param.m_eBattleType == KBATTLE_TYPE_MISSION)
    {
        int nCanEnterPlayerCount = pScene->GetCanEnterPlayerCount();
        if (pScene->m_nEnterPlayerCount == nCanEnterPlayerCount)
        {
            pScene->LoadNpcInMission();
            pScene->CallOnPlayersInMissionReadyScript();
            pScene->m_bLoadNpcAfterAllPlayerEnter = true;
        }
    }

    DoSyncSceneHeroDataEnd(nConnIndex);

    LogInfo("account %s apply scene hero data", pPlayer->m_szAccount);

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (pPlayer)
        {
            pPlayer->ReturnToHall();
        }
    }

    return;
}

void KPlayerServer::OnLoadingComplete(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	BOOL        bResult     = false;
	BOOL        bRetCode    = false;
    KScene*     pScene      = NULL;	
    KC2S_Loading_Complete* pNoitfy = (KC2S_Loading_Complete*)pData;
    KPlayer* pPlayer = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsWaitForNewMapLoading);

    pScene = g_pSO3World->GetScene(pPlayer->m_dwMapID, pPlayer->m_nCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);

    bRetCode = pScene->PlayerSetReady(pPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);

    LogInfo("account %s loading complete", pPlayer->m_szAccount);

	bResult = true;
Exit0:
    if (!bResult)
    {
        if (pPlayer)
        {
            pPlayer->ReturnToHall();
        }
    }
	return;
}

// 玩家登出
void KPlayerServer::OnPlayerLogout(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_PlayerLogout*   pPlayerLogout   = (KC2S_PlayerLogout*)pData;
    KPlayer*             pPlayer         = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer && (pPlayer->m_eGameStatus == gsPlaying || pPlayer->m_eGameStatus == gsInHall));

    Detach(nConnIndex);

    KGLogPrintf(KGLOG_INFO, "Player logout: %s", pPlayer->m_szName);

Exit0:
    return;
}

void KPlayerServer::OnPingSignal(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Ping_Signal*    pPingSignal = (KC2S_Ping_Signal*)pData;
    KS2C_PingSignal      PingEcho;

    PingEcho.protocolID = s2c_ping_signal;
    PingEcho.time = pPingSignal->time;

    Send(nConnIndex, &PingEcho, sizeof(PingEcho));
}

void KPlayerServer::OnSubmitLimitPlayInfo( char* pData, size_t nSize, int nConnIndex, int nFrame )
{
	KC2S_Submit_Limitplay_Info*	pInfo	= (KC2S_Submit_Limitplay_Info*)pData;

	BOOL				bRetCode        = false;
	KPlayer*			pPlayer			= NULL;

	pPlayer = GetPlayerByConnection(nConnIndex);
	KGLOG_PROCESS_ERROR(pPlayer);

	g_RelayClient.DoSubmitLimitPlayInfo((DWORD)pPlayer->m_nGroupID, pPlayer->m_dwClientIP, pPlayer->m_szAccount, pInfo->szName, pInfo->szID, pInfo->szEmail);

Exit0:
	return;
}

void KPlayerServer::OnChangeFaceDirection(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	BOOL							bAdjust     = false;
	KC2S_ChangeFaceDirection*       pChangeFaceDirection   = (KC2S_ChangeFaceDirection*)pData;
    KHero*                          pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL                            bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero);

    pHero->m_MoveCount = pChangeFaceDirection->movecount;

    bAdjust = pHero->CheckMoveAdjust(nFrame, pChangeFaceDirection->moveparam, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    if(!pHero->m_pCastingSkill || pHero->m_pCastingSkill->m_bCanSpecifyCastingDir)
    {
        pHero->UpdateFaceDir(pChangeFaceDirection->direction8 * 32);
        g_PlayerServer.DoSyncFaceDir(pHero, pChangeFaceDirection->direction8);
    }

    if (bAdjust)
    {
        DoAdjustHeroMove(pHero, KCHANGE_FACE);
    }

Exit0:
	return;
}
void KPlayerServer::OnMoveAction(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bAdjust     = false;
    KC2S_MoveAction*        pMoveAction = (KC2S_MoveAction*)pData;
    KHero*                  pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL                    bReverseFailed = false;

    KG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    //LogInfo("recv move action from hero: %d, movecount:%d", pHero->m_dwID, pMoveAction->movecount);

 	pHero->m_MoveCount = pMoveAction->movecount;

    bAdjust = pHero->CheckMoveAdjust(nFrame, pMoveAction->moveparam, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);
	
	// TODO lijuntao
    pHero->ApplyMoveAction(pMoveAction->byMoveState, pMoveAction->direction);

    if (bAdjust)
    {
        DoAdjustHeroMove(pHero, KMOVE_FAILED);
    }
   
Exit0:
    return;
}

void KPlayerServer::OnHeroJump(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_HeroJump*     pMoveCommand    = (KC2S_HeroJump*)pData;
    KHero*             pHero           = GetFightingHeroByConnection(nConnIndex);
    BOOL               bRetCode        = false;
    BOOL               bReverseFailed  = false;

    KG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pHero->m_MoveCount = pMoveCommand->movecount;

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    bRetCode = pHero->Jump(pMoveCommand->bMove, pMoveCommand->direction8 * 32, false);
    if (!bRetCode)
    {
        DoAdjustHeroMove(pHero, KHERO_JUMP);
    }

Exit0:
    return;
}

void KPlayerServer::OnOperateAITeammate(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Operate_AI_Teammate* pCommand = (KC2S_Operate_AI_Teammate*)pData;
    KPlayer* pPlayer = GetPlayerByConnection(nConnIndex);
    KHero* pAITeammate = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pAITeammate = pPlayer->m_pAITeammate;
    KGLOG_PROCESS_ERROR(pAITeammate);

    KG_PROCESS_ERROR(pAITeammate->m_pScene);
    KG_PROCESS_ERROR(pAITeammate->m_pScene->m_eSceneState == ssFighting);

    pAITeammate->OwnerOperateAITeammate(pCommand->actionIdx);

Exit0:
    return;
}

void KPlayerServer::OnHeroSkill(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL               bRetCode    = false;
    KC2S_HeroSkill*    pCommand    = (KC2S_HeroSkill*)pData;
    KHero*             pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL               bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pHero->m_MoveCount = pCommand->movecount;

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);
    
    bRetCode = pHero->CastSkill(pCommand->skillID, pCommand->step, (KTWO_DIRECTION)pCommand->faceDir);
    if (!bRetCode)
        DoAdjustHeroMove(pHero, KCAST_SKILL_FAILED);
Exit0:
    return; 
}

void KPlayerServer::OnHeroSubSkill(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_HeroSubSkill*  pCommand = (KC2S_HeroSubSkill*)pData;
    KHero*              pHero    = GetFightingHeroByConnection(nConnIndex);
    BOOL                bRetCode = false;
    BOOL                bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero && pHero->m_pScene);

    pHero->m_MoveCount = pCommand->movecount;

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    bRetCode = pHero->CastSkill(pCommand->skillID, pCommand->step, pHero->m_eFaceDir, true);
    if (!bRetCode)
        DoAdjustHeroMove(pHero, KCAST_SUB_SKILL_FAILED);

Exit0:
    return;
}

void KPlayerServer::OnHeroGrabSkill(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_HeroGrabSkill*  pCommand = (KC2S_HeroGrabSkill*)pData;
    KHero*              pHero    = GetFightingHeroByConnection(nConnIndex);
    BOOL                bRetCode = false;
    BOOL                bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero && pHero->m_pScene);

    pHero->m_MoveCount = pCommand->movecount;

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    bRetCode = pHero->CastGrabSkill(pCommand->skillID, pCommand->step, pCommand->targetHeroID);
    if (!bRetCode)
        DoAdjustHeroMove(pHero, KGRAB_SKILL_FAILED);

Exit0:
    return;
}

void KPlayerServer::OnPlayerTakeBall(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bRetCode    = false;
    KC2S_TakeBall*  pCommand    = (KC2S_TakeBall*)pData;
    KBall*          pBall       = NULL;
    KHero*          pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL            bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    bRetCode = pHero->TakeBall(false);
    if (!bRetCode)
    {
        pBall = pHero->m_pScene->GetBall();
        KGLOG_PROCESS_ERROR(pBall);

        KGLogPrintf(KGLOG_INFO, "Take ball pos error, sync ball state!");
        g_PlayerServer.DoSyncSceneObject(pBall);
    }
    
Exit0:
    return;  
}

void KPlayerServer::OnPlayerShootBasket(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_ShootBasket*   pCommand        = (KC2S_ShootBasket*)pData;
    BOOL                bAdjust         = false;
    DWORD               dwBasketID      = pCommand->basketID;
    KHero*              pHero           = GetFightingHeroByConnection(nConnIndex);
    KPlayer*            pPlayer         = NULL;
    BOOL                bReverseFailed  = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    bAdjust = pHero->CheckMoveAdjust(nFrame, pCommand->moveParam, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    pHero->m_MoveCount = pCommand->movecount;
    if (!pHero->IsInAir())
    {
        pHero->Stand(false);
    }

    KGLOG_PROCESS_ERROR(pCommand->aimatLoop == (nFrame - pHero->m_nLastAimTime));
    pHero->m_nShootTime = pCommand->aimatLoop;
    pHero->ShootBasket(dwBasketID);

    if (bAdjust)
    {
        DoAdjustHeroMove(pHero, KSHOOT_BASKET);
    }

    pPlayer = pHero->GetOwner();
    assert(pPlayer);

    pPlayer->OnEvent(peInstallBasket);

Exit0:
    return;
}

void KPlayerServer::OnPlayerAimAt(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KC2S_Aimat*     pCommand    = (KC2S_Aimat*)pData;
    KHero*          pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL            bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    bRetCode = pHero->AimAt(pCommand->aimat, pCommand->direction, pCommand->holdingObjID, false);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnPlayerPickupObject( char* pData, size_t nSize, int nConnIndex, int nFrame )
{
    BOOL                bResult     = false;
    BOOL                bRetCode    = false;
    KC2S_PickupDoodad*  pCommand    = (KC2S_PickupDoodad*)pData;
    KHero*              pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL                bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    bRetCode = pHero->TakeObject(pCommand->objectID);
    if (!bRetCode)
    {
        KDoodad* pObject = pHero->m_pScene->GetDoodadById(pCommand->objectID);
        KGLOG_PROCESS_ERROR(pObject);

        g_PlayerServer.DoSyncSceneObject(pObject);
        goto Exit0;
    }

    bResult = true;
Exit0:
    if (pHero)
        KGLogPrintf(KGLOG_INFO, "On player:%d take object:%d, result: %d", pHero->m_dwID, pCommand->objectID, bResult);
    return;
}

void KPlayerServer::OnPlayerUseDoodad( char* pData, size_t nSize, int nConnIndex, int nFrame )
{
    BOOL            bRetCode    = false;
    KC2S_UseDoodad* pCommand    = (KC2S_UseDoodad*)pData;
    KHero*          pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL            bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    bRetCode = pHero->UseDoodad(pCommand->objectID, pCommand->direction);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnPlayerDropObject(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                bRetCode    = false;
    KC2S_DropDoodad*    pCommand    = (KC2S_DropDoodad*)pData;
    KHero*              pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL                bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    bRetCode = pHero->DropObject(pCommand->objectID);
    KG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnPlayerChangeRunMode(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	BOOL               bRetCode    = false;
	KC2S_RunMode*      pCommand    = (KC2S_RunMode*)pData;
    KHero*             pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL                bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

	pHero->SetRunMode(pCommand->isRunMode);
	g_PlayerServer.DoSyncRunMode(pHero);

Exit0:
    return;
}

void KPlayerServer::OnPlayerPassBall(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	BOOL               bRetCode    = false;
	KC2S_PassBall*     pCommand   = (KC2S_PassBall*)pData;
    KHero*             pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL                bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

	pHero->PassBallTo(pCommand->teammateID);
	
    KGLogPrintf(KGLOG_INFO, "RecvFromClient OnPlayerPassBall");

Exit0:
    return;
}

void KPlayerServer::OnPlayerShootObject(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                bRetCode    = false;
    KC2S_ShootDoodad*   pCommand    = (KC2S_ShootDoodad*)pData;
    KHero*              pHero       = GetFightingHeroByConnection(nConnIndex);
    BOOL                bReverseFailed = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pHero->AdjustVirtualFrame(nFrame, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    bRetCode = pHero->ShootDoodad(pCommand->objectID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}


void KPlayerServer::OnPlayerGetAwardRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bRetCode = false;
    KC2S_Award*     pCommand = (KC2S_Award*)pData;
    KPlayer*        pPlayer  = GetPlayerByConnection(nConnIndex);
    KAWARD_CARD_TYPE eType = (KAWARD_CARD_TYPE)pCommand->awardCardType;

    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);

    KGLOG_PROCESS_ERROR(eType >= 0 && eType < KAWARD_CARD_TYPE_TOTAL);

    pPlayer->Award(eType, pCommand->chooseIndex);

Exit0:
    return;
}

void KPlayerServer::OnPlayerBuyGoodsRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KC2S_Buy_Goods* pCommand    = (KC2S_Buy_Goods*)pData;
    KPlayer*        pPlayer     = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pCommand->dwShopID > 0);
    KGLOG_PROCESS_ERROR(pCommand->dwGoodsID > 0);
    KGLOG_PROCESS_ERROR(pCommand->wCount > 0);

    bRetCode = pPlayer->BuyGoods(pCommand->dwShopID, pCommand->dwGoodsID, pCommand->wCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    DoBuyGoodsRespond(pPlayer, pCommand->dwShopID, pCommand->dwGoodsID, bResult);
    return;
}

void KPlayerServer::OnPlayerSellGoodsRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                bResult     = false;
    BOOL                bRetCode    = false;
    KC2S_Sell_Goods*    pCommand    = (KC2S_Sell_Goods*)pData;
    KPlayer*            pPlayer     = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->SellGoods(
        pCommand->uPackageType, pCommand->uPackageIndex, 
        pCommand->uPos, pCommand->dwTabType, pCommand->dwTabIndex, pCommand->uGenTIme, pCommand->wCount
    );
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    DoSellGoodsRespond(
        pPlayer, pCommand->uPackageType, pCommand->uPackageIndex, pCommand->uPos, 
        pCommand->dwTabType, pCommand->dwTabIndex, pCommand->uGenTIme, bResult
    );
    return;
}

void KPlayerServer::OnPlayerRepairSingleEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KC2S_Repair_Single_Equip*   pCommand    = (KC2S_Repair_Single_Equip*)pData;
    KPlayer*                    pPlayer     = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = g_pSO3World->m_ShopMgr.OnPlayerRepairSingleEquip(
        pPlayer, pCommand->uPackageType, pCommand->uPackageIndex, pCommand->uPos, 
        pCommand->dwTabType, pCommand->dwTabIndex, pCommand->uGenTIme
    );
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    DoRepairSingleEquipRespond(pPlayer, bResult);
    return;
}

void KPlayerServer::OnPlayerRepairAllEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KC2S_Repair_All_Equip*      pCommand    = (KC2S_Repair_All_Equip*)pData;
    KPlayer*                    pPlayer     = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = g_pSO3World->m_ShopMgr.OnPlayerRepairAllEquip(pPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    DoRepairAllEquipRespond(pPlayer, bResult);
    return;
}

void KPlayerServer::OnPlayerPresentGoodsRequeset(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KC2S_Present_Goods_Request* pCommand    = (KC2S_Present_Goods_Request*)pData;
    KPlayer*                    pPlayer     = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);
	pCommand->szDstName[countof(pCommand->szDstName) - 1] = '\0';

    KGLOG_PROCESS_ERROR(pPlayer->m_pbyPresentGoodInfo == NULL);

    pPlayer->m_pbyPresentGoodInfo = (BYTE*)KMEMORY_ALLOC(nSize);
    assert(pPlayer->m_pbyPresentGoodInfo);

    memcpy(pPlayer->m_pbyPresentGoodInfo, pData, nSize);

    g_LSClient.DoRemoteCall("OnCheckCanPresentRequest", (int)pPlayer->m_dwID, pCommand->szDstName);

    PLAYER_LOG(pPlayer, "item,present_goods,%d,%d-%d-%d", pCommand->szDstName, pCommand->nShopID, pCommand->nGoodsID, pCommand->nCount);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnPlayerChangeTeamLogoRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    KPlayer*                        pPlayer         = GetPlayerByConnection(nConnIndex);
    KC2S_Change_TeamLogo_Request*   pCommand        = (KC2S_Change_TeamLogo_Request*)pData;
    std::vector<WORD>::iterator     RetIter;
    KTEAM_LOGO_INFO*                pTeamLogoInfo   = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->SetTeamLogo(pCommand->wTeamLogoFrameID, pCommand->wTeamLogoEmblemID));

    bResult = true;
Exit0:
    return;
}


void KPlayerServer::OnPlayerClearTeamLogoNewFlag( char* pData, size_t nSize, int nConnIndex, int nFrame )
{
	BOOL                            bResult         = false;
	BOOL                            bRetCode        = false;
	KPlayer*                        pPlayer         = NULL;
	
	pPlayer = GetPlayerByConnection(nConnIndex);
	KGLOG_PROCESS_ERROR(pPlayer);
	
	for (vector<WORD>::iterator it = pPlayer->m_vecOwnTeamLogo.begin(); it != pPlayer->m_vecOwnTeamLogo.end(); ++it)
	{
		bResult = true;
		*it &= 0x7fff;
	}
	
Exit0:
	return;
}



void KPlayerServer::OnApplyCreateRoom(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KApply_Create_Room* pApply      = (KApply_Create_Room*)pData;
    KPlayer*            pPlayer     = GetPlayerByConnection(nConnIndex);

    pApply->szRoomName[countof(pApply->szRoomName) - 1] = '\0';
    if (!pApply->hasPassword)
        pApply->szPassWord[0] = '\0';

    g_pSO3World->m_piTextFilter->Replace(pApply->szRoomName);

    g_RelayClient.DoApplyCreateRoom(pPlayer, pApply->dwMapID, pApply->szRoomName, pApply->szPassWord);

Exit0:
    return;
}

void KPlayerServer::OnApplyJoinRoom(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KApply_Join_Room*   pApply      = (KApply_Join_Room*)pData;
    KPlayer*            pPlayer     = GetPlayerByConnection(nConnIndex);
    KRoomBaseInfo*      pRoomBaseInfo = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pRoomBaseInfo = g_pSO3World->m_RoomCache.GetRoomBaseInfo(pApply->dwRoomID);
    KG_PROCESS_ERROR(pRoomBaseInfo);

    if (pRoomBaseInfo->bFighting)
    {
        DoDownwardNotify(pPlayer, KMESSAGE_ROOM_IS_FIGHTING);
        goto Exit0;
    }

    g_RelayClient.DoApplyJoinRoom(pPlayer, pApply->dwRoomID, pApply->szPassWord);

Exit0:
    return;
}

void KPlayerServer::OnInvitePlayerJoinRoom(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KInvite_Player_Join_Room*   pApply      = (KInvite_Player_Join_Room*)pData;
    KPlayer*                    pPlayer     = GetPlayerByConnection(nConnIndex);

    pApply->szDestPlayerName[countof(pApply->szDestPlayerName) - 1] = '\0';

    g_RelayClient.DoInvitePlayerJoinRoom(pPlayer->m_dwID, pApply->szDestPlayerName);
}

void KPlayerServer::OnSwitchRoomHost(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KApply_Switch_Room_Host*    pApply      = (KApply_Switch_Room_Host*)pData;
    KPlayer*                    pPlayer     = GetPlayerByConnection(nConnIndex);

    g_RelayClient.DoSwitchRoomHost(pPlayer->m_dwID, pApply->dwNewHostID);
}

void KPlayerServer::OnSwtichBattleMap(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KApply_Switch_Battle_Map*   pApply      = (KApply_Switch_Battle_Map*)pData;
    KPlayer*                    pPlayer     = GetPlayerByConnection(nConnIndex);

    g_RelayClient.DoSwitchBattleMap(pPlayer->m_dwID, pApply->dwNewMapID);
}

void KPlayerServer::OnSetRoomPassword(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KApply_Set_Room_Password*   pApply      = (KApply_Set_Room_Password*)pData;
    KPlayer*                    pPlayer     = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(sizeof(pApply->szNewPassword) == cdRoomPasswordMaxLen);

    g_RelayClient.DoSetRoomPassword(pPlayer->m_dwID, pApply->szNewPassword);

Exit0:
    return;
}

void KPlayerServer::OnApplyLeaveRoom(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KApply_Leave_Room*   pApply     = (KApply_Leave_Room*)pData;
    KPlayer*            pPlayer     = GetPlayerByConnection(nConnIndex);

    g_RelayClient.DoApplyLeaveRoom(pPlayer->m_dwID);
}

void KPlayerServer::OnApplyKickoutOther(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KApply_Kickout_Other*   pApply      = (KApply_Kickout_Other*)pData;
    KPlayer*                pPlayer     = GetPlayerByConnection(nConnIndex);

    g_RelayClient.DoApplyKickoutOther(pPlayer->m_dwID, pApply->dwOtherPlayerID);
}

void KPlayerServer::OnApplyRoomInfo(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KApply_Room_Info* pApply = (KApply_Room_Info*)pData;
    g_pSO3World->m_RoomCache.DoSyncRoomInfo(nConnIndex, pApply->wPageIndex);
}

void KPlayerServer::OnApplySetReady(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KApply_Set_Ready*   pApply      = (KApply_Set_Ready*)pData;
    KPlayer*            pPlayer     = GetPlayerByConnection(nConnIndex);

    g_RelayClient.DoApplySetReady(pPlayer->m_dwID, pApply->bReady);
}

void KPlayerServer::OnApplyStartGame(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KApply_Start_Game*   pApply     = (KApply_Start_Game*)pData;
    KPlayer*             pPlayer    = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);

    g_RelayClient.DoApplyStartGame(pPlayer->m_dwID);

Exit0:
    return;
}

void KPlayerServer::OnRoomMemberChangePosRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KRoomMember_Change_Pos_Request*   pApply      = (KRoomMember_Change_Pos_Request*)pData;
    KPlayer*                    pPlayer     = GetPlayerByConnection(nConnIndex);
    KSIDE_TYPE                  eNewSide    = (KSIDE_TYPE)pApply->byNewSide;

    KG_PROCESS_ERROR(eNewSide >= sidBegin && eNewSide < sidTotal);

    KGLOG_PROCESS_ERROR(pApply->nNewPos >= 0 && pApply->nNewPos < MAX_TEAM_MEMBER_COUNT);

    g_RelayClient.DoRoomMemberChangePosRequest(pPlayer->m_dwID, eNewSide, pApply->nNewPos);

Exit0:
    return;
}

void KPlayerServer::OnChangeRoomName(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KChange_RoomName*   pApply      = (KChange_RoomName*)pData;
    KPlayer*            pPlayer     = GetPlayerByConnection(nConnIndex);

    pApply->newRoomName[countof(pApply->newRoomName) - 1] = '\0';

    g_pSO3World->m_piTextFilter->Replace(pApply->newRoomName);

    g_RelayClient.DoChangeRoomName(pPlayer->m_dwID, pApply->newRoomName);

Exit0:
    return;
}

void KPlayerServer::OnAutoJoinRoom(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_AutoJoinRoom*  pRequest = (KC2S_AutoJoinRoom*)pData;
    KPlayer*            pPlayer = GetPlayerByConnection(nConnIndex);

    g_RelayClient.DoAutoJoinRoom(pPlayer);
}

void KPlayerServer::OnApplySetRoomAIMode(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Apply_Set_Room_Aimode* pRequest = (KC2S_Apply_Set_Room_Aimode*)pData;
    KPlayer* pPlayer = NULL;
        
    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoApplySetRoomAIMode(pPlayer->m_dwID, (KSIDE_TYPE)pRequest->bySide, pRequest->byPos, pRequest->bIsAIMode);

Exit0:
    return;
}

void KPlayerServer::OnTryJoinRoom(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL bRetCode = false;
    KC2S_TryJoinRoom* pRequest = (KC2S_TryJoinRoom*)pData;
    KPlayer* pPlayer = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->TryJoinRoom(pRequest->dwRoomID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnAcceptOrRefuseJoinRoom(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL bRetCode = false;
    KC2S_Accept_Or_Refuse_Join_Room* pRequest = (KC2S_Accept_Or_Refuse_Join_Room*)pData;
    KPlayer* pPlayer = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->AcceptOrRefuseJoinRoom(pRequest->acceptCode, pRequest->dwRoomID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnCreateTeamRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL bRetCode = false;
    KC2S_Create_Team_Request* pRequest = (KC2S_Create_Team_Request*)pData;
    KPlayer*            pPlayer = NULL;
    
    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->CreateTeam();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnTeamInvitePlayerRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL bRetCode = false;
    KC2S_Team_Invite_Player_Request* pRequest = (KC2S_Team_Invite_Player_Request*)pData;
    KPlayer* pPlayer = GetPlayerByConnection(nConnIndex);

    pRequest->szTargetPlayer[countof(pRequest->szTargetPlayer) - 1] = '\0';

    bRetCode = pPlayer->TeamInvitePlayer(pRequest->szTargetPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnTeamKickoutPlayerRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode = false;
    KC2S_Team_Kickout_Player_Request*   pRequest = (KC2S_Team_Kickout_Player_Request*)pData;
    KPlayer*                            pPlayer  = NULL;
    
    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->TeamKickoutPlayer(pRequest->dwPlayerID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnTeamLeaveRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bRetCode    = false;
    KC2S_Team_Leave_Request*    pRequest    = (KC2S_Team_Leave_Request*)pData;
    KPlayer*                    pPlayer     = NULL;
    
    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->LeaveTeam();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnTeamAcceptOrRefuseInvite(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                               bRetCode = false;
    KC2S_Team_Accept_Or_Refuse_Invite* pRequest = (KC2S_Team_Accept_Or_Refuse_Invite*)pData;
    KPlayer*                           pPlayer  = NULL;
    
    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pRequest->szInviterName[countof(pRequest->szInviterName) - 1] = '\0';

    bRetCode = pPlayer->TeamAcceptOrRefuseInvite(pRequest->accpetCode, pRequest->szInviterName);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnTeamReadyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                     bRetCode = false;
    KC2S_Team_Ready_Request* pRequest = (KC2S_Team_Ready_Request*)pData;
    KPlayer*                 pPlayer  = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = g_RelayClient.DoTeamReadyRequest(pPlayer->m_dwID, pRequest->bReady);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnAutoMatchRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_AutoMatch_Request* pRequest    = (KC2S_AutoMatch_Request*)pData;
    KPlayer*                pPlayer     = GetPlayerByConnection(nConnIndex);

    g_RelayClient.DoAutoMatchRequest(pPlayer);
}

void KPlayerServer::OnCancelAutoMatchRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Cancel_AutoMatch_Request* pRequest = (KC2S_Cancel_AutoMatch_Request*)pData;
    KPlayer*                   pPlayer  = GetPlayerByConnection(nConnIndex);

    g_RelayClient.DoCancelAutoMatchRequest(pPlayer);
}

void KPlayerServer::OnPlayerListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayerListRequest* pRequest = (KPlayerListRequest*)pData;
    KPlayer* aPlayers[cdMaxPlayerListLen] = {NULL};
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pRequest->pageIndex > 0);
    g_pSO3World->FindPlayers(pRequest->pageIndex, aPlayers, cdMaxPlayerListLen);

    bRetCode = DoSyncPlayerListStart(nConnIndex);
    KGLOG_PROCESS_ERROR(bRetCode);

    for (int i = 0; i < cdMaxPlayerListLen; ++i)
    {
        KPlayer* pPlayer = aPlayers[i];
        if (!pPlayer)
            break;

        bRetCode = DoSyncPlayerInfo(nConnIndex, pPlayer);
        KGLOG_PROCESS_ERROR(bRetCode);
    }  

    bRetCode = DoSyncPlayerListEnd(nConnIndex, pRequest->pageIndex);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnTalkMessage(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bRetCode        = false;
    KTalk_Message*          pTalkMsg        = (KTalk_Message*)pData;
    size_t                  uTalkDataLen    = 0;
    KPlayer*                pPlayer         = GetPlayerByConnection(nConnIndex);
    DWORD                   dwTeamID        = ERROR_ID;
    int                     nCoolDownID     = 0;
    KHero*                  pHero           = NULL;

    KGLOG_PROCESS_ERROR(nSize > sizeof(KTalk_Message));
    KGLOG_PROCESS_ERROR(pPlayer);

    pTalkMsg->szReceiver[sizeof(pTalkMsg->szReceiver) - 1] = '\0';

    uTalkDataLen = nSize - sizeof(KTalk_Message);
    KGLOG_PROCESS_ERROR(uTalkDataLen <= cdDefaultTalkDataLength);

    KGLOG_PROCESS_ERROR(nSize == sizeof(KTalk_Message) + pTalkMsg->talkdataLength);

    pTalkMsg->talkdata[pTalkMsg->talkdataLength - 1] = '\0';

    // 检查Cool Down
    switch (pTalkMsg->byMsgType)
    {
    case trRoom:
        nCoolDownID = g_pSO3World->m_Settings.m_ConstList.nRoomChannelCoolDownID;
        break;

    case trWorld:
        nCoolDownID = g_pSO3World->m_Settings.m_ConstList.nWorldChannelCoolDownID;
        break;

    default:
        break;
    }

    if (nCoolDownID)
    {
        int nDuration = g_pSO3World->m_Settings.m_CoolDownList.GetCoolDownValue(nCoolDownID);
        KGLOG_PROCESS_ERROR(nDuration > 0);

        if (!pPlayer->m_CDTimerList.CheckTimer(nCoolDownID))
        {
            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_TALK_CD);
            goto Exit0;
        }

        nDuration = MAX(1, nDuration);

        pPlayer->m_CDTimerList.ResetTimer(nCoolDownID, nDuration);
    }

    // 检查聊天次数限制
    bRetCode = pPlayer->CheckTalkDailyCount(pTalkMsg->byMsgType);
    KG_PROCESS_ERROR(bRetCode);

    // 敏感词过滤
    bRetCode = g_pSO3World->m_piTextFilter->Replace((char*)pTalkMsg->talkdata);
    KGLOG_PROCESS_ERROR(bRetCode);

    // 聊天消息转发
    switch (pTalkMsg->byMsgType)
    {
    case trWhisper:
        if( g_pSO3World->m_nCurrentTime > pPlayer->m_nForbidTalkTime )
        {
            g_LSClient.DoTalkMessage(
                pTalkMsg->byMsgType, pPlayer->m_dwID, pPlayer->m_szName, 
                ERROR_ID, pTalkMsg->szReceiver, uTalkDataLen, pTalkMsg->talkdata
            );
        }
        else
        {
            // 通知客户端，禁言状态不能聊天
            // ...
        }
        break;

    case trFace:
        break;
    case trClub:
        if (pPlayer->m_dwClubID)
        {
            g_LSClient.DoTalkMessage(
                pTalkMsg->byMsgType, pPlayer->m_dwID, pPlayer->m_szName, 
                ERROR_ID, pTalkMsg->szReceiver, uTalkDataLen, pTalkMsg->talkdata
            );
        }
        break;
    case trRoom:
    case trTeam:
        if( g_pSO3World->m_nCurrentTime > pPlayer->m_nForbidTalkTime )
        {
            pHero = pPlayer->GetFightingHero();
            if (pHero) // 在场景中
            {
                KMAP_SCENE_PLAYERS mapPlayers;
                assert(pHero->m_pScene);
                pHero->m_pScene->GetGrouppedHeroWithOwner(mapPlayers);

                for (KMAP_SCENE_PLAYERS::iterator it = mapPlayers.begin(); it != mapPlayers.end(); ++it)
                {
                    KPlayer* pTarget = it->first;
                    KHero* pTargetHero = pTarget->GetFightingHero();
                    assert(pTarget);
                    assert(pTargetHero);

                    if (pTalkMsg->byMsgType == trTeam && pTargetHero->m_nSide != pHero->m_nSide)
                        continue;

                    DoTalkMessage(
                        pTalkMsg->byMsgType, pPlayer->m_dwID, pPlayer->m_szName, 0, ERROR_ID, 
                        pTarget->m_dwID, pTarget->m_szName, uTalkDataLen, pTalkMsg->talkdata
                        );
                }

                goto Exit0;
            }

            g_LSClient.DoTalkMessage(
                pTalkMsg->byMsgType, pPlayer->m_dwID, pPlayer->m_szName,
                ERROR_ID, pTalkMsg->szReceiver, uTalkDataLen, pTalkMsg->talkdata
                );
        }
        else
        {
            // 通知客户端，禁言状态不能聊天
            // ...
        }
        break;

    case trPhysicalServer:
        if( g_pSO3World->m_nCurrentTime > pPlayer->m_nForbidTalkTime )
        {
            DoTalkMessage(pTalkMsg->byMsgType, pPlayer->m_dwID, pPlayer->m_szName, 0,  ERROR_ID, ERROR_ID, NULL, uTalkDataLen, pTalkMsg->talkdata);
        }
        else
        {
            // 通知客户端，禁言状态不能聊天
            // ...
        }
        break;

    case trSmallHorn:
        bRetCode = pPlayer->CostGroupTalkItem();
        KGLOG_PROCESS_ERROR(bRetCode);

        g_LSClient.DoTalkMessage(
            pTalkMsg->byMsgType, pPlayer->m_dwID, pPlayer->m_szName, 
            ERROR_ID, pTalkMsg->szReceiver, uTalkDataLen, pTalkMsg->talkdata
       	);

        break;
    case trBigHorn:  
        bRetCode = pPlayer->CostGlobalTalkItem();
        KGLOG_PROCESS_ERROR(bRetCode);

        g_LSClient.DoTalkMessage(
            pTalkMsg->byMsgType, pPlayer->m_dwID, pPlayer->m_szName, 
            ERROR_ID, pTalkMsg->szReceiver, uTalkDataLen, pTalkMsg->talkdata
        );
        break;

    case trWorld:
        if( g_pSO3World->m_nCurrentTime > pPlayer->m_nForbidTalkTime )
        {
            g_LSClient.DoTalkMessage(
                pTalkMsg->byMsgType, pPlayer->m_dwID, pPlayer->m_szName, 
                ERROR_ID, pTalkMsg->szReceiver, uTalkDataLen, pTalkMsg->talkdata
                );
        }
        else
        {
            // 通知客户端，禁言状态不能聊天
            // ...
        }
        break;

    default:
        break;
    }

Exit0:
    return;
}

void KPlayerServer::OnClientLog(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KClientLog* pPak        = (KClientLog*)pData;
    KPlayer*    pPlayer     = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(sizeof(KClientLog) + pPak->logdataLength == nSize);
    
    pPak->logdata[pPak->logdataLength] = '\0';

    //KGLogPrintf(KGLOG_INFO, "[Client log of player:%s, frame:%d] %s", pPlayer->m_szName, g_pSO3World->m_nGameLoop, pPak->logdata);

Exit0:
    return;
}

void KPlayerServer::OnGMCommand(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bRetCode    = false;
    KC2S_GMCommand*  pPak        = (KC2S_GMCommand*)pData;
    KPlayer*        pPlayer     = NULL;

    KGLOG_PROCESS_ERROR(sizeof(KC2S_GMCommand) + pPak->commandLength == nSize);
    pPak->command[pPak->commandLength] = '\0';

    pPlayer = GetPlayerByConnection(nConnIndex);
    KG_PROCESS_ERROR(pPlayer);

    if (!m_nClientGM)
    {
        bRetCode = g_pSO3World->m_Settings.m_GMInfoList.CheckIsPermission(pPlayer->m_szAccount);
        KG_PROCESS_ERROR(bRetCode);
    }

    bRetCode = g_pSO3World->m_ScriptManager.ExecutePlayerScript(pPlayer, pPak->command);
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLogPrintf(KGLOG_INFO, "GameManager: %s  Call Command: %s", pPlayer->m_szName, pPak->command);

Exit0:
    return;
}

struct _GetFellowshipDataFunc 
{
    BOOL operator()(DWORD dwPlayerID, DWORD dwAlliedPlayerID)
    {
        g_PlayerServer.DoSyncOneFellowship(dwPlayerID, dwAlliedPlayerID);
        return true;
    }
};

struct _GetBlackListDataFunc 
{
    BOOL operator()(DWORD dwPlayerID, DWORD dwAlliedPlayerID)
    {
        g_PlayerServer.DoSyncOneBlackList(dwPlayerID, dwAlliedPlayerID);
        return true;
    }
};


void KPlayerServer::OnGetFellowshipListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode                    = false;
    KPlayer*                            pPlayer                     = GetPlayerByConnection(nConnIndex);
    KGet_Fellowship_List_Request*       pGetFellowshipListRequest   = (KGet_Fellowship_List_Request*)pData;
    _GetFellowshipDataFunc              GetFellowshipDataFunc;
    _GetBlackListDataFunc               GetBlackListDataFunc;
    KFELLOWSHIP_REQUEST                 eRequestType = (KFELLOWSHIP_REQUEST)pGetFellowshipListRequest->eType;

    assert(pPlayer);

    switch (eRequestType)
    {
    case KFELLOWSHIP_REQUEST_FRIEND:
        DoSyncFellowshipGroupName(pPlayer->m_dwID);
        g_pSO3World->m_FellowshipMgr.TraverseFellowshipID(pPlayer->m_dwID, GetFellowshipDataFunc);
        DoSyncFellowshipList(pPlayer->m_dwID, 0, NULL);  // Indicate the end of sync. 
        break;

    case KFELLOWSHIP_REQUEST_BLACKLIST:
        g_pSO3World->m_FellowshipMgr.TraverseBlackListID(pPlayer->m_dwID, GetBlackListDataFunc);
        DoSyncBlackList(pPlayer->m_dwID, 0, NULL);
        break;

    default:
        break;
    }

Exit0:
    return;
}

void KPlayerServer::OnAddFellowshipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bRetCode        = false;
    KPlayer*                    pPlayer         = GetPlayerByConnection(nConnIndex);
    KAdd_Fellowship_Request* pAddFellowship  = (KAdd_Fellowship_Request*)pData;

    assert(pPlayer);

    pAddFellowship->szAlliedPlayerName[sizeof(pAddFellowship->szAlliedPlayerName) - 1] = '\0';

    KGLOG_PROCESS_ERROR(pAddFellowship->type == KFELLOWSHIP_REQUEST_FRIEND ||pAddFellowship->type == KFELLOWSHIP_REQUEST_BLACKLIST);

    g_LSClient.DoPlayerAddFellowshipRequest(
        pPlayer->m_dwID, pAddFellowship->szAlliedPlayerName, pAddFellowship->type
    );

Exit0:
    return;
}

void KPlayerServer::OnDelFellowshipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bRetCode            = false;
    DWORD                       dwAlliedPlayerID    = ERROR_ID;
    KPlayer*                    pPlayer             = GetPlayerByConnection(nConnIndex);
    KGFellowship*               pFellowship         = NULL;
    KDel_Fellowship_Request*    pDelFellowship      = (KDel_Fellowship_Request*)pData;

    assert(pPlayer);

    dwAlliedPlayerID = pDelFellowship->uAlliedPlayerID;
    KGLOG_PROCESS_ERROR(dwAlliedPlayerID != ERROR_ID);

    switch (pDelFellowship->type)
    {
    case KFELLOWSHIP_REQUEST_FRIEND:
        pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(pPlayer->m_dwID, dwAlliedPlayerID);
        KGLOG_PROCESS_ERROR(pFellowship);

        g_pSO3World->m_FellowshipMgr.DelFellowship(pPlayer->m_dwID, dwAlliedPlayerID);
        break;

    case KFELLOWSHIP_REQUEST_BLACKLIST:
        g_pSO3World->m_FellowshipMgr.DelBlackList(pPlayer->m_dwID, dwAlliedPlayerID);
        break;

    default:
        goto Exit0;
    }

    DoDelFellowshipRespond(nConnIndex, dwAlliedPlayerID, pDelFellowship->type, true);

Exit0:
    return;
}

void KPlayerServer::OnAddFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                    bRetCode    = false;
    KPlayer*                                pPlayer     = GetPlayerByConnection(nConnIndex);
    KAdd_Fellowship_Group_Request*       pAddGroup   = (KAdd_Fellowship_Group_Request*)pData;

    assert(pPlayer);

    pAddGroup->szGroupName[sizeof(pAddGroup->szGroupName) - 1] = '\0';

    g_pSO3World->m_FellowshipMgr.AddFellowshipGroup(pPlayer->m_dwID, pAddGroup->szGroupName);

Exit0:
    return;
}

void KPlayerServer::OnDelFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode    = false;
    DWORD                               dwGroupID   = 0;
    KPlayer*                            pPlayer     = GetPlayerByConnection(nConnIndex);
    KDel_Fellowship_Group_Request*   pDelGroup   = (KDel_Fellowship_Group_Request*)pData;

    assert(pPlayer);

    dwGroupID = pDelGroup->uGroupID;
    KGLOG_PROCESS_ERROR(dwGroupID < KG_FELLOWSHIP_MAX_CUSTEM_GROUP);

    bRetCode = g_pSO3World->m_FellowshipMgr.DelFellowshipGroup(pPlayer->m_dwID, dwGroupID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnRenameFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                    bRetCode    = false;
    DWORD                                   dwGroupID   = 0;
    KPlayer*                                pPlayer     = GetPlayerByConnection(nConnIndex);
    KRename_Fellowship_Group_Request*    pRename     = (KRename_Fellowship_Group_Request*)pData;

    assert(pPlayer);

    dwGroupID = pRename->uGroupID;
    KGLOG_PROCESS_ERROR(dwGroupID < KG_FELLOWSHIP_MAX_CUSTEM_GROUP);

    pRename->szNewName[sizeof(pRename->szNewName) - 1] = '\0';

    g_pSO3World->m_FellowshipMgr.RenameFellowshipGroup(pPlayer->m_dwID, dwGroupID, pRename->szNewName);

Exit0:
    return;
}

void KPlayerServer::OnSetFellowshipRemarkRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bRetCode            = false;
    DWORD                       dwAlliedPlayerID    = ERROR_ID;
    KPlayer*                    pPlayer             = GetPlayerByConnection(nConnIndex);
    KSet_Fellowship_Remark*  pSetRemark          = (KSet_Fellowship_Remark*)pData;
    KGFellowship*               pFellowship         = NULL;

    assert(pPlayer);

    pSetRemark->szRemark[sizeof(pSetRemark->szRemark) - 1] = '\0';

    dwAlliedPlayerID = pSetRemark->uAlliedPlayerID;
    KGLOG_PROCESS_ERROR(dwAlliedPlayerID != ERROR_ID);

    pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(pPlayer->m_dwID, dwAlliedPlayerID);
    KG_PROCESS_ERROR(pFellowship);

    strncpy(pFellowship->m_szRemark, pSetRemark->szRemark, sizeof(pSetRemark->szRemark));
    pFellowship->m_szRemark[sizeof(pSetRemark->szRemark) - sizeof('\0')] = '\0';

Exit0:
    return;
}

void KPlayerServer::OnSetFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bRetCode            = false;
    DWORD                       dwAlliedPlayerID    = 0;
    DWORD                       dwGroupID           = 0;
    KPlayer*                    pPlayer             = GetPlayerByConnection(nConnIndex);
    KSet_Fellowship_Group*      pSetGroup           = (KSet_Fellowship_Group*)pData;
    KGFellowship*               pFellowship         = NULL;
    KG_FELLOWSHIP_GROUPNAMES_MAP::iterator  it;

    assert(pPlayer);

    dwAlliedPlayerID = pSetGroup->uAlliedPlayerID;
    KGLOG_PROCESS_ERROR(dwAlliedPlayerID != ERROR_ID);

    dwGroupID = pSetGroup->uGroupID;
    KGLOG_PROCESS_ERROR(dwGroupID < KG_FELLOWSHIP_MAX_CUSTEM_GROUP);

    it = g_pSO3World->m_FellowshipMgr.m_GroupNamesMap.find(pPlayer->m_dwID);
    KG_PROCESS_ERROR(it != g_pSO3World->m_FellowshipMgr.m_GroupNamesMap.end());

    KGLOG_PROCESS_ERROR(dwGroupID <= (DWORD)it->second.nGroupCount);

    pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(pPlayer->m_dwID, dwAlliedPlayerID);
    KG_PROCESS_ERROR(pFellowship);

    pFellowship->m_dwGroupID = dwGroupID;

Exit0:
    return;
}

void KPlayerServer::OnSyncUserPreferences(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                            bRetCode            = false;
    KPlayer*                        pPlayer             = NULL;
    KC2S_Sync_User_Preferences*     pUserPreferences    = (KC2S_Sync_User_Preferences*)pData;

    KGLOG_PROCESS_ERROR(nSize == sizeof(KC2S_Sync_User_Preferences) + pUserPreferences->userdataLength);

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->m_UserPreferences.SetUserPreferences(
        pUserPreferences->nOffset, (int)pUserPreferences->userdataLength, (BYTE*)pUserPreferences->userdata
    );
    KG_PROCESS_ERROR(bRetCode);

Exit0:
    return ;
}

void KPlayerServer::OnRenameRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                            bRetCode        = false;
    KPlayer*                        pPlayer         = NULL;
    KC2S_Rename_Request*            pRequest        = (KC2S_Rename_Request*)pData;
    KGENDER                         eGender         = gNone;

    pRequest->szNewName[sizeof(pRequest->szNewName) - 1] = '\0';

    eGender = (KGENDER)pRequest->byGender;
    KGLOG_PROCESS_ERROR(eGender == gMale || eGender == gFemale);

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    KGLOG_PROCESS_ERROR(pPlayer->m_bCanRename);
    KGLOG_PROCESS_ERROR(!pPlayer->m_bIsRenaming);
    
    if (pPlayer->m_eGender != eGender)
    {
        pPlayer->m_eGender = eGender;
        g_RelayClient.SaveRoleData(pPlayer);
        g_RelayClient.DoUpdatePlayerGender(pPlayer);
        g_LSClient.DoUpdatePlayerGender(pPlayer->m_dwID, pPlayer->m_eGender);
    }
    pPlayer->m_bIsRenaming = true;
    g_RelayClient.DoRenameRequest(pPlayer->m_dwID, pRequest->szNewName);

Exit0:
    return;
}

void KPlayerServer::OnGetTrainingHeroListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bRetCode            = false;
    KPlayer*                    pPlayer             = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);
    
    bRetCode = DoSyncTrainingHeroList(pPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnBeginTrainingHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode    = false;
    KC2S_Begin_Training_Hero_Request*   pRequest    = (KC2S_Begin_Training_Hero_Request*)pData;
    KPlayer*                            pPlayer     = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!pPlayer->m_bIsMatching);

    bRetCode = pPlayer->m_HeroTrainingMgr.BeginTrainingHero(pRequest->heroTemplateID, pRequest->trainingID);
    KGLOG_PROCESS_ERROR(bRetCode);

    PLAYER_LOG(pPlayer, "training,begintraining,%d,%d", pRequest->trainingID, pRequest->heroTemplateID);

Exit0:
    return;
}

void KPlayerServer::OnEndTrainingHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
}

void KPlayerServer::OnBuyTrainingSeatRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
}

void KPlayerServer::OnUpgradeTeacherLevel(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                            bRetCode    = false;
    KC2S_Upgrade_Teacher_Level*     pRequest    = (KC2S_Upgrade_Teacher_Level*)pData;
    KPlayer*                        pPlayer     = GetPlayerByConnection(nConnIndex);

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!pPlayer->m_bIsMatching);

    bRetCode = pPlayer->m_HeroTrainingMgr.UpgradeTeacherLevel();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnSendMailRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bRetCode        = false;
    KPlayer*                pPlayer         = NULL;
    KC2S_Send_Mail_Request*  pRequest        = (KC2S_Send_Mail_Request*)pData;
    int                     nRequestID      = pRequest->byRequestID;
    BOOL                    bItem           = false;
    IItem*                  piItem          = NULL;
    const KItemProperty*	pItemProperty	= NULL;
    int                     nDelayTime      = 0;
    size_t                  uItemDataLen    = 0;
    unsigned int            uMailLen        = 0;
    int                     nPlayerMoney    = 0;
    KMail*                  pMail           = NULL;
    BYTE*                   pbyOffset       = NULL;
    BYTE*                   pbyBuffer       = NULL;
    char*                   pszCheckTxt        = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pRequest->szDstName[sizeof(pRequest->szDstName) - 1] = '\0';
    pRequest->szTitle[sizeof(pRequest->szTitle) - 1] = '\0';

    KGLOG_PROCESS_ERROR(nSize == sizeof(KC2S_Send_Mail_Request) + pRequest->textLength);
    KGLOG_PROCESS_ERROR(pRequest->textLength <= g_pSO3World->m_Settings.m_ConstList.nMailTextSize);
    
    bRetCode = g_pSO3World->m_piTextFilter->Check((char*)pRequest->szTitle);
    if (!bRetCode)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_TEXT_FORBIT);
        goto Exit0;
    }

    pszCheckTxt = (char*)m_byTempData;
    memcpy(pszCheckTxt, pRequest->text, pRequest->textLength);
    pszCheckTxt[pRequest->textLength] = '\0';

    bRetCode = g_pSO3World->m_piTextFilter->Check(pszCheckTxt);
    if (!bRetCode)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_TEXT_FORBIT);
        goto Exit0;
    }

    if (STR_CASE_CMP(pPlayer->m_szName, pRequest->szDstName) == 0)  // 不能给自己发邮件
    {
        DoSendMailRespond(nConnIndex, nRequestID, mrcDstNotSelf);
        goto Exit0;
    }
    //手续费
    if (!pPlayer->m_MoneyMgr.CanAddMoney(emotMoney,-g_pSO3World->m_Settings.m_ConstList.nMailCostMoney))
    {
        DoSendMailRespond(nConnIndex, nRequestID, mrcNotEnoughMoney);
        goto Exit0;
    }

    pMail       = (KMail*)m_byTempData;
    bRetCode    = g_pSO3World->FillMail(
        eMailType_Player, pPlayer->m_szName,  pRequest->szTitle,
        (const char*)pRequest->text, (int)pRequest->textLength, NULL, 0, pMail, uMailLen
    );

    KGLOG_PROCESS_ERROR(bRetCode);

    pMail->dwSenderID = pPlayer->m_dwID; 
    pMail->dwMailID = pRequest->byRequestID;

    g_LSClient.DoSendMailRequest(nRequestID, pRequest->szDstName, pMail, uMailLen);

    PLAYER_LOG(pPlayer, "friends,mail,%s", pRequest->szDstName);

Exit0:
    return;
}
void KPlayerServer::OnGetMaillistRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bRetCode    = false;
    KPlayer*                    pPlayer     = NULL;
    KC2S_Get_Maillist_Request*   pRequest    = (KC2S_Get_Maillist_Request*)pData;
    
    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoGetMailListRequest(pPlayer->m_dwID, pRequest->dwStartID);

Exit0:
    return;
}

void KPlayerServer::OnQueryMailContent(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bRetCode    = false;
    KPlayer*                pPlayer     = NULL;
    KC2S_Query_Mail_Content* pPak        = (KC2S_Query_Mail_Content*)pData;

    KGLOG_PROCESS_ERROR(pPak->dwMailID);

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoQueryMailContent(pPlayer->m_dwID, pPak->dwMailID, pPak->byMailType);

Exit0:
    return;
}

void KPlayerServer::OnAcquireMailMoneyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                            bRetCode    = false;
    KPlayer*                        pPlayer     = NULL;
    KC2S_Acquire_Mail_Money_Request* pPak        = (KC2S_Acquire_Mail_Money_Request*)pData;

    KGLOG_PROCESS_ERROR(pPak->dwMailID);

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoAcquireMailMoneyRequest(pPlayer->m_dwID, pPak->dwMailID);

Exit0:
    return;
}

void KPlayerServer::OnAcquireMailAllRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                            bRetCode    = false;
    KPlayer*                        pPlayer     = NULL;
    KC2S_Acquire_Mail_All_Request* pPak        = (KC2S_Acquire_Mail_All_Request*)pData;

    KGLOG_PROCESS_ERROR(pPak->dwMailID);

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoAcquireMailAllRequest(pPlayer->m_dwID, pPak->dwMailID);

Exit0:
    return;
}

void KPlayerServer::OnAcquireMailItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                            bRetCode    = false;
    KPlayer*                        pPlayer     = NULL;
    KC2S_Acquire_Mail_Item_Request* pPak        = (KC2S_Acquire_Mail_Item_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoAcquireMailItemRequest(pPlayer->m_dwID, pPak->dwMailID, pPak->byIndex);

Exit0:
    return;
}

void KPlayerServer::OnSetMailRead(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                bRetCode    = false;
    KPlayer*            pPlayer     = NULL;
    KC2S_Set_Mail_Read* pPak        = (KC2S_Set_Mail_Read*)pData;
    
    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoSetMailRead(pPlayer->m_dwID, pPak->dwMailID);

Exit0:
    return;
}

void KPlayerServer::OnDeleteMail(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                bRetCode    = false;
    KPlayer*            pPlayer     = NULL;
    KC2S_Delete_Mail*   pPak        = (KC2S_Delete_Mail*)pData;
    
    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoDeleteMail(pPlayer->m_dwID, pPak->dwMailID);

Exit0:
    return;
}

void KPlayerServer::OnBuyHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bRetCode    = false;
    KPlayer*                pPlayer     = NULL;
    KC2S_Buy_Hero_Request*  pRequest    = (KC2S_Buy_Hero_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    
    bRetCode = pPlayer->BuyHero(pRequest->wHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnFireHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bRetCode    = false;
    KPlayer*                pPlayer     = NULL;
    KC2S_Fire_Hero_Request*  pRequest    = (KC2S_Fire_Hero_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    
    bRetCode = pPlayer->FireHero(pRequest->wHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnBuyHeroSoltRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bRetCode    = false;
    KPlayer*                pPlayer     = NULL;
    KC2S_Buy_Hero_Solt_Request*  pRequest    = (KC2S_Buy_Hero_Solt_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->BuyHeroSlot();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnSelectMainHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                            bRetCode    = false;
    KPlayer*                        pPlayer     = NULL;
    KC2S_Select_Main_Hero_Request*  pRequest    = (KC2S_Select_Main_Hero_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->SelectMainHero(pRequest->dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_RelayClient.DoSelectHeroRequest(pPlayer);

Exit0:
    return;
}

void KPlayerServer::OnSelectAssistHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode    = false;
    KPlayer*                            pPlayer     = NULL;
    KC2S_Select_Assist_Hero_Request*    pRequest    = (KC2S_Select_Assist_Hero_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->SelectAssistHero(pRequest->dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_RelayClient.DoSelectHeroRequest(pPlayer);

Exit0:
    return;
}

void KPlayerServer::OnSetVIPAutoRepairFlag(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                            bRetCode	= false;
    KPlayer*                        pPlayer     = NULL;
    KC2S_Set_VIP_AutoRepair_Flag*   pRequest    = (KC2S_Set_VIP_AutoRepair_Flag*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->SetVIPAutoRepairFlag(pRequest->bAutoRepair);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnGetQuestListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL     bRetCode    = false;
    KPlayer* pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = DoSyncQuestList(pPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnSetQuestValueRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    QUEST_RESULT_CODE               eRetCode    = qrcInvalid;
    KPlayer*                        pPlayer     = NULL;
    KC2S_Set_Quest_Value_Request*   pRequest    = (KC2S_Set_Quest_Value_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_QuestList.ClientSetQuestValue(pRequest->questIndex, pRequest->valueIndex, pRequest->newValue);

Exit0:
    return;
}

void KPlayerServer::OnAcceptQuestRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    QUEST_RESULT_CODE           eRetCode    = qrcInvalid;
	KPlayer*                    pPlayer     = NULL;
    KC2S_Accept_Quest_Request*  pRequest    = (KC2S_Accept_Quest_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    eRetCode = pPlayer->m_QuestList.Accept(pRequest->questID);
    KGLOG_PROCESS_ERROR(eRetCode == qrcSuccess);

Exit0:
    return;
}

void KPlayerServer::OnFinishQuestRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	QUEST_RESULT_CODE eRetCode = qrcInvalid;
    KPlayer* pPlayer = NULL;
    KC2S_Finish_Quest_Request* pRequest = (KC2S_Finish_Quest_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    eRetCode = pPlayer->m_QuestList.Finish(pRequest->questID, pRequest->presentChoice1, pRequest->presentChoice2);
    KGLOG_PROCESS_ERROR(eRetCode == qrcSuccess);

Exit0:
    return;
}

void KPlayerServer::OnCancelQuestRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    QUEST_RESULT_CODE eRetCode = qrcInvalid;
    KPlayer* pPlayer = NULL;
    KC2S_Cancel_Quest_Request* pRequest = (KC2S_Cancel_Quest_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    eRetCode = pPlayer->m_QuestList.Cancel(pRequest->questIndex);
    KGLOG_PROCESS_ERROR(eRetCode == qrcSuccess);

Exit0:
    return;
}

void KPlayerServer::OnDoneQuestDirectlyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL  bRetCode = false;
    KPlayer* pPlayer = NULL;
    int nQuestIndex = -1;
    KC2S_Done_Quest_Directly_Request* pRequest = (KC2S_Done_Quest_Directly_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->m_QuestList.CanClientForceFinish(pRequest->dwQuestID);
    KGLOG_PROCESS_ERROR(bRetCode);

    nQuestIndex = pPlayer->m_QuestList.GetQuestIndex(pRequest->dwQuestID);
	if (nQuestIndex == -1) // 
    {
        pPlayer->m_QuestList.SetQuestState(pRequest->dwQuestID, qsFinished);
        goto Exit0;
    }

    bRetCode = pPlayer->m_QuestList.Finish(pRequest->dwQuestID, 0, 4, true);
    KGLOG_PROCESS_ERROR(bRetCode == qrcSuccess);

Exit0:
    return;
}

void KPlayerServer::OnExchangeItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL     bRetCode = false;
    KPlayer* pPlayer = NULL;
    KC2S_Exchange_Item_Request* pRequest = (KC2S_Exchange_Item_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_ItemList.ExchangeItem(
        pRequest->uSrcPackageType, pRequest->uSrcPackageIndex, pRequest->uSrcPos,
        pRequest->uDestPackageType, pRequest->uDestPackageIndex, pRequest->uDestPos
    );

Exit0:
    return;
}

void KPlayerServer::OnDestroyItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL bRetCode = false;
    KPlayer* pPlayer = NULL;
    KC2S_Destroy_Item_Request* pRequest = (KC2S_Destroy_Item_Request*)pData;
    IItem* pItem = NULL;
    const KItemProperty* pItemProperty = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pItem = pPlayer->m_ItemList.GetItem(pRequest->uPackageType, pRequest->uPackageIndex, pRequest->uPos);
    KGLOG_PROCESS_ERROR(pItem);

    pItemProperty = pItem->GetProperty();
    assert(pItemProperty);

    bRetCode = pItem->CanDestroy();
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLOG_PROCESS_ERROR(pItemProperty->dwTabType == pRequest->dwTabType);
    KGLOG_PROCESS_ERROR(pItemProperty->dwTabIndex == pRequest->dwTabIndex);
    KGLOG_PROCESS_ERROR(pItemProperty->nGenTime == (time_t)pRequest->uGenTime);

    bRetCode = pPlayer->m_ItemList.DestroyItem(
        pRequest->uPackageType, pRequest->uPackageIndex, pRequest->uPos
    );
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnUseItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL bRetCode = false;
    KPlayer* pPlayer = NULL;
    KC2S_Use_Item_Request* pRequest = (KC2S_Use_Item_Request*)pData;
    KHero* pHero = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_ItemList.UseItem(
        pRequest->uPackageType, pRequest->uPackageIndex, pRequest->uPos
    );

Exit0:
    return;
}

void KPlayerServer::OnApplyEnterMissionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL bRetCode = false;
    KPlayer* pPlayer = NULL;
    KC2S_Apply_Enter_Mission* pRequest = (KC2S_Apply_Enter_Mission*)pData;
    uint32_t dwMapID = 0;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->EnterMission(pRequest->byMissionType, pRequest->byMissionStep, pRequest->byMissionLevel);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return; 
}

void KPlayerServer::OnApplyLeaveMissionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL        bRetCode    = false;
    KPlayer*    pPlayer     = NULL;
    KHero*      pHero       = NULL;
    KMission*   pMission    = NULL;
    KMISSION_AWARD_DATA awardData[MAX_TEAM_MEMBER_COUNT];
    KHero* teammates[MAX_TEAM_MEMBER_COUNT] = {NULL};
    int  nScore[MAX_TEAM_MEMBER_COUNT] = {0, 0};
    BOOL bHasWard[KAWARD_CARD_TYPE_TOTAL] = {0};

    memset(awardData, 0, sizeof(awardData));

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pHero = pPlayer->GetFightingHero();
    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    pMission = g_pSO3World->m_MissionMgr.GetMission(pHero->m_pScene->m_dwMapID);
    KGLOG_PROCESS_ERROR(pMission);

    pHero->m_pScene->CallGlobalLeaveSceneScript(pHero);

    pHero->Detach();
    if (pPlayer->m_pAITeammate)
        pPlayer->m_pAITeammate->Detach();
    pPlayer->ReturnToHall();

    DoMissionBattleFinished(nConnIndex, pHero->m_nSide, nScore, awardData, teammates, bHasWard, 0);
    DoPlayerEnterHallNotify(nConnIndex);
    DoLeaveMissionRespond(nConnIndex, pMission->eType, pMission->nStep, pMission->nLevel);

Exit0:
    return; 
}

void KPlayerServer::OnGetDailyQuestRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    std::vector<DWORD> vecRandomQuest;
    g_pSO3World->m_Settings.m_QuestInfoList.GetDailyQuest(vecRandomQuest);
    DoSyncRandomDailyQuest(nConnIndex, vecRandomQuest);
}

void KPlayerServer::OnCreatePveTeamRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                            bRetCode    = false;
    DWORD                           dwMapID     = ERROR_ID;
    KPlayer*                        pPlayer     = NULL;
    KC2S_Create_Pve_Team_Request*   pRequest    = (KC2S_Create_Pve_Team_Request*)pData;

    pRequest->szRoomName[countof(pRequest->szRoomName) - 1] = '\0';

    g_pSO3World->m_piTextFilter->Replace(pRequest->szRoomName);

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    if (pRequest->byPVEMode == KPVE_CONTINUOUS_CHALLENGE)
    {
        pPlayer->SetLastChallengeStep((pPlayer->GetLastChallengeStep() - 1) / cdChallengeSmallStepCount * cdChallengeSmallStepCount + 1);
        
        if (pRequest->byReStart)
        {        
            pPlayer->SetStartChallengeStep(1);
            pPlayer->SetLastChallengeStep(1);
            pPlayer->m_vecChallengeAwardItem.clear();
        }

        dwMapID = g_pSO3World->m_MissionMgr.GetMissionMap(pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1);
        KGLOG_PROCESS_ERROR(dwMapID > 0);

        bRetCode = pPlayer->CanStartContinuousChallenge();
        KGLOG_PROCESS_ERROR(bRetCode);

        g_RelayClient.DoCreatePveTeamRequest(
            pPlayer, pRequest->byPVEMode, pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1,
            dwMapID, pRequest->szRoomName, pRequest->szPassword
        );
    }
    else
    {
        dwMapID = g_pSO3World->m_MissionMgr.GetMissionMap(pRequest->byMissionType, pRequest->byMissionStep, pRequest->byMissionLevel);
        KGLOG_PROCESS_ERROR(dwMapID > 0);

        g_RelayClient.DoCreatePveTeamRequest(
            pPlayer, pRequest->byPVEMode, pRequest->byMissionType, pRequest->byMissionStep, 
            pRequest->byMissionLevel, dwMapID, pRequest->szRoomName, pRequest->szPassword
        );
    }
    
Exit0:
    return;
}

void KPlayerServer::OnQuickStartPveRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	BOOL                            bRetCode    = false;
	DWORD                           dwMapID     = ERROR_ID;
	KPlayer*                        pPlayer     = NULL;
	KC2S_Quick_Start_Pve_Request*   pRequest    = (KC2S_Quick_Start_Pve_Request*)pData;

	pRequest->szRoomName[countof(pRequest->szRoomName) - 1] = '\0';

	g_pSO3World->m_piTextFilter->Replace(pRequest->szRoomName);

	pPlayer = GetPlayerByConnection(nConnIndex);
	KGLOG_PROCESS_ERROR(pPlayer);

	if (pRequest->byPVEMode == KPVE_CONTINUOUS_CHALLENGE)
	{
		pPlayer->SetLastChallengeStep((pPlayer->GetLastChallengeStep() - 1) / cdChallengeSmallStepCount * cdChallengeSmallStepCount + 1);

		if (pRequest->byReStart)
        {
			pPlayer->SetLastChallengeStep(1);
            pPlayer->m_vecChallengeAwardItem.clear();
        }

        dwMapID = g_pSO3World->m_MissionMgr.GetMissionMap(pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1);
		KGLOG_PROCESS_ERROR(dwMapID > 0);

		bRetCode = pPlayer->CanStartContinuousChallenge();
		KGLOG_PROCESS_ERROR(bRetCode);

		g_RelayClient.DoQuickStartPveRequest(
            pPlayer, pRequest->byPVEMode, pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1,
			dwMapID, pRequest->szRoomName, pRequest->szPassword
		);
	}
	else
	{
		dwMapID = g_pSO3World->m_MissionMgr.GetMissionMap(pRequest->byMissionType, pRequest->byMissionStep, pRequest->byMissionLevel);
		KGLOG_PROCESS_ERROR(dwMapID > 0);

		g_RelayClient.DoQuickStartPveRequest(
			pPlayer, pRequest->byPVEMode, pRequest->byMissionType, pRequest->byMissionStep, 
			pRequest->byMissionLevel, dwMapID, pRequest->szRoomName, pRequest->szPassword
		);
	}

Exit0:
	return;
}

void KPlayerServer::OnApplyJoinPveTeamRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL        bRetCode    = false;
    KPlayer*    pPlayer     = NULL;
    KC2S_Apply_Join_Pve_Team_Request* pRequest = (KC2S_Apply_Join_Pve_Team_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    if (pRequest->byPVEMode == KPVE_CONTINUOUS_CHALLENGE)
    {
        bRetCode = pPlayer->CanStartContinuousChallenge();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    g_RelayClient.DoApplyJoinPveTeamRequest(pPlayer, pRequest->dwTeamID, pRequest->szPassword);

Exit0:
    return;
}

void KPlayerServer::OnApplyPveTeamInfoRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Apply_Pve_Team_Info_Request* pRequest = (KC2S_Apply_Pve_Team_Info_Request*)pData;
    KPlayer*    pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoApplyPveTeamInfoRequest(
        pPlayer->m_dwID, pRequest->byPage, pRequest->byPVEMode, pRequest->byMissionType, pRequest->byMissionStep
    );

Exit0:
    return;
}

void KPlayerServer::OnPveTeamInvitePlayerRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Pve_Team_Invite_Player_Request* pRequest = (KC2S_Pve_Team_Invite_Player_Request*)pData;
    KPlayer*    pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pRequest->szDestPlayerName[countof(pRequest->szDestPlayerName) - 1] = '\0';

    g_RelayClient.DoPveTeamInvitePlayerRequest(pPlayer->m_dwID, pRequest->szDestPlayerName);

Exit0:
    return;
}

void KPlayerServer::OnPveTeamKickoutPlayerRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Pve_Team_Kickout_Player_Request* pRequest = (KC2S_Pve_Team_Kickout_Player_Request*)pData;
    KPlayer*    pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoPveTeamKickoutPlayerRequest(pPlayer->m_dwID, pRequest->dwDestPlayerID);

Exit0:
    return;
}

void KPlayerServer::OnPveTeamLeaveRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Pve_Team_Leave_Request* pRequest = (KC2S_Pve_Team_Leave_Request*)pData;
    KPlayer*    pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_vecChallengeAwardItem.clear();

    g_RelayClient.DoPveTeamLeaveRequest(pPlayer->m_dwID);

Exit0:
    return;
}

void KPlayerServer::OnPveTeamAcceptOrRefuseInvite(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Pve_Team_Accept_Or_Refuse_Invite* pRequest = (KC2S_Pve_Team_Accept_Or_Refuse_Invite*)pData;
    KPlayer*    pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pRequest->szInviterName[countof(pRequest->szInviterName) - 1] = '\0';

    g_RelayClient.DoPveTeamAcceptOrRefuseInvite(pPlayer, pRequest->acceptCode, pRequest->szInviterName);

Exit0:
    return;
}

void KPlayerServer::OnPveTeamReadyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Pve_Team_Ready_Request* pRequest = (KC2S_Pve_Team_Ready_Request*)pData;
    KPlayer*    pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoPveTeamReadyRequest(pPlayer, pRequest->bReady);

Exit0:
    return;
}

void KPlayerServer::OnApplySetPveTeamAiMode(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Apply_Set_PVE_Room_Aimode* pRequest = (KC2S_Apply_Set_PVE_Room_Aimode*)pData;
    KPlayer*    pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoPveTeamSetAiMode(pPlayer, pRequest->bIsAIMode);

Exit0:
    return;
}

void KPlayerServer::OnPveTeamStartGameRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Pve_Team_Start_Game_Request* pRequest = (KC2S_Pve_Team_Start_Game_Request*)pData;
    KPlayer*    pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoPveTeamStartGameRequest(pPlayer, pPlayer->m_bNeedCheckCanEnter, true);

Exit0:
    return;
}

void KPlayerServer::OnPveTeamChangeMissionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Pve_Team_Change_Mission* pRequest = (KC2S_Pve_Team_Change_Mission*)pData;
    KPlayer* pPlayer = NULL;
    DWORD dwMapID = ERROR_ID;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    dwMapID = g_pSO3World->m_MissionMgr.GetMissionMap(pRequest->byMissionType, pRequest->byMissionStep, pRequest->byMissionLevel);
    KGLOG_PROCESS_ERROR(dwMapID);

    g_RelayClient.DoPveTeamChangeMissionRequest(pPlayer->m_dwID, pRequest->byMissionType, pRequest->byMissionStep, pRequest->byMissionLevel, dwMapID);

Exit0:
    return;
}

void KPlayerServer::OnTryJoinPveTeam(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Try_Join_Pve_Team* pRequest = (KC2S_Try_Join_Pve_Team*)pData;
    KPlayer* pPlayer = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoPveTeamTryJoin(pPlayer, pRequest->byPVEMode, pRequest->uTeamID);

Exit0:
    return;
}

void KPlayerServer::OnAutoJoinPveTeam(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Auto_Join_Pve_Team* pRequest = (KC2S_Auto_Join_Pve_Team*)pData;
    KPlayer* pPlayer = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoPveTeamAutoJoin(pPlayer, pRequest->byPVEMode, pRequest->missionType, pRequest->missionStep, pRequest->openLevel, pRequest->validLevel);

Exit0:
    return;
}

void KPlayerServer::OnApplyExtendPlayerPackage(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL        bRetCode        = false;
    KPlayer*    pPlayer         = NULL;
    KPackage*   pPackage        = NULL;
    int         nPackageSize    = 0;
    int         nExtendPackageIndex = -1;
    int         nCostMoney      = 0;
    KCUSTOM_CONSUME_INFO                CCInfo;    
    KC2S_Apply_Extend_Player_Package*   pRequest = (KC2S_Apply_Extend_Player_Package*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPackage = pPlayer->m_ItemList.GetPlayerPackage(pRequest->uPackageIndex);
    KGLOG_PROCESS_ERROR(pPackage);

    nPackageSize = pPackage->GetSize();
    nExtendPackageIndex = (nPackageSize - cdPlayerPageSize) / cdPerExtendPlayerPackageSize;
    KGLOG_PROCESS_ERROR(nExtendPackageIndex >= 0);
    KGLOG_PROCESS_ERROR(nExtendPackageIndex < cdExtendPlayerPackageMaxCount);

    nCostMoney = g_pSO3World->m_Settings.m_ConstList.nExtendPackageSlotPrice[nExtendPackageIndex];
    KGLOG_PROCESS_ERROR(nCostMoney > 0);
    
    bRetCode = pPlayer->m_MoneyMgr.CanAddMoney(emotCoin, -nCostMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));
    CCInfo.nValue1 = pRequest->uPackageIndex;

    bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(pPlayer, uctExtendPlayerPackage, nCostMoney, &CCInfo);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnCutStackItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL        bRetCode        = false;
    KPlayer*    pPlayer         = NULL;
    KC2S_Cut_Stack_Item_Request* pRequest = (KC2S_Cut_Stack_Item_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_ItemList.CutStackItem(
        pRequest->uSrcPackageType, pRequest->uSrcPackageIndex, pRequest->uSrcPos,
        pRequest->uDestPackageType, pRequest->uDestPackageIndex, pRequest->uDestPos, pRequest->wCutNum
    );

Exit0:
    return;
}

void KPlayerServer::OnSendNetworkDelay(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bRetCode        = false;
    KPlayer*                    pPlayer         = NULL;
    KConnectionData* 	        pConnData       = NULL;
    struct in_addr              RemoteAddr      = { 0 };
    u_short                     wRemotePortNet  = 0;
    u_short                     wRemotePortHost = 0;
    char*                       pszRetString    = NULL;
    KC2S_Send_Network_Delay*    pRequest        = (KC2S_Send_Network_Delay*)pData;

    pConnData = GetConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pConnData);
    
    pPlayer = pConnData->pPlayer;
    KGLOG_PROCESS_ERROR(pPlayer);

    pConnData->piSocketStream->GetRemoteAddress(&RemoteAddr, &wRemotePortNet);

    pszRetString = inet_ntoa(RemoteAddr);
    KGLOG_PROCESS_ERROR(pszRetString);

    PLAYER_LOG(pPlayer, "client,delay,%s,%d,%d,%d", pszRetString, pRequest->nServerDelay, pRequest->nPingDelay, (pRequest->nServerDelay - pRequest->nPingDelay));

Exit0:
    return;
}


void KPlayerServer::OnActivityAwardRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Activity_Award_Request* pRequest = (KC2S_Activity_Award_Request* )pData;
    KPlayer* pPlayer = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoCheckAndUpdateActivityStateRequest(pPlayer->m_dwID, pRequest->dwActivityID, KACTIVITYITEM_STATE_RECORDED, KACTIVITYITEM_STATE_PROCESSED);

Exit0:
    return;
}

void KPlayerServer::OnStatClientInfo(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Stat_Client_Info* pRequest = (KC2S_Stat_Client_Info* )pData;
    KPlayer* pPlayer = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    PLAYER_LOG(pPlayer, "client,statClientInfo,%d,%d,%d", pRequest->logicFps, pRequest->renderFps, pRequest->memory);

Exit0:
    return;
}

void KPlayerServer::OnStatClientLoadingInfo(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Stat_Client_Loading_Info* pRequest = (KC2S_Stat_Client_Loading_Info* )pData;
    KPlayer* pPlayer = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    PLAYER_LOG(pPlayer, "client,statClientLoadingInfo,%u,%u", pRequest->downloadSpeed, pRequest->loadingTime);

Exit0:
    return;
}

void KPlayerServer::OnNormalShootBall(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    KHero*                  pHero           = NULL;
    BOOL                    bAdjust         = false;
    BOOL                    bWithBall       = false;
    KBall*                  pBall           = NULL;
    KPlayer*                pPlayer         = NULL;
    BOOL                    bReverseFailed  = false;
    KC2S_Normal_ShootBall*  pCommand        = (KC2S_Normal_ShootBall*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pHero = pPlayer->GetFightingHero();
    KGLOG_PROCESS_ERROR(pHero);

    bAdjust = pHero->CheckMoveAdjust(nFrame, pCommand->moveParam, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    pHero->m_MoveCount = pCommand->movecount;
    bWithBall = pHero->HasFollowerWithStateAndType(mosToken, sotBall);
    if (!bWithBall)
    {
        pBall = pHero->m_pScene->GetBall();
        KGLOG_PROCESS_ERROR(pBall);

        g_PlayerServer.DoSyncSceneObject(pBall);
        KGLogPrintf(KGLOG_INFO, "Shoot ball pos error, sync ball state!");
        goto Exit0;
    }

    KGLOG_PROCESS_ERROR(pCommand->aimatLoop == (nFrame - pHero->m_nLastAimTime));
    bRetCode = pHero->NormalShootBall(pCommand->aimatLoop, pCommand->interferenceHeroID);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bAdjust)
    {
        DoAdjustHeroMove(pHero, KSHOOT_BALL);
    }

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnSkillShootBall(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    KHero*                  pHero           = NULL;
    BOOL                    bAdjust         = false;
    BOOL                    bWithBall       = false;
    KBall*                  pBall           = NULL;
    KPlayer*                pPlayer         = NULL;
    BOOL                    bReverseFailed  = false;
    KC2S_Skill_ShootBall*   pCommand        = (KC2S_Skill_ShootBall*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pHero = pPlayer->GetFightingHero();
    KGLOG_PROCESS_ERROR(pHero);

    bAdjust = pHero->CheckMoveAdjust(nFrame, pCommand->moveParam, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    pHero->m_MoveCount = pCommand->movecount;
    bWithBall = pHero->HasFollowerWithStateAndType(mosToken, sotBall);
    if (!bWithBall)
    {
        pBall = pHero->m_pScene->GetBall();
        KGLOG_PROCESS_ERROR(pBall);

        g_PlayerServer.DoSyncSceneObject(pBall);
        LogInfo("Shoot ball pos error, sync ball state!");
        goto Exit0;
    }

    bRetCode = pHero->SkillShootBall(pCommand->targetBasketSocketID);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bAdjust)
    {
        DoAdjustHeroMove(pHero, KSHOOT_BALL);
    }

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnNormalSlamBall(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    KHero*                  pHero           = NULL;
    BOOL                    bAdjust         = false;
    BOOL                    bWithBall       = false;
    KBall*                  pBall           = NULL;
    KPlayer*                pPlayer         = NULL;
    BOOL                    bReverseFailed  = false;
    KC2S_Normal_SlamBall*   pCommand        = (KC2S_Normal_SlamBall*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pHero = pPlayer->GetFightingHero();
    KGLOG_PROCESS_ERROR(pHero);

    bAdjust = pHero->CheckMoveAdjust(nFrame, pCommand->moveParam, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    pHero->m_MoveCount = pCommand->movecount;
    bWithBall = pHero->HasFollowerWithStateAndType(mosToken, sotBall);
    if (!bWithBall)
    {
        pBall = pHero->m_pScene->GetBall();
        KGLOG_PROCESS_ERROR(pBall);

        g_PlayerServer.DoSyncSceneObject(pBall);
        LogInfo("Shoot ball pos error, sync ball state!");
        goto Exit0;
    }

    bRetCode = pHero->NormalSlamBall();
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bAdjust)
    {
        DoAdjustHeroMove(pHero, KSHOOT_BALL);
    }

    pPlayer->OnEvent(peSlamDunk);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnSkillSlamBall(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    KHero*                  pHero           = NULL;
    BOOL                    bAdjust         = false;
    BOOL                    bWithBall       = false;
    KBall*                  pBall           = NULL;
    KPlayer*                pPlayer         = NULL;
    BOOL                    bReverseFailed  = false;
    KC2S_Skill_SlamBall*    pCommand        = (KC2S_Skill_SlamBall*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pHero = pPlayer->GetFightingHero();
    KGLOG_PROCESS_ERROR(pHero);

    bAdjust = pHero->CheckMoveAdjust(nFrame, pCommand->moveParam, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    pHero->m_MoveCount = pCommand->movecount;
    bWithBall = pHero->HasFollowerWithStateAndType(mosToken, sotBall);
    if (!bWithBall)
    {
        pBall = pHero->m_pScene->GetBall();
        KGLOG_PROCESS_ERROR(pBall);

        g_PlayerServer.DoSyncSceneObject(pBall);
        LogInfo("Shoot ball pos error, sync ball state!");
        goto Exit0;
    }

    bRetCode = pHero->SkillSlamBall(pCommand->targetBasketSocketID);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bAdjust)
    {
        DoAdjustHeroMove(pHero, KSHOOT_BALL);
    }

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnMakingMachineInfoRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL     bResult  = false;
    BOOL     bRetCode = false;
    KPlayer* pPlayer  = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = DoSyncMakingMachineInfo(pPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}


void KPlayerServer::OnProduceItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	BOOL        bResult  = false;
	BOOL        bRetCode = false;
	KPlayer*    pPlayer  = NULL;
    KC2S_ProduceItem_Request* pRequest = (KC2S_ProduceItem_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
	
    bRetCode = pPlayer->ProduceItem(pRequest->recipeID);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return;
}

void KPlayerServer::OnApplyUpgradeGymEquip(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KPlayer*        pPlayer     = NULL;
    KC2S_Apply_Upgrade_Gym_Equip* pRequest = (KC2S_Apply_Upgrade_Gym_Equip*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->m_Gym.UpgradeEquip();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnApplyUseGymEquip(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KPlayer*        pPlayer     = NULL;
    KC2S_Apply_Use_Gym_Equip* pRequest = (KC2S_Apply_Use_Gym_Equip*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->m_Gym.HeroUseEquip(pRequest->wHeroTemplateID, (KGYM_TYPE)(int)pRequest->byEquipType, pRequest->byCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnApplyDirectEndUseGymEquip(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KPlayer*        pPlayer     = NULL;
    KC2S_Direct_End_Use_Gym_Equip* pRequest = (KC2S_Direct_End_Use_Gym_Equip*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->m_Gym.DirectEndUseEquipRequest(pRequest->wHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnApplyBuyGymSlot(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    return;
}

void KPlayerServer::OnApplyUpgradeSafeBox(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KPlayer*        pPlayer     = NULL;
    KC2S_Apply_Upgrade_Safe_Box* pRequest = (KC2S_Apply_Upgrade_Safe_Box*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->m_MoneyMgr.UpgradeSafeBox();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnStrengthenEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KPlayer*        pPlayer     = NULL;
    KC2S_Strengthen_Equip* pRequest = (KC2S_Strengthen_Equip*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->StrengthenEquip(
        pRequest->uPackageTypeEquipIn, pRequest->uPackageIndexEquipIn, pRequest->uPosEquipIn, 
		pRequest->uCurStrengthenLevel, pRequest->bUseLuckyItem, 
        pRequest->uPackageTypeLuckyItemIn, pRequest->uPackageIndexLuckyItemIn, pRequest->uPosLuckyItemIn
	);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnBuyFatiguePointRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KPlayer*        pPlayer     = NULL;
    KC2S_Buy_Fatigue_Point* pRequest = (KC2S_Buy_Fatigue_Point*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->BuyFatiguePoint(pRequest->byTodayBuySN);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnReportItemRead(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer*        pPlayer     = NULL;
	KC2S_Report_Item_Read* pRequest = (KC2S_Report_Item_Read*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_Secretary.OnItemRead(pRequest->reportIndex);

Exit0:
	return;
}

void KPlayerServer::OnBuyFashionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KC2S_Buy_Fashion_Request* pRequest = (KC2S_Buy_Fashion_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_Wardrobe.OnBuyFashionRequest(pRequest->FashionIndex);

Exit0:
    return;
}

void KPlayerServer::OnChangeHeroFashionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KC2S_Change_Hero_Fashion_Request* pRequest  = (KC2S_Change_Hero_Fashion_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->ChangeFashion(pRequest->heroTemplateID, pRequest->targetFashionIndex);

Exit0:
    return;
}

void KPlayerServer::OnChargeFashionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KC2S_Charge_Fashion_Request* pRequest   = (KC2S_Charge_Fashion_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_Wardrobe.OnChargeFashionRequest(pRequest->fashionIndex, pRequest->chargeType);

Exit0:
    return;
}

void KPlayerServer::OnBuyCheerleaingSlot(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KC2S_Buy_Cheerleading_Slot* pRequest = (KC2S_Buy_Cheerleading_Slot*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_CheerleadingMgr.BuySlot(pRequest->slotIndex);

Exit0:
    return;
}

void KPlayerServer::OnUpdateCheerleadingSlotInfo(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer*        pPlayer     = NULL;
    KC2S_Update_Cheerleading_Slotinfo* pRequest = (KC2S_Update_Cheerleading_Slotinfo*)pData;
    KCHEERLEADING_SLOTINFO slotInfo[cdMaxCheerleadingSlotCount];

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    for (int i = 0; i < countof(slotInfo); ++i)
    {
        slotInfo[i].dwCheerleadingIndex = pRequest->slotInfo[i].cheerleadingIndex;
        slotInfo[i].nOffsetX = pRequest->slotInfo[i].offsetX;
        slotInfo[i].nFaceTo  = pRequest->slotInfo[i].faceTo;
    }

    pPlayer->m_CheerleadingMgr.UpdateCheerleadingSlotInfo(slotInfo);

Exit0:
    return;
}

void KPlayerServer::OnRechareCheerleadingItem(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer*        pPlayer     = NULL;
    KC2S_Recharge_Cheerleading_Item* pRequest = (KC2S_Recharge_Cheerleading_Item*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_CheerleadingMgr.OnRechargeCheerleadingItemRequest(pRequest->cheerleadingIdx, pRequest->lifeType);

Exit0:
    return;
}

void KPlayerServer::OnClearUpgradeCDRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer*        pPlayer     = NULL;
    KC2S_Clear_UpgradeCD_Request* pRequest = (KC2S_Clear_UpgradeCD_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    if (pRequest->bFree)
    {
        pPlayer->m_UpgradeQueue.FreeClearCDRequest((KUPGRADE_TYPE)(int)pRequest->byUpgradeType, pRequest->wHeroTemplateID);
        goto Exit0;
    }

    pPlayer->m_UpgradeQueue.ClearCDRequest((KUPGRADE_TYPE)(int)pRequest->byUpgradeType, pRequest->wHeroTemplateID);

Exit0:
    return;
}

void KPlayerServer::OnUpgradeGymEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer*        pPlayer     = NULL;
    KC2S_Upgrade_Gym_Equip_Request* pRequest = (KC2S_Upgrade_Gym_Equip_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_Gym.UpgradeEquip();

Exit0:
    return;
}

void KPlayerServer::OnHeroUseEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer*        pPlayer     = NULL;
    KC2S_Hero_Use_Equip_Request* pRequest = (KC2S_Hero_Use_Equip_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_Gym.HeroUseEquip(pRequest->wHeroTemplateID, (KGYM_TYPE)(int)pRequest->byEquipType, pRequest->byCount);

Exit0:
    return;
}

void KPlayerServer::OnDirectFinishUseEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer*        pPlayer     = NULL;
    KC2S_Direct_Finish_Use_Equip_Request* pRequest = (KC2S_Direct_Finish_Use_Equip_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_Gym.DirectEndUseEquipRequest(pRequest->wHeroTemplateID);

Exit0:
    return;
}

void KPlayerServer::OnBsBuyLandRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Bs_Buy_Land*   pRequest    = (KC2S_Bs_Buy_Land*)pData;
    KPlayer*            pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_BusinessStreet.BuyLandRequest(pRequest->wID);

Exit0:
    return;
}

void KPlayerServer::OnBsBuildStoreRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Bs_Build_Store*    pRequest    = (KC2S_Bs_Build_Store*)pData;
    KPlayer*                pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_BusinessStreet.BuildStoreRequest(pRequest->wID, (KSTORE_TYPE)(int)pRequest->byType);

Exit0:
    return;
}

void KPlayerServer::OnBsUpgradeStoreRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Bs_Upgrade_Store*   pRequest    = (KC2S_Bs_Upgrade_Store*)pData;
    KPlayer*            pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_BusinessStreet.UpgradeStoreRequest(pRequest->wID);

Exit0:
    return;
}

void KPlayerServer::OnBsGetStoreProfit(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Bs_Get_Store_Profit*   pRequest    = (KC2S_Bs_Get_Store_Profit*)pData;
    KPlayer*            pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_BusinessStreet.GetMoney(pRequest->wID);

Exit0:
    return;
}

void KPlayerServer::OnBsClearGetStoreProfitCD(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Bs_Clear_Get_Store_Profit_CD*   pRequest    = (KC2S_Bs_Clear_Get_Store_Profit_CD*)pData;
    KPlayer*            pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_BusinessStreet.ClearGetMoneyCDRequest(pRequest->wID);

Exit0:
    return;
}

void KPlayerServer::OnBsGetAllStoreProfit(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Bs_Get_All_Store_Profit*   pRequest    = (KC2S_Bs_Get_All_Store_Profit*)pData;
    KPlayer*            pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_BusinessStreet.GetAllMoney(false);

Exit0:
    return;
}

void KPlayerServer::OnBsClearAllGetStoreProfitCD(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Bs_Clear_All_Get_Store_Profit_CD*   pRequest    = (KC2S_Bs_Clear_All_Get_Store_Profit_CD*)pData;
    KPlayer*            pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_BusinessStreet.ClearAllGetMoneyCDRequest();

Exit0:
    return;
}

void KPlayerServer::OnDialogCompleted(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	BOOL		bRetCode	= false;
	KPlayer*    pPlayer     = NULL;
	KScene*		pScene		= NULL;

	pPlayer = GetPlayerByConnection(nConnIndex);
	KGLOG_PROCESS_ERROR(pPlayer);

	pScene = g_pSO3World->GetScene(pPlayer->m_dwMapID, pPlayer->m_nCopyIndex);
	KG_PROCESS_ERROR(pScene);

	bRetCode = pScene->m_DramaPlayer.ResumeDrama();
	KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
	return;
}

void KPlayerServer::OnBsChangeStoreType(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Bs_Change_Store_Type*   pRequest    = (KC2S_Bs_Change_Store_Type*)pData;
    KPlayer*    pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_BusinessStreet.ChangeStoreTypeRequest(pRequest->wID, (KSTORE_TYPE)(int)pRequest->byType);

Exit0:
    return;
}

void KPlayerServer::OnScenePause(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Scene_Pause*   pRequest    = (KC2S_Scene_Pause*)pData;
    KPlayer*            pPlayer     = NULL;
    KHero*              pHero       = NULL;
    KScene*             pScene      = NULL;
    KMission*           pMission    = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pHero = pPlayer->GetFightingHero();
    KGLOG_PROCESS_ERROR(pHero);

    pScene = pHero->m_pScene;
    KGLOG_PROCESS_ERROR(pScene);

    pMission = g_pSO3World->m_MissionMgr.GetMission(pScene->m_dwMapID);
	KGLOG_PROCESS_ERROR(pMission);

    KGLOG_PROCESS_ERROR(pMission->nCanPause == 1);

    pScene->m_bPause = pRequest->bPause;

Exit0:
    return;
}

void KPlayerServer::OnStartDramaRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bRetCode    = false;
    KC2S_Start_Drama_Request*   pRequest    = (KC2S_Start_Drama_Request*)pData;
    KPlayer*                    pPlayer     = NULL;
    KHero*                      pHero       = NULL;
    KScene*                     pScene      = NULL;
    KMapParams*                 pMapInfo    = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pHero = pPlayer->GetFightingHero();
    KGLOG_PROCESS_ERROR(pHero);

    pScene = pHero->m_pScene;
    KGLOG_PROCESS_ERROR(pScene);

    pMapInfo = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(pScene->m_dwMapID);
    KGLOG_PROCESS_ERROR(pMapInfo);

    KGLOG_PROCESS_ERROR(pMapInfo->nType == emtTrainMap);

    bRetCode = pScene->m_DramaPlayer.StartDrama(pRequest->wDramaID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnCreateClubRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{ 
    BOOL                bRetCode    = false;
    KC2S_Create_Club*   pRequest    = (KC2S_Create_Club*)pData;
    KPlayer*            pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID == 0);

    if (pPlayer->m_nLevel < g_pSO3World->m_Settings.m_ConstList.nLevelForCreateClub)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_CLUB_CREATE_LEVEL_LIMIT);
        goto Exit0;
    }
    // 其他条件的判断
    pRequest->strClubName[sizeof(pRequest->strClubName) - 1] = '\0';
    bRetCode = g_pSO3World->m_piNameFilter->Check((char*)pRequest->strClubName);
    if (!bRetCode)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_CLUB_NAME_FORBIT);
        goto Exit0;
    }
    KGLOG_PROCESS_ERROR(pPlayer->m_MoneyMgr.GetMoney(emotMoney) >= g_pSO3World->m_Settings.m_ConstList.nMoneyForCreateClub);

    g_LSClient.DoCreateClubRequest(pPlayer->m_dwID, pRequest->strClubName);

Exit0:
    return;
}

void KPlayerServer::OnGetMemberListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Get_Member_List*   pRequest    = (KC2S_Get_Member_List*)pData;
    KPlayer*                pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KG_PROCESS_ERROR(pPlayer->m_dwClubID);

    g_LSClient.DoApplyClubMemberList(pPlayer->m_dwID, pPlayer->m_dwClubID, pRequest->dwVersion);

Exit0:
    return;
}

void KPlayerServer::OnApplyClubRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Apply_Club*   pRequest    = (KC2S_Apply_Club*)pData;
    KPlayer*           pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID == 0);

    if(pPlayer->m_nClubApplyNum >= g_pSO3World->m_Settings.m_ConstList.nPlayerApplyLimit)
    {
        //提示客户端
        //DoDownwardNotify(pPlayer, KMESSAGE_CLUB_APPLY_LIMIT);
        DoApplyJoinClubRepsond(pPlayer->m_nConnIndex, pRequest->dwClubID, KAPPLY_JOIN_CLUB_APPLYCOUNT_LIMIT);
        goto Exit0;
    }

    if (g_pSO3World->m_nCurrentTime < pPlayer->m_nQuitClubTime + g_pSO3World->m_Settings.m_ConstList.nAddClubCoolDown)
    {
        //DoDownwardNotify(pPlayer, KMESSAGE_CLUB_ADD_COOLDOWN);
        DoApplyJoinClubRepsond(pPlayer->m_nConnIndex, pRequest->dwClubID, KAPPLY_JOIN_CLUB_COOLDOWN);
        goto Exit0;
    }

    ++pPlayer->m_nClubApplyNum;

    g_LSClient.DoApplyJoinClubRequest(pPlayer->m_dwID, pRequest->dwClubID);

Exit0:
    return;
}

void KPlayerServer::OnAcceptAppRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Accept_App*   pRequest    = (KC2S_Accept_App*)pData;
    KPlayer*           pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID);
    //副会长以上才能收人
    KGLOG_PROCESS_ERROR(pPlayer->m_nClubPost >= eClub_Post_Vice);
    g_LSClient.DoClubAcceptApplyRequest(pPlayer->m_dwID, pPlayer->m_dwClubID, pRequest->dwPlayerID, pRequest->byType);

Exit0:
    return;
}

void KPlayerServer::OnInviteJoinClub(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Invite_Player*   pRequest    = (KC2S_Invite_Player*)pData;
    KPlayer*              pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID);
    //精英以上才能收人
    KGLOG_PROCESS_ERROR(pPlayer->m_nClubPost >= eClub_Post_Elite);
    g_LSClient.DoClubInviteRequest(pPlayer->m_dwID, pPlayer->m_dwClubID, pRequest->szPlayerName);

Exit0:
    return;
}

void KPlayerServer::OnAccetpInviteRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Accept_Invite*   pRequest    = (KC2S_Accept_Invite*)pData;
    KPlayer*              pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID == 0);

    if (g_pSO3World->m_nCurrentTime < pPlayer->m_nQuitClubTime + g_pSO3World->m_Settings.m_ConstList.nAddClubCoolDown)
    {
        DoApplyJoinClubRepsond(pPlayer->m_nConnIndex, pRequest->dwClubID, KAPPLY_JOIN_CLUB_COOLDOWN);
        //DoDownwardNotify(pPlayer, KMESSAGE_CLUB_ADD_COOLDOWN);
        goto Exit0;
    }

    g_LSClient.DoAcceptClubInviteRequest(pPlayer->m_dwID, pRequest->dwClubID, pRequest->dwLauncherID, (int)pRequest->byRepsond);

    if (pRequest->byRepsond == KACCEPT_INVITE_OK)
    {
        PLAYER_LOG(pPlayer, "club,invite,%d,%d,%d", pPlayer->m_nLevel, pPlayer->m_nTotalGameTime, (g_pSO3World->m_nCurrentTime - pPlayer->m_nCreateTime));
    }

Exit0:
    return;
}

void KPlayerServer::OnSetClubPostRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Set_Club_Post*   pRequest    = (KC2S_Set_Club_Post*)pData;
    KPlayer*              pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwID != pRequest->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer->m_nClubPost == eClub_Post_Leader || pPlayer->m_nClubPost > pRequest->byPost);

    g_LSClient.DoClubSetPostRequest(pPlayer->m_dwID, pPlayer->m_dwClubID, pRequest->dwPlayerID, pRequest->byPost);
Exit0:
    return;
}

void KPlayerServer::OnDelMemberRequest(char* pData, size_t nSize, int nConnIndex, int nFrame) 
{
    KC2S_Del_Club_Member*   pRequest    = (KC2S_Del_Club_Member*)pData;
    KPlayer*                pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwID == pRequest->dwPlayerID || pPlayer->m_nClubPost >= eClub_Post_Vice);

    g_LSClient.DoDelMemberRequest(pPlayer->m_dwID, pPlayer->m_dwClubID, pRequest->dwPlayerID);
Exit0:
    return;
}

void KPlayerServer::OnSetClubTxtRequest(char* pData, size_t nSize, int nConnIndex, int nFrame) 
{
    BOOL                 bRetCode    = false;
    KC2S_Set_Club_Txt*   pRequest    = (KC2S_Set_Club_Txt*)pData;
    KPlayer*             pPlayer     = NULL;
    char*                pszCheckTxt    = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID);
    KGLOG_PROCESS_ERROR(pRequest->byTxtType == eClub_Txt_Remark || pPlayer->m_nClubPost >= eClub_Post_Vice);

    pszCheckTxt = (char*)m_byTempData;
    memcpy(pszCheckTxt, pRequest->Txtdata, pRequest->TxtdataLength);
    pszCheckTxt[pRequest->TxtdataLength] = '\0';

    bRetCode = g_pSO3World->m_piTextFilter->Check(pszCheckTxt);
    if (!bRetCode)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_TEXT_FORBIT);
        if (pRequest->byTxtType == eClub_Txt_Remark)
            g_PlayerServer.DoModifyRemarkFail(pPlayer);
        goto Exit0;
    }

    g_LSClient.DoSetDescriptRequest(pPlayer->m_dwID, pPlayer->m_dwClubID, pRequest->byTxtType, pRequest->TxtdataLength, pRequest->Txtdata);
Exit0:
    return;
}

void KPlayerServer::OnGetRandClubRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	KC2S_Set_Club_Txt*   pRequest    = (KC2S_Set_Club_Txt*)pData;
	KPlayer*             pPlayer     = NULL;

	pPlayer = GetPlayerByConnection(nConnIndex);
	KGLOG_PROCESS_ERROR(pPlayer);
	KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID == 0);

	g_LSClient.DoRandomClubRequest(pPlayer->m_dwID);
Exit0:
	return;
}

void KPlayerServer::OnGetClubListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	KC2S_Get_Club_List*  pRequest    = (KC2S_Get_Club_List*)pData;
	KPlayer*             pPlayer     = NULL;

	pPlayer = GetPlayerByConnection(nConnIndex);
	KGLOG_PROCESS_ERROR(pPlayer);
	KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID);

	g_LSClient.DoGetClubList(pPlayer->m_dwID, pRequest->dwPageIndex);
Exit0:
	return;
}

void KPlayerServer::OnGetClubApplyNumRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Apply_List*   pRequest    = (KC2S_Apply_List*)pData;
    KPlayer*           pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID);
    KGLOG_PROCESS_ERROR(pPlayer->m_nClubPost >= eClub_Post_Vice);

    g_LSClient.DoGetApplyNumRequest(pPlayer->m_dwID);
Exit0:
    return;
}

void KPlayerServer::OnGetApplyListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Apply_List*   pRequest    = (KC2S_Apply_List*)pData;
    KPlayer*           pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID);
    KGLOG_PROCESS_ERROR(pPlayer->m_nClubPost >= eClub_Post_Vice);

    g_LSClient.DoGetApplyListRequest(pPlayer->m_dwID);
Exit0:
    return;
}

void KPlayerServer::OnSwitchApplyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Switch_Apply*   pRequest    = (KC2S_Switch_Apply*)pData;
    KPlayer*             pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID);
    KGLOG_PROCESS_ERROR(pPlayer->m_nClubPost >= eClub_Post_Vice);

    g_LSClient.DoSwitchApplyRequest(pPlayer->m_dwID, pRequest->bySwitch); 
Exit0:
    return;
}

void KPlayerServer::OnFindClubRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KC2S_Find_Club*   pRequest    = (KC2S_Find_Club*)pData;
    KPlayer*          pPlayer     = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_dwClubID == 0);

    g_LSClient.DoFindClubRequest(pPlayer->m_dwID, pRequest->szFindName);
Exit0:
    return;
}

void KPlayerServer::OnFinishAchievementRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KPlayer* pPlayer = NULL;
    KC2S_Finish_Achievement* pRequest = (KC2S_Finish_Achievement*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
	
    bRetCode = pPlayer->m_Achievement.FinishAchievement(pRequest->achievementID);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return;
}

void KPlayerServer::OnIncreaseAchievementValue(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult = false;
    BOOL                        bRetCode        = false;
    KPlayer*                    pPlayer         = NULL;           
    KC2S_Increase_Achievement_Value* pRequest = (KC2S_Increase_Achievement_Value*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->m_Achievement.ClientIncreaseValue(pRequest->achievementID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnGetTesterAward(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KPlayer*                    pPlayer     = NULL;           
    KC2S_Get_Tester_Award*      pRequest    = (KC2S_Get_Tester_Award*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = g_pSO3World->m_ActivityCenter.GetTesterAward(pPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}


void KPlayerServer::OnGetLevelupAward(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KPlayer*                    pPlayer     = NULL;           
    KC2S_Get_Levelup_Award*     pRequest    = (KC2S_Get_Levelup_Award*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = g_pSO3World->m_ActivityCenter.GetLevelupAward(pPlayer, pRequest->byLevel);
    KGLOG_PROCESS_ERROR(bRetCode);

    PLAYER_LOG(pPlayer, "activity,onlineaward,%d", pRequest->byLevel);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnViewOnlineAward(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KPlayer*                    pPlayer     = NULL;           
    KC2S_View_Online_Award*     pRequest    = (KC2S_View_Online_Award*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = g_pSO3World->m_ActivityCenter.ViewOnlineAward(pPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);


    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnGetOnlineAward(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KPlayer*                    pPlayer     = NULL;           
    KC2S_Get_Online_Award*      pRequest    = (KC2S_Get_Online_Award*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = g_pSO3World->m_ActivityCenter.GetOnlineAward(pPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);

    PLAYER_LOG(pPlayer, "activity,onlineaward");

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnGetAwardTableInfo(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KPlayer*                    pPlayer     = NULL;           
    KC2S_Get_Award_Table_Info*  pRequest    = (KC2S_Get_Award_Table_Info*)pData;
    KAWARD_ITEM*                pAwardItem[10]  = {0};
    int                         nItemCount      = 0;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    nItemCount = countof(pAwardItem);
    bRetCode = g_pSO3World->m_AwardMgr.GetAwardItems(pRequest->wAwardTableID, nItemCount, pAwardItem);
    KGLOG_PROCESS_ERROR(bRetCode);

    DoSyncAwardItemInfo(nConnIndex, pRequest->wAwardTableID, nItemCount, pAwardItem);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnClientCallRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nTopIndex = 0;
    DWORD dwScriptID = ERROR_ID;
    KPlayer* pPlayer = NULL;  
    const static char* pcszScriptName = "scripts/global/ClientCall.lua";
    const static char* pcszFunctionName = "ClientCall";
    KC2S_Client_Call_Request* pPak = (KC2S_Client_Call_Request*)pData;
    ILuaScriptEx* pHolder = NULL;
    Lua_State* L = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    dwScriptID = g_FileNameHash(pcszScriptName);
    KGLOG_PROCESS_ERROR(dwScriptID);

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(dwScriptID, pcszFunctionName);
    KGLOG_PROCESS_ERROR(bRetCode);

    pHolder = g_pSO3World->m_ScriptManager.GetScriptHolder();
    KGLOG_PROCESS_ERROR(pHolder);

    L = pHolder->GetLuaState();
    KGLOG_PROCESS_ERROR(L);

    lua_pushstring(L, "player");
    pPlayer->LuaGetObj(L);
    lua_settable(L, LUA_GLOBALSINDEX);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push((int)pPak->wCallID);
    g_pSO3World->m_ScriptManager.Push(pPak->nParam1);
    g_pSO3World->m_ScriptManager.Push(pPak->nParam1);
    g_pSO3World->m_ScriptManager.Call(dwScriptID, pcszFunctionName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

void KPlayerServer::OnVIPChargeSelf(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KPlayer*                    pPlayer     = NULL;           
    KC2S_Vip_Charge_Self*       pRequest    = (KC2S_Vip_Charge_Self*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);
    
    bRetCode = pPlayer->ChargeVIPTimeRequest(pRequest->byType);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnVIPChargeFriend(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KPlayer*                    pPlayer     = NULL;           
    KC2S_Vip_Charge_Friend*     pRequest    = (KC2S_Vip_Charge_Friend*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->PresentVIPTimeToFriendRequest(pRequest->dwPlayerID, pRequest->byType);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KPlayerServer::OnRequestPlayerValueGroup(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer*                    pPlayer     = NULL;           
    KC2S_Request_PlayerValue_Group*  pRequest    = (KC2S_Request_PlayerValue_Group*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoSyncPlayerValueGroup(pPlayer, pRequest->groupID);

Exit0:
    return;
}

void KPlayerServer::OnGetRandomQuest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer*                    pPlayer     = NULL;           
    KC2S_Get_Random_Quest*      pRequest    = (KC2S_Get_Random_Quest*)pData;
    DWORD                       dwQuestID   = 0;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    dwQuestID = g_pSO3World->m_Settings.m_QuestInfoList.GetRandomQuest(
        pPlayer->GetMainHeroLevel(), pPlayer->m_nRandomQuestIndex
    );

    DoSyncRandomQuest(pPlayer->m_nConnIndex, dwQuestID);

Exit0:
    return;
}

void KPlayerServer::OnSkipRandomQuest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer*                pPlayer     = NULL;           
    KC2S_Skip_Random_Quest* pRequest    = (KC2S_Skip_Random_Quest*)pData;
    DWORD                   dwQuestID   = 0;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    KGLOG_PROCESS_ERROR(pPlayer->m_nSkipRandomQuestTime < g_pSO3World->m_Settings.m_ConstList.nMaxSkipRandomQuestTime);

    dwQuestID = g_pSO3World->m_Settings.m_QuestInfoList.GetRandomQuest(
        pPlayer->GetMainHeroLevel(), pPlayer->m_nRandomQuestIndex
    );

    PLAYER_LOG(pPlayer, "quest,cancellinkquest,%u", dwQuestID);

    pPlayer->m_QuestList.ClearRandomQuest();

    ++pPlayer->m_nSkipRandomQuestTime;
    ++pPlayer->m_nRandomQuestIndex;

    dwQuestID = g_pSO3World->m_Settings.m_QuestInfoList.GetRandomQuest(
        pPlayer->GetMainHeroLevel(), pPlayer->m_nRandomQuestIndex
    );

    DoSyncRandomQuest(pPlayer->m_nConnIndex, dwQuestID);

Exit0:
    return;
}

void KPlayerServer::OnSelectHeroSkill(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL bRetCode = false;
    KPlayer* pPlayer = NULL;
    KC2S_Select_Hero_Skill* pRequest = (KC2S_Select_Hero_Skill*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->SelectHeroSkill(pRequest->wHeroTemplateID, pRequest->bySkillGroup, pRequest->bySlotIndex, pRequest->wSkillID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnCastSlamBallSkill(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bRetCode        = false;
    KHero*                  pHero           = NULL;
    BOOL                    bAdjust         = false;
    BOOL                    bWithBall       = false;
    KBall*                  pBall           = NULL;
    KPlayer*                pPlayer         = NULL;
    BOOL                    bReverseFailed  = false;
    KC2S_Cast_SlamBall_Skill*    pCommand        = (KC2S_Cast_SlamBall_Skill*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pHero = pPlayer->GetFightingHero();
    KGLOG_PROCESS_ERROR(pHero);

    bAdjust = pHero->CheckMoveAdjust(nFrame, pCommand->moveParam, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    pHero->m_MoveCount = pCommand->movecount;
    bWithBall = pHero->HasFollowerWithStateAndType(mosToken, sotBall);
    if (!bWithBall)
    {
        pBall = pHero->m_pScene->GetBall();
        KGLOG_PROCESS_ERROR(pBall);

        g_PlayerServer.DoSyncSceneObject(pBall);
        LogInfo("Shoot ball pos error, sync ball state!");
        goto Exit0;
    }

    bRetCode = pHero->CastSlamBallSkill(pCommand->wSlamSkillID, pCommand->wTargetSocketID, pCommand->wInterferenceID);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bAdjust)
    {
        DoAdjustHeroMove(pHero, KSHOOT_BALL);
    }

Exit0:
    return;
}

void KPlayerServer::OnNormalSlamBallEx(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    KHero*                  pHero           = NULL;
    BOOL                    bAdjust         = false;
    BOOL                    bWithBall       = false;
    KBall*                  pBall           = NULL;
    KPlayer*                pPlayer         = NULL;
    BOOL                    bReverseFailed  = false;
    KC2S_Normal_SlamBallEx*   pCommand      = (KC2S_Normal_SlamBallEx*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pHero = pPlayer->GetFightingHero();
    KGLOG_PROCESS_ERROR(pHero);

    bAdjust = pHero->CheckMoveAdjust(nFrame, pCommand->moveParam, bReverseFailed);
    KGLOG_PROCESS_ERROR(!bReverseFailed);

    pHero->m_MoveCount = pCommand->movecount;
    bWithBall = pHero->HasFollowerWithStateAndType(mosToken, sotBall);
    if (!bWithBall)
    {
        pBall = pHero->m_pScene->GetBall();
        KGLOG_PROCESS_ERROR(pBall);

        g_PlayerServer.DoSyncSceneObject(pBall);
        LogInfo("Shoot ball pos error, sync ball state!");
        goto Exit0;
    }

    bRetCode = pHero->NormalSlamBallEx(pCommand->wTargetSocketID, pCommand->wInterferenceID);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bAdjust)
    {
        DoAdjustHeroMove(pHero, KSHOOT_BALL);
    }

    pPlayer->OnEvent(peSlamDunk);

    bResult = true;
Exit0:
    return;
}


void KPlayerServer::OnApplyActiveAward(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode        = false;
    KPlayer*                            pPlayer         = NULL;
    KC2S_Apply_Active_Award*            pRequest        = (KC2S_Apply_Active_Award*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->m_ActivePlayer.ApplyAward(pRequest->wActiveAwardID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayerServer::OnFindRoleInTopListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode        = false;
    KPlayer*                            pPlayer         = NULL;
	int									nGroup = 0;
    KC2S_FindRole_In_TopList*            pRequest       = (KC2S_FindRole_In_TopList*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

	nGroup = pRequest->bAllGroup ? 0 : pPlayer->m_nGroupID;
    g_LSClient.DoFindRoleInTopListRequest(pPlayer->m_dwID, pRequest->topListType, nGroup, pRequest->targetRoleName);

Exit0:
    return;
}

void KPlayerServer::OnRequestTopListPage(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode        = false;
    KPlayer*                            pPlayer         = NULL;
	int									nGroup = 0;
    KC2S_Request_TopList_Page*          pRequest      = (KC2S_Request_TopList_Page*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

	nGroup = pRequest->bAllGroup ? 0 : pPlayer->m_nGroupID;
    g_LSClient.DoRequestTopListPage(pPlayer->m_dwID, pRequest->topListType, nGroup, pRequest->pageIndex);

Exit0:
    return;
}

void KPlayerServer:: OnRequestFriendHeroList(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode        = false;
    KPlayer*                            pPlayer         = NULL;
    KC2S_Request_Friend_Hero_List*      pRequest      = (KC2S_Request_Friend_Hero_List*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoRequestFriendHeroList(pPlayer->m_dwID, pRequest->dwFriendID);
Exit0:
    return;
}

void KPlayerServer:: OnRequestFriendHeroInfo(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode        = false;
    KPlayer*                            pPlayer         = NULL;
    KC2S_Request_Friend_Hero_Info*      pRequest      = (KC2S_Request_Friend_Hero_Info*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoRequestFriendHeroInfo(pPlayer->m_dwID, pRequest->dwFriendID, pRequest->dwHeroTemplateID);
Exit0:
    return;
}

void KPlayerServer::OnSetRefuseStranger(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                bRetCode        = false;
    KPlayer*                            pPlayer         = NULL;
    KC2S_Set_Refuse_Stranger*           pRequest        = (KC2S_Set_Refuse_Stranger*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_bRefuseStranger = pRequest->bySet;
Exit0:
    return;
}

struct KRandomPlayerListTraverse
{
    BOOL operator()(KPlayer* pPlayer)
    {
        ++nCurIndex;
        if (nCurIndex < nStartIndex)
            return true;
        
        if (pPlayer->m_eGameStatus != gsInHall)
            return true;

        if (pPlayer == pExcept)
            return true;        

        vecResult.push_back(pPlayer);
        if (vecResult.size() >= cdInviteHallPlayerCount)
            return false;

        return true;
    }

    int nCurIndex;
    int nStartIndex;
    KPlayer* pExcept;
    std::vector<KPlayer*> vecResult;
};

void KPlayerServer::OnRequestRandomHallPlayerList(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    BOOL                                    bRetCode        = false;
    KPlayer*                                pPlayer         = NULL;
    KC2S_Request_Random_Hall_Player_List*   pRequest        = (KC2S_Request_Random_Hall_Player_List*)pData;
    KRandomPlayerListTraverse               Tranverse;
    size_t                                  uTotalCount     = 0;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    uTotalCount = g_pSO3World->m_PlayerSet.GetObjCount();

    Tranverse.nCurIndex     = -1;
    Tranverse.pExcept       = pPlayer;
    Tranverse.vecResult.reserve(cdInviteHallPlayerCount);
    Tranverse.nStartIndex   = 0;
    if (uTotalCount > cdInviteHallPlayerCount)
        Tranverse.nStartIndex = g_Random(uTotalCount);

    g_pSO3World->m_PlayerSet.Traverse(Tranverse);
    
    DoSyncRandomPlayerList(pPlayer->m_nConnIndex, Tranverse.vecResult);

Exit0:
    return;   
}

void KPlayerServer::OnGetEnterSceneHeroListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KScene* pScene = NULL;
    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pScene = g_pSO3World->GetScene(pPlayer->m_dwMapID, pPlayer->m_nCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);
    DoGetEnterSceneHeroListRequest(nConnIndex, pScene);
    g_LSClient.DoClubInfosRequest(pPlayer->m_dwID, pScene);
Exit0:
    return;
}

void KPlayerServer::OnSyncEnterSceneProgress(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KScene* pScene = NULL;
    KC2S_Sync_Enter_Scene_Progress* pRequest = (KC2S_Sync_Enter_Scene_Progress*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pScene = g_pSO3World->GetScene(pPlayer->m_dwMapID, pPlayer->m_nCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);

    DoSyncEnterSceneProgress(nConnIndex, pPlayer->m_dwID, pRequest->indexInTeam, pRequest->progress, pScene);
Exit0:
    return;
}

void KPlayerServer::OnReqeustRecommendFriend(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoApplyRecommendFriend(pPlayer->m_dwID, pPlayer->m_nHighestHeroLevel);

Exit0:
    return;
}

void KPlayerServer::OnDecomposeEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KC2S_Decompose_Equip_Request* pRequest = (KC2S_Decompose_Equip_Request*)pData;

    KGLOG_PROCESS_ERROR(nSize == sizeof(KC2S_Decompose_Equip_Request) + pRequest->itemsLength * sizeof(KDecomposeItems));

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_ItemList.DecomposeEquip(pRequest->itemsLength, pRequest->items);

Exit0:
    return;
}

void KPlayerServer::OnRequestClubName(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KC2S_Request_Club_Name* pRequest = (KC2S_Request_Club_Name*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_LSClient.DoRequestClubName(pPlayer->m_dwID, pRequest->clubID);

Exit0:
    return;
}

void KPlayerServer::OnCloseBattleReport(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_bLuckDrawing = false;

Exit0:
    return;
}

void KPlayerServer::OnRankQuickStartRequest(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KC2S_Rank_Quick_Start_Request* pRequest = (KC2S_Rank_Quick_Start_Request*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoRankQuickStart(pPlayer);

Exit0:
    return;
}

void KPlayerServer::OnChangeRoomHost(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KC2S_Change_Room_Host* pRequest = (KC2S_Change_Room_Host*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoChangeRoomHost(pPlayer->m_dwID, pRequest->roomType, pRequest->newHostID);

Exit0:
    return;
}

void KPlayerServer::OnFreePVPQucikStart(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KC2S_FreePVP_Quick_Start* pRequest = (KC2S_FreePVP_Quick_Start*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_RelayClient.DoFreePVPQuickStart(pPlayer, pRequest->mapID, pRequest->roomName, pRequest->password);

Exit0:
    return;
}

void KPlayerServer::OnChangeCurrentCity(char* pData, size_t nSize, int nConnIndex, int nFrame)
{
    KPlayer* pPlayer = NULL;
    KC2S_Change_Current_City* pRequest = (KC2S_Change_Current_City*)pData;

    pPlayer = GetPlayerByConnection(nConnIndex);
    KGLOG_PROCESS_ERROR(pPlayer);

    KGLOG_PROCESS_ERROR(pRequest->byCity == emitPVE1 || pRequest->byCity == emitPVE2);

    pPlayer->m_nCurrentCity = pRequest->byCity;
    
Exit0:
    return;
}
