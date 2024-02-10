/*
 * Copyright (C) 2020+     Project "Sol" <https://gitlab.com/opfesoft/sol>, released under the GNU AGPLv3 license: https://gitlab.com/opfesoft/mod-morphsummon/-/blob/master/LICENSE.md
 * Copyright (C) 2016-2020 AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/mod-morphsummon/blob/master/LICENSE.md
*/

#include "MorphSummon.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "Config.h"
#include "Pet.h"
#include "ScriptedGossip.h"
#include "ScriptedCreature.h"
#include "SpellAuras.h"
#include "Group.h"
#include "ScriptMgrMacros.h"

std::map<std::string, uint32> warlock_imp;
std::map<std::string, uint32> warlock_voidwalker;
std::map<std::string, uint32> warlock_succubus;
std::map<std::string, uint32> warlock_felhunter;
std::map<std::string, uint32> warlock_felguard;
std::map<std::string, uint32> felguard_weapon;
std::map<std::string, uint32> death_knight_ghoul;
std::map<std::string, uint32> mage_water_elemental;
std::map<std::string, uint32> custom_auras;
std::list<uint32> randomModelIds;
std::list<uint32> randomVisualEffectSpells;
std::list<uint32> randomMainHandEquip;
uint32 minTimeVisualEffect;
uint32 maxTimeVisualEffect;
bool morphSummonAnnounce;

enum MorphSummonGossip
{
    MORPH_PAGE_SIZE                         =       13,
    MORPH_PAGE_START_WARLOCK_IMP            =      101,
    MORPH_PAGE_START_WARLOCK_VOIDWALKER     =      201,
    MORPH_PAGE_START_WARLOCK_SUCCUBUS       =      301,
    MORPH_PAGE_START_WARLOCK_FELHUNTER      =      401,
    MORPH_PAGE_START_WARLOCK_FELGUARD       =      501,
    MORPH_PAGE_START_DEATH_KNIGHT_GHOUL     =      601,
    MORPH_PAGE_START_MAGE_WATER_ELEMENTAL   =      701,
    MORPH_PAGE_START_FELGUARD_WEAPON        =      801,
    MORPH_PAGE_MAX                          =      901,
    MORPH_MAIN_MENU                         =       50,
    MORPH_CLOSE_MENU                        =       60,
    MORPH_NEW_NAME                          =       70,
    MORPH_PAGE_START_CUSTOM_AURAS           = 10000101,
    MORPH_PAGE_MAX_CUSTOM_AURAS             = 10000201,
    MORPH_REMOVE_CUSTOM_AURAS               = 10000010,
    MORPH_GOSSIP_TEXT_HELLO                 =   601072,
    MORPH_GOSSIP_TEXT_SORRY                 =   601073,
    MORPH_GOSSIP_TEXT_CHOICE                =   601074,
    MORPH_GOSSIP_MENU_HELLO                 =    61072,
    MORPH_GOSSIP_MENU_SORRY                 =    61073,
    MORPH_GOSSIP_MENU_CHOICE                =    61074,
    MORPH_GOSSIP_OPTION_POLYMORPH           =        0,
    MORPH_GOSSIP_OPTION_FELGUARD_WEAPON     =        1,
    MORPH_GOSSIP_OPTION_NEW_NAME            =        2,
    MORPH_GOSSIP_OPTION_TOGGLE_CUSTOM_AURAS =        3,
    MORPH_GOSSIP_OPTION_REMOVE_CUSTOM_AURAS =        4,
    MORPH_GOSSIP_OPTION_SORRY               =        0,
    MORPH_GOSSIP_OPTION_CHOICE_BACK         =        0,
    MORPH_GOSSIP_OPTION_CHOICE_NEXT         =        1,
    MORPH_GOSSIP_OPTION_CHOICE_PREVIOUS     =        2,
};

enum MorphSummonSpells
{
    SUMMON_IMP                              =    688,
    SUMMON_VOIDWALKER                       =    697,
    SUMMON_SUCCUBUS                         =    712,
    SUMMON_FELHUNTER                        =    691,
    SUMMON_FELGUARD                         =  30146,
    RAISE_DEAD                              =  52150,
    SUMMON_WATER_ELEMENTAL                  =  70908,
};

enum MorphEffectSpells
{
    SUBMERGE                                =  53421,
    SHADOW_SUMMON_VISUAL                    =  53708,
};

enum MorphSummonEvents
{
    MORPH_EVENT_CAST_SPELL                  =      1,
};

enum MorphSummonModelIds
{
    MORPH_DEFAULT_MODEL_ID                  =  15665,
};

void MorphSummonScriptMgr::OnAfterPolymorph(Player* player, Pet* pet, uint32 spell, bool polymorphPet, uint32 morphId)
{
    FOREACH_SCRIPT(MorphSummonModuleScript)->OnAfterPolymorph(player, pet, spell, polymorphPet, morphId);
}

MorphSummonModuleScript::MorphSummonModuleScript(const char* name) : ModuleScript(name)
{
    ScriptRegistry<MorphSummonModuleScript>::AddScript(this);
}

class MorphSummon_PlayerScript : public PlayerScript
{
public:
    MorphSummon_PlayerScript() : PlayerScript("MorphSummon_PlayerScript") { }

    void OnLogin(Player* player) override
    {
        if (morphSummonAnnounce)
        {
            ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00MorphSummon |rmodule.");
        }
    }

    void OnAfterGuardianInitStatsForLevel(Player* player, Guardian* guardian) override
    {
        if (Pet* pet = guardian->ToPet())
        {
            if (pet->GetUInt32Value(UNIT_CREATED_BY_SPELL) == SUMMON_WATER_ELEMENTAL)
            {
                // The size of the water elemental model is not automatically scaled, so needs to be done here
                CreatureDisplayInfoEntry const* displayInfo = sCreatureDisplayInfoStore.LookupEntry(pet->GetNativeDisplayId());
                pet->SetObjectScale(0.85f / displayInfo->scale);
            }
            else if (pet->GetUInt32Value(UNIT_CREATED_BY_SPELL) == SUMMON_FELGUARD)
            {
                if (QueryResult result = CharacterDatabase.PQuery("SELECT `FelguardItemID` FROM `mod_morphsummon_felguard_weapon` WHERE `PlayerGUIDLow` = %u", player->GetGUIDLow()))
                {
                    Field* fields = result->Fetch();
                    pet->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, fields[0].GetUInt32());
                }
            }
        }
    }

    void OnAfterCastPetAuras(Player* player, Pet* pet) override
    {
        if (!player || !pet)
            return;

        if (QueryResult result = CharacterDatabase.PQuery("SELECT `CustomAura` FROM `mod_morphsummon_custom_auras` WHERE `PlayerGUIDLow` = %u AND `PetID` = %u", player->GetGUIDLow(), pet->GetCharmInfo()->GetPetNumber()))
            do
            {
                Field* fields = result->Fetch();
                uint32 spellId = fields[0].GetUInt32();
                if (!pet->HasAura(spellId))
                    pet->AddAura(spellId, pet);
            } while (result->NextRow());
    }
};

class MorphSummon_CreatureScript : public CreatureScript
{
public:
    MorphSummon_CreatureScript() : CreatureScript("npc_morphsummon") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        return CreateMainMenu(player, creature);
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        ClearGossipMenuFor(player);

        if (action == MORPH_MAIN_MENU)
        {
            return CreateMainMenu(player, creature);
        }
        else if (action == MORPH_CLOSE_MENU)
        {
            CloseGossipMenuFor(player);
            return true;
        }
        else if (action == MORPH_NEW_NAME)
        {
            GenerateNewName(player);
            return CreateMainMenu(player, creature);
        }
        else if (action >= MORPH_PAGE_START_WARLOCK_IMP && action < MORPH_PAGE_START_WARLOCK_VOIDWALKER)
        {
            AddGossip(player, action, warlock_imp, MORPH_PAGE_START_WARLOCK_IMP, MORPH_PAGE_MAX);
        }
        else if (action >= MORPH_PAGE_START_WARLOCK_VOIDWALKER && action < MORPH_PAGE_START_WARLOCK_SUCCUBUS)
        {
            AddGossip(player, action, warlock_voidwalker, MORPH_PAGE_START_WARLOCK_VOIDWALKER, MORPH_PAGE_MAX);
        }
        else if (action >= MORPH_PAGE_START_WARLOCK_SUCCUBUS && action < MORPH_PAGE_START_WARLOCK_FELHUNTER)
        {
            AddGossip(player, action, warlock_succubus, MORPH_PAGE_START_WARLOCK_SUCCUBUS, MORPH_PAGE_MAX);
        }
        else if (action >= MORPH_PAGE_START_WARLOCK_FELHUNTER && action < MORPH_PAGE_START_WARLOCK_FELGUARD)
        {
            AddGossip(player, action, warlock_felhunter, MORPH_PAGE_START_WARLOCK_FELHUNTER, MORPH_PAGE_MAX);
        }
        else if (action >= MORPH_PAGE_START_WARLOCK_FELGUARD && action < MORPH_PAGE_START_DEATH_KNIGHT_GHOUL)
        {
            AddGossip(player, action, warlock_felguard, MORPH_PAGE_START_WARLOCK_FELGUARD, MORPH_PAGE_MAX);
        }
        else if (action >= MORPH_PAGE_START_DEATH_KNIGHT_GHOUL && action < MORPH_PAGE_START_MAGE_WATER_ELEMENTAL)
        {
            AddGossip(player, action, death_knight_ghoul, MORPH_PAGE_START_DEATH_KNIGHT_GHOUL, MORPH_PAGE_MAX);
        }
        else if (action >= MORPH_PAGE_START_MAGE_WATER_ELEMENTAL && action < MORPH_PAGE_START_FELGUARD_WEAPON)
        {
            AddGossip(player, action, mage_water_elemental, MORPH_PAGE_START_MAGE_WATER_ELEMENTAL, MORPH_PAGE_MAX);
        }
        else if (action >= MORPH_PAGE_START_FELGUARD_WEAPON && action < MORPH_PAGE_MAX)
        {
            AddGossip(player, action, felguard_weapon, MORPH_PAGE_START_FELGUARD_WEAPON, MORPH_PAGE_MAX);
        }
        else if (action >= MORPH_PAGE_MAX && action < MORPH_REMOVE_CUSTOM_AURAS)
        {
            if (sender >= MORPH_PAGE_START_WARLOCK_IMP && sender < MORPH_PAGE_START_WARLOCK_VOIDWALKER)
            {
                Polymorph(player, action, sender, MORPH_PAGE_START_WARLOCK_IMP, MORPH_PAGE_START_WARLOCK_VOIDWALKER, SUMMON_IMP, warlock_imp, true);
            }
            else if (sender >= MORPH_PAGE_START_WARLOCK_VOIDWALKER && sender < MORPH_PAGE_START_WARLOCK_SUCCUBUS)
            {
                Polymorph(player, action, sender, MORPH_PAGE_START_WARLOCK_VOIDWALKER, MORPH_PAGE_START_WARLOCK_SUCCUBUS, SUMMON_VOIDWALKER, warlock_voidwalker, true);
            }
            else if (sender >= MORPH_PAGE_START_WARLOCK_SUCCUBUS && sender < MORPH_PAGE_START_WARLOCK_FELHUNTER)
            {
                Polymorph(player, action, sender, MORPH_PAGE_START_WARLOCK_SUCCUBUS, MORPH_PAGE_START_WARLOCK_FELHUNTER, SUMMON_SUCCUBUS, warlock_succubus, true);
            }
            else if (sender >= MORPH_PAGE_START_WARLOCK_FELHUNTER && sender < MORPH_PAGE_START_WARLOCK_FELGUARD)
            {
                Polymorph(player, action, sender, MORPH_PAGE_START_WARLOCK_FELHUNTER, MORPH_PAGE_START_WARLOCK_FELGUARD, SUMMON_FELHUNTER, warlock_felhunter, true);
            }
            else if (sender >= MORPH_PAGE_START_WARLOCK_FELGUARD && sender < MORPH_PAGE_START_DEATH_KNIGHT_GHOUL)
            {
                Polymorph(player, action, sender, MORPH_PAGE_START_WARLOCK_FELGUARD, MORPH_PAGE_START_DEATH_KNIGHT_GHOUL, SUMMON_FELGUARD, warlock_felguard, true);
            }
            else if (sender >= MORPH_PAGE_START_DEATH_KNIGHT_GHOUL && sender < MORPH_PAGE_START_MAGE_WATER_ELEMENTAL)
            {
                Polymorph(player, action, sender, MORPH_PAGE_START_DEATH_KNIGHT_GHOUL, MORPH_PAGE_START_MAGE_WATER_ELEMENTAL, RAISE_DEAD, death_knight_ghoul, true);
            }
            else if (sender >= MORPH_PAGE_START_MAGE_WATER_ELEMENTAL && sender < MORPH_PAGE_START_FELGUARD_WEAPON)
            {
                Polymorph(player, action, sender, MORPH_PAGE_START_MAGE_WATER_ELEMENTAL, MORPH_PAGE_START_FELGUARD_WEAPON, SUMMON_WATER_ELEMENTAL, mage_water_elemental, true);
            }
            else if (sender >= MORPH_PAGE_START_FELGUARD_WEAPON && sender < MORPH_PAGE_MAX)
            {
                Polymorph(player, action, sender, MORPH_PAGE_START_FELGUARD_WEAPON, MORPH_PAGE_MAX, SUMMON_FELGUARD, felguard_weapon, false);
            }
        }
        else if (action == MORPH_REMOVE_CUSTOM_AURAS)
        {
            if (Pet* pet = player->GetPet())
            {
                if (QueryResult result = CharacterDatabase.PQuery("SELECT `CustomAura` FROM `mod_morphsummon_custom_auras` WHERE `PlayerGUIDLow` = %u AND `PetID` = %u", player->GetGUIDLow(), pet->GetCharmInfo()->GetPetNumber()))
                    do
                    {
                        Field* fields = result->Fetch();
                        pet->RemoveAurasDueToSpell(fields[0].GetUInt32());
                    } while (result->NextRow());

                CharacterDatabase.PExecute("DELETE FROM `mod_morphsummon_custom_auras` WHERE `PlayerGUIDLow` = %u AND `PetID` = %u", player->GetGUIDLow(), pet->GetCharmInfo()->GetPetNumber());
            }

            CloseGossipMenuFor(player);
            return true;
        }
        else if (action >= MORPH_PAGE_START_CUSTOM_AURAS && action < MORPH_PAGE_MAX_CUSTOM_AURAS)
        {
            AddGossip(player, action, custom_auras, MORPH_PAGE_START_CUSTOM_AURAS, MORPH_PAGE_MAX_CUSTOM_AURAS);
        }
        else if (action >= MORPH_PAGE_MAX_CUSTOM_AURAS)
        {
            ToggleCustomAura(player, creature, action, sender, MORPH_PAGE_START_CUSTOM_AURAS, custom_auras);
        }

        SendGossipMenuFor(player, MORPH_GOSSIP_TEXT_CHOICE, creature->GetGUID());

        return true;
    }

    struct npc_morphsummonAI : public ScriptedAI
    {
        npc_morphsummonAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset() override
        {
            if (!randomModelIds.empty())
                me->SetDisplayId(acore::Containers::SelectRandomContainerElement(randomModelIds));
            else
                me->SetDisplayId(MORPH_DEFAULT_MODEL_ID);

            if (!randomMainHandEquip.empty())
            {
                SetEquipmentSlots(false, acore::Containers::SelectRandomContainerElement(randomMainHandEquip), EQUIP_UNEQUIP, EQUIP_UNEQUIP);
                me->SetSheath(SHEATH_STATE_MELEE);
            }
            else
            {
                SetEquipmentSlots(false, EQUIP_UNEQUIP, EQUIP_UNEQUIP, EQUIP_UNEQUIP);
                me->SetSheath(SHEATH_STATE_UNARMED);
            }

            events.Reset();
            events.ScheduleEvent(MORPH_EVENT_CAST_SPELL, urand(minTimeVisualEffect, maxTimeVisualEffect));
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            switch (events.ExecuteEvent())
            {
                case MORPH_EVENT_CAST_SPELL:
                    if (!randomVisualEffectSpells.empty())
                        DoCast(me, acore::Containers::SelectRandomContainerElement(randomVisualEffectSpells), true);
                    events.ScheduleEvent(MORPH_EVENT_CAST_SPELL, urand(minTimeVisualEffect, maxTimeVisualEffect));
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_morphsummonAI(creature);
    }

private:
    bool CreateMainMenu(Player* player, Creature* creature)
    {
        bool sorry = false;

        if (Pet* pet = player->GetPet())
        {
            switch (pet->GetUInt32Value(UNIT_CREATED_BY_SPELL))
            {
                case SUMMON_IMP:
                    if (!warlock_imp.empty())
                    {
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_POLYMORPH, GOSSIP_SENDER_MAIN, MORPH_PAGE_START_WARLOCK_IMP);
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_NEW_NAME, GOSSIP_SENDER_MAIN, MORPH_NEW_NAME);
                    }
                    else
                        sorry = true;
                    break;
                case SUMMON_VOIDWALKER:
                    if (!warlock_voidwalker.empty())
                    {
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_POLYMORPH, GOSSIP_SENDER_MAIN, MORPH_PAGE_START_WARLOCK_VOIDWALKER);
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_NEW_NAME, GOSSIP_SENDER_MAIN, MORPH_NEW_NAME);
                    }
                    else
                        sorry = true;
                    break;
                case SUMMON_SUCCUBUS:
                    if (!warlock_succubus.empty())
                    {
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_POLYMORPH, GOSSIP_SENDER_MAIN, MORPH_PAGE_START_WARLOCK_SUCCUBUS);
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_NEW_NAME, GOSSIP_SENDER_MAIN, MORPH_NEW_NAME);
                    }
                    else
                        sorry = true;
                    break;
                case SUMMON_FELHUNTER:
                    if (!warlock_felhunter.empty())
                    {
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_POLYMORPH, GOSSIP_SENDER_MAIN, MORPH_PAGE_START_WARLOCK_FELHUNTER);
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_NEW_NAME, GOSSIP_SENDER_MAIN, MORPH_NEW_NAME);
                    }
                    else
                        sorry = true;
                    break;
                case SUMMON_FELGUARD:
                    if (!warlock_felguard.empty())
                    {
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_POLYMORPH, GOSSIP_SENDER_MAIN, MORPH_PAGE_START_WARLOCK_FELGUARD);
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_NEW_NAME, GOSSIP_SENDER_MAIN, MORPH_NEW_NAME);

                        if (!felguard_weapon.empty())
                        {
                            AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_FELGUARD_WEAPON, GOSSIP_SENDER_MAIN, MORPH_PAGE_START_FELGUARD_WEAPON);
                        }
                    }
                    else if (!felguard_weapon.empty())
                    {
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_FELGUARD_WEAPON, GOSSIP_SENDER_MAIN, MORPH_PAGE_START_FELGUARD_WEAPON);
                    }
                    else
                        sorry = true;
                    break;
                case RAISE_DEAD:
                    if (!death_knight_ghoul.empty())
                    {
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_POLYMORPH, GOSSIP_SENDER_MAIN, MORPH_PAGE_START_DEATH_KNIGHT_GHOUL);
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_NEW_NAME, GOSSIP_SENDER_MAIN, MORPH_NEW_NAME);
                    }
                    else
                        sorry = true;
                    break;
                case SUMMON_WATER_ELEMENTAL:
                    if (!mage_water_elemental.empty())
                        AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_POLYMORPH, GOSSIP_SENDER_MAIN, MORPH_PAGE_START_MAGE_WATER_ELEMENTAL);
                    else
                        sorry = true;
                    break;
                default:
                    sorry = true;
            }
        }
        else
        {
            sorry = true;
        }

        if (sorry)
        {
            AddGossipItemFor(player, MORPH_GOSSIP_MENU_SORRY, MORPH_GOSSIP_OPTION_SORRY, GOSSIP_SENDER_MAIN, MORPH_CLOSE_MENU);
            SendGossipMenuFor(player, MORPH_GOSSIP_TEXT_SORRY, creature->GetGUID());
        }
        else
        {
            if (!custom_auras.empty() && player->GetPet() && player->GetPet()->getPetType() == SUMMON_PET)
            {
                AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_TOGGLE_CUSTOM_AURAS, GOSSIP_SENDER_MAIN, MORPH_PAGE_START_CUSTOM_AURAS);
                AddGossipItemFor(player, MORPH_GOSSIP_MENU_HELLO, MORPH_GOSSIP_OPTION_REMOVE_CUSTOM_AURAS, GOSSIP_SENDER_MAIN, MORPH_REMOVE_CUSTOM_AURAS);
            }

            SendGossipMenuFor(player, MORPH_GOSSIP_TEXT_HELLO, creature->GetGUID());
        }

        return true;
    }

    void AddGossip(Player *player, uint32 action, std::map<std::string, uint32> &map, uint32 pageStart, int pageMax)
    {
        AddGossipItemFor(player, MORPH_GOSSIP_MENU_CHOICE, MORPH_GOSSIP_OPTION_CHOICE_BACK, GOSSIP_SENDER_MAIN, MORPH_MAIN_MENU);
        uint32 page = action - pageStart + 1;
        uint32 maxPage = map.size() / MORPH_PAGE_SIZE + (map.size() % MORPH_PAGE_SIZE != 0);

        if (page > 1)
            AddGossipItemFor(player, MORPH_GOSSIP_MENU_CHOICE, MORPH_GOSSIP_OPTION_CHOICE_PREVIOUS, GOSSIP_SENDER_MAIN, pageStart + page - 2);

        if (page < maxPage)
            AddGossipItemFor(player, MORPH_GOSSIP_MENU_CHOICE, MORPH_GOSSIP_OPTION_CHOICE_NEXT, GOSSIP_SENDER_MAIN, pageStart + page);

        uint32 count = 1;

        for (auto model : map)
        {
            if (count > (page - 1) * MORPH_PAGE_SIZE && count <= page * MORPH_PAGE_SIZE)
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, model.first, action, model.second + pageMax);

            count++;
        }
    }

    void Polymorph(Player* player, uint32 action, uint32 sender, uint32 startPage, uint32 maxPage, uint32 spell, std::map<std::string, uint32> &modelMap, bool polymorphPet)
    {
        if (Pet* pet = player->GetPet())
        {
            if (sender >= startPage && sender < maxPage)
            {
                if (pet->GetUInt32Value(UNIT_CREATED_BY_SPELL) == spell)
                {
                    uint32 morphId = action - MORPH_PAGE_MAX;

                    if (polymorphPet)
                    {
                        pet->SetDisplayId(morphId);
                        pet->SetNativeDisplayId(morphId);

                        if (spell == SUMMON_WATER_ELEMENTAL)
                        {
                            // The size of the water elemental model is not automatically scaled, so needs to be done here
                            CreatureDisplayInfoEntry const* displayInfo = sCreatureDisplayInfoStore.LookupEntry(pet->GetNativeDisplayId());
                            pet->SetObjectScale(0.85f / displayInfo->scale);
                        }

                        if (Aura* aura = pet->AddAura(SUBMERGE, pet))
                            aura->SetDuration(2000);
                        pet->CastSpell(pet, SHADOW_SUMMON_VISUAL, true);
                    }
                    else
                    {
                        pet->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, morphId);
                        CharacterDatabase.PExecute("REPLACE INTO `mod_morphsummon_felguard_weapon` (`PlayerGUIDLow`, `FelguardItemID`) VALUES (%u, %u)", player->GetGUIDLow(), morphId);
                    }

                    sMorphSummonScriptMgr->OnAfterPolymorph(player, pet, spell, polymorphPet, morphId);
                }
            }

            AddGossip(player, sender, modelMap, startPage, MORPH_PAGE_MAX);
        }
    }

    void GenerateNewName(Player* player)
    {
        if (Pet* pet = player->GetPet())
        {
            std::string new_name = sObjectMgr->GeneratePetName(pet->GetEntry());

            if (!new_name.empty())
            {
                pet->SetName(new_name);
                Unit* owner = pet->GetOwner();

                if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && owner->ToPlayer()->GetGroup())
                    owner->ToPlayer()->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_NAME);

                pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(NULL)));
            }
        }
    }

    void ToggleCustomAura(Player* player, Creature* creature, uint32 entry, uint32 sender, uint32 startPage, std::map<std::string, uint32> &auraMap)
    {
        if (Pet* pet = player->GetPet())
        {
            uint32 spellId = entry - MORPH_PAGE_MAX_CUSTOM_AURAS;
            bool customAuraFound = false;
            if (QueryResult result = CharacterDatabase.PQuery("SELECT `CustomAura` FROM `mod_morphsummon_custom_auras` WHERE `PlayerGUIDLow` = %u AND `PetID` = %u", player->GetGUIDLow(), pet->GetCharmInfo()->GetPetNumber()))
                do
                {
                    Field* fields = result->Fetch();
                    if (fields[0].GetUInt32() == spellId)
                        customAuraFound = true;
                } while (result->NextRow());

            if (customAuraFound)
            {
                pet->RemoveAurasDueToSpell(spellId);
                CharacterDatabase.PExecute("DELETE FROM `mod_morphsummon_custom_auras` WHERE `PlayerGUIDLow` = %u AND `PetID` = %u AND `CustomAura` = %u", player->GetGUIDLow(), pet->GetCharmInfo()->GetPetNumber(), spellId);
                creature->MonsterWhisper("Custom aura removed", player, false);
            }
            else
            {
                if (!pet->HasAura(spellId))
                    pet->AddAura(spellId, pet);
                CharacterDatabase.PExecute("INSERT INTO `mod_morphsummon_custom_auras` (`PlayerGUIDLow`, `PetID`, `CustomAura`) VALUES (%u, %u, %u)", player->GetGUIDLow(), pet->GetCharmInfo()->GetPetNumber(), spellId);
                creature->MonsterWhisper("Custom aura added", player, false);
            }
        }

        AddGossip(player, sender, auraMap, startPage, MORPH_PAGE_MAX_CUSTOM_AURAS);
    }
};

class MorphSummon_WorldScript : public WorldScript
{
public:
    MorphSummon_WorldScript() : WorldScript("MorphSummon_WorldScript") { }

    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        morphSummonAnnounce = sConfigMgr->GetBoolDefault("MorphSummon.Announce", true);

        randomVisualEffectSpells.clear();
        std::stringstream stringStream;
        std::string delimitedValue;
        stringStream.str(sConfigMgr->GetStringDefault("MorphSummon.RandomVisualEffectSpells", "45959,50772"));

        while (std::getline(stringStream, delimitedValue, ','))
        {
            uint32 spellId = atoi(delimitedValue.c_str());
            randomVisualEffectSpells.push_back(spellId);
        }

        randomMainHandEquip.clear();
        stringStream.clear();
        stringStream.str(sConfigMgr->GetStringDefault("MorphSummon.RandomMainHandEquip", "28658,32374"));

        while (std::getline(stringStream, delimitedValue, ','))
        {
            uint32 itemId = atoi(delimitedValue.c_str());
            randomMainHandEquip.push_back(itemId);
        }

        randomModelIds.clear();
        stringStream.clear();
        stringStream.str(sConfigMgr->GetStringDefault("MorphSummon.RandomModelIds", "15665"));

        while (std::getline(stringStream, delimitedValue, ','))
        {
            uint32 modelId = atoi(delimitedValue.c_str());
            randomModelIds.push_back(modelId);
        }

        minTimeVisualEffect = sConfigMgr->GetIntDefault("MorphSummon.MinTimeVisualEffect", 30000);
        maxTimeVisualEffect = sConfigMgr->GetIntDefault("MorphSummon.MaxTimeVisualEffect", 90000);

        warlock_imp.clear();
        warlock_voidwalker.clear();
        warlock_succubus.clear();
        warlock_felhunter.clear();
        warlock_felguard.clear();
        felguard_weapon.clear();
        death_knight_ghoul.clear();
        mage_water_elemental.clear();
        custom_auras.clear();

        LoadMap(sConfigMgr->GetStringDefault("MorphSummon.Warlock.Imp", ""), warlock_imp);
        LoadMap(sConfigMgr->GetStringDefault("MorphSummon.Warlock.Voidwalker", ""), warlock_voidwalker);
        LoadMap(sConfigMgr->GetStringDefault("MorphSummon.Warlock.Succubus", ""), warlock_succubus);
        LoadMap(sConfigMgr->GetStringDefault("MorphSummon.Warlock.Felhunter", ""), warlock_felhunter);
        LoadMap(sConfigMgr->GetStringDefault("MorphSummon.Warlock.Felguard", ""), warlock_felguard);
        LoadMap(sConfigMgr->GetStringDefault("MorphSummon.Warlock.Felguard.Weapon", ""), felguard_weapon);
        LoadMap(sConfigMgr->GetStringDefault("MorphSummon.DeathKnight.Ghoul", ""), death_knight_ghoul);
        LoadMap(sConfigMgr->GetStringDefault("MorphSummon.Mage.WaterElemental", ""), mage_water_elemental);
        LoadMap(sConfigMgr->GetStringDefault("MorphSummon.CustomAuras", ""), custom_auras);
    }

private:
    static void LoadMap(std::string param, std::map<std::string, uint32> &map)
    {
        std::string delimitedValue;
        std::stringstream stringStream;
        std::string name;
        int count = 0;

        stringStream.str(param);

        while (std::getline(stringStream, delimitedValue, ','))
        {
            if (count % 2 == 0)
            {
                name = delimitedValue;
            }
            else
            {
                uint32 id = atoi(delimitedValue.c_str());
                map[name] = id;
            }

            count++;
        }
    }
};

void AddMorphSummonScripts()
{
    new MorphSummon_WorldScript();
    new MorphSummon_PlayerScript();
    new MorphSummon_CreatureScript();
}
