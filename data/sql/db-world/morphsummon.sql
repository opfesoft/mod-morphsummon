-- mod-morphsummon

SET @ENTRY           := 601072;
SET @MODELID         := 15665;               -- Senior Sergeant Grimsford (creature ID 15703, not used anymore)
SET @NAME            := 'Cet Keres';
SET @SUBNAME         := 'Polymorphologist';
SET @SCRIPTNAME      := 'npc_morphsummon';
SET @NPC_TEXT_HELLO  := @ENTRY;
SET @NPC_TEXT_SORRY  := @NPC_TEXT_HELLO + 1;
SET @NPC_TEXT_CHOICE := @NPC_TEXT_HELLO + 2;
SET @MENU_HELLO      := 61072;
SET @MENU_SORRY      := @MENU_HELLO + 1;
SET @MENU_CHOICE     := @MENU_HELLO + 2;

DELETE FROM `creature_template` WHERE `entry` = @ENTRY;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`)
VALUES
(@ENTRY,0,0,0,0,0,@MODELID,0,0,0,@NAME,@SUBNAME,NULL,0,80,80,2,35,1,1,1.14286,1,0,0,1,2000,2000,8,0,2048,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,3,1,1,1,1,0,0,1,0,2,@SCRIPTNAME);

DELETE FROM `creature_text` WHERE `CreatureId` = @ENTRY;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`)
VALUES
(@ENTRY,0,0,'Custom aura added',15,0,100,0,0,0,0,0,'Cet Keres'),
(@ENTRY,1,0,'Custom aura removed',15,0,100,0,0,0,0,0,'Cet Keres');

DELETE FROM `npc_text` WHERE `ID` IN (@NPC_TEXT_HELLO,@NPC_TEXT_SORRY,@NPC_TEXT_CHOICE);
INSERT INTO `npc_text` (`ID`, `text0_0`)
VALUES
(@NPC_TEXT_HELLO, 'Greetings, $N. If you are looking for ways to change the appearance of your summoned creature, I can help you.'),
(@NPC_TEXT_SORRY, 'Greetings, $N. I am sorry, but you don''t have a summoned creature that I can polymorph.'),
(@NPC_TEXT_CHOICE, 'Please make your choice:');

DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (@MENU_HELLO,@MENU_SORRY,@MENU_CHOICE);
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`)
VALUES
(@MENU_HELLO,0,0,'Choose polymorph',0,0,0,0,0,0,0,'',0),
(@MENU_HELLO,1,0,'Choose Felguard weapon',0,0,0,0,0,0,0,'',0),
(@MENU_HELLO,2,0,'Generate new name',0,0,0,0,0,0,0,'',0),
(@MENU_HELLO,3,0,'Toggle custom auras',0,0,0,0,0,0,0,'',0),
(@MENU_HELLO,4,0,'Remove custom auras',0,0,0,0,0,0,0,'',0),
(@MENU_SORRY,0,0,'Ah, nevermind.',0,0,0,0,0,0,0,'',0),
(@MENU_CHOICE,0,0,'Back..',0,0,0,0,0,0,0,'',0),
(@MENU_CHOICE,1,4,'Next..',0,0,0,0,0,0,0,'',0),
(@MENU_CHOICE,2,4,'Previous..',0,0,0,0,0,0,0,'',0);
